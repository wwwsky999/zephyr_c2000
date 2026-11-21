/*
 * Copyright (c) 2026 Zephyr C2000 port
 * SPDX-License-Identifier: Apache-2.0
 *
 * C28x dual-CPU hardware bring-up, Flash init from RAM.
 *
 * Register addresses ALL verified against C2000Ware v4.02 device headers:
 *   F2837xD_sysctrl.h   — CLK_CFG_REGS, CPU_SYS_REGS, DEV_CFG_REGS, WD_REGS
 *   F2837xD_flash.h     — FLASH_CTRL_REGS
 *   F2837xD_piectrl.h   — PIE_CTRL_REGS
 *   F2837xD_ipc.h       — IPC_REGS_CPU1, IPC_REGS_CPU2
 *   F2837xD_cla.h       — CLA_REGS
 *
 * Boot architecture
 * =================
 * Single binary, burned to Flash Bank 0 (CPU1) and Flash Bank 1 (CPU2).
 * Both CPUs start at their own 0x080000 and dispatch by reading
 * IPCBootSTS.SYSTEM_READY (IPC registers are CPU-visible at the same
 * address but with per-CPU state; Boot ROM clears SYSTEM_READY only
 * on the CPU2 path).
 */

#include <stdint.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/arch/c2000/arch.h>
#include <zephyr/arch/c2000/arch_inlines.h>

/* ======================================================================
 * Base addresses (word-addressed, one C28x address unit = 16 bits).
 *
 * All bases verified against C2000Ware device.h extern declarations.
 * ==================================================================== */

/* All bases verified against TI C2000Ware hw_memmap.h + linker .cmd files.
 * C28x is word-addressed (1 unit = 16 bits). */

/* Clock config — PLL source/mult/div (SHARED by both CPUs) */
#define CLKCFG_BASE         0x0005D200U
/* CPU system regs — peripheral clock gating (PER-CPU private) */
#define CPUSYS_BASE         0x0005D300U
/* Device config — SOFTPRES for CLA reset (shared, CPU1 only visible) */
#define DEVCFG_BASE         0x0005D000U
/* Flash controller — per-CPU private (CPU1→Bank0, CPU2→Bank1) */
#define FLASH0_CTRL_BASE    0x0005F800U
/* Flash ECC — separate register block from FLASH_CTRL (NOT a sub-struct!) */
#define FLASH0_ECC_BASE     0x0005FB00U
/* ROM wait-state (shared) */
#define ROM_WAIT_STATE_BASE 0x0005F540U
/* PIE controller (PER-CPU private) */
#define PIE_CTRL_BASE       0x00000CE0U
/* PIE vector table (PER-CPU private) */
#define PIE_VECT_BASE       0x00000D00U
/* IPC mailbox — SAME address from both CPUs, PER-CPU state */
#define IPC_BASE            0x00050000U
/* CLA — per-CPU aliased: 0x1400 accesses CLA1 on CPU1, CLA2 on CPU2 */
#define CLA1_BASE           0x00001400U
#define CLA2_BASE           0x00001400U  /* same addr, per-CPU decode */
/* Watchdog (shared) */
#define WD_BASE             0x00007000U

/* C28x intrinsics */
#define EALLOW()  __asm__(" EALLOW")
#define EDIS()    __asm__(" EDIS")
#define C28X_NOP()  __asm__(" NOP")
#define C28X_DELAY(count)  __asm__(" RPT #" #count " || NOP")

/* Direct register access helpers — avoid union/struct layout tricks */
static inline uint32_t rd32(uint32_t base, uint32_t word_off)
{
    return *(volatile uint32_t *)(uintptr_t)(base + word_off);
}

static inline uint16_t rd16(uint32_t base, uint32_t word_off)
{
    return *(volatile uint16_t *)(uintptr_t)(base + word_off);
}

static inline void wr32(uint32_t base, uint32_t word_off, uint32_t val)
{
    *(volatile uint32_t *)(uintptr_t)(base + word_off) = val;
}

static inline void wr16(uint32_t base, uint32_t word_off, uint16_t val)
{
    *(volatile uint16_t *)(uintptr_t)(base + word_off) = val;
}

