# Copyright (c) 2026 Zephyr C2000 port
# SPDX-License-Identifier: Apache-2.0
#
# hex2000 property set for Zephyr's bintools abstraction.
#
# hex2000's CLI (``hex2000 -i <elf> -o <out.hex>``) does not map onto
# Zephyr's objcopy-oriented bintools template.  To keep the pre-link
# (zephyr_pre0) build flowing we disable the standard hex output step;
# a dedicated post-build hex2000 invocation will be added once the full
# kernel is linkable.
#
# disassembly/symbols/readelf/strip are also stubbed out because the TI
# tools are not drop-in replacements for the GNU names Zephyr expects.

# hex2000 path was discovered by the toolchain find_program.  CMAKE_OBJCOPY
# points at hex2000 after target.cmake runs find_program(CMAKE_OBJCOPY ...).
set_property(TARGET bintools PROPERTY elfconvert_command "${CMAKE_OBJCOPY}")
set_property(TARGET bintools PROPERTY elfconvert_formats "")

# All flags are empty until we replace the hex generation with a proper
# custom command (once the kernel links).
foreach(_prop_flag
    elfconvert_flag elfconvert_flag_final
    elfconvert_flag_strip_all elfconvert_flag_strip_debug
    elfconvert_flag_strip_unneeded elfconvert_flag_compress_debug_sections
    elfconvert_flag_intarget elfconvert_flag_outtarget
    elfconvert_flag_section_remove elfconvert_flag_section_only
    elfconvert_flag_section_rename elfconvert_flag_gapfill
    elfconvert_flag_srec_len elfconvert_flag_infile elfconvert_flag_outfile
    )
  set_property(TARGET bintools PROPERTY ${_prop_flag} "")
endforeach()

# Zephyr uses these for post-debug disassemble/symbols; they are not wired.
set_property(TARGET bintools PROPERTY disassembly_command "")
set_property(TARGET bintools PROPERTY disassembly_flag "")
set_property(TARGET bintools PROPERTY disassembly_flag_final "")
set_property(TARGET bintools PROPERTY disassembly_flag_inline_source "")
set_property(TARGET bintools PROPERTY disassembly_flag_no_aliases "")
set_property(TARGET bintools PROPERTY disassembly_flag_all "")
set_property(TARGET bintools PROPERTY disassembly_flag_infile "")
set_property(TARGET bintools PROPERTY disassembly_flag_outfile "")

set_property(TARGET bintools PROPERTY readelf_command "")
set_property(TARGET bintools PROPERTY readelf_flag "")
set_property(TARGET bintools PROPERTY readelf_flag_final "")
set_property(TARGET bintools PROPERTY readelf_flag_headers "")
set_property(TARGET bintools PROPERTY readelf_flag_infile "")
set_property(TARGET bintools PROPERTY readelf_flag_outfile "")

set_property(TARGET bintools PROPERTY strip_command "")
set_property(TARGET bintools PROPERTY strip_flag "")
set_property(TARGET bintools PROPERTY strip_flag_final "")
set_property(TARGET bintools PROPERTY strip_flag_all "")
set_property(TARGET bintools PROPERTY strip_flag_debug "")
set_property(TARGET bintools PROPERTY strip_flag_dwo "")
set_property(TARGET bintools PROPERTY strip_flag_infile "")
set_property(TARGET bintools PROPERTY strip_flag_outfile "")

set_property(TARGET bintools PROPERTY symbols_command "")
set_property(TARGET bintools PROPERTY symbols_flag "")
set_property(TARGET bintools PROPERTY symbols_final "")
set_property(TARGET bintools PROPERTY symbols_infile "")
set_property(TARGET bintools PROPERTY symbols_outfile ">")
