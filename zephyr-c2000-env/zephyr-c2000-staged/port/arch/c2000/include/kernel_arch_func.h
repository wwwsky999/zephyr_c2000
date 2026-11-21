/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_C2000_INCLUDE_KERNEL_ARCH_FUNC_H_
#define ZEPHYR_ARCH_C2000_INCLUDE_KERNEL_ARCH_FUNC_H_

#ifndef _ASMLANGUAGE
#include <kernel_arch_data.h>

#ifdef __cplusplus
extern "C" {
#endif

static ALWAYS_INLINE void arch_kernel_init(void)
{
}

static inline bool arch_is_in_isr(void)
{
	return arch_curr_cpu()->nested != 0U;
}

extern FUNC_NORETURN void z_c2000_arch_switch(void *switch_to,
					      void **switched_from);

static ALWAYS_INLINE void arch_switch(void *switch_to, void **switched_from)
{
	z_c2000_arch_switch(switch_to, switched_from);
}

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_ARCH_C2000_INCLUDE_KERNEL_ARCH_FUNC_H_ */