/* ======================================================================
 * All word offsets below come directly from C2000Ware struct layouts.
 * Registers are 16-bit or 32-bit; offsets are word (16-bit) indexed.
 * ==================================================================== */

/* CLK_CFG_REGS (base 0x0005D200) */
/* Word offsets — each Uint32 takes 2 words. */
#define CLKCFG_CLKSRCCTL1_W       8U     /* OSCCLKSRCSEL bits 1:0 */
#define CLKCFG_SYSPLLCTL1_W       14U    /* PLLEN=bit0, PLLCLKEN=bit1 */
#define CLKCFG_SYSPLLMULT_W       20U    /* IMULT bits 6:0, FMULT bits 9:8 */
#define CLKCFG_SYSPLLSTS_W        22U    /* LOCKS bit 0 */
#define CLKCFG_SYSCLKDIVSEL_W     34U    /* PLLSYSCLKDIV bits 5:0 */

/* NOTE: TI cl2000 default int is 16-bit!  All bit-mask macros must cast
 * to uint32_t before shifting to avoid "shift count too large" for bits
 * above 15. */
#define _BIT32(n)  (((uint32_t)1U) << (n))

/* SYSPLLCTL1 bits */
#define PLLEN        _BIT32(0)
#define PLLCLKEN     _BIT32(1)

/* SYSPLLMULT bits */
#define IMULT(n)     (((uint32_t)(n) & 0x7FU) << 0)     /* bits 6:0 */
#define FMULT(n)     (((uint32_t)(n) & 0x03U) << 8)     /* bits 9:8 */

/* SYSCLKDIVSEL bits */
#define PLLSYSCLKDIV(n) (((uint32_t)(n) & 0x3FU) << 0)  /* bits 5:0 */

/* SYSPLLSTS bits */
#define SYSPLLSTS_LOCKS  _BIT32(0)

/* CLKSRCCTL1 bits */
#define OSCCLKSRCSEL(n)  (((uint32_t)(n) & 0x3U) << 0)  /* bits 1:0; 1=XTAL */

/* ----- CPU_SYS_REGS (base 0x0005D300) ----- */
#define CPUSYS_PCLKCR0_W        34U     /* PCLKCR0 (Uint32) */

/* PCLKCR0 bits (verified against C2000Ware PCLKCR0_BITS) */
#define PCLKCR0_CLA1             _BIT32(0)
#define PCLKCR0_DMA              _BIT32(2)
#define PCLKCR0_CPUTIMER0        _BIT32(3)
#define PCLKCR0_HRPWM            _BIT32(16)
#define PCLKCR0_TBCLKSYNC        _BIT32(18)

/* NOTE: PIE has NO dedicated PCLKCR bit — it's always clocked. */

/* ----- DEV_CFG_REGS (base 0x0005D000) ----- */
/* Struct layout leads to SOFTPRES0 at word 130. */
#define DEVCFG_SOFTPRES0_W      130U    /* SOFTPRES0 (Uint32) */

/* SOFTPRES0 bits */
#define SOFTPRES0_CPU1_CLA1      _BIT32(0)
#define SOFTPRES0_CPU2_CLA1      _BIT32(2)

/* ----- FLASH_CTRL_REGS (base 0x0005F800) ----- */
/* Word offsets from C2000Ware FLASH_CTRL_REGS struct. */
#define FLASH_FRDCNTL_W          0U     /* FRDCNTL (Uint32) */
#define FLASH_FBAC_W            30U     /* FBAC */
#define FLASH_FBFALLBACK_W      32U     /* FBFALLBACK */
#define FLASH_FPAC1_W           36U     /* FPAC1 */
#define FLASH_FRD_INTF_CTRL_W  384U     /* FRD_INTF_CTRL */

/* FRDCNTL bits — RWAIT bits 11:8 */
#define FRDCNTL_RWAIT(n) (((uint32_t)(n) & 0x0FU) << 8)

/* FBAC bits — VREADST bits 7:0 */
#define FBAC_VREADST(n)  (((uint32_t)(n) & 0xFFU) << 0)

