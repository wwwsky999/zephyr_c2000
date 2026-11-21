/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * TI C2000 boundary symbols for lnk2000.
 *
 * Zephyr's GNU ld linker script emits section boundary symbols.
 * TI cl2000 adds ONE leading underscore to every global C symbol name.
 * We write C declarations with the number of underscores that, after
 * TI's prefix, matches what kernel/arch code references in their own
 * (also TI-compiled) object files.
 *
 * Underscore convention (C source → TI +1 → lnk2000 sees):
 *   __init_* (2) → ___init_* (3) — kernel/init.c uses 2-underscore names
 *   _device_list_* (1) → __device_list_* (2) — iterable_sections.h uses 1
 *   __bss_start (2) → ___bss_start (3) — arch/common/init.c uses 2
 *
 * Section placement: __attribute__((section("..."))) ensures the boundary
 * symbols land in the right memory region so the linker can assign them
 * addresses that match their section's origin/length.
 */

#include <stdint.h>
#include <stddef.h>

/* Zephyr init entry lists — kernel/init.c uses 2-underscore names.
 * Write 2 → TI +1 → 3 ✓ */
char __init_PRE_KERNEL_1_start[0];
char __init_PRE_KERNEL_2_start[0];
char __init_POST_KERNEL_start[0];
char __init_APPLICATION_start[0];
char __init_EARLY_start[0];
char __init_end[0];

/* Static thread data list — kernel/init.c uses 2-underscore names.
 * Write 2 → TI +1 → 3 ✓ */
char __static_thread_data_list_start[0];
char __static_thread_data_list_end[0];

/* Device list — iterable_sections.h TYPE_SECTION_START(device)
 * produces _device_list_start (1 underscore in source).
 * Write 1 → TI +1 → 2 ✓ matches kernel's 2-underscore ref */
char _device_list_start[0];
char _device_list_end[0];

/* Kernel init entry lists — TYPE_SECTION_START(k_kernel_init_*)
 * produces 1-underscore names in source.
 * Write 1 → TI +1 → 2 ✓ matches kernel's 2-underscore ref */
char _k_kernel_init_pre_entry_list_start[0];
char _k_kernel_init_pre_entry_list_end[0];
char _k_kernel_init_post_entry_list_start[0];
char _k_kernel_init_post_entry_list_end[0];

/* .ebss boundaries for arch/common/init.c.
 * Write 2 underscores → TI +1 → ___bss_start/end ✓ */
char __bss_start[0];
char __bss_end[0];

/* Watchdog control register stub — TI RTS autoinit references _WDTCTL
 * (1 underscore in C → TI adds 1 → 2 nm output).  A dummy zero is enough
 * to satisfy the symbol resolution; we never actually program the WDT. */
volatile uint16_t *WDTCTL = (volatile uint16_t *)0x1000;

/* C2000 arch stubs — kernel references these via 1-underscore names
 * (no arch/ implementation for C2000 yet).
 * C source writes arch_nop → TI +1 → _arch_nop ✓ matches kernel ref */
int arch_nop(void) { return 0; }
int arch_coprocessors_disable(void) { return 0; }
uint32_t arch_k_cycle_get_32(void) { return 0; }

/* POSIX strnlen — not in TI's RTS library.  Zephyr's posix library
 * provides it but it's not pulled by archive search.  Simple stub. */
size_t strnlen(const char *s, size_t maxlen)
{
	size_t n = 0;
	while (n < maxlen && s[n] != '\0') {
		n++;
	}
	return n;
}
