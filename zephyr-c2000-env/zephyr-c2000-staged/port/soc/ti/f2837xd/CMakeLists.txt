# Copyright (c) 2026 Zephyr C2000 port
# SPDX-License-Identifier: Apache-2.0

add_subdirectory(${CONFIG_SOC_SERIES})

# lnk2000 uses a fixed .cmd file; no generated GNU linker script is provided
# during stage 1.  SOC_LINKER_SCRIPT is intentionally left unset.