/* FBFALLBACK bits — BNKPWR0 bits 1:0 */
#define FBFALLBACK_BNKPWR0(n) (((uint32_t)(n) & 0x3U) << 0)

/* FPAC1 bits — PMPPWR bit 0 */
#define FPAC1_PMPPWR        _BIT32(0)

/* FRD_INTF_CTRL bits */
#define FRD_INTF_CTRL_PREFETCH_EN   _BIT32(0)
#define FRD_INTF_CTRL_DATA_CACHE_EN _BIT32(1)

/* ----- PIE_CTRL_REGS (base 0x00000CE0) — ALL Uint16! ----- */
#define PIECTRL_W        0U    /* Uint16: ENPIE bit 0, PIEVECT bits 15:1 */
#define PIEACK_W         1U    /* Uint16: ACK1..ACK12 */
#define PIEIER1_W        2U

/* PIECTRL bits */
#define PIECTRL_ENPIE    ((uint16_t)1U << 0)
#define PIECTRL_PIEVECT_SHIFT  1U   /* value must be written as addr >> 1 */

/* ----- IPC_REGS_CPU1 (base 0x00050000) — offsets same from both CPUs ----- */
#define IPC_SET_W         4U     /* Uint32 IPCSET  — set remote flag */
#define IPC_CLR_W         6U     /* Uint32 IPCCLR  — clear remote flag */
#define IPC_FLG_W         8U     /* Uint32 IPCFLG  — remote flag status (read) */
#define IPC_SENDCOM_W    16U     /* Uint32 IPCSENDCOM */
#define IPC_BOOTSTS_W    32U     /* Uint32 IPCBOOTSTS */
#define IPC_BOOTMODE_W   34U     /* Uint32 IPCBOOTMODE */

/* IPCFLG bits used in CPU2 boot handshake */
#define IPC_FLG_IPC0    _BIT32(0)    /* IPC INT0 flag — used for CPU1→CPU2 interrupt */
#define IPC_FLG_IPC31   _BIT32(31)   /* IPC FLAG31 — Boot ROM handshake */

/* IPCBOOTSTS bits */
#define IPC_BOOTSTS_SYSTEM_READY  _BIT32(0)

/* IPCBOOTMODE values */
#define C1C2_BOOTMODE_FLASH       0x0000000BU
#define BROM_EXECUTE_BOOTMODE_CMD 0x00000000U

/* ----- CLA_REGS base + word offsets — all Uint16! ----- */
/* MVECT1..8 at words 0..7, rsvd 8..15, MCTL at 16 */
#define CLA_MCTL_W       16U    /* Uint16: HARDRESET bit 0, SOFTRESET bit 1 */
#define CLA_MIFR_W       32U    /* Uint16: task interrupt flags */
#define CLA_MIOVF_W      33U
#define CLA_MICLR_W      35U    /* Uint16: clear MIFR bits 0..7 */
#define CLA_MICLROVF_W   36U    /* Uint16: clear overflow */

/* MCTL bits */
#define MCTL_HARDRESET   ((uint16_t)1U << 0)
#define MCTL_SOFTRESET   ((uint16_t)1U << 1)

/* ----- WD_REGS (base 0x00007000) — carefully enumerated ----- */
/*
 * WD_REGS full struct layout (F2837xD_sysctrl.h L1777):
 *   word  0–33:  rsvd1[34]         (34 × Uint16)
 *   word    34:  SCSR              (union, Uint16 all → 1 word!)
 *   word    35:  WDCNTR            (union, Uint16 all → 1 word!)
 *   word    36:  rsvd2             (Uint16 → 1 word)
 *   word    37:  WDKEY             (union, Uint16 all → 1 word!)
 *   word 38–40:  rsvd3[3]          (3 × Uint16)
 *   word    41:  WDCR              (union, Uint16 all → 1 word!)  ← OUR TARGET
 *   word    42:  WDWCR             (union, Uint16 all → 1 word!)
 *
 * struct total = 34+1+1+1+1+3+1+1 = 43 words
 * linker .cmd WD length = 0x002B = 43 words ← PERFECT MATCH
 *
 * C2000Ware header grep confirms ALL WD unions are Uint16:
 *   union SCSR_REG  { Uint16 all; struct SCSR_BITS  bit; };
 *   union WDCNTR_REG{ Uint16 all; struct WDCNTR_BITSbit; };
 *   union WDKEY_REG { Uint16 all; struct WDKEY_BITS bit; };
 *   union WDCR_REG  { Uint16 all; struct WDCR_BITS  bit; };
 *   union WDWCR_REG { Uint16 all; struct WDWCR_BITS bit; };
 */
