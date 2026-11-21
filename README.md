# Zephyr RTOS C2000 (C28x) 移植

将 Zephyr RTOS 4.5 移植到 TI C2000 系列 DSP（TMS320C28x 内核），使用 TI CGT `cl2000` 工具链（ti-cgt-c2000_22.6.2.LTS）真实编译验证，目标板为 LAUNCHXL-F28379D / F28377D。

核心工作：

- 新增 `c2000` 架构层（线程切换、中断、fatal/panic、prep_c、offsets），适配 C28x 特性：16-bit `int` / 32-bit `long`、eABI、硬件 FPU32、ePIE 向量表、TI-COFF/ELF 双格式
- 新增 `ti-cgt-c2000` 工具链集成（cmake compiler/linker/bintools）
- 新增 F2837xD HAL（cputimer、PIE、SCI、GPIO、sysctl）与 C2000Ware 头文件接入
- 新增串口（SCI）与 CPU Timer 驱动、launchxl_f28379d / f28377d 板级定义与 devicetree
- 自动 patch 脚本处理上游不兼容点（`util.h` LOG2 常量表达式、posix_types、cbprintf 内联汇编、gen_offset_header COFF 支持等）

## 仓库结构

```
├── zephyr-c28x-port/      # C28x 移植层源码（arch/boards/soc/toolchain 骨架）
├── zephyr-c2000-env/      # 预置环境：Zephyr 4.5 源码树 + 待安装的完整移植包
│   └── zephyr-c2000-staged/
│       ├── port/          # arch/c2000、soc、boards、dts、cmake、drivers
│       ├── hal/f2837xd/   # F2837xD 片上外设 HAL
│       ├── third_party/c2000ware/
│       └── scripts/install-into-zephyr.sh
├── c28x-verify/           # 最小内核核心独立编译核查工程
└── doc/reqmgmt/           # strictdoc 需求管理文档
```

## 使用方法

### 方法一：安装到 Zephyr 源码树，west 全量构建

把移植包安装进 Zephyr 4.5 源码树，再用 west 构建完整固件（hello_world 已验证通过）。

```bash
# 1. 准备 Zephyr 4.5 源码树（仓库已附带 zephyr-c2000-env/zephyr，也可自行 checkout）
cd zephyr-c2000-env

# 2. 安装移植层：拷贝 arch/include/soc/boards/dts/cmake/HAL/驱动，
#    并自动 patch arch/Kconfig、archs.yml、toolchain.h、util.h 等发现点
./zephyr-c2000-staged/scripts/install-into-zephyr.sh ./zephyr

# 3. 指向 TI CGT 工具链并构建
export TI_C2000_CGT_PATH=/path/to/ti-cgt-c2000_22.6.2.LTS
west build -b launchxl_f28379d -DZEPHYR_TOOLCHAIN_VARIANT=ti-cgt-c2000 \
           zephyr/samples/hello_world
```

说明：

- 脚本幂等，可重复执行；只修改少数发现点（`arch/Kconfig`、`archs.yml`、`include/zephyr/arch/cpu.h`、`drivers/{serial,timer}/{Kconfig,CMakeLists.txt}` 等）
- devicetree 预处理依赖主机 gcc（DTS 前端），Windows 下使用 MinGW gcc 即可
- 构建产物为 `zephyr.elf` / `zephyr.bin`，reset 向量入口由 linker 脚本指定

### 方法二：最小内核核心独立编译核查（c28x-verify）

不依赖完整 Zephyr 构建系统，直接用 `cl2000` 编译链接内核核心源码，快速核查移植层在 C28x ABI 下能否真实通过编译（区别于静态检查）。

```bash
cd c28x-verify

# 1. 修改 CMakeLists.txt / toolchain.cmake 中的 CGT_C2000_DIR 为本机 TI CGT 路径
#    默认: D:/ti/ccs2010/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS

# 2. 配置 + 构建（Unix Makefiles，含自定义 linker.cmd 与 libc.a 链接）
cmake -B build -G "Unix Makefiles"
cmake --build build
```

产出 `kernel_core.out` / `.map`：以 `-v28 --abi=eabi --float_support=fpu32 --c99` 编译 `src/kernel_core.c`（内核核心代码 + `zephyr-c28x-port/arch/c28x/include` 头文件），验证线程/内核结构体定义在 C28x 上可编译、可链接。

## 环境要求

| 组件 | 版本 |
|------|------|
| TI Code Composer Studio | 20.x（含 ti-cgt-c2000 22.6.2.LTS） |
| CMake | ≥ 3.20（全量构建路径 ≥ 3.5，实测 4.2.1） |
| west | ≥ 0.14 |
| Python | 3.x（patch 脚本、DTS/offsets 工具） |
| 主机 gcc | 任意较新版本（仅作 DTS 预处理器） |

## 当前状态

- [x] hello_world 在 f28377d/tms320f28377d 上用 ti-cgt-c2000 工具链全量构建通过（80 个编译步骤，产出 zephyr.elf/zephyr.bin）
- [x] 内核核心源码 cl2000 独立编译核查通过
- [x] F2837xD HAL、SCI 串口、CPU Timer 驱动接入
- [ ] 板级上板运行验证（stage 2，见 `zephyr-c2000-env/zephyr-c2000-staged/port/PORTING.md`）

## License

移植层新增文件遵循 Zephyr 项目惯例（Apache-2.0），第三方组件（Zephyr、C2000Ware、TI CGT）各自保留原许可。
