# Copyright (c) 2026 Zephyr C2000 port
# SPDX-License-Identifier: Apache-2.0
#
# TI linker backend for lnk2000.
#
# STATUS: stage 2.  Zephyr's generic link flow emits a GNU ld script and -Wl
# options; lnk2000 consumes one or more .cmd files and has a different option
# syntax.  This backend keeps the configure step working and records the real
# linker, but a full Zephyr image is not linkable yet:
#   1. a fixed F2837xD .cmd that defines the Zephyr symbols
#      (__kernel_ram_start/end, _image_ram_start, __text_region_start, ...),
#   2. a lnk2000-specific link rule (replacing CMAKE_C_LINK_EXECUTABLE),
#   3. a post-link hex2000 step instead of objcopy,
#   4. a size report through ofd2000.
# See port/PORTING.md.

set_property(TARGET linker PROPERTY devices_start_symbol "_device_list_start")

find_program(CMAKE_LINKER
  NAMES lnk2000
  PATHS ${TOOLCHAIN_HOME}
  PATH_SUFFIXES bin
  NO_DEFAULT_PATH
  REQUIRED)

# lnk2000 has no -Wl, prefix: options are passed directly.
set_ifndef(LINKERFLAGPREFIX "")

macro(toolchain_ld_configure_files)
  # Zephyr expects these fragments for the GNU ld generator.  lnk2000 does not;
  # keep the hook so the build system can continue.
endmacro()

function(toolchain_ld_force_undefined_symbols)
  # lnk2000 has no --undefined equivalent; symbols are pulled in through the
  # .cmd file once the fixed linker script exists.
endfunction()

# Zephyr's generic flow asks for a generated linker script for each pass.
# lnk2000 consumes a static .cmd file and does not run it through the C
# preprocessor, so just copy the board/SoC .cmd into the build tree.
macro(configure_linker_script linker_script_gen linker_pass_define)
  set(extra_dependencies ${ARGN})
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${linker_script_gen}
    DEPENDS
      ${LINKER_SCRIPT}
      ${extra_dependencies}
    COMMAND ${CMAKE_COMMAND} -E copy ${LINKER_SCRIPT}
            ${CMAKE_CURRENT_BINARY_DIR}/${linker_script_gen}
    COMMENT "Copying TI linker command file ${linker_script_gen}"
    VERBATIM
  )
endmacro()

function(toolchain_ld_link_elf)
  cmake_parse_arguments(LINK
    ""
    "TARGET_ELF;OUTPUT_MAP;LINKER_SCRIPT"
    "LIBRARIES_PRE_SCRIPT;LIBRARIES_POST_SCRIPT;DEPENDENCIES"
    ${ARGN})

  if(LINK_LINKER_SCRIPT)
    # lnk2000 consumes the .cmd file as a plain linker input.  Passing it
    # through target_link_libraries places it after --run_linker (where
    # lnk2000 sees it), instead of through target_link_options which would
    # put it in the compiler phase before --run_linker where cl2000 tries
    # to compile it as C.
    target_link_libraries(${LINK_TARGET_ELF} ${LINK_LINKER_SCRIPT})
    set_property(TARGET ${LINK_TARGET_ELF} PROPERTY
      LINK_DEPENDS ${LINK_LINKER_SCRIPT})
  endif()

  # lnk2000 searches .lib archives normally — it pulls only members whose
  # symbols are needed, just like GNU ld.  The OFFSETS_LIB is an object
  # library (no .a output), so it must be expanded explicitly.
  target_link_libraries(${LINK_TARGET_ELF}
    ${LINK_LIBRARIES_PRE_SCRIPT}
    ${WHOLE_ARCHIVE_LIBS}
    ${NO_WHOLE_ARCHIVE_LIBS}
    ${LINK_LIBRARIES_POST_SCRIPT}
    $<TARGET_OBJECTS:${OFFSETS_LIB}>
    ${LINK_DEPENDENCIES}
  )

  # hex2000 replaces objcopy for the Intel-HEX output: the bintools
  # abstraction is stubbed out for TI CGT, so emit the .hex in a POST_BUILD
  # step instead.  Every link pass produces <pass>.hex; the final "zephyr"
  # target lands at ${PROJECT_BINARY_DIR}/zephyr/zephyr.hex.
  find_program(TI_HEX2000
    NAMES hex2000
    HINTS ${TOOLCHAIN_HOME}/bin
    NO_DEFAULT_PATH
    REQUIRED
  )
  add_custom_command(TARGET ${LINK_TARGET_ELF} POST_BUILD
    COMMAND ${TI_HEX2000} -i $<TARGET_FILE:${LINK_TARGET_ELF}>
            -o $<TARGET_FILE_DIR:${LINK_TARGET_ELF}>/$<TARGET_FILE_BASE_NAME:${LINK_TARGET_ELF}>.hex
    VERBATIM
  )
  # TI COFF convention: the linker product is a .out file (CCS loader /
  # flash tools expect that name).  The image links in COFF mode, so a copy
  # with the .out suffix is byte-identical to the linker output.
  add_custom_command(TARGET ${LINK_TARGET_ELF} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${LINK_TARGET_ELF}>
            $<TARGET_FILE_DIR:${LINK_TARGET_ELF}>/$<TARGET_FILE_BASE_NAME:${LINK_TARGET_ELF}>.out
    VERBATIM
  )
