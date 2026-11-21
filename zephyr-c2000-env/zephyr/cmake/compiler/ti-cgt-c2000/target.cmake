# Copyright (c) 2026 Zephyr C2000 port
# SPDX-License-Identifier: Apache-2.0
#
# Target phase: resolve cl2000/lnk2000 helpers and describe how C28x objects are
# produced.  Zephyr clears CMAKE_C_COMPILER between the host and target phase,
# so all compiler discovery has to happen here.

# lnk2000 consumes .cmd command files and does not understand the GNU driver
# arguments CMake would use for a try_compile() executable.  Compile-only
# checks are enough for the toolchain capability tests.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(NO_BUILD_TYPE_WARNING 1)
set(CMAKE_NOT_USING_CONFIG_FLAGS 1)

find_program(CMAKE_C_COMPILER
  NAMES cl2000
  PATHS ${TOOLCHAIN_HOME}
  PATH_SUFFIXES bin
  NO_DEFAULT_PATH
  REQUIRED)
find_program(CMAKE_CXX_COMPILER
  NAMES cl2000
  PATHS ${TOOLCHAIN_HOME}
  PATH_SUFFIXES bin
  NO_DEFAULT_PATH
  REQUIRED)
find_program(CMAKE_ASM_COMPILER
  NAMES cl2000
  PATHS ${TOOLCHAIN_HOME}
  PATH_SUFFIXES bin
  NO_DEFAULT_PATH
  REQUIRED)
find_program(CMAKE_AR
  NAMES ar2000
  PATHS ${TOOLCHAIN_HOME}
  PATH_SUFFIXES bin
  NO_DEFAULT_PATH)
find_program(CMAKE_RANLIB
  NAMES ar2000
  PATHS ${TOOLCHAIN_HOME}
  PATH_SUFFIXES bin
  NO_DEFAULT_PATH)

message(STATUS "Found TI C2000 C compiler ${CMAKE_C_COMPILER}")

# cl2000 does not accept the GCC -o flag when compiling; the output object has
# to be named with --output_file.
set(CMAKE_C_COMPILE_OBJECT
  "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES> <FLAGS> --compile_only --output_file=<OBJECT> <SOURCE>")
set(CMAKE_CXX_COMPILE_OBJECT
  "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> --compile_only --output_file=<OBJECT> <SOURCE>")
set(CMAKE_ASM_COMPILE_OBJECT
  "<CMAKE_ASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> --compile_only --output_file=<OBJECT> <SOURCE>")

# F2837xD: C28x CPU with the 32-bit FPU and the trigonometric math unit.
#
# C28x is word-addressed (one address unit == 16 bits).  TI CGT 21.6.0's
# lnk2000 only understands TI-COFF inputs; EABI/ELF objects cause the linker
# to silently report "no input section is linked in".  We therefore use the
# default COFF ABI (no explicit --abi flag) throughout the build.
#
# These flags must live in CMAKE_*_FLAGS (not just TOOLCHAIN_C_FLAGS) because
# Zephyr's pre-link stage (zephyr_pre0) compiles misc/empty_file.c without
# linking against zephyr_interface.
#
# CMAKE_*_FLAGS are space-separated strings.  Build them as a single string to
# avoid CMake list->string semicolon conversion.
set(C2000_ARCH_FLAGS
  -v28
  --float_support=fpu32
  --tmu_support=tmu0
)

string(REPLACE ";" " " C2000_ARCH_FLAGS_STR "${C2000_ARCH_FLAGS}")
# Stay on the default TI COFF ABI (--abi=coffabi).  --abi=eabi switches
# cl2000 to ELF output, which lnk2000 cannot consume and which also
# disables --disable_clink (our "whole-archive" workaround).  The default
# ABI gives symbols a single leading underscore for both definitions and
# references — Zephyr's code consistently uses bare names such as
# ``z_impl_k_queue_init`` so definitions and references match up.
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${C2000_ARCH_FLAGS_STR} --include_path=${TOOLCHAIN_HOME}/include --c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C2000_ARCH_FLAGS_STR} --include_path=${TOOLCHAIN_HOME}/include --c++14")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${C2000_ARCH_FLAGS_STR}")

# --disable_clink is injected after --run_linker by the linker backend
# (cmake/linker/ti-cgt-c2000/target.cmake).  cl2000 ignores unknown flags
# before --run_linker so we must not try to pass them here.

# Also expose the same flags through TOOLCHAIN_C_FLAGS for the Zephyr
# capability probes that read that variable directly.
foreach(flag ${C2000_ARCH_FLAGS})
  list(APPEND TOOLCHAIN_C_FLAGS ${flag})
endforeach()
list(APPEND TOOLCHAIN_C_FLAGS --include_path=${TOOLCHAIN_HOME}/include)
list(APPEND TOOLCHAIN_C_FLAGS --c11)

# Zephyr's generic implementation queries --print-libgcc-file-name, which
# cl2000 does not implement.  Point the linker at the C2000 EABI runtime
# library directly instead.
function(compiler_set_linker_properties)
  set_linker_property(PROPERTY lib_include_dir "-L\"${TOOLCHAIN_HOME}/lib\"")
  set_linker_property(PROPERTY
    rt_library "\"${TOOLCHAIN_HOME}/lib/rts2800_fpu32_eabi.lib\"")
endfunction()
