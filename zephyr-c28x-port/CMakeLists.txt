# Zephyr C28x 移植项目
#
# Copyright (C) 2026 WWWSKY999. All rights reserved.

cmake_minimum_required(VERSION 3.5)

# 项目名称
project(zephyr-c28x-port)

# 添加子目录
add_subdirectory(arch/c28x)
add_subdirectory(soc/ti/tms320f2837x)
add_subdirectory(boards/ti/f28377d)
add_subdirectory(toolchain/c2000)

# 默认构建目标
add_executable(zephyr main.c)

# 设置系统信息
set(CMAKE_SYSTEM_NAME "C28x")
set(CMAKE_SYSTEM_PROCESSOR "c28x")

# 系统启动地址
set(CMAKE_SYSTEM_START_ADDRESS "0x20000000")

# 存储器映射
set(CMAKE_SYSTEM_MEMORY {
    "Flash": "0x00000000-0x1FFFFFFF",
    "RAM": "0x20000000-0x2007FFFF",
    "Peripherals": "0x40000000-0x5FFFFFFF"
})