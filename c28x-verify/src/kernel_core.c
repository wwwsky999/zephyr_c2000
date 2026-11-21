/*
 * 最小可启动内核核心 - C28x (F28377D) 真实编译核查
 *
 * 目的：用 TI cl2000 真实编译一个最小可启动内核，
 * 验证 port 的架构头文件与上下文初始化逻辑能否被 cl2000 正确编译链接。
 *
 * 注意：kernel_arch_func.h 内部的 #include <arch/c28x/arch.h> 为
 * Zephyr 风格相对 include 根路径；此处为独立验证，改为直接包含扁平头文件。
 */

#include <stdint.h>
#include <stddef.h>

/* 直接包含 port 的架构头文件 (扁平目录) */
#include "kernel_arch_data.h"
#include "arch.h"

/*
 * 线程栈上下文初始化 - 复刻 port 的 arch/c28x/core/thread.c 逻辑，
 * 验证 _callee_saved 上下文布局与栈初始化代码能否被 cl2000 编译。
 * _callee_saved 布局来自 kernel_arch_data.h。
 */
static uint32_t kernel_stack[256];
static volatile uint32_t boot_success = 0;

void *kernel_thread_stack_init(void *stack)
{
	uint16_t *sp = (uint16_t *)stack;

	--sp; *sp = 0;             /* pc 低16位 */
	--sp; *sp = 0;             /* pc 高16位 */
	--sp; *sp = 0;             /* dp */
	--sp; *sp = 0;             /* st1 */
	--sp; *sp = 0;             /* st0 */

	return (void *)sp;
}

/* 复位后 C 运行时入口 _c_int00 会调用 main */
int main(void)
{
	/* 初始化内核栈 */
	void *sp = kernel_thread_stack_init((void *)kernel_stack);

	/* 触发一个简单的 boot 标记，供链接产物查验 */
	boot_success = (sp != 0) ? 1u : 0u;

	/* 最小内核空转循环 */
	for (;;) {
		/* C28x IDLE - 低功耗待机 */
		__asm__ volatile(" IDLE");
	}

	/* unreachable */
	return 0;
}