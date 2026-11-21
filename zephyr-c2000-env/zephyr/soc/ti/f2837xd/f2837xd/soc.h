/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SOC_TI_F2837XD_SOC_H_
#define SOC_TI_F2837XD_SOC_H_

#include <zephyr/types.h>

/* ePIE line index: (group - 1) * 8 + (intx - 1), 0..95. */
#define C2000_PIE_LINE(group, intx) ((((group) - 1U) * 8U) + ((intx) - 1U))

void soc_irq_connect(unsigned int line);

#endif /* SOC_TI_F2837XD_SOC_H_ */
