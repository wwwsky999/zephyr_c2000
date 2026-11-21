# TI F28377D 开发板支持
#
# Copyright (C) 2026 WWWSKY999. All rights reserved.

cmake_minimum_required(VERSION 3.5)

# 板级配置
set(CMAKE_SYSTEM_NAME "C28x")
set(CMAKE_SYSTEM_PROCESSOR "c28x")

# 系统控制寄存器基地址
set(SYSCTRL_BASE 0x1F800000)

# 系统时钟频率
set(SYSCLK_FREQ 150000000)

# 存储器配置
set(MEMORY_MAP {
    "0x00000000": "Flash (0x00000000-0x1FFFFFFF, 32M)",
    "0x20000000": "RAM (0x20000000-0x2007FFFF, 128K)",
    "0x20080000": "RAM (0x20080000-0x200FFFFF, 128K)",
    "0x40000000": "外设 (0x40000000-0x5FFFFFFF, 1.5M)"
})

# 系统启动地址
set(BOOT_ADDR 0x20000000)

# 下载器配置 (示例)
set(DOWNLOADER "ccs")

# 系统初始化脚本
set(STARTUP_SCRIPT "startup.s")

# 标准板级配置
include(board_common.cmake)