/*
 * Copyright (C) 2026 WWWSKY999. All rights reserved.
 *
 * Zephyr 内核接入 - C28x 架构内核内联函数
 */

#ifndef _ARCH_C28X_KERNEL_ARCH_FUNC_H_
#define _ARCH_C28X_KERNEL_ARCH_FUNC_H_

#include <stdint.h>
#include <arch/c28x/arch.h>

/* 
 * 中断状态保存与恢复。
 * C28x 使用 INTM 位 (ST1) 控制中断使能。
 * 内联汇编实现中断开/关。
 */
static inline unsigned int arch_irq_lock(void)
{
	unsigned int key;
	/* 保存当前 INTM 状态并关闭中断 */
	__asm__ volatile("PUSH ST1\n\t"
			 "SETC INTM\n\t"
			 "POP  %0\n\t"
			 : "=r"(key)
			 :
			 : "memory");
	return key;
}

static inline void arch_irq_unlock(unsigned int key)
{
	/* 恢复之前的中断状态 (key 中保存的 ST1) */
	__asm__ volatile("PUSH %0\n\t"
			 "POP  ST1\n\t"
			 :
			 : "r"(key)
			 : "memory");
}

static inline bool arch_irq_is_enabled(unsigned int key)
{
	/* 从 ST1 的 INTM 位判断 (第 11 位为 INTM, 0 为使能) */
	return !((key >> 11) & 0x1);
}

/* 
 * 空闲函数：进入低功耗待机
 * C28x 的 IDLE 指令
 */
static inline void arch_cpu_idle(void)
{
	__asm__ volatile("IDLE" ::: "memory");
}

/* 
 * 数据同步/内存屏障
 * C28x 无强制内存屏障指令，空实现即可
 */
static inline void arch_data_sync(void)
{
	/* 无操作 */
}

static inline void arch_data_barrier(void)
{
	/* 无操作 */
}

#endif /* _ARCH_C28X_KERNEL_ARCH_FUNC_H_ */