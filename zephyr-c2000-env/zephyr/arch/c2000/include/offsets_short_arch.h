/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * Arch-specific thread structure offsets for C2000.
 *
 * Stage 2 leaves arch-specific thread layout untouched (pure C-level
 * thread.c).  All the offsets that offsets_short.h needs come from the
 * generic offsets.c (___cpu_t_nested_OFFSET, ___thread_t_arch_OFFSET ...)
 * so there is nothing arch-specific to add here yet.
 */

#ifndef ZEPHYR_ARCH_C2000_INCLUDE_OFFSETS_SHORT_ARCH_H_
#define ZEPHYR_ARCH_C2000_INCLUDE_OFFSETS_SHORT_ARCH_H_

#include <zephyr/offsets.h>

/* nothing for now */

#endif /* ZEPHYR_ARCH_C2000_INCLUDE_OFFSETS_SHORT_ARCH_H_ */
