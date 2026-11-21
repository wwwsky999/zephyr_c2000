/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_C2000_ARCH_H_
#define ZEPHYR_INCLUDE_ARCH_C2000_ARCH_H_

#include <zephyr/devicetree.h>
#include <zephyr/arch/c2000/thread.h>
#include <zephyr/arch/c2000/exception.h>
#include <zephyr/arch/common/sys_io.h>
#include <zephyr/arch/common/ffs.h>
#include <zephyr/sw_isr_table.h>
#include <zephyr/kernel_structs.h>
#include <zephyr/sys/util.h>
#include <zephyr/irq.h>

/*
 * C28x stack elements are 16-bit words.  The alignment below is expressed in
 * target "chars" (the unit used by sizeof on C28x), not in 8-bit bytes.
 */
#define ARCH_STACK_PTR_ALIGN 2

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Software interrupt-nesting state.  The C28x core exposes the global
 * interrupt mask (ST1.INTM) only through assembly, so the port keeps the
 * state in software.  This is sufficient for a uniprocessor build.
 */
extern unsigned int z_c2000_irq_nesting;

static ALWAYS_INLINE unsigned int arch_irq_lock(void)
{
	unsigned int key = z_c2000_irq_nesting;

	z_c2000_irq_nesting = 1U;
	__asm("  DINT");

	return key;
}

static inline void arch_irq_unlock(unsigned int key)
{
	if (key == 0U) {
		z_c2000_irq_nesting = 0U;
		__asm("  EINT");
	}
}

static inline bool arch_irq_unlocked(unsigned int key)
{
	return key == 0U;
}

static ALWAYS_INLINE bool arch_cpu_irqs_are_enabled(void)
{
	return z_c2000_irq_nesting == 0U;
}

static ALWAYS_INLINE _cpu_t *arch_curr_cpu(void)
{
	return &_kernel.cpus[0];
}

/* IRQ_CONNECT() is resolved at runtime by the SoC ePIE layer because the
 * TI linker cannot run Zephyr's gen_isr_tables.py step.
 */
void z_c2000_irq_connect(unsigned int irq, void (*isr)(const void *),
			 const void *param);

#define ARCH_IRQ_CONNECT(irq_p, priority_p, isr_p, isr_param_p, flags_p) \
	z_c2000_irq_connect(irq_p, (void (*)(const void *))(isr_p), isr_param_p)

#ifdef __cplusplus
}
#endif

#endif /* !_ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_C2000_ARCH_H_ */
