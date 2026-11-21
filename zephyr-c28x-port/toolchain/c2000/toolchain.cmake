# C2000 工具链配置
#
# Copyright (C) 2026 WWWSKY999. All rights reserved.

# 编译器路径
set(CMAKE_C_COMPILER "D:/CCS/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/bin/cl2000.exe")
set(CMAKE_CXX_COMPILER "D:/CCS/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/bin/cl2000.exe")

# 编译器选项
set(CMAKE_C_FLAGS "-g -O2 --abi=eabi -v28 --float_support=fpu32")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

# 链接器脚本
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--entry=Reset_Handler")

# 目标架构
set(CMAKE_SYSTEM_NAME "C28x")
set(CMAKE_SYSTEM_PROCESSOR "c28x")

# 系统根目录
set(CMAKE_SYSTEM_ROOT "${CMAKE_SOURCE_DIR}")

# 系统启动地址
set(CMAKE_SYSTEM_START_ADDRESS "0x20000000")

# 存储器映射
set(CMAKE_SYSTEM_MEMORY {
    "Flash": "0x00000000-0x1FFFFFFF",
    "RAM": "0x20000000-0x2007FFFF",
    "Peripherals": "0x40000000-0x5FFFFFFF"
})