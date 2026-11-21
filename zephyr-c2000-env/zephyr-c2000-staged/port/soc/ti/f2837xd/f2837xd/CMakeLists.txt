# Copyright (c) 2026 Zephyr C2000 port
# SPDX-License-Identifier: Apache-2.0

# lnk2000 consumes F2837xD_zephyr.cmd directly.  linker.ld documents the same
# memory map for reference but is not used by the TI toolchain.
set(SOC_LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/F2837xD_zephyr.cmd CACHE INTERNAL "")

# F28379D is a dual-core device; the C2000Ware device headers require
# exactly one of CPU1 or CPU2 to be defined.  Zephyr boots CPU1 first, so
# default to CPU1.
zephyr_compile_definitions(CPU1 _CPU1)

# C2000Ware device headers and the shared F2837xD HAL.
zephyr_include_directories(.)
zephyr_include_directories(../c2000ware/include)
zephyr_include_directories(../hal/include)

zephyr_library_sources(
	soc.c
	c2000_zephyr_symbols.c
	../hal/sysctl.c
	../hal/gpio.c
	../hal/sci.c
	../hal/pie.c
	../hal/cputimer.c
)

# The register structure definitions must be linked with the C2000Ware
# command file that places them at the peripheral base addresses.
zephyr_library_sources(../c2000ware/source/F2837xD_GlobalVariableDefs.c)
