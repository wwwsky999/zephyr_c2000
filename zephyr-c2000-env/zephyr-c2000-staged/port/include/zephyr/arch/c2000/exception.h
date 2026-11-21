/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ARCH_C2000_EXCEPTION_H_
#define ZEPHYR_INCLUDE_ARCH_C2000_EXCEPTION_H_

#ifndef _ASMLANGUAGE
#include <zephyr/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Frame pushed by the C28x interrupt prologue.  The compiler saves ACC,
 * P, ST0/ST1 and the return address; the port adds the remaining XAR
 * registers in the assembly wrapper (stage 2).
 */
struct arch_esf {
	uint32_t acc;
	uint32_t xar0;
	uint32_t xar1;
	uint32_t xar2;
	uint32_t xar3;
	uint32_t xar4;
	uint32_t xar5;
	uint32_t xar6;
	uint32_t xar7;
	uint32_t p;
	uint32_t st0;
	uint32_t st1;
	uint32_t rpc;
};

#ifdef __cplusplus
}
#endif

#endif /* _ASMLANGUAGE */

#endif /* ZEPHYR_INCLUDE_ARCH_C2000_EXCEPTION_H_ */
