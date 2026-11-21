/*
 * Shared board-support HAL for the TI F2837xD family, used by the Zephyr
 * C2000 port drivers (system timer, SCI serial, SoC glue).
 *
 * Register access goes through the volatile structures defined by
 * C2000Ware's F2837xD_GlobalVariableDefs.c and placed at the peripheral
 * base addresses by F2837xD_zephyr.cmd.
 */
#ifndef ZEPHYR_SOC_TI_F2837XD_HAL_H_
#define ZEPHYR_SOC_TI_F2837XD_HAL_H_

#include <stdint.h>

#include "F2837xD_device.h"

/*
 * SYSCLK produced by the PLL bring-up sequence (prep_c.c).  LSPCLK feeds
 * the SCI prescaler and keeps its reset divider of 4 (LOSPCP = 2).
 */
#define HAL_SYSCLK_HZ	200000000U
#define HAL_LSPCLK_HZ	(HAL_SYSCLK_HZ / 4U)

/*
 * ISR signature expected by the ePIE vector table.  The C28x automatically
 * saves context on interrupt entry; the ISR must end with IRET, so every
 * handler installed through this HAL is declared with __interrupt.
 */
typedef __interrupt void (*hal_isr_t)(void);

/* Number of ePIE vector slots (32 CPU-level entries + 12 groups x 16). */
#define HAL_PIE_VECT_ENTRIES	224U

/* Watchdog / peripheral clock / low-speed clock bring-up. */
void hal_sysctl_init(void);

/* Expanded-PIE bring-up: default vectors, group enables cleared. */
void hal_pie_init(void);

/*
 * Install an ISR on an ePIE line.  The line index follows the SoC
 * convention: line = (group - 1) * 8 + (intx - 1) with group 1..12 and
 * intx 1..8 (0..95).
 */
void hal_pie_register(unsigned int line, hal_isr_t isr);

/* Spurious-interrupt sink acknowledged into every PIE group. */
__interrupt void hal_pie_default_isr(void);

/* SCIA pin mux + 8N1 + FIFO mode bring-up. */
void hal_gpio_init(void);
void hal_sci_init(uint32_t baud);
void hal_sci_putc(char c);

/* CPU timer 0 as the system tick source. */
void hal_cputimer0_init(uint32_t prd, hal_isr_t isr);
void hal_cputimer0_start(void);

#endif /* ZEPHYR_SOC_TI_F2837XD_HAL_H_ */
