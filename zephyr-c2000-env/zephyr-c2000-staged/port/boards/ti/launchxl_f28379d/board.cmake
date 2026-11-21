# Copyright (c) 2026 Zephyr C2000 port
# SPDX-License-Identifier: Apache-2.0
#
# Flashing is done with TI UniFlash / CCS.  No Zephyr runner is registered.

if(CONFIG_BOARD_LAUNCHXL_F28379D)
  # No runner: use `dslite`/UniFlash with the generated .hex file.
endif()
