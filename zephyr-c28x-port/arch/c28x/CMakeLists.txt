# C28x 架构构建系统
#
# Copyright (C) 2026 WWWSKY999. All rights reserved.

# 架构头文件包含路径
zephyr_include_directories(
    include
)

# 架构核心源文件
zephyr_sources(
    core/thread.c
)