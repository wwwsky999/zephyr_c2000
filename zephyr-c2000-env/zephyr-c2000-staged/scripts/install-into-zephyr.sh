#!/usr/bin/env bash
# Install the C2000 port skeleton into a Zephyr 4.5.0 tree.
#
#   ./scripts/install-into-zephyr.sh /path/to/zephyr
#
# The script is idempotent and only edits a handful of discovery points:
#   arch/Kconfig, arch/archs.yml, include/zephyr/arch/cpu.h,
#   drivers/{serial,timer}/{Kconfig,CMakeLists.txt}
set -euo pipefail

ZEPHYR_BASE=${1:-${ZEPHYR_BASE:-}}
if [ -z "$ZEPHYR_BASE" ] || [ ! -f "$ZEPHYR_BASE/Kconfig.zephyr" ]; then
	echo "usage: $0 /path/to/zephyr" >&2
	exit 1
fi

ROOT=$(cd "$(dirname "$0")/.." && pwd)

echo "installing C2000 port into $ZEPHYR_BASE"
cp -a "$ROOT/port/arch/." "$ZEPHYR_BASE/arch/"
cp -a "$ROOT/port/include/." "$ZEPHYR_BASE/include/"
cp -a "$ROOT/port/soc/." "$ZEPHYR_BASE/soc/"
cp -a "$ROOT/port/boards/." "$ZEPHYR_BASE/boards/"
cp -a "$ROOT/port/dts/." "$ZEPHYR_BASE/dts/"
cp -a "$ROOT/port/cmake/." "$ZEPHYR_BASE/cmake/"

# Shared HAL + C2000Ware headers live next to the SoC.
mkdir -p "$ZEPHYR_BASE/soc/ti/f2837xd/hal" "$ZEPHYR_BASE/soc/ti/f2837xd/c2000ware"
cp -a "$ROOT/hal/f2837xd/." "$ZEPHYR_BASE/soc/ti/f2837xd/hal/"
cp -a "$ROOT/third_party/c2000ware/." "$ZEPHYR_BASE/soc/ti/f2837xd/c2000ware/"

# Driver sources and Kconfig fragments.
cp "$ROOT/port/drivers/serial/uart_f2837xd_sci.c" "$ZEPHYR_BASE/drivers/serial/"
cp "$ROOT/port/drivers/serial/Kconfig.f2837xd" "$ZEPHYR_BASE/drivers/serial/"
cp "$ROOT/port/drivers/timer/f2837xd_cputimer.c" "$ZEPHYR_BASE/drivers/timer/"
cp "$ROOT/port/drivers/timer/Kconfig.f2837xd" "$ZEPHYR_BASE/drivers/timer/"

# gen_offset_header.py: replace ELF-only upstream with a version that also
# auto-detects TI-COFF (lnk2000's only input format).
cp "$ROOT/port/scripts/build/gen_offset_header.py" "$ZEPHYR_BASE/scripts/build/gen_offset_header.py"

python3 - "$ZEPHYR_BASE" <<'PY'
import pathlib
import sys

base = pathlib.Path(sys.argv[1])

def patch_before(rel, needle, addition):
    p = base / rel
    text = p.read_text()
    if addition.strip() in text:
        return
    if needle not in text:
        raise SystemExit(f"marker not found in {rel}: {needle!r}")
    p.write_text(text.replace(needle, addition + needle, 1))
    print(f"patched {rel}")

def append_once(rel, line):
    p = base / rel
    text = p.read_text()
    if line in text:
        return
    if not text.endswith("\n"):
        text += "\n"
    p.write_text(text + line + "\n")
    print(f"patched {rel}")

patch_before("arch/Kconfig", "config ARC\n", """config C2000
\tbool
\tselect ARCH_IS_SET
\tselect ATOMIC_OPERATIONS_C
\tselect USE_SWITCH
\tselect USE_SWITCH_SUPPORTED
\thelp
\t  Texas Instruments TMS320C28x (C2000) architecture

""")

append_once("arch/archs.yml", """  - name: c2000
    path: c2000
    full_name: TI C2000 (C28x)""")

patch_before("include/zephyr/arch/cpu.h", "#elif defined(CONFIG_ARCH_IS_SET)\n",
             """#elif defined(CONFIG_C2000)
#include <zephyr/arch/c2000/arch.h>
""")

patch_before("include/zephyr/arch/arch_inlines.h",
             "#elif defined(CONFIG_ARCH_IS_SET)\n",
             """#elif defined(CONFIG_C2000)
#include <zephyr/arch/c2000/arch_inlines.h>
""")

patch_before("include/zephyr/arch/exception.h",
             "#elif defined(CONFIG_ARCH_IS_SET)\n",
             """#elif defined(CONFIG_C2000)
#include <zephyr/arch/c2000/exception.h>
""")

