/*
 * TMS320F2837xD linker command file for lnk2000 — single-binary dual-CPU build.
 *
 * C28x is word-addressed: one address unit == 16 bits.
 * cl2000 with --gen_func_subsections=on emits functions into
 * ".text:<funcname>" subsections; ".text*" catches all of them.
 *
 * Memory map overview (F28379D LaunchPad)
 * ========================================
 *
 * BOTH CPUs compile against this SAME linker command file.  They see
 * the addresses from their OWN perspective:
 *
 *   CPU1 view: 0x080000 → Flash Bank 0  (physical)
 *   CPU2 view: 0x080000 → Flash Bank 1  (physical)
 *   CPU1 RAMLS0: 0x008000 → CPU1's RAMLS0
 *   CPU2 RAMLS0: 0x008000 → CPU2's RAMLS0
 *
 * The burning step writes the SAME hex image to both banks; MemCfg
 * remapping (done by CPU2 Boot ROM) makes each CPU's 0x080000 land
 * in a different physical Flash bank.  This is how "single image"
 * works for F2837xD dual-core.
 *
 * CPU1 RAM (local + global):
 *   RAMM0   0x000000–0x0003FF   1 KB
 *   RAMM1   0x000400–0x0007FF   1 KB
 *   RAMLS0  0x008000–0x009FFF   8 KB   — ramfuncs RUN + .zephyr_abs
 *   RAMLS1  0x00A000–0x00BFFF   8 KB   — heap, .sysmem, .data, .bss
 *   RAMLS2–5 0x00C000–0x013FFF  32 KB
 *   RAMGS0–15 0x014000–0x033FFF  128 KB  — global shared
 *
 * CPU2 RAM (local + global — CPU2 sees its own private LS/GS):
 *   RAMM0   0x000000–0x0003FF
 *   RAMM1   0x000400–0x0007FF
 *   RAMLS0  0x008000–0x009FFF  — ramfuncs RUN (CPU2's own RAMLS0)
 *   RAMLS1  0x00A000–0x00BFFF  — .sysmem, .data, .bss
 *   RAMGS0–2  0x00C000–0x00EFFF
 *   RAMGS14–15 0x01A000–0x01BFFF  — shared (highest GS)
 *
 * Flash (CPU1/CPU2 view of 0x080000):
 *   FLASH0  0x080000–0x09FFFF  128 KB  — program flash (single image)
 *   FLASH1–4 0x0A0000–0x0BFFFF  — reserved in this build
 *
 * .TI.ramfunc — Flash init MUST run from RAM
 * ============================================
 * TI TRM: "Executing Flash control register writes out of OTP/Flash
 * will yield unpredictable results."  The lnk2000 LOAD/RUN section
 * pairs the .TI.ramfunc section at two addresses:
 *
 *   LOAD = FLASH0  → burned into the Flash image
 *   RUN  = RAMLS0  → executed from RAM (both CPUs have their own RAMLS0)
 *
 * lnk2000 emits three boundary symbols:
 *   LOAD_START(_RamfuncsLoadStart) → name after one leading underscore
 *   LOAD_SIZE(_RamfuncsLoadSize)   → same
 *   RUN_START(_RamfuncsRunStart)   → same
 *
 * cl2000 prepends ONE underscore to every C global, so C source
 * declaring `extern uint16_t RamfuncsLoadStart;` asks the linker for
 * `_RamfuncsLoadStart` — exactly the name lnk2000 emits.  This is
 * identical to how C2000Ware device.c does its ramfuncs copy.
 */

