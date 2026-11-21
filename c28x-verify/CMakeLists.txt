cmake_minimum_required(VERSION 3.20)
project(c28x_min_kernel C)

# 使用 C2000 (C28x) 工具链
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake")

set(CGT_C2000_DIR "D:/ti/ccs2010/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS")

# 链接器命令文件
set(LINKER_CMD "${CMAKE_CURRENT_SOURCE_DIR}/linker.cmd")

# 生成可执行文件
add_executable(app
    src/kernel_core.c
)

# 头文件路径
target_include_directories(app PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../zephyr-c28x-port/arch/c28x/include"
)

# cl2000 编译选项
target_compile_options(app PRIVATE
    -v28 --abi=eabi --float_support=fpu32 --c99
)

# 自定义链接规则：使用 lnk2000 链接
set(CMAKE_C_LINK_EXECUTABLE
    "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> \
     <OBJECTS> -o <TARGET> ${LINKER_CMD} ${CGT_C2000_DIR}/lib/libc.a"
)