def replace_once(rel, old, new):
    p = base / rel
    text = p.read_text()
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"marker not found in {rel}")
    p.write_text(text.replace(old, new, 1))
    print(f"patched {rel}")

# C28x has 16-bit int and 32-bit long.
replace_once("include/zephyr/kernel.h",
             "BUILD_ASSERT(sizeof(int32_t) == sizeof(int));",
             """#if defined(CONFIG_C2000)
BUILD_ASSERT(sizeof(int32_t) == sizeof(long));
#else
BUILD_ASSERT(sizeof(int32_t) == sizeof(int));
#endif""")

# posix_types.h redefines pid_t/uid_t/gid_t/dev_t/ino_t without guarding
# against a prior <sys/types.h>.  TI's RTS ships the same macros
# (_PID_T_DECLARED etc.) so we extend the existing CONFIG_ARCMWDT_LIBC check
# to cover TI C2000 CGT as well.
replace_once("include/zephyr/posix/posix_types.h",
             "#if !defined(CONFIG_ARCMWDT_LIBC)\ntypedef int pid_t;\n#endif",
             "#if !defined(CONFIG_ARCMWDT_LIBC) && !defined(_PID_T_DECLARED)\ntypedef int pid_t;\n#define _PID_T_DECLARED\n#endif")

# C28x compiler abstraction layer.
patch_before("include/zephyr/toolchain.h",
             '#else\n#error "Invalid/unknown toolchain configuration"',
             '#elif defined(__TI_COMPILER_VERSION__)\n'
             '#include <zephyr/toolchain/ti_c2000.h>\n')

# cl2000 ships its own exact-width typedefs; bypass the GCC-only redefinition.
patch_before("include/zephyr/toolchain/zephyr_stdint.h",
             "#if __SIZEOF_INT__ != 4",
             "#if defined(__TI_COMPILER_VERSION__)\n"
             "#include <zephyr/toolchain/ti_c2000_stdint.h>\n"
             "#else\n")
patch_before("include/zephyr/toolchain/zephyr_stdint.h",
             "#endif /* ZEPHYR_INCLUDE_TOOLCHAIN_ZEPHYR_STDINT_H_ */",
             "#endif /* !__TI_COMPILER_VERSION__ */\n")

# C28x assembler alignment directives.
patch_before("include/zephyr/toolchain/common.h",
             "  #else\n    #error Architecture unsupported\n  #endif\n\n"
             "#endif /* defined(_ASMLANGUAGE) && !defined(_LINKER) */",
             "  #elif defined(CONFIG_C2000)\n"
             "    #define   ALIGN(x)    .align    x\n")

# cl2000 parses -ffunction-sections as its own -f listing-file option.
replace_once("arch/common/CMakeLists.txt",
             "zephyr_cc_option(-ffunction-sections -fdata-sections)",
             """if(CONFIG_C2000)
  zephyr_cc_option(--gen_func_subsections=on --gen_data_subsections=on)
else()
  zephyr_cc_option(-ffunction-sections -fdata-sections)
endif()""")

# cbprintf_complete.c's _ldiv5 (32-bit path) uses a GCC extended-asm
# constraint to hide a constant from the optimizer.  cl2000 does not
# support that syntax; C28x has hardware division plus TI RTS 64-bit
# helpers, so fall back to a plain divide.
replace_once("lib/os/cbprintf_complete.c",
             "\t__asm__ (\"\" : \"+r\" (m));",
             """#ifdef CONFIG_C2000
	(void)m;
#else
	__asm__ ("" : "+r" (m));
#endif""")

# configs.c uses CONFIG_* symbols as array names, but autoconf.h defines
# them as macros.  GCC's GEN_ABSOLUTE_SYM_KCONFIG stringizes the name with
# #name; our array encoding needs the bare identifier, so #undef each
# CONFIG_ symbol before emitting it.
replace_once("misc/generated/CMakeLists.txt",
             'string(REGEX REPLACE "#define (CONFIG_[A-Za-z|_|0-9]*) (.*)" "GEN_ABSOLUTE_SYM_KCONFIG(\\\\1, \\\\2)" config ${line})',
             'string(REGEX REPLACE "#define (CONFIG_[A-Za-z|_|0-9]*) (.*)" "#undef \\\\1\\nGEN_ABSOLUTE_SYM_KCONFIG(\\\\1, \\\\2)" config ${line})')

# C2000 uses ePIE vectors so the generic ISR table mechanism's undefined
# symbols (_sw_isr_table, _irq_vector_table) are never resolved.  lnk2000
# also lacks a GNU --undefined= equivalent, so drop these link directives.
replace_once("arch/common/CMakeLists.txt",
             "  zephyr_library_link_libraries(isr_tables)\n\n"
             "  zephyr_link_libraries($<TARGET_PROPERTY:linker,undefined>_sw_isr_table)\n"
             "  zephyr_link_libraries($<TARGET_PROPERTY:linker,undefined>_irq_vector_table)",
             "  zephyr_library_link_libraries(isr_tables)\n\n"
             "  if(NOT CONFIG_C2000)\n"
             "    zephyr_link_libraries($<TARGET_PROPERTY:linker,undefined>_sw_isr_table)\n"
             "    zephyr_link_libraries($<TARGET_PROPERTY:linker,undefined>_irq_vector_table)\n"
             "  endif()")