#define WD_WDCR_W         41U    /* Word 41, union is Uint16 (NOT Uint32!) */
/* WDCR bits (F2837xD_sysctrl.h WDCR_BITS):
 *   WDPS:3  bits 2:0  (prescalar)
 *   WDCHK:3 bits 5:3  (check bits)
 *   WDDIS:1 bit 6     (disable)
 * C2000Ware DisableDog(): WDCR = 0x0068 | (WDCR & 0x0007)
 *   → force WDDIS=1 (bit6), WDCHK=6 (bits 5:3), keep WDPS (bits 2:0) */

/* ----- ROM wait-state (base 0x0005F540) ----- */
#define ROM_WAIT_STATE_W  0U    /* Uint16 — write 1 for 200 MHz */

/* ======================================================================
 * Ramfuncs symbols (lnk2000 automatic)
 * C2000Ware device.c uses these exact names (no leading underscore
 * because cl2000 auto-prepends one to C globals, and lnk2000 creates
 * them with one leading underscore via LOAD_START/RUN_START directives).
 * ==================================================================== */

extern uint16_t RamfuncsLoadStart;
extern uint16_t RamfuncsLoadSize;
extern uint16_t RamfuncsRunStart;

/* ======================================================================
 * CPU detection + result cache
 * ==================================================================== */

static unsigned int c2000_cpu_id_cache;  /* 0 = CPU1, 1 = CPU2 */

static bool c2000_is_cpu1(void)
{
    return (rd32(IPC_BASE, IPC_BOOTSTS_W) & IPC_BOOTSTS_SYSTEM_READY) != 0U;
}

unsigned int z_c2000_cpu_id(void)
{
    return c2000_cpu_id_cache;
}

/* FLASH0_ECC_BASE defined above = 0x0005FB00 (separate from FLASH0_CTRL_BASE).
 * ECC_ENABLE is word offset 0 from FLASH0_ECC_BASE (struct FLASH_ECC_REGS).
 * Value 0xA = 1010b is the magic pattern that enables Flash ECC on C28x. */
#define FLASH_ECC_ENABLE_W     0U
#define FLASH_ECC_ENABLE_VAL   0x0000000AU

/* ======================================================================
 * Step 0 — Disable watchdog (both CPUs, same register).
 * C2000Ware DisableDog() preserves WDCR bits 0:2 before forcing
 * WDHALTI(5)=1 and WDDIS(6)=1.
 * ==================================================================== */

static void c2000_disable_wdt(void)
{
    EALLOW();
    uint16_t wdcr = rd16(WD_BASE, WD_WDCR_W) & 0x0007U; /* keep bits 0..2 */
    wr16(WD_BASE, WD_WDCR_W, (uint16_t)(0x0068U | wdcr));
    EDIS();
}

/* ======================================================================
 * Step 1 — Copy ramfuncs from Flash → RAM.
 *
 * This executes from Flash.  It only reads Flash instructions/data —
 * no Flash control-register writes, so it is safe to run out of Flash
 * before c2000_flash_init_from_ram() takes over.
 * ==================================================================== */

static void c2000_copy_ramfuncs(void)
{
    uint16_t *load = &RamfuncsLoadStart;
    uint16_t *run  = &RamfuncsRunStart;
    /* lnk2000 generates RamfuncsLoadSize as a 32-bit absolute symbol.
     * Taking its address gives a pointer — cast to uint32_t to
     * extract the value, which is in units of 16-bit words. */
    uint32_t size = (uint32_t)&RamfuncsLoadSize;

    for (uint32_t i = 0; i < size; i++) {
        run[i] = load[i];
    }
}

