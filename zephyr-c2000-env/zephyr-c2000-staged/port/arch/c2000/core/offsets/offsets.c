/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * Absolute symbols describing kernel structure layout.  gen_offset_header.py
 * turns these into include/generated/zephyr/offsets.h, which switch.S will
 * consume once the context switch exists (stage 2).
 */

#include <gen_offset.h>
#include <zephyr/kernel.h>
#include <kernel_arch_data.h>
#include <kernel_offsets.h>

/* Priority is needed when the assembly context switch inspects a thread. */
GEN_OFFSET_SYM(_thread_base_t, prio);

GEN_ABSOLUTE_SYM(K_THREAD_SIZEOF, sizeof(struct k_thread));
GEN_ABSOLUTE_SYM(__struct_arch_esf_SIZEOF, sizeof(struct arch_esf));
GEN_ABSOLUTE_SYM(__callee_saved_t_SIZEOF,
		 ROUND_UP(sizeof(_callee_saved_t), ARCH_STACK_PTR_ALIGN));

GEN_ABS_SYM_END