/*
 * TMS320F2837xD linker command file for lnk2000 — single-binary dual-CPU build.
 *
 * C28x is WORD-addressed: one address unit == 16 bits.
 * Length values are in 16-bit words, NOT bytes.
 *
 * C28x has a UNIFIED memory map — PAGE 0 and PAGE 1 refer to the SAME
 * physical addresses.  We omit PAGE separation entirely and define every
 * region once with its real origin/length.  The linker places sections
 * where we tell it, regardless of PAGE.
 *
 * Memory map verified against TI C2000Ware F2837xD_FPU_GENERIC_lnk.cmd
 * (device_support/f2837xd + device_support/f28p551x flash examples).
 *
 * Dual-core: BOTH CPUs burn the SAME hex image.  Flash bank selection and
 * RAM ownership are handled by hardware (MemCfg registers) and CPU2 Boot ROM.
 * CPU1 and CPU2 each have their own private RAMLS/RAMD/RAMGS banks — the
 * linker cmd describes the per-CPU view (0x080000 = "my flash bank").
 */

MEMORY
{
    /* ===== M0/M1 — verified against 2837xD_FLASH_lnk_cpu1.cmd =====
     * BOOT_RSVD occupies 0x000002–0x000122 (0x121 = 289 words).
     * We skip BOOT_RSVD and carve M0 from 0x000123 (733 words). */
    RAMM0   : origin = 0x000123, length = 0x0002DD   /* 733 words (~1.5 KB) */
    RAMM1   : origin = 0x000400, length = 0x000400   /* 1024 words (2 KB) — stack */

    /* ===== Local Shared RAM — 6 blocks × 4 KB each, verified =====
     * contiguous from 0x008000 to 0x00BFFF (24 KB total).
     * Each block is exactly 0x0800 words (4 KB). */
    RAMLS0  : origin = 0x008000, length = 0x000800   /* 2048 words — ramfuncs RUN ONLY */
    RAMLS1  : origin = 0x008800, length = 0x000800   /* .sysmem kernel heap */
    RAMLS2  : origin = 0x009000, length = 0x000800   /* .data / .bss */
    RAMLS3  : origin = 0x009800, length = 0x000800   /* .data / .bss */
    RAMLS4  : origin = 0x00A000, length = 0x000800   /* .data / .bss */
    RAMLS5  : origin = 0x00A800, length = 0x000800   /* .data / .bss */

    /* ===== Local Data RAM — 2 blocks × 4 KB each =====
     * 2837xD_FLASH_lnk_cpu1.cmd PAGE0 has RAMD0=0x00B000, PAGE1 has RAMD1=0x00B800 */
    RAMD0   : origin = 0x00B000, length = 0x000800   /* 2048 words — CPU1 data RAM */
    RAMD1   : origin = 0x00B800, length = 0x000800   /* 2048 words — CPU1 data RAM */

    /* ===== Global Shared RAM — 16 blocks × 8 KB each, verified =====
     * 0x00C000–0x01BFFF (128 KB total per CPU, owned by that CPU's MemCfg).
     * Each block is 0x1000 words (8 KB). */
    RAMGS0  : origin = 0x00C000, length = 0x001000   /* 4096 words — .far data */
    RAMGS1  : origin = 0x00D000, length = 0x002000   /* 8192 words — .noinit (merged GS1+GS2, 官方 .ebss 用 GS0+GS1) */
    RAMGS3  : origin = 0x00F000, length = 0x001000
    RAMGS4  : origin = 0x010000, length = 0x001000
    RAMGS5  : origin = 0x011000, length = 0x001000
    RAMGS6  : origin = 0x012000, length = 0x001000
    RAMGS7  : origin = 0x013000, length = 0x001000
    RAMGS8  : origin = 0x014000, length = 0x001000
    RAMGS9  : origin = 0x015000, length = 0x001000
    RAMGS10 : origin = 0x016000, length = 0x001000
    RAMGS11 : origin = 0x017000, length = 0x001000
    RAMGS12 : origin = 0x018000, length = 0x001000
    RAMGS13 : origin = 0x019000, length = 0x001000
    RAMGS14 : origin = 0x01A000, length = 0x001000
    RAMGS15 : origin = 0x01B000, length = 0x001000

    /* ===== Peripheral register frames — from C2000Ware =====
     * F2837xD_Headers_nonBIOS_cpu1.cmd (device_support/f2837xd/headers).
     * F2837xD_GlobalVariableDefs.c places every register structure in a
     * named section; each frame must sit at its fixed hardware base
     * address or every access through CpuTimer0Regs / SciaRegs / ... would
     * read the wrong location. */

    ACCESSPROTECTION : origin = 0x0005F4C0, length = 0x00000040
    ADCA             : origin = 0x00007400, length = 0x00000080
    ADCB             : origin = 0x00007480, length = 0x00000080
    ADCC             : origin = 0x00007500, length = 0x00000080
    ADCD             : origin = 0x00007580, length = 0x00000080
    ADCARESULT       : origin = 0x00000B00, length = 0x00000018
    ADCBRESULT       : origin = 0x00000B20, length = 0x00000018
    ADCCRESULT       : origin = 0x00000B40, length = 0x00000018
    ADCDRESULT       : origin = 0x00000B60, length = 0x00000018
    ANALOGSUBSYS     : origin = 0x0005D180, length = 0x00000048
    CANA             : origin = 0x00048000, length = 0x00000200
    CANB             : origin = 0x0004A000, length = 0x00000200
    CLA1             : origin = 0x00001400, length = 0x00000080
    CLB1DATAEXCH     : origin = 0x00003200, length = 0x00000200
    CLB2DATAEXCH     : origin = 0x00003600, length = 0x00000200
    CLB3DATAEXCH     : origin = 0x00003A00, length = 0x00000200
    CLB4DATAEXCH     : origin = 0x00003E00, length = 0x00000200
    CLB1LOGICCFG     : origin = 0x00003000, length = 0x00000052
    CLB2LOGICCFG     : origin = 0x00003400, length = 0x00000052
    CLB3LOGICCFG     : origin = 0x00003800, length = 0x00000052
    CLB4LOGICCFG     : origin = 0x00003C00, length = 0x00000052
    CLB1LOGICCTRL    : origin = 0x00003100, length = 0x00000040
    CLB2LOGICCTRL    : origin = 0x00003500, length = 0x00000040
    CLB3LOGICCTRL    : origin = 0x00003900, length = 0x00000040
    CLB4LOGICCTRL    : origin = 0x00003D00, length = 0x00000040
    CLBXBAR          : origin = 0x00007A40, length = 0x00000040
    CLKCFG           : origin = 0x0005D200, length = 0x00000032
    CMPSS1           : origin = 0x00005C80, length = 0x00000020
    CMPSS2           : origin = 0x00005CA0, length = 0x00000020
    CMPSS3           : origin = 0x00005CC0, length = 0x00000020
    CMPSS4           : origin = 0x00005CE0, length = 0x00000020
    CMPSS5           : origin = 0x00005D00, length = 0x00000020
    CMPSS6           : origin = 0x00005D20, length = 0x00000020
    CMPSS7           : origin = 0x00005D40, length = 0x00000020
    CMPSS8           : origin = 0x00005D60, length = 0x00000020
    CPUTIMER0        : origin = 0x00000C00, length = 0x00000008
    CPUTIMER1        : origin = 0x00000C08, length = 0x00000008
    CPUTIMER2        : origin = 0x00000C10, length = 0x00000008
    CPUSYS           : origin = 0x0005D300, length = 0x00000082
    DACA             : origin = 0x00005C00, length = 0x00000008
    DACB             : origin = 0x00005C10, length = 0x00000008
    DACC             : origin = 0x00005C20, length = 0x00000008
    DCSMCOMMON       : origin = 0x0005F070, length = 0x00000008
    DCSMZ1           : origin = 0x0005F000, length = 0x00000024
    DCSMZ2           : origin = 0x0005F040, length = 0x00000024
    DEVCFG           : origin = 0x0005D000, length = 0x0000012E
    DMACLASRCSEL     : origin = 0x00007980, length = 0x0000001A
    DMA              : origin = 0x00001000, length = 0x00000200
    ECAP1            : origin = 0x00005000, length = 0x00000020
    ECAP2            : origin = 0x00005020, length = 0x00000020
    ECAP3            : origin = 0x00005040, length = 0x00000020
    ECAP4            : origin = 0x00005060, length = 0x00000020
    ECAP5            : origin = 0x00005080, length = 0x00000020
    ECAP6            : origin = 0x000050A0, length = 0x00000020
    EMIF1CONFIG      : origin = 0x0005F480, length = 0x00000020
    EMIF2CONFIG      : origin = 0x0005F4A0, length = 0x00000020
    EMIF1            : origin = 0x00047000, length = 0x00000070
    EMIF2            : origin = 0x00047800, length = 0x00000070
    EPWM1            : origin = 0x00004000, length = 0x00000100
    EPWM2            : origin = 0x00004100, length = 0x00000100
    EPWM3            : origin = 0x00004200, length = 0x00000100
    EPWM4            : origin = 0x00004300, length = 0x00000100
    EPWM5            : origin = 0x00004400, length = 0x00000100
    EPWM6            : origin = 0x00004500, length = 0x00000100
    EPWM7            : origin = 0x00004600, length = 0x00000100
    EPWM8            : origin = 0x00004700, length = 0x00000100
    EPWM9            : origin = 0x00004800, length = 0x00000100
    EPWM10           : origin = 0x00004900, length = 0x00000100
    EPWM11           : origin = 0x00004A00, length = 0x00000100
    EPWM12           : origin = 0x00004B00, length = 0x00000100
    EPWMXBAR         : origin = 0x00007A00, length = 0x00000040
    EQEP1            : origin = 0x00005100, length = 0x00000022
    EQEP2            : origin = 0x00005140, length = 0x00000022
    EQEP3            : origin = 0x00005180, length = 0x00000022
    FLASH0CTRL       : origin = 0x0005F800, length = 0x00000182
    FLASH0ECC        : origin = 0x0005FB00, length = 0x00000028
    FLASHPUMPSEMAPHORE : origin = 0x00050024, length = 0x00000002
    GPIOCTRL         : origin = 0x00007C00, length = 0x00000180
    GPIODATA         : origin = 0x00007F00, length = 0x00000030
    I2CA             : origin = 0x00007300, length = 0x00000022
    I2CB             : origin = 0x00007340, length = 0x00000022
    INPUTXBAR        : origin = 0x00007900, length = 0x00000020
    IPC              : origin = 0x00050000, length = 0x00000024
    MEMORYERROR      : origin = 0x0005F500, length = 0x00000040
    MEMCFG           : origin = 0x0005F400, length = 0x00000080
    MCBSPA           : origin = 0x00006000, length = 0x00000024
    MCBSPB           : origin = 0x00006040, length = 0x00000024
    NMIINTRUPT       : origin = 0x00007060, length = 0x00000007
    OUTPUTXBAR       : origin = 0x00007A80, length = 0x00000040
    PIECTRL          : origin = 0x00000CE0, length = 0x0000001A
    PIEVECTTABLE     : origin = 0x00000D00, length = 0x00000200
    ROMPREFETCH      : origin = 0x0005E608, length = 0x00000002
    ROMWAITSTATE     : origin = 0x0005F540, length = 0x00000002
    SCIA             : origin = 0x00007200, length = 0x00000010
    SCIB             : origin = 0x00007210, length = 0x00000010
    SCIC             : origin = 0x00007220, length = 0x00000010
    SCID             : origin = 0x00007230, length = 0x00000010
    SDFM1            : origin = 0x00005E00, length = 0x00000080
    SDFM2            : origin = 0x00005E80, length = 0x00000080
    SPIA             : origin = 0x00006100, length = 0x00000010
    SPIB             : origin = 0x00006110, length = 0x00000010
    SPIC             : origin = 0x00006120, length = 0x00000010
    SYNCSOC          : origin = 0x00007940, length = 0x00000006
    UID              : origin = 0x000703C0, length = 0x00000010
    UPP              : origin = 0x00006200, length = 0x00000048
    WD               : origin = 0x00007000, length = 0x0000002B
    XBAR             : origin = 0x00007920, length = 0x00000020
    XINT             : origin = 0x00007070, length = 0x0000000B

    /* ===== Flash — single bank for both CPUs (MemCfg remaps) =====
     * CPU1 sees 0x080000 = Flash Bank 0 (physical 0x080000)
     * CPU2 sees 0x080000 = Flash Bank 1 (physical 0x100000)
     * We simplify: one 128 KB region covering both views (bank 0 = only used).
     * Official splits into FLASHA..FLASHN sectors (0x080002–0x0BE000). */
    FLASH0  : origin = 0x080000, length = 0x020000   /* 131072 words (256 KB) */
}