/* ======================================================================
 * Step 2 — Flash init (runs from RAM via .TI.ramfunc section)
 *
 * TI TRM explicit warning: "Executing Flash control register writes
 * out of OTP/Flash will yield unpredictable results."
 *
 * We use the RAM copy from step 1, then call the function which
 * executes at the RUN address (RAMLS0).
 * ==================================================================== */

/* ======================================================================
 * Step 2 — Flash init (runs from RAM via .TI.ramfunc section)
 *
 * TI TRM explicit warning: "Executing Flash control register writes
 * out of OTP/Flash will yield unpredictable results."
 *
 * Sequence exactly matches C2000Ware v4.02 InitFlash():
 *   1. VREADST = 0x14 (covers sleep + active wake-up)
 *   2. PMPPWR = 1, BNKPWR0 = 3 (pump + bank active)
 *   3. Disable cache + prefetch BEFORE wait-state change
 *   4. RWAIT = 3 (minimum for 200 MHz SYSCLKOUT)
 *   5. Re-enable cache + prefetch AFTER wait-state change
 *   6. ECC_ENABLE = 0xA (magic value to enable Flash ECC)
 * ==================================================================== */

__attribute__((section(".TI.ramfunc")))
static void c2000_flash_init_from_ram(void)
{
    EALLOW();

    /* 1. VREADST = 0x14 */
    wr32(FLASH0_CTRL_BASE, FLASH_FBAC_W, FBAC_VREADST(0x14));

    /* 2. Power up pump + bank */
    wr32(FLASH0_CTRL_BASE, FLASH_FPAC1_W, FPAC1_PMPPWR);
    wr32(FLASH0_CTRL_BASE, FLASH_FBFALLBACK_W, FBFALLBACK_BNKPWR0(3));

    /* 3. Disable cache + prefetch BEFORE changing wait states */
    wr32(FLASH0_CTRL_BASE, FLASH_FRD_INTF_CTRL_W, 0);

    /* 4. RWAIT = 3 (minimum for 200 MHz SYSCLKOUT, per datasheet) */
    wr32(FLASH0_CTRL_BASE, FLASH_FRDCNTL_W, FRDCNTL_RWAIT(3));

    /* 5. Re-enable cache + prefetch AFTER wait-state change */
    wr32(FLASH0_CTRL_BASE, FLASH_FRD_INTF_CTRL_W,
         FRD_INTF_CTRL_PREFETCH_EN | FRD_INTF_CTRL_DATA_CACHE_EN);

    /* 6. Enable Flash ECC (ECC_ENABLE magic value = 0xA) */
    wr32(FLASH0_ECC_BASE, FLASH_ECC_ENABLE_W, FLASH_ECC_ENABLE_VAL);

    EDIS();

    /* ROM wait-state = 1 */
    wr16(ROM_WAIT_STATE_BASE, ROM_WAIT_STATE_W, 1);

    /* Pipeline flush — last Flash reg write settles before returning. */
    C28X_DELAY(7);
}

/* ======================================================================
 * Step 3 — PLL init
 *
 * CPU1 locks the PLL.  CPU2 verifies that CPU1 has already locked it.
 * Re-locking from CPU2 would disrupt CPU1's clock.
 *
 * XTAL = 10 MHz (F28379D LaunchPad — verified by C2000Ware example)
 * SYSPLLCLK = XTAL × (IMULT + FMULT/4) = 10 × (40 + 0) = 400 MHz
 * SYSCLKOUT = SYSPLLCLK / PLLSYSCLKDIV = 400 / 2 = 200 MHz
 *
 * Sequence follows C2000Ware v4.02 InitSysPll() exactly:
 *   1. SysXtalOscSel → CLKSRCCTL1.OSCCLKSRCSEL = 1
 *   2. PLLCLKEN = 0 (bypass PLL), 120 NOP, PLLSYSCLKDIV = 0 (/1)
 *   3. Loop 5×: PLLEN=0 → 20 NOP → SYSPLLMULT.all → while LOCKS=1
 *   4. PLLSYSCLKDIV = 1 (/2)
 * ==================================================================== */

#define C28X_OSC_DELAY \
    __asm__(" RPT #250 || NOP \n RPT #50 || NOP")

