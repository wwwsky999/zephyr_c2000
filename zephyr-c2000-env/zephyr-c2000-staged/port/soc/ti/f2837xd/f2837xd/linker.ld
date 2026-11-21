/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * TMS320F2837xD memory map.
 *
 * NOTE: lnk2000 does not consume GNU ld scripts.  The image is linked with
 * F2837xD_Zephyr.cmd in stage 2.  This file exists so Zephyr's build system
 * has a linker script to reference, and documents the region layout shared by
 * both linkers.
 *
 * C28x is word addressed: 1 CPU address unit == 16 bits.
 */

MEMORY
{
	/* Dedicated M0/M1 RAM, word addressable, 1K words each */
	RAMM0      (rwx) : ORIGIN = 0x000000, LENGTH = 0x000400
	RAMM1      (rwx) : ORIGIN = 0x000400, LENGTH = 0x000400

	/* Local shared RAM (LS) and global shared RAM (GS) */
	RAMLS      (rwx) : ORIGIN = 0x008000, LENGTH = 0x001000
	RAMGS      (rwx) : ORIGIN = 0x00A000, LENGTH = 0x002000

	/* Flash and its mirrored regions */
	FLASH      (rx)  : ORIGIN = 0x080000, LENGTH = 0x100000
	FLASH_OTP  (rx)  : ORIGIN = 0x078000, LENGTH = 0x000800

	/* Peripheral frame */
	PERIPH     (rw)  : ORIGIN = 0x400000, LENGTH = 0x020000
}

ENTRY(code_start)

SECTIONS
{
	.text : {
		KEEP(*(.text.code_start))
		*(.text)
		*(.text.*)
	} > FLASH

	.rodata : {
		*(.rodata)
		*(.rodata.*)
	} > FLASH

	.data : {
		*(.data)
		*(.data.*)
	} > RAMLS AT > FLASH

	.bss (NOLOAD) : {
		*(.bss)
		*(.bss.*)
		*(COMMON)
	} > RAMLS

	.stack (NOLOAD) : {
		. = ALIGN(4);
		*(.stack)
	} > RAMM1
}
