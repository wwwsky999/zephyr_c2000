/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * GPIO bring-up for the SCIA console pins of the LAUNCHXL-F28379D:
 * GPIO28 = SCIRXDA (input, pull-up), GPIO29 = SCITXDA (output).  Both pins
 * select the SCI-A function through mux option 1 of GPAMUX2.
 */
#include <f2837xd_hal.h>

void hal_gpio_init(void)
{
	EALLOW;
	/* Pull-up on the receive line. */
	GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0U;
	/* Mux option 1: 28 = SCIRXDA, 29 = SCITXDA. */
	GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1U;
	GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1U;
	/* RX is an input, TX an output. */
	GpioCtrlRegs.GPADIR.bit.GPIO28 = 0U;
	GpioCtrlRegs.GPADIR.bit.GPIO29 = 1U;
	EDIS;
}
