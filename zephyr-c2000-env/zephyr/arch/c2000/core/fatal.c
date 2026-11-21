/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>

FUNC_NORETURN void arch_system_halt(unsigned int reason)
{
	ARG_UNUSED(reason);

	arch_irq_lock();
	for (;;) {
		__asm("  NOP");
	}
}
