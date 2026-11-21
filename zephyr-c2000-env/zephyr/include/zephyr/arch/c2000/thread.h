/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_C2000_THREAD_H_
#define ZEPHYR_INCLUDE_ARCH_C2000_THREAD_H_

#ifndef _ASMLANGUAGE
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * C28x callee-saved state.  The context switch (stage 2) must save/restore
 * the 32-bit accumulator, XAR0-XAR7, P, ST0, ST1 and RPC on the thread stack.
 * The layout is intentionally left empty until switch.S is written, so that
 * offsets are not baked into the headers prematurely.
 */
struct _callee_saved {
	/* TODO(stage 2): accumulator, XAR0..XAR7, P, ST0/ST1, RPC */
};

typedef struct _callee_saved _callee_saved_t;

struct _thread_arch {
	/* empty */
};

typedef struct _thread_arch _thread_arch_t;

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_C2000_THREAD_H_ */