endfunction()

# Override CMake's default TI link rule.  The default places <LINK_FLAGS>
# (which carries -O2 from Zephyr's optimization setting) after --run_linker,
# where lnk2000 misinterprets -O2 as -o 2.  Move all compiler-style flags
# before --run_linker so only genuine linker options reach lnk2000.
#
# lnk2000's --disable_clink must come AFTER --run_linker — cl2000 rejects
# unknown flags during the compile phase and silently drops them.
macro(toolchain_linker_finalize)
  # Force lnk2000 to search archives.  Without these -u options, every .obj
  # passed directly on the command line has zero undefined symbols so the
  # linker has no reason to pull members from any .lib.  CMake's Ninja
  # generator splits list elements with &&, so each "-u sym" must be a
  # single string — NOT a CMake list of "-u"; "sym".
  set(C2000_FORCE_SYMBOLS
    "-u" "_main"
    "-u" "_z_c2000_start"
    "-u" "__kernel"
  )

  # TI runtime support library (RTS) — cl2000 auto-links it only when the
  # compiler phase runs; calling lnk2000 directly (as we do via --run_linker
  # and target_link_libraries) skips that step so we must add it explicitly.
  set(C2000_RTS_LIB "${TOOLCHAIN_HOME}/lib/rts2800_fpu32.lib")

  # Hardcoded -u flags to force lnk2000 to pull these symbols from archives.
  # Each "-u sym" is a single string; CMake Ninja generator would otherwise
  # split "-u"; "_sym" into two arguments.
  set(C2000_FORCE_U
    "-u _main"
    "-u _z_c2000_start"
    "-u __kernel"
    "-u _z_thread_entry"
    "-u _sys_clock_set_timeout"
    "-u _sys_clock_elapsed"
    "-u _printk"
    "-u _printk_panic"
    "-u _arch_cpu_idle"
    "-u _arch_new_thread"
    "-u _arch_early_memcpy"
    "-u _arch_k_cycle_get_32"
    "-u _arch_nop"
    "-u _arch_coprocessors_disable"
    "-u _atomic_get"
    "-u _z_impl_atomic_set"
    "-u _z_c2000_arch_switch"
    "-u _z_fatal_error"
    "-u _hal_cputimer0_init"
    "-u _hal_cputimer0_start"
  )
  # Convert CMake list to space-separated string for template expansion.
  string(REPLACE ";" " " C2000_FORCE_U_STR "${C2000_FORCE_U}")

  foreach(lang C CXX ASM)
    set(CMAKE_${lang}_LINK_EXECUTABLE
      "<CMAKE_${lang}_COMPILER> <FLAGS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> --run_linker --disable_clink -e=_z_c2000_start ${C2000_FORCE_U_STR} -u ___TI_auto_init_nobinit --output_file=<TARGET> --map_file=zephyr/<TARGET_NAME>.map <OBJECTS> <LINK_LIBRARIES> ${C2000_RTS_LIB}")
  endforeach()
endmacro()

# Pass compiler options straight through to the driver; they are consumed
# before --run_linker.
function(toolchain_linker_add_compiler_options)
  add_link_options(${ARGV})
endfunction()
