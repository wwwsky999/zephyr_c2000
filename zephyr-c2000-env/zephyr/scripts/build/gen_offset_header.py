#!/usr/bin/env python3
#
# Copyright (c) 2017 Intel Corporation.
#
# SPDX-License-Identifier: Apache-2.0
#

"""
Scan an object file and emit a header with _OFFSET / _SIZEOF macros.

Dispatches to ELF (pyelftools) or TI-COFF (struct-based) based on the file
magic.  TI CGT's cl2000 21.6.0 defaults to TI-COFF when --abi=eabi is not
specified; lnk2000 only consumes TI-COFF inputs, so we build everything in
COFF mode on C28x.
"""

import argparse
import os
import struct
import sys


# ---------------------------------------------------------------------------
# TI-COFF parser (uses TI's nm2000 + ofd2000 tools)
# ---------------------------------------------------------------------------

def _find_ti_tool(name):
    """Locate a TI CGT tool (nm2000, ofd2000, hex2000, etc.).

    Search order: PATH, TI_C2000_CGT_PATH/bin, common install locations.
    Returns the absolute path or raises FileNotFoundError.
    """
    import shutil
    path = shutil.which(name)
    if path:
        return path

    cgt = os.environ.get('TI_C2000_CGT_PATH', '')
    if cgt:
        # Windows binaries carry a .exe suffix.
        for candidate_name in (name, name + '.exe'):
            candidate = os.path.join(cgt, 'bin', candidate_name)
            if os.path.isfile(candidate):
                return candidate

    # Common locations
    for base in ('/usr/local/ti', '/opt/ti', '/workspace/tools/ti-cgt-c2000',
                 os.path.expanduser('~/ti')):
        import glob
        matches = glob.glob(os.path.join(base, 'ti-cgt-c2000_*', 'bin', name))
        if matches:
            return sorted(matches)[-1]
        # Also try base directly (base IS the CGT root)
        direct = os.path.join(base, 'bin', name)
        if os.path.isfile(direct):
            return direct

    raise FileNotFoundError(f"cannot find TI tool '{name}'")


def _coff_parse_with_tools(obj_path):
    """Parse TI-COFF object using nm2000 and ofd2000.

    Returns (symbols, section_sizes) where symbols is a list of
    (name, value, section_name) tuples and section_sizes maps
    section_name -> size.
    """
    import subprocess

    nm2000 = _find_ti_tool('nm2000')
    ofd2000 = _find_ti_tool('ofd2000')

    # nm2000 -l -g: long format, external globals only
    result = subprocess.run(
        [nm2000, '-l', '-g', obj_path],
        capture_output=True, text=True, check=True)

    symbols = []
    for line in result.stdout.splitlines():
        # Format: "[idx]    |0xVALUE|CLASS   |SECTION          |NAME"
        # Pipe-separated; there are blank lines and headers to skip.
        if '|' not in line:
            continue
        parts = [p.strip() for p in line.split('|')]
        if len(parts) < 5:
            continue
        value_str = parts[1]
        sclass = parts[2]
        section = parts[3]
        name = parts[4]
        if not value_str.startswith('0x'):
            continue  # skip header lines
        value = int(value_str, 16)
        symbols.append((name, value, section, sclass))

    # ofd2000: dump section table
    result = subprocess.run(
        [ofd2000, obj_path],
        capture_output=True, text=True, check=True)

    section_sizes = {}
    in_section_table = False
    for line in result.stdout.splitlines():
        if 'Section Information' in line:
            in_section_table = True
            continue
        if in_section_table:
            # Section lines look like:
            # "  6 .zephyr_abs  0x00000000 0x00000000 0x10e1 1 Y"
            # Columns: id name load_addr run_addr size align alloc
            stripped = line.strip()
            if not stripped or stripped.startswith('-'):
                continue
            # Relocation table headers etc. have leading keywords
            if stripped.startswith(('Relocation', 'Symbol', 'Hash',
                                    "Section ", " id ", "Number")):
                continue
            parts = stripped.split()
            if len(parts) < 6:
                continue
            try:
                # First token is section id (decimal), second is name
                _id = int(parts[0])
                sect_name = parts[1]
                size_str = parts[4]  # 5th column is size in hex
                if size_str.startswith('0x'):
                    section_sizes[sect_name] = int(size_str, 16)
            except ValueError:
                continue

    return symbols, section_sizes


