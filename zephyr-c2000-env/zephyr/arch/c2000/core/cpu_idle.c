/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>

void arch_cpu_idle(void)
{
	/* C28x low-power idle: the core stops until an interrupt is taken. */
	__asm("  IDLE");
}
