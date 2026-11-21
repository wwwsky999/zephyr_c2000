/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_ALLOCA_H_
#define ZEPHYR_INCLUDE_ALLOCA_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* cl2000 has no alloca builtin. Users of this declaration still need the
 * C28x stack-allocation helper before their code can be linked.
 */
void *alloca(size_t size);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_ALLOCA_H_ */