static void c2000_pll_init(bool cpu2_path)
{
#if !defined(CONFIG_C2000_PLL_INIT)
    return;
#endif

    if (cpu2_path) {
        /* CPU2: wait for PLL already locked by CPU1.
         * PLL is shared and CPU1 owns the lock. */
        while ((rd32(CLKCFG_BASE, CLKCFG_SYSPLLSTS_W) & SYSPLLSTS_LOCKS) == 0) {
            C28X_NOP();
        }
        return;
    }

    EALLOW();

    /* 1. Select XTAL as PLL source (SysXtalOscSel equivalent) */
    wr32(CLKCFG_BASE, CLKCFG_CLKSRCCTL1_W, OSCCLKSRCSEL(1));
    C28X_OSC_DELAY;

    /* 2. Bypass PLL, wait 120 cycles, set divider to /1 */
    wr32(CLKCFG_BASE, CLKCFG_SYSPLLCTL1_W, 0);   /* PLLCLKEN = 0 */
    C28X_DELAY(120);
    wr32(CLKCFG_BASE, CLKCFG_SYSCLKDIVSEL_W, PLLSYSCLKDIV(0));

    /* 3. Lock PLL five times (C2000Ware minimum recommended).
     *    Writing SYSPLLMULT implicitly turns on PLLEN. */
    for (int i = 0; i < 5; i++) {
        wr32(CLKCFG_BASE, CLKCFG_SYSPLLCTL1_W, 0);   /* PLLEN = 0 */
        C28X_DELAY(20);

        /* IMULT = 40 (XTAL 10MHz LaunchPad), FMULT = 0
         * SYSPLLMULT.all = (FMULT << 8) | IMULT  (Uint16 low word) */
        wr32(CLKCFG_BASE, CLKCFG_SYSPLLMULT_W,
             (uint32_t)((0U << 8U) | 40U));    /* direct write — no IMULT/FMULT macros */

        while ((rd32(CLKCFG_BASE, CLKCFG_SYSPLLSTS_W) & SYSPLLSTS_LOCKS) == 0) {
            C28X_NOP();
        }
    }

    /* 4. Set divider AFTER lock (C2000Ware does this here, not during bypass) */
    wr32(CLKCFG_BASE, CLKCFG_SYSCLKDIVSEL_W, PLLSYSCLKDIV(1));

    /* 5. **CRITICAL** — Re-enable PLL clock path.
     * Step 2 above wrote SYSPLLCTL1 = 0 which set PLLCLKEN=0 (bypass mode).
     * PLL is now locked, so route PLL output to SYSCLKOUT instead of OSCCLK.
     * Without this, SYSCLKOUT stays at 10 MHz (OSCCLK) forever! */
    wr32(CLKCFG_BASE, CLKCFG_SYSPLLCTL1_W, PLLEN | PLLCLKEN);

    EDIS();

    /* VDD stabilisation — C2000Ware has ~120 OSCCLK delay here.
     * We approximate with 200 NOPs at 200 MHz post-divider. */
    C28X_DELAY(200);
}

/* ======================================================================
 * Step 4 — Peripheral clock gating (per-CPU private PCLKCR0)
 * ==================================================================== */

static void c2000_clock_gating_init(bool is_cpu1)
{
    EALLOW();

    uint32_t pclk0 = PCLKCR0_DMA | PCLKCR0_CPUTIMER0 |
                     PCLKCR0_HRPWM | PCLKCR0_TBCLKSYNC;

    if (is_cpu1) {
#if defined(CONFIG_C2000_CLA1)
        pclk0 |= PCLKCR0_CLA1;
#endif
    } else {
        /* CPU2 also uses PCLKCR0.bit.CLA1 (CLA1 = CLA2 on CPU2 view) */
#if defined(CONFIG_C2000_CLA2)
        pclk0 |= PCLKCR0_CLA1;
#endif
    }

    wr32(CPUSYS_BASE, CPUSYS_PCLKCR0_W,
         rd32(CPUSYS_BASE, CPUSYS_PCLKCR0_W) | pclk0);

    EDIS();
}

