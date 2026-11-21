/*
 * F28377D 最小可启动内核 - 链接器命令文件
 * 内存映射依据 port 的 toolchain.cmake / board.cmake 声明：
 *   Flash : 0x00000000 - 0x1FFFFFFF
 *   RAM   : 0x20000000 - 0x2007FFFF (128K)
 *   RAM   : 0x20080000 - 0x200FFFFF (128K)
 * 外设   : 0x40000000 - 0x5FFFFFFF
 * 说明：本文件用于验证 cl2000/lnk2000 真实链接可行性，地址按 port 声明配置。
 */

MEMORY
{
    /* 程序代码区 (Flash) */
    FLASH    : origin = 0x080000, length = 0x00100000

    /* 数据 RAM 区 (F2837x M0/M1 + LS + GS) */
    RAM_M0   : origin = 0x000000, length = 0x000800
    RAM_M1   : origin = 0x000800, length = 0x000800
    RAMLS    : origin = 0x008000, length = 0x00020000
    RAMGS    : origin = 0x20000000, length = 0x00080000
}

SECTIONS
{
    /* 复位与入口代码 */
    .reset      : > FLASH,   ALIGN(2)

    /* 代码 */
    .text       : > FLASH,   ALIGN(2)

    /* 已初始化数据 */
    .data       : > RAMLS,   ALIGN(2)
    .switch     : > RAMLS,   ALIGN(2)

    /* 未初始化数据 */
    .bss        : > RAMLS,   ALIGN(2)
    .sysmem     : > RAMLS,   ALIGN(2)

    /* 栈 */
    .stack      : > RAMLS,   ALIGN(2)

    /* CIO 重定向 */
    .cio        : > RAMLS,   ALIGN(2)
}

/* C 运行时入口由命令行 -e _c_int00 指定 */