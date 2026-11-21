# Copyright (c) 2026 Zephyr C2000 port
# SPDX-License-Identifier: Apache-2.0
#
# TI object file utilities.  hex2000 replaces objcopy (Intel HEX only),
# dis2000 replaces objdump and ofd2000 dumps ELF objects.

find_program(CMAKE_OBJCOPY
  NAMES hex2000
  PATHS ${TOOLCHAIN_HOME}
  PATH_SUFFIXES bin
  NO_DEFAULT_PATH
  REQUIRED)
find_program(CMAKE_OBJDUMP
  NAMES dis2000
  PATHS ${TOOLCHAIN_HOME}
  PATH_SUFFIXES bin
  NO_DEFAULT_PATH)
find_program(CMAKE_NM
  NAMES nm2000
  PATHS ${TOOLCHAIN_HOME}
  PATH_SUFFIXES bin
  NO_DEFAULT_PATH)
find_program(CMAKE_STRIP
  NAMES strip2000
  PATHS ${TOOLCHAIN_HOME}
  PATH_SUFFIXES bin
  NO_DEFAULT_PATH)
find_program(CMAKE_READELF
  NAMES ofd2000
  PATHS ${TOOLCHAIN_HOME}
  PATH_SUFFIXES bin
  NO_DEFAULT_PATH)

include(${ZEPHYR_BASE}/cmake/bintools/ti-cgt-c2000/target_bintools.cmake)
