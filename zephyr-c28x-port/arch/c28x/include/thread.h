/*
 * Copyright (C) 2026 WWWSKY999. All rights reserved.
 *
 * C28x 架构线程定义头文件
 */

#ifndef _ARCH_C28X_THREAD_H_
#define _ARCH_C28X_THREAD_H_

#include <zephyr/arch/c28x/kernel_arch_data.h>

/* 栈对齐：C28x 以 16-bit word 为最小单元，栈按 2 字节对齐即可 */
#define ARCH_STACK_PTR_ALIGN	2
#define ARCH_THREAD_STACK_ALIGN	2

#endif /* _ARCH_C28X_THREAD_H_ */