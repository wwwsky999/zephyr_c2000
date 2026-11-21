/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * Expanded-PIE (ePIE) bring-up for the F2837xD.  The peripheral vector
 * table lives in PIE RAM at 0x000D00 (section PieVectTableFile, placed by
 * F2837xD_zephyr.cmd) and is populated at runtime, so every unused slot is
 * routed to a default handler that just acknowledges the group.
 */
#include <f2837xd_hal.h>

/* CPU-level entries (reserved, TIMER1/2, traps, USER1-12) come first. */
#define HAL_PIE_CPU_ENTRIES	32U

__interrupt void hal_pie_default_isr(void)
{
	/* Spurious interrupt: acknowledge every PIE group and return. */
	PieCtrlRegs.PIEACK.all = 0xFFFFU;
}

void hal_pie_register(unsigned int line, hal_isr_t isr)
{
	volatile PINT *entry = (volatile PINT *)&PieVectTable;

	/* Line = (group - 1) * 8 + (intx - 1), 0..95, maps linearly onto
	 * the peripheral slots which start right after the CPU vectors. */
	entry[HAL_PIE_CPU_ENTRIES + line] = isr;
}

void hal_pie_init(void)
{
	volatile PINT *entry = (volatile PINT *)&PieVectTable;
	unsigned int i;

	/* Fill every slot with the default handler first: a spurious
	 * interrupt acknowledges instead of jumping to NULL. */
	for (i = 0U; i < HAL_PIE_VECT_ENTRIES; i++) {
		entry[i] = hal_pie_default_isr;
	}

	/* Disable all peripheral group enables until drivers opt in. */
	PieCtrlRegs.PIEIER1.all = 0U;
	PieCtrlRegs.PIEIER2.all = 0U;
	PieCtrlRegs.PIEIER3.all = 0U;
	PieCtrlRegs.PIEIER4.all = 0U;
	PieCtrlRegs.PIEIER5.all = 0U;
	PieCtrlRegs.PIEIER6.all = 0U;
	PieCtrlRegs.PIEIER7.all = 0U;
	PieCtrlRegs.PIEIER8.all = 0U;
	PieCtrlRegs.PIEIER9.all = 0U;
	PieCtrlRegs.PIEIER10.all = 0U;
	PieCtrlRegs.PIEIER11.all = 0U;
	PieCtrlRegs.PIEIER12.all = 0U;

	/* Clear any latched group acknowledgement. */
	PieCtrlRegs.PIEACK.all = 0xFFFFU;

	/* Fetch peripheral vectors from PIE RAM (base 0x000D00). */
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1U;
}
