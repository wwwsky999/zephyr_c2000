/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * SCI-A console bring-up and polled transmit.  8N1 at the requested baud
 * rate from LSPCLK (SYSCLK / 4); FIFO enhancements on with the receiver
 * level set to one byte so the driver can simply poll RXFFST.
 */
#include <f2837xd_hal.h>

/* BRR = LSPCLK / (8 * baud) - 1, 16-bit value. */
#define HAL_SCI_BRR(baud)	((HAL_LSPCLK_HZ / (8U * (baud))) - 1U)

void hal_sci_init(uint32_t baud)
{
	uint32_t brr = HAL_SCI_BRR(baud);

	/* Pin mux is separate from the peripheral: SCIA on GPIO28/29. */
	hal_gpio_init();

	/* Hold the SCI in reset while it is reconfigured. */
	SciaRegs.SCICTL1.bit.SWRESET = 0U;
	SciaRegs.SCIFFTX.bit.TXFIFORESET = 0U;
	SciaRegs.SCIFFRX.bit.RXFIFORESET = 0U;

	/* 8 data bits, no parity, one stop bit, idle-line mode. */
	SciaRegs.SCICCR.bit.SCICHAR = 7U;
	SciaRegs.SCICCR.bit.PARITYENA = 0U;
	SciaRegs.SCICCR.bit.STOPBITS = 0U;
	SciaRegs.SCICCR.bit.LOOPBKENA = 0U;

	/* Baud rate from LSPCLK. */
	SciaRegs.SCIHBAUD.all = (uint16_t)((brr >> 8U) & 0xFFU);
	SciaRegs.SCILBAUD.all = (uint16_t)(brr & 0xFFU);

	/* FIFO mode with one-byte receive threshold. */
	SciaRegs.SCIFFTX.all = 0x0000U;
	SciaRegs.SCIFFRX.all = 0x0000U;
	SciaRegs.SCIFFTX.bit.SCIRST = 1U;
	SciaRegs.SCIFFTX.bit.SCIFFENA = 1U;
	SciaRegs.SCIFFTX.bit.TXFIFORESET = 1U;
	SciaRegs.SCIFFRX.bit.RXFIFORESET = 1U;
	SciaRegs.SCIFFRX.bit.RXFFIL = 1U;

	/* Enable the transmitter and receiver, then release reset. */
	SciaRegs.SCICTL1.bit.TXENA = 1U;
	SciaRegs.SCICTL1.bit.RXENA = 1U;
	SciaRegs.SCICTL1.bit.SWRESET = 1U;
}

void hal_sci_putc(char c)
{
	/* Block until the TX FIFO drains completely. */
	while (SciaRegs.SCIFFTX.bit.TXFFST != 0U) {
		;
	}

	/* The SCI transmits the low 8 bits of the 16-bit word. */
	SciaRegs.SCITXBUF.all = (uint16_t)c;
}
