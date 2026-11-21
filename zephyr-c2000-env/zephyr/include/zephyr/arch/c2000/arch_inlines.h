/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_C2000_ARCH_INLINES_H_
#define ZEPHYR_INCLUDE_ARCH_C2000_ARCH_INLINES_H_

#include <zephyr/kernel_structs.h>

/* Runtime CPU id — returns 0 for CPU1, 1 for CPU2.
 * Implemented in prep_c.c; the value is read from IPCBootSTS at boot
 * time and cached in a global (IPC registers don't change after Boot ROM
 * hands off). */
extern unsigned int z_c2000_cpu_id(void);

static ALWAYS_INLINE unsigned int arch_cpu_id(void)
{
	return z_c2000_cpu_id();
}

static ALWAYS_INLINE unsigned int arch_num_cpus(void)
{
	return CONFIG_MP_MAX_NUM_CPUS;
}

#endif /* ZEPHYR_INCLUDE_ARCH_C2000_ARCH_INLINES_H_ */
