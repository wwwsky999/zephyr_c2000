/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * SoC glue for TMS320F2837xD.  The register-level work lives in the shared
 * HAL (hal/f2837xd) so that the same code is exercised by the standalone
 * bring-up image.
 */
#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/arch/c2000/arch.h>

#include "soc.h"
#include "f2837xd_hal.h"

void soc_early_init_hook(void)
{
	hal_sysctl_init();
	hal_pie_init();
}

void soc_irq_connect(unsigned int line)
{
	/* STAGE 2: install a per-line ePIE trampoline here.  The C28x vector
	 * table has one entry per PIE line, so 96 small assembly stubs (or a
	 * dispatcher that reads PIEIFR) are required.
	 */
	ARG_UNUSED(line);
}
