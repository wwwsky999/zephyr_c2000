# Copyright (c) 2026 Zephyr C2000 port
# SPDX-License-Identifier: Apache-2.0
#
# TI C2000 Code Generation Tools (cl2000/lnk2000/hex2000), EABI mode.
#
# Usage:
#   export TI_C2000_CGT_PATH=/path/to/ti-cgt-c2000_21.6.0.LTS
#   west build -b launchxl_f28379d -DZEPHYR_TOOLCHAIN_VARIANT=ti-cgt-c2000 ...
#
# STATUS: stage-2 scaffolding.  The compiler and link flags are real, but
# Zephyr's linker/bintools abstractions still need the TI .cmd/hex2000
# backend.  See port/PORTING.md before attempting a build.

zephyr_get(TI_C2000_CGT_PATH)
if(NOT EXISTS "${TI_C2000_CGT_PATH}/bin/cl2000" AND
   NOT EXISTS "${TI_C2000_CGT_PATH}/bin/cl2000.exe")
  message(FATAL_ERROR
    "TI_C2000_CGT_PATH must point at a C2000 CGT installation "
    "(got '${TI_C2000_CGT_PATH}')")
endif()

set(TOOLCHAIN_HOME ${TI_C2000_CGT_PATH})
set(CROSS_COMPILE ${TOOLCHAIN_HOME}/bin/)

set(COMPILER ti-cgt-c2000)
set(LINKER ti-cgt-c2000)
set(BINTOOLS ti-cgt-c2000)

set(TOOLCHAIN_HAS_NEWLIB OFF CACHE BOOL "True if toolchain supports NewLib")
set(TOOLCHAIN_HAS_PICOLIBC OFF CACHE BOOL "True if toolchain supports Picolibc")
set(TOOLCHAIN_HAS_THREADS OFF CACHE BOOL "True if toolchain supports threads")
