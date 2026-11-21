/*
 * Copyright (C) 2026 WWWSKY999. All rights reserved.
 * 
 * TI C2000 (C28x) 架构基础定义
 * 注意：C28x 架构中 char, int, short 均为 16 位 (1 word)。
 * 没有任何原生的 8 位类型。
 */

#ifndef _ARCH_C28X_H_
#define _ARCH_C28X_H_

#include <stdint.h>
#include <stddef.h>

/* 架构基础定义 */
#define ARCH_C28X_WORD_SIZE      2   /* 16-bit word */
#define ARCH_C28X_ADDR_SIZE     4   /* 32-bit address (22-bit used) */

/* 
 * 类型陷阱处理：
 * 在 C28x 上，sizeof(char) == 1, 但实际上这个 '1' 代表 1 个 word (16 bits)。
 * 为了兼容 Zephyr 内核中对 '字节' 的定义，我们定义一个宏用于在处理 8-bit 数据时强制掩码。
 */
#define C28X_U8_MASK            0x00FF
#define C28X_S8_MASK            0xFF80 /* 符号位在最高位 */

/* 对齐定义 */
#define ARCH_C28X_ALIGNMENT      2

/* 
 * 基础类型别名 (用于在架构代码中明确表达意图)
 * 实际上 cl2000 的 uint8_t 就是 unsigned short
 */
typedef uint16_t arch_u8_t; 
typedef uint16_t arch_u16_t;
typedef uint32_t arch_u32_t;

#endif /* _ARCH_C28X_H_ */
