/*
 * Copyright (C) 2026 WWWSKY999. All rights reserved.
 *
 * TI TMS320F2837x SoC 定义
 */

#ifndef _SOC_TI_F2837X_H_
#define _SOC_TI_F2837X_H_

#include <zephyr/types.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/arch/c28x/kernel_arch_data.h>

/* SoC 基础定义 */
#define SOC_NAME "TMS320F2837x"
#define SOC_FAMILY "F2837xD"

/* 系统控制寄存器定义 (简化版) */
#define SYSCTRL_BASE	0x1F800000	/* 系统控制寄存器基地址 */

/* 外设寄存器基址 (简化示例) */
#define PERIPH_BASE	0x14000000

/* 系统时钟频率 (典型值) */
#define SYSCLK_FREQ	150000000UL

#endif /* _SOC_TI_F2837X_H_ */