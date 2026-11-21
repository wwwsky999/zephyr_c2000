/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * C28x ePIE interrupt plumbing.
 *
 * The TI linker uses command files (.cmd) instead of GNU ld scripts, so
 * CONFIG_GEN_ISR_TABLES is disabled and the vector table is populated at
 * runtime through the SoC layer.
 */
#include <zephyr/kernel.h>
#include <zephyr/irq.h>
#include <zephyr/arch/c2000/arch.h>

unsigned int z_c2000_irq_nesting;

struct c2000_irq_entry {
	void (*isr)(const void *arg);
	const void *arg;
};

static struct c2000_irq_entry c2000_irq_table[CONFIG_NUM_IRQS];

/* SoC-layer hook (implemented by soc/ti/f2837xd); no upstream header
 * declares the per-arch SoC hook, so declare it here.
 */
extern void soc_irq_connect(unsigned int irq);

void z_c2000_irq_connect(unsigned int irq, void (*isr)(const void *),
			 const void *param)
{
	if (irq >= CONFIG_NUM_IRQS) {
		return;
	}

	c2000_irq_table[irq].isr = isr;
	c2000_irq_table[irq].arg = param;

	/* The SoC layer installs a per-line trampoline in the ePIE vector
	 * table.  Until the trampoline generator is implemented the entry is
	 * only recorded; see port/PORTING.md (stage 2).
	 */
	soc_irq_connect(irq);
}

void z_c2000_irq_dispatch(unsigned int irq)
{
	if (irq >= CONFIG_NUM_IRQS) {
		return;
	}

	if (c2000_irq_table[irq].isr != NULL) {
		c2000_irq_table[irq].isr(c2000_irq_table[irq].arg);
	}
}
