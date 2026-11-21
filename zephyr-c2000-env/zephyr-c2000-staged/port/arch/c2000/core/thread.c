/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * Thread creation and context switching.
 *
 * STAGE 2 TODO: the C28x context switch has to be written in assembly
 * (arch/c2000/core/switch.S) because cl2000 does not accept GNU-style
 * extended inline assembly.  The routines below are the lib-side skeleton.
 */
#include <zephyr/kernel.h>
#include <zephyr/kernel_structs.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/arch/c2000/arch.h>

void arch_new_thread(struct k_thread *thread, k_thread_stack_t *stack,
		     char *stack_ptr, k_thread_entry_t entry,
		     void *p1, void *p2, void *p3)
{
	ARG_UNUSED(stack);
	ARG_UNUSED(stack_ptr);
	ARG_UNUSED(entry);
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	/* TODO(stage 2): build the initial interrupt-return frame on the
	 * thread stack (RPC = thread_entry, ACC = p1, XAR4 = p2, XAR5 = p3).
	 */
	thread->callee_saved = (struct _callee_saved){0};
}

FUNC_NORETURN void z_c2000_arch_switch(void *switch_to, void **switched_from)
{
	ARG_UNUSED(switch_to);
	ARG_UNUSED(switched_from);

	/* TODO(stage 2): save the callee-saved C28x registers on the current
	 * stack, store SP into *switched_from, load SP from switch_to and
	 * restore the registers.
	 */
	arch_system_halt(0);
}