# util.h's LOG2/LOG2CEIL/NHPOT use __builtin_clz, which our ti_c2000.h
# overrides to inline function calls.  cl2000 rejects function calls inside
# _Static_assert, so swap the helper macros for ternary-chain compile-time
# constant expressions on TI CGT.  C28x has no unsigned long long so the
# 64-bit log2 path and NHPOT's 1ULL sentinel are also redirected.
replace_once("include/zephyr/sys/util.h",
             "#define __z_log2d(x) (32 - __builtin_clz(x) - 1)\n"
             "#define __z_log2q(x) (64 - __builtin_clzll(x) - 1)\n"
             "#define __z_log2(x)  (sizeof(__typeof__(x)) > 4 ? __z_log2q(x) : __z_log2d(x))",
             "#if defined(__TI_COMPILER_VERSION__)\n"
             "/* cl2000: LOG2 helpers must be integral constant expressions. */\n"
             "#define __z_log2d(x) \\\n"
             "\t((x) < 2  ? 0 : (x) < 4  ? 1 : (x) < 8  ? 2 : (x) < 16 ? 3 : \\\n"
             "\t (x) < 32 ? 4 : (x) < 64 ? 5 : (x) < 128 ? 6 : (x) < 256 ? 7 : \\\n"
             "\t (x) < 512 ? 8 : (x) < 1024 ? 9 : (x) < 2048 ? 10 : \\\n"
             "\t (x) < 4096 ? 11 : (x) < 8192 ? 12 : (x) < 16384 ? 13 : \\\n"
             "\t (x) < 32768 ? 14 : (x) < 65536 ? 15 : (x) < 131072 ? 16 : \\\n"
             "\t (x) < 262144 ? 17 : (x) < 524288 ? 18 : (x) < 1048576 ? 19 : \\\n"
             "\t (x) < 2097152 ? 20 : (x) < 4194304 ? 21 : (x) < 8388608 ? 22 : \\\n"
             "\t (x) < 16777216 ? 23 : (x) < 33554432 ? 24 : (x) < 67108864 ? 25 : \\\n"
             "\t (x) < 134217728 ? 26 : (x) < 268435456 ? 27 : (x) < 536870912 ? 28 : \\\n"
             "\t (x) < 1073741824 ? 29 : (x) < 2147483648UL ? 30 : 31)\n"
             "#define __z_log2q(x) __z_log2d(x)\n"
             "#define __z_log2(x) __z_log2d(x)\n"
             "#else\n"
             "#define __z_log2d(x) (32 - __builtin_clz(x) - 1)\n"
             "#define __z_log2q(x) (64 - __builtin_clzll(x) - 1)\n"
             "#define __z_log2(x)  (sizeof(__typeof__(x)) > 4 ? __z_log2q(x) : __z_log2d(x))\n"
             "#endif")

# NHPOT's (1ULL << 63) sentinel is also u64-only; on C28x use 32-bit max.
replace_once("include/zephyr/sys/util.h",
             "#define NHPOT(x) ((x) < 1 ? 1 : ((x) > (1ULL << 63) ? 0 : 1ULL << LOG2CEIL(x)))",
             "#if defined(__TI_COMPILER_VERSION__)\n"
             "#define NHPOT(x) ((x) < 1 ? 1 : ((x) > 0x80000000UL ? 0 : 1UL << LOG2CEIL(x)))\n"
             "#else\n"
             "#define NHPOT(x) ((x) < 1 ? 1 : ((x) > (1ULL << 63) ? 0 : 1ULL << LOG2CEIL(x)))\n"
             "#endif")

append_once("drivers/serial/Kconfig", 'rsource "Kconfig.f2837xd"')
append_once("drivers/serial/CMakeLists.txt",
            "zephyr_library_sources_ifdef(CONFIG_UART_F2837XD_SCI uart_f2837xd_sci.c)")
append_once("drivers/timer/Kconfig", 'source "drivers/timer/Kconfig.f2837xd"')
append_once("drivers/timer/CMakeLists.txt",
            "zephyr_library_sources_ifdef(CONFIG_TI_F2837XD_CPUTIMER f2837xd_cputimer.c)")
PY

echo
echo "done. next:"
echo "  export TI_C2000_CGT_PATH=/path/to/ti-cgt-c2000_21.6.0.LTS"
echo "  west build -b launchxl_f28379d -DZEPHYR_TOOLCHAIN_VARIANT=ti-cgt-c2000 samples/hello_world"
echo
echo "NOTE: the kernel/linker backend is stage 2; see port/PORTING.md."
