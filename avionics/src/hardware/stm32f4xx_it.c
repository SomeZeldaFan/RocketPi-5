#include "isr_flags.h"

/*
 * stm32f4xx_it.c — STUB
 *
 * HAL ISR handler implementations live here per ST convention. The handlers
 * below are the five application-relevant ISRs from architecture §4.2 plus
 * the four default fault handlers.
 *
 * Strict rules (D043 + §4.2):
 *   - ISRs do flag-set / pointer-advance work ONLY. No math, no function
 *     calls into application code, no register reads beyond DMA status.
 *   - Every ISR < 10 lines of code in the real implementation.
 *   - Every ISR-shared variable goes through isr_flags.h.
 *
 * Real implementation will replace each comment block with one or two
 * statements that perform the documented flag-set / pointer-advance.
 *
 * Note: this file currently has no executable bodies so any HAL hook into
 * these symbols will require linking to a CubeIDE-generated weak default;
 * resolution of build system + IDE integration is deferred (see plan).
 */

/* -------------------------------------------------------------------------
 * SPI1 DMA Transfer Complete — IMU-1 (BMI160) sample arrived
 * Real implementation:
 *   imu1_data_ready = 1U;
 *   // swap double-buffer page index
 * ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
 * SPI2 DMA Transfer Complete — IMU-2 (ICM-42688-P) sample arrived
 * Real implementation:
 *   imu2_data_ready = 1U;
 *   // swap double-buffer page index
 * ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
 * I2C DMA Transfer Complete — barometer phase complete
 * Real implementation:
 *   baro_data_ready = 1U;
 * ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
 * UART1 DMA RX — radio bytes arrived
 * Real implementation:
 *   // advance ring buffer write pointer to the position reported by DMA
 *   // (the variable is module-private to sik_radio.c — extern interface TBD)
 * ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
 * TIM2 update — fixed-rate loop tick fired
 * Real implementation:
 *   tick_flag = 1U;
 * ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
 * Fault handlers — infinite-loop stubs in the real build, observed via
 * debugger. The watchdog will eventually fire and reset the MCU. These
 * are intentionally empty here; the real implementation will be:
 *   for (;;) { __NOP(); }
 * with possibly a register snapshot to a backup RAM location for
 * post-mortem inspection on the next boot.
 * ------------------------------------------------------------------------- */

/* HardFault_Handler  — real impl: spin, watchdog eventually resets */
/* MemManage_Handler  — real impl: spin, watchdog eventually resets */
/* BusFault_Handler   — real impl: spin, watchdog eventually resets */
/* UsageFault_Handler — real impl: spin, watchdog eventually resets */
