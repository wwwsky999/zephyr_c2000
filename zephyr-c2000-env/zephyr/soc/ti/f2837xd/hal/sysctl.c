/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * System control bring-up: watchdog disable and peripheral clock gating for
 * the peripherals exercised by the Zephyr port (CPU timer 0, SCI-A).
 */
#include <f2837xd_hal.h>

void hal_sysctl_init(void)
{
	/*
	 * Kill the watchdog.  WDCHK must always be written as 101b while
	 * WDDIS releases the counter; the WDPS prescaler bits stay at reset.
	 */
	EALLOW;
	WdRegs.WDCR.all = 0x0068U | (WdRegs.WDCR.all & 0x0007U);
	EDIS;

	/* Clock the peripherals the port drives. */
	EALLOW;
	CpuSysRegs.PCLKCR0.bit.CPUTIMER0 = 1U;
	CpuSysRegs.PCLKCR7.bit.SCI_A = 1U;
	EDIS;
}
