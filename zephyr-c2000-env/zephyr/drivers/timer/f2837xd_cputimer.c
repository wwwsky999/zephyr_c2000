/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * CPU timer 0 system clock.  Stage 1 uses a fixed 1 kHz tick; a tickless
 * implementation can program PRD from the next timeout once the kernel
 * builds for C28x.
 */
#include <zephyr/kernel.h>
#include <zephyr/drivers/timer/system_timer.h>
#include <zephyr/sys/clock.h>

#include <f2837xd_hal.h>

#define C2000_TICK_HZ CONFIG_SYS_CLOCK_TICKS_PER_SEC

/*
 * The ePIE fetches this handler directly, so it must return with IRET
 * (__interrupt) to restore the automatic context save.
 */
static __interrupt void c2000_timer_isr(void)
{
	sys_clock_announce(1);
}

void sys_clock_driver_init(void)
{
	hal_cputimer0_init(HAL_SYSCLK_HZ / C2000_TICK_HZ, c2000_timer_isr);
	hal_cputimer0_start();
}

void sys_clock_set_timeout(uint32_t ticks, bool idle)
{
	ARG_UNUSED(ticks);
	ARG_UNUSED(idle);
}

uint32_t sys_clock_elapsed(void)
{
	return 0;
}
