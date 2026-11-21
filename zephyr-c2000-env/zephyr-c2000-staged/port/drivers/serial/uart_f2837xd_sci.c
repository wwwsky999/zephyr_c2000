/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * Polling SCI-A console driver.  C28x characters are 16 bits wide; the SCI
 * sends the low 8 bits of each word.
 */
#define DT_DRV_COMPAT ti_f2837xd_scia

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <f2837xd_hal.h>

struct uart_f2837xd_config {
	uint32_t current_speed;
};

static int uart_f2837xd_init(const struct device *dev)
{
	const struct uart_f2837xd_config *cfg = dev->config;

	hal_sci_init(cfg->current_speed);
	return 0;
}

static int uart_f2837xd_poll_in(const struct device *dev, unsigned char *c)
{
	ARG_UNUSED(dev);

	if (SciaRegs.SCIFFRX.bit.RXFFST == 0U) {
		return -1;
	}

	*c = (unsigned char)(SciaRegs.SCIRXBUF.all & 0xffU);
	return 0;
}

static void uart_f2837xd_poll_out(const struct device *dev, unsigned char c)
{
	ARG_UNUSED(dev);

	hal_sci_putc((char)c);
}

static int uart_f2837xd_err_check(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

static const struct uart_driver_api uart_f2837xd_api = {
	.poll_in = uart_f2837xd_poll_in,
	.poll_out = uart_f2837xd_poll_out,
	.err_check = uart_f2837xd_err_check,
};

#define UART_F2837XD_INIT(n)						\
	static const struct uart_f2837xd_config uart_f2837xd_cfg_##n = { \
		.current_speed = DT_INST_PROP(n, current_speed),		\
	};								\
									\
	DEVICE_DT_INST_DEFINE(n, uart_f2837xd_init, NULL, NULL,		\
			      &uart_f2837xd_cfg_##n, PRE_KERNEL_1,	\
			      CONFIG_SERIAL_INIT_PRIORITY,		\
			      &uart_f2837xd_api);

DT_INST_FOREACH_STATUS_OKAY(UART_F2837XD_INIT)
