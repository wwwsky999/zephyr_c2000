# C2000 (C28x) 工具链 - 真实编译核查
# 使用 TI CGT 22.6.2.LTS cl2000 (本机实际安装路径)
set(CGT_C2000_DIR "D:/ti/ccs2010/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS")

set(CMAKE_SYSTEM_NAME "C28x")
set(CMAKE_SYSTEM_PROCESSOR "c28x")
set(CMAKE_SYSTEM_VERSION 1)

# TI C2000 编译器
set(CMAKE_C_COMPILER "${CGT_C2000_DIR}/bin/cl2000.exe")
set(CMAKE_CXX_COMPILER "${CGT_C2000_DIR}/bin/cl2000.exe")
set(CMAKE_ASM_COMPILER "${CGT_C2000_DIR}/bin/cl2000.exe")

# cl2000 生成的对象/可执行文件不使用 .exe 后缀
set(CMAKE_EXECUTABLE_SUFFIX "")
set(CMAKE_OBJECT_SUFFIX ".obj")

# cl2000 不支持 CMake 的标准 --version 探测，强制跳过编译器检测
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# 编译选项：C28x 架构 (与 port 的 toolchain.cmake 声明一致)
set(CMAKE_C_FLAGS "-g -O2 -v28 --abi=eabi --float_support=fpu32 --c99")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

# 链接选项：生成 map 文件 (入口点由 linker.cmd 的 ENTRY 指定)
set(CMAKE_EXE_LINKER_FLAGS "--map_file=app.map")

# CGT 自带头文件路径
set(CMAKE_C_STANDARD_INCLUDE_PATH "${CGT_C2000_DIR}/include")
set(CMAKE_CXX_STANDARD_INCLUDE_PATH "${CGT_C2000_DIR}/include")