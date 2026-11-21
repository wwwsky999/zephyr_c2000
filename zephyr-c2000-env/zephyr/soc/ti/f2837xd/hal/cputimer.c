/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * CPU timer 0 as the Zephyr system tick.  The timer runs from SYSCLK with
 * a prescaler of one (TPR/TPRH reset values); PRD counts full SYSCLK ticks.
 */
#include <f2837xd_hal.h>

/* CPU interrupt line that PIE group 1 fans out from. */
#define HAL_TIMER0_M_INT	M_INT1

void hal_cputimer0_init(uint32_t prd, hal_isr_t isr)
{
	/* Stop and reload the timer before reprogramming it. */
	CpuTimer0Regs.TCR.bit.TSS = 1U;
	CpuTimer0Regs.TPR.all = 0U;
	CpuTimer0Regs.TPRH.all = 0U;
	CpuTimer0Regs.PRD.all = prd;

	/* Install the handler on ePIE line 1.7: line = (1-1)*8 + (7-1). */
	hal_pie_register(6U, isr);

	/* Opt the group-1 slot 7 interrupt in and clear stale acks. */
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1U;
	PieCtrlRegs.PIEACK.all = 0xFFFFU;

	/* Unmask CPU INT1, the carrier interrupt of PIE group 1. */
	IER |= HAL_TIMER0_M_INT;
}

void hal_cputimer0_start(void)
{
	/* Reload the prescaler counters and start the timer. */
	CpuTimer0Regs.TCR.bit.TRB = 1U;
	CpuTimer0Regs.TCR.bit.TIE = 1U;
	CpuTimer0Regs.TCR.bit.TSS = 0U;
}
