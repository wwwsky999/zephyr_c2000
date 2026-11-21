# Copyright (c) 2026 Zephyr C2000 port
# SPDX-License-Identifier: Apache-2.0

zephyr_library_sources_ifdef(CONFIG_C2000
	cpu_idle.c
	fatal.c
	irq.c
	thread.c
	prep_c.c
)
