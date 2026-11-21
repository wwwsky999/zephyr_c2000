/*
 * Copyright (C) 2026 WWWSKY999. All rights reserved.
 *
 * Zephyr 内核接入 - C28x 架构内核数据定义
 */

#ifndef _ARCH_C28X_KERNEL_ARCH_DATA_H_
#define _ARCH_C28X_KERNEL_ARCH_DATA_H_

#include <stdint.h>

/* 
 * 线程栈架构扩展部分。
 * C28x 使用 16-bit 字栈，需要保存的寄存器包括：
 * - 程序计数器 PC (32-bit)
 * - 栈指针 SP
 * - 状态寄存器 ST0, ST1
 * - 数据页寄存器 DP
 * - 帧指针 FP / 局部帧指针
 * - 16 个数据寄存器 (R0H - R7H, AR0 - AR7)
 * 在此最小移植中，我们简化为仅保留基本的上下文。
 */
struct _callee_saved {
	uint32_t sp;      /* 栈指针 */
	uint16_t st0;     /* 状态寄存器 ST0 */
	uint16_t st1;     /* 状态寄存器 ST1 */
	uint16_t dp;      /* 数据页指针 */
	uint16_t pc;      /* 程序计数器 (低16位) */
};

struct _caller_saved {
	/* 最小移植：调用者保存的寄存器不在此处保存 */
};

/* 线程基元 */
typedef struct _callee_saved _callee_saved_t;
typedef struct _caller_saved _caller_saved_t;

#define _STACK_ALIGN 4

#endif /* _ARCH_C28X_KERNEL_ARCH_DATA_H_ */