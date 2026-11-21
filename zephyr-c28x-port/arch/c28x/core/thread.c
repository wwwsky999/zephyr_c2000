/*
 * Copyright (C) 2026 WWWSKY999. All rights reserved.
 *
 * C28x 架构线程栈初始化与上下文切换
 * 基于 cl2000 (EABI) 编译器
 */

#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/arch/c28x/thread.h>
#include <zephyr/arch/c28x/kernel_arch_data.h>
#include <zephyr/toolchain.h>

/*
 * 新线程栈初始化。
 * 返回初始化后的 SP，供切换代码使用。
 * 采用 C28x EABI 调用约定。
 */
void *z_arch_thread_stack_init(void *stack, k_thread_entry_t entry,
			       void *p1, void *p2, void *p3)
{
	char *stk = stack;
	uint16_t *sp;

	/* 栈增长方向：C28x 栈从高地址向低地址增长 (向下) */
	stk = (char *)Z_STACK_PTR_ALIGN(stk + Z_KERNEL_STACK_LEN(0) - 1);
	sp = (uint16_t *)stk;

	/*
	 * 保存被调用者寄存器的上下文字段。
	 * 在栈上预留上下文空间。
	 */
	--sp; *sp = 0;             /* pc 低16位 */
	--sp; *sp = 0;             /* pc 高16位 */
	--sp; *sp = 0;             /* dp */
	--sp; *sp = 0;             /* st1 */
	--sp; *sp = 0;             /* st0 */

	return (void *)sp;
}

/*
 * 上下文切换入口 (由汇编实现真正的保存/恢复)。
 * 在此我们仅提供 C 语言接口占位。
 */
void z_arch_switch(void *switch_to, void **switch_from)
{
	/* 实际切换由汇编 switch.S 实现 */
}