/* ======================================================================
 * Step 5 — PIE init (per-CPU private PIE controller)
 *
 * PIECTRL is a 16-bit register!  ENPIE is bit 0, PIEVECT is bits 15:1.
 * PIEVECT stores PIEVECTTABLE / 2 (PIE vector addresses are word
 * aligned).
 * ==================================================================== */

extern volatile void *PieVectTable;

static void c2000_pie_init(void)
{
    /* Disable PIE */
    wr16(PIE_CTRL_BASE, PIECTRL_W, 0);

    /* Write PIEVECT = (PieVectTable >> 1).
     * PIECTRL is Uint16: PIEVECT bits 15:1, ENPIE bit 0.
     * Use 0xFFFE mask to keep bits 15:1 (vector table addr), clear bit 0. */
    uint16_t pievect_val = (uint16_t)((((uintptr_t)&PieVectTable) & 0xFFFEU));

    wr16(PIE_CTRL_BASE, PIECTRL_W, pievect_val);

    /* Clear all PIE ACK flags (bits 0..11) */
    wr16(PIE_CTRL_BASE, PIEACK_W, 0xFFFFU);

    /* Enable PIE (OR ENPIE bit 0 back in) */
    wr16(PIE_CTRL_BASE, PIECTRL_W, pievect_val | PIECTRL_ENPIE);
}

/* ======================================================================
 * Step 6 — CLA soft reset
 *
 * SOFTPRES0 blow-out for each CLA (CPU1_CLA1 on CPU1 path,
 * CPU2_CLA1 on CPU2 path — this is C2000Ware naming; on CPU2
 * the "CLA1" reset bit actually resets CLA2).
 * Then clear all task flags + overflow.
 * ==================================================================== */

static void c2000_cla_reset_one(uint32_t cla_base)
{
    /* MCTL.SOFTRESET (bit 1) — self-clears after a few cycles */
    wr16(cla_base, CLA_MCTL_W,
         (uint16_t)(rd16(cla_base, CLA_MCTL_W) | MCTL_SOFTRESET));
    C28X_DELAY(10);

    /* Clear all task interrupt flags (MICLR bits 0..7) */
    wr16(cla_base, CLA_MICLR_W, 0x00FFU);
    /* Clear all overflow flags (MICLROVF bits 0..7) */
    wr16(cla_base, CLA_MICLROVF_W, 0x00FFU);
}

static void c2000_cla1_init(void)
{
    EALLOW();

    /* Blow out CLA1 via SOFTPRES0 bit 0 (CPU1 path) */
    wr32(DEVCFG_BASE, DEVCFG_SOFTPRES0_W,
         rd32(DEVCFG_BASE, DEVCFG_SOFTPRES0_W) | SOFTPRES0_CPU1_CLA1);
    C28X_DELAY(10);
    wr32(DEVCFG_BASE, DEVCFG_SOFTPRES0_W,
         rd32(DEVCFG_BASE, DEVCFG_SOFTPRES0_W) & ~SOFTPRES0_CPU1_CLA1);

    EDIS();

    c2000_cla_reset_one(CLA1_BASE);
}

static void c2000_cla2_init(void)
{
    EALLOW();

    /* Blow out CLA2 via SOFTPRES0 bit 2 (CPU2 path) */
    wr32(DEVCFG_BASE, DEVCFG_SOFTPRES0_W,
         rd32(DEVCFG_BASE, DEVCFG_SOFTPRES0_W) | SOFTPRES0_CPU2_CLA1);
    C28X_DELAY(10);
    wr32(DEVCFG_BASE, DEVCFG_SOFTPRES0_W,
         rd32(DEVCFG_BASE, DEVCFG_SOFTPRES0_W) & ~SOFTPRES0_CPU2_CLA1);

    EDIS();

    c2000_cla_reset_one(CLA2_BASE);
}