def _coff_compute_values(symbols, section_sizes):
    """Compute {symbol_name: value} for _OFFSET/_SIZEOF symbols in TI-COFF.

    Each GEN_ABSOLUTE_SYM(name, target) emits ``const char name[target+1]``
    in .zephyr_abs.  nm2000 reports each symbol's offset within the owning
    section.  Adjacent symbols delimit each other's arrays, so
    ``target = next_offset - cur_offset - 1``.  The last symbol closes at
    the section size (from ofd2000).
    """
    # Collect qualifying symbols per section.
    per_section = {}
    for name, value, section, sclass in symbols:
        if not name.endswith(('_OFFSET', '_SIZEOF')):
            continue
        if sclass != 'C_EXT':
            continue
        per_section.setdefault(section, []).append((value, name))

    results = {}
    for sect_name, entries in per_section.items():
        entries.sort(key=lambda x: x[0])
        sect_size = section_sizes.get(sect_name, 0)
        for i, (cur_value, cur_name) in enumerate(entries):
            if i + 1 < len(entries):
                next_value = entries[i + 1][0]
            else:
                next_value = sect_size
            results[cur_name] = max(0, next_value - cur_value - 1)

    return results


# ---------------------------------------------------------------------------
# ELF parser
# ---------------------------------------------------------------------------

def _elf_compute_values(obj_path):
    from elftools.elf.elffile import ELFFile
    from elftools.elf.sections import SymbolTableSection

    results = {}
    with open(obj_path, 'rb') as f:
        obj = ELFFile(f)
        stabs = None
        for section in obj.iter_sections():
            if isinstance(section, SymbolTableSection):
                stabs = section
                break
        if stabs is None:
            return results

        zephyr_abs_sect = obj.get_section_by_name('.zephyr_abs')

        for sym in stabs.iter_symbols():
            name = sym.name
            if isinstance(name, bytes):
                name = str(name, 'ascii')
            if not name.endswith(('_OFFSET', '_SIZEOF')):
                continue
            if sym.entry['st_info']['bind'] != 'STB_GLOBAL':
                continue

            shndx = sym.entry['st_shndx']
            if shndx == 'SHN_ABS':
                results[name] = sym.entry['st_value']
            elif zephyr_abs_sect is not None:
                if sym.entry['st_size'] > 0:
                    # C28x EABI: one char = 2 octets in ELF; GEN_ABSOLUTE_SYM
                    # emits array[N+1] where N is the target, so
                    # section_bytes = (N+1)*2, hence N = st_size//2 - 1.
                    results[name] = sym.entry['st_size'] // 2 - 1

    return results


# ---------------------------------------------------------------------------
# Top-level
# ---------------------------------------------------------------------------

def gen_offset_header(input_file, output_file):
    basename = os.path.basename(output_file.name).upper().replace('.', '_').replace('-', '_')
    include_guard = f"__GEN_{basename}__"

    with open(input_file, 'rb') as f:
        magic = f.read(4)

    if magic[:4] == b'\x7fELF':
        values = _elf_compute_values(input_file)
    elif magic[:2] in (b'\x0c\x00', b'\xc2\x00') or magic[:2] == b'\x00\x00':
        # TI-COFF: COFF_MACHINE field at offset 2.  Little-endian machine 0x00C2
        # is TI C2000.  Some COFF files have a leading 0x0000 stamp.
        try:
            symbols, section_sizes = _coff_parse_with_tools(input_file)
            values = _coff_compute_values(symbols, section_sizes)
        except Exception as e:
            print(f"warning: failed to parse as COFF: {e}", file=sys.stderr)
            values = {}
    else:
        print(f"warning: unrecognized object format (magic {magic!r})", file=sys.stderr)
        values = {}

    output_file.write(
        f"""/* THIS FILE IS AUTO GENERATED.  PLEASE DO NOT EDIT.
 *
 * This header file provides macros for the offsets of various structure
 * members.  These offset macros are primarily intended to be used in
 * assembly code.
 */

#ifndef {include_guard}
#define {include_guard}\n\n"""
    )

    for name in sorted(values):
        output_file.write(f"#define {name} 0x{values[name]:x}\n")

    output_file.write(f"\n#endif /* {include_guard} */\n")
    return 0


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        allow_abbrev=False,
    )

    parser.add_argument("-i", "--input", required=True, help="Input object file")
    parser.add_argument("-o", "--output", required=True, help="Output header file")

    args = parser.parse_args()

    with open(args.input, 'rb') as input_file, open(args.output, 'w') as output_file:
        sys.exit(gen_offset_header(args.input, output_file))
