# Copyright (c) 2026 Zephyr C2000 port
# SPDX-License-Identifier: Apache-2.0
#
# Host phase.  cl2000 cannot consume the GCC-style flags Zephyr passes to the
# devicetree preprocessor, so devicetree sources are run through the host gcc
# (same approach as the IAR and armclang toolchains).  Code generation for the
# target always goes through cl2000 in target.cmake.

if(NOT CMAKE_DTS_PREPROCESSOR)
  find_program(CMAKE_DTS_PREPROCESSOR gcc)
endif()
if(NOT CMAKE_DTS_PREPROCESSOR)
  message(FATAL_ERROR "host gcc is required to preprocess devicetree files")
endif()