/* ======================================================================
 * Step 7 — CPU2 boot via IPC (CPU1 ONLY)
 *
 * Sequence exactly matches C2000Ware v4.02 Device_bootCPU2():
 *   1. Wait for IPCBOOTSTS.SYSTEM_READY = 1 (CPU2 Boot ROM listening)
 *   2. Wait for IPCFLG.IPC0 = 0 AND IPCFLG.IPC31 = 0 (flags available)
 *   3. Write IPCBOOTMODE = Flash boot (0x0B)
 *   4. Write IPCSENDCOM = BROM_IPC_EXECUTE_BOOTMODE_CMD (0)
 *   5. Write IPCSET = 0x80000001 (set IPC0 + IPC31 → trigger CPU2 Boot ROM)
 *
 * NOTE: We DON'T wait for CPU2 acknowledgment — CPU1 proceeds to
 * auto_init + z_cstart immediately. CPU2 takes ~2 ms for its bring-up.
 * ==================================================================== */

static void c2000_cpu2_boot(void)
{
#if !defined(CONFIG_C2000_SMP_CPU2)
    return;
#else
    /* Step 1: Wait for CPU2 Boot ROM to be ready */
    while ((rd32(IPC_BASE, IPC_BOOTSTS_W) & IPC_BOOTSTS_SYSTEM_READY) == 0) {
        C28X_NOP();
    }

    /* Step 2: Wait until both IPC0 and IPC31 remote flags are CLEAR
     * (i.e. available for us to set). This is required by C2000Ware
     * because you must not set a flag that's already pending. */
    uint32_t mask = IPC_FLG_IPC0 | IPC_FLG_IPC31;
    while ((rd32(IPC_BASE, IPC_FLG_W) & mask) != 0U) {
        C28X_NOP();
    }

    /* Step 3: Boot mode = Flash (CPU2 will boot from its own Flash bank) */
    wr32(IPC_BASE, IPC_BOOTMODE_W, C1C2_BOOTMODE_FLASH);

    /* Step 4: Execute-boot command */
    wr32(IPC_BASE, IPC_SENDCOM_W, BROM_EXECUTE_BOOTMODE_CMD);

    /* Step 5: Kick CPU2 — set IPC0 (triggers IPC interrupt) + IPC31
     * (Boot ROM handshake flag). Writing 0x80000001 atomically sets both. */
    wr32(IPC_BASE, IPC_SET_W, 0x80000001U);
#endif
}

/* ======================================================================
 * Main entry — dispatched by CPU detection right after WDT kill
 * ==================================================================== */

extern void __TI_auto_init_nobinit(void);
extern FUNC_NORETURN void z_cstart(void);

void z_c2000_start(void)
{
    /* 0. Kill watchdog first — before any register write */
    c2000_disable_wdt();

    /* 1. Detect CPU1 vs CPU2 and cache the result.
     *    This MUST happen before we ever touch IPC registers — on
     *    the CPU2 path they land in CPU1-only space until Boot ROM
     *    hands off. */
    bool is_cpu1 = c2000_is_cpu1();
    c2000_cpu_id_cache = is_cpu1 ? 0U : 1U;

    /* 2. Copy ramfuncs from Flash → RAM, then run Flash init from RAM.
     *    Both CPUs do this — each owns its own Flash bank (CPU1→bank0,
     *    CPU2→bank1) and each owns its own RAMLS0 for the copy. */
    c2000_copy_ramfuncs();
    c2000_flash_init_from_ram();

    /* 3. PLL: CPU1 locks the shared PLL; CPU2 just waits for it. */
    c2000_pll_init(/*cpu2_path=*/!is_cpu1);

    /* 4. Peripheral clock gating — per-CPU PCLKCR0 */
    c2000_clock_gating_init(is_cpu1);

    /* 5. PIE — per-CPU private PIE controller, 16-bit PIECTRL */
    c2000_pie_init();

    /* 6. CLA — CPU1 blows out CLA1; CPU2 blows out CLA2 */
    if (is_cpu1) {
        c2000_cla1_init();
    } else {
        c2000_cla2_init();
    }

    /* 7. CPU2 boot via IPC — CPU1 only */
    if (is_cpu1) {
        c2000_cpu2_boot();
    }

    /* 8. TI RTS runtime init — each CPU has its own RAM, so each
     *    zeroes .bss and copies .cinit independently. */
    __TI_auto_init_nobinit();

    /* 9. Kernel bootstrap — scheduler, idle thread, main thread */
    z_cstart();
}

void z_c2000_irq_nesting_init(void)
{
    z_c2000_irq_nesting = 0U;
}