SECTIONS
{
    /* Init boundary symbols — absolute symbols for Zephyr struct offsets.
     * These are CONST data (GEN_ABSOLUTE_SYM macro creates const char arrays),
     * so they can live in Flash with the rest of the code.  No need for RAM! */
    .zephyr_abs : > FLASH0, ALIGN(8)

    /* ===== Runtime code + const data (execute from Flash) ===== */
    .text*       : >> FLASH0, ALIGN(8)
    .const       : >> FLASH0, ALIGN(8)
    .econst      : >> FLASH0, ALIGN(8)
    .switch      : >> FLASH0, ALIGN(8)
    .pinit       : >  FLASH0, ALIGN(8)
    .init_array  : >  FLASH0, ALIGN(8)
    .cinit*      : >  FLASH0, ALIGN(8)   /* TI RTS auto-init tables */

    /* ===== ramfuncs — Flash init MUST run from RAM =====
     *
     * .TI.ramfunc is where cl2000 puts __attribute__((section(".TI.ramfunc")))
     * functions.  We give it a LOAD address in FLASH0 and a RUN address in
     * RAMLS0.  lnk2000 will emit the section at both places: a copy in
     * Flash (burned to the hex image) and a run address in RAM (jumped to
     * by prep_c.c after memcpy).
     *
     * LOAD_START/RUN_START directives tell lnk2000 to create boundary
     * symbols at the run/load origins with the specified names.  We use
     * ONE leading underscore because that's exactly what cl2000 emits
     * for C globals declared without underscores (TI naming rule). */
    .TI.ramfunc : {} LOAD = FLASH0,
                     RUN  = RAMLS0,
                     LOAD_START(_RamfuncsLoadStart),
                     LOAD_SIZE(_RamfuncsLoadSize),
                     LOAD_END(_RamfuncsLoadEnd),
                     RUN_START(_RamfuncsRunStart),
                     RUN_SIZE(_RamfuncsRunSize),
                     RUN_END(_RamfuncsRunEnd),
                     ALIGN(8)

    /* ===== Writable data — RAM ===== */
    .data*       : >> RAMLS2 | RAMLS3 | RAMLS4 | RAMLS5
    .bss*        : >> RAMLS2 | RAMLS3 | RAMLS4 | RAMLS5
    .ebss*       : >> RAMLS2 | RAMLS3 | RAMLS4 | RAMLS5
    .sysmem      : >  RAMLS1   /* kernel heap — RAMLS1 (4KB) */
    .esysmem     : >  RAMLS1
    .far         : >  RAMGS0   /* large (far pointer) data */

    /* No-init — survives warm reset (I/O, mailbox buffers) */
    .noinit*     : >> RAMGS1

    /* Stack */
    .stack       : >  RAMM1,  align(2)

    /* ===== Peripheral register structures (F2837xD_GlobalVariableDefs.c) =====
     * Named sections from #pragma DATA_SECTION; NOINIT, placed at the
     * hardware base addresses of the frames above.  Without placement the
     * linker would allocate them wherever it likes and every register
     * access would target the wrong bus address. */
    PieVectTableFile             : > PIEVECTTABLE, type = NOINIT
    PieCtrlRegsFile              : > PIECTRL,      type = NOINIT
    CpuTimer0RegsFile            : > CPUTIMER0,    type = NOINIT
    CpuTimer1RegsFile            : > CPUTIMER1,    type = NOINIT
    CpuTimer2RegsFile            : > CPUTIMER2,    type = NOINIT
    ClkCfgRegsFile               : > CLKCFG,       type = NOINIT
    CpuSysRegsFile               : > CPUSYS,       type = NOINIT
    DevCfgRegsFile               : > DEVCFG,       type = NOINIT
    GpioCtrlRegsFile             : > GPIOCTRL,     type = NOINIT
    GpioDataRegsFile             : > GPIODATA,     type = NOINIT
    SciaRegsFile                 : > SCIA,         type = NOINIT
    ScibRegsFile                 : > SCIB,         type = NOINIT
    ScicRegsFile                 : > SCIC,         type = NOINIT
    ScidRegsFile                 : > SCID,         type = NOINIT
    WdRegsFile                   : > WD,           type = NOINIT
    AccessProtectionRegsFile     : > ACCESSPROTECTION, type = NOINIT
    AdcaRegsFile                 : > ADCA,         type = NOINIT
    AdcbRegsFile                 : > ADCB,         type = NOINIT
    AdccRegsFile                 : > ADCC,         type = NOINIT
    AdcdRegsFile                 : > ADCD,         type = NOINIT
    AdcaResultRegsFile           : > ADCARESULT,   type = NOINIT
    AdcbResultRegsFile           : > ADCBRESULT,   type = NOINIT
    AdccResultRegsFile           : > ADCCRESULT,   type = NOINIT
    AdcdResultRegsFile           : > ADCDRESULT,   type = NOINIT
    AnalogSubsysRegsFile         : > ANALOGSUBSYS, type = NOINIT
    CanaRegsFile                 : > CANA,         type = NOINIT
    CanbRegsFile                 : > CANB,         type = NOINIT
    Cla1RegsFile                 : > CLA1,         type = NOINIT
    Clb1DataExchRegsFile         : > CLB1DATAEXCH, type = NOINIT
    Clb2DataExchRegsFile         : > CLB2DATAEXCH, type = NOINIT
    Clb3DataExchRegsFile         : > CLB3DATAEXCH, type = NOINIT
    Clb4DataExchRegsFile         : > CLB4DATAEXCH, type = NOINIT
    Clb1LogicCfgRegsFile         : > CLB1LOGICCFG, type = NOINIT
    Clb2LogicCfgRegsFile         : > CLB2LOGICCFG, type = NOINIT
    Clb3LogicCfgRegsFile         : > CLB3LOGICCFG, type = NOINIT
    Clb4LogicCfgRegsFile         : > CLB4LOGICCFG, type = NOINIT
    Clb1LogicCtrlRegsFile        : > CLB1LOGICCTRL, type = NOINIT
    Clb2LogicCtrlRegsFile        : > CLB2LOGICCTRL, type = NOINIT
    Clb3LogicCtrlRegsFile        : > CLB3LOGICCTRL, type = NOINIT
    Clb4LogicCtrlRegsFile        : > CLB4LOGICCTRL, type = NOINIT
    ClbXbarRegsFile              : > CLBXBAR,      type = NOINIT
    Cmpss1RegsFile               : > CMPSS1,       type = NOINIT
    Cmpss2RegsFile               : > CMPSS2,       type = NOINIT
    Cmpss3RegsFile               : > CMPSS3,       type = NOINIT
    Cmpss4RegsFile               : > CMPSS4,       type = NOINIT
    Cmpss5RegsFile               : > CMPSS5,       type = NOINIT
    Cmpss6RegsFile               : > CMPSS6,       type = NOINIT
    Cmpss7RegsFile               : > CMPSS7,       type = NOINIT
    Cmpss8RegsFile               : > CMPSS8,       type = NOINIT
    DacaRegsFile                 : > DACA,         type = NOINIT
    DacbRegsFile                 : > DACB,         type = NOINIT
    DaccRegsFile                 : > DACC,         type = NOINIT
    DcsmCommonRegsFile           : > DCSMCOMMON,   type = NOINIT
    DcsmZ1RegsFile               : > DCSMZ1,       type = NOINIT
    DcsmZ2RegsFile               : > DCSMZ2,       type = NOINIT
    DmaClaSrcSelRegsFile         : > DMACLASRCSEL, type = NOINIT
    DmaRegsFile                  : > DMA,          type = NOINIT
    ECap1RegsFile                : > ECAP1,        type = NOINIT
    ECap2RegsFile                : > ECAP2,        type = NOINIT
    ECap3RegsFile                : > ECAP3,        type = NOINIT
    ECap4RegsFile                : > ECAP4,        type = NOINIT
    ECap5RegsFile                : > ECAP5,        type = NOINIT
    ECap6RegsFile                : > ECAP6,        type = NOINIT
    Emif1ConfigRegsFile          : > EMIF1CONFIG,  type = NOINIT
    Emif2ConfigRegsFile          : > EMIF2CONFIG,  type = NOINIT
    Emif1RegsFile                : > EMIF1,        type = NOINIT
    Emif2RegsFile                : > EMIF2,        type = NOINIT
    EPwm1RegsFile                : > EPWM1,        type = NOINIT
    EPwm2RegsFile                : > EPWM2,        type = NOINIT
    EPwm3RegsFile                : > EPWM3,        type = NOINIT
    EPwm4RegsFile                : > EPWM4,        type = NOINIT
    EPwm5RegsFile                : > EPWM5,        type = NOINIT
    EPwm6RegsFile                : > EPWM6,        type = NOINIT
    EPwm7RegsFile                : > EPWM7,        type = NOINIT
    EPwm8RegsFile                : > EPWM8,        type = NOINIT
    EPwm9RegsFile                : > EPWM9,        type = NOINIT
    EPwm10RegsFile               : > EPWM10,       type = NOINIT
    EPwm11RegsFile               : > EPWM11,       type = NOINIT
    EPwm12RegsFile               : > EPWM12,       type = NOINIT
    EPwmXbarRegsFile             : > EPWMXBAR,     type = NOINIT
    EQep1RegsFile                : > EQEP1,        type = NOINIT
    EQep2RegsFile                : > EQEP2,        type = NOINIT
    EQep3RegsFile                : > EQEP3,        type = NOINIT
    Flash0CtrlRegsFile           : > FLASH0CTRL,   type = NOINIT
    Flash0EccRegsFile            : > FLASH0ECC,    type = NOINIT
    FlashPumpSemaphoreRegsFile   : > FLASHPUMPSEMAPHORE, type = NOINIT
    I2caRegsFile                 : > I2CA,         type = NOINIT
    I2cbRegsFile                 : > I2CB,         type = NOINIT
    InputXbarRegsFile            : > INPUTXBAR,    type = NOINIT
    IpcRegsFile                  : > IPC,          type = NOINIT
    MemoryErrorRegsFile          : > MEMORYERROR,  type = NOINIT
    MemCfgRegsFile               : > MEMCFG,       type = NOINIT
    McbspaRegsFile               : > MCBSPA,       type = NOINIT
    McbspbRegsFile               : > MCBSPB,       type = NOINIT
    NmiIntruptRegsFile           : > NMIINTRUPT,   type = NOINIT
    OutputXbarRegsFile           : > OUTPUTXBAR,   type = NOINIT
    RomPrefetchRegsFile          : > ROMPREFETCH,  type = NOINIT
    RomWaitStateRegsFile         : > ROMWAITSTATE, type = NOINIT
    Sdfm1RegsFile                : > SDFM1,        type = NOINIT
    Sdfm2RegsFile                : > SDFM2,        type = NOINIT
    SpiaRegsFile                 : > SPIA,         type = NOINIT
    SpibRegsFile                 : > SPIB,         type = NOINIT
    SpicRegsFile                 : > SPIC,         type = NOINIT
    SyncSocRegsFile              : > SYNCSOC,      type = NOINIT
    UidRegsFile                  : > UID,          type = NOINIT
    UppRegsFile                  : > UPP,          type = NOINIT
    XbarRegsFile                 : > XBAR,         type = NOINIT
    XintRegsFile                 : > XINT,         type = NOINIT
}
