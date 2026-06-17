#ifndef ISR_FLAGS_H
#define ISR_FLAGS_H

#include "avionics_types.h"

/*
 * isr_flags.h — manifest of every ISR/main-loop boundary crossing
 *
 * Every variable shared between an ISR and the main loop is declared here.
 * Definitions live in the owning driver .c file (one definition per variable).
 * Purpose: a reviewer auditing for volatile correctness opens this one file
 * and sees every boundary crossing in the system.
 *
 * Rule: if a variable is written by an ISR and read by the main loop (or
 * vice-versa), it MUST appear in this file. No exceptions. Adding an ISR
 * without updating this file is a review finding.
 *
 * The ring buffer write pointer for UART RX is managed entirely inside
 * sik_radio.c and is not listed here — it is not read directly by the main
 * loop; c2 reads buffered bytes through the sik_radio API.
 *
 * Flag-clear discipline (D048): the main loop clears a *_data_ready flag while
 * snapshotting the matching *_active_page index inside a BASEPRI critical section
 * (mask priority >= 1 so the DMA-TC level is held while TIM2@0 keeps firing),
 * then consumes the buffer OUTSIDE the section. This closes the read-modify-write
 * race on the flag (clear vs a concurrent ISR set) without dropping ticks. See
 * the NVIC scheme block in platform.h.
 */

/* SPI1 DMA TC ISR (IMU-1) sets; main loop reads (and clears) at tick [1] */
extern volatile_flag_t imu1_data_ready;

/* SPI1 DMA TC ISR (IMU-1) swaps; main loop reads under BASEPRI at tick [1] (D048) */
extern volatile_flag_t imu1_active_page;

/* SPI2 DMA TC ISR (IMU-2) sets; main loop reads (and clears) at tick [2] */
extern volatile_flag_t imu2_data_ready;

/* SPI2 DMA TC ISR (IMU-2) swaps; main loop reads under BASEPRI at tick [2] (D048) */
extern volatile_flag_t imu2_active_page;

/* I2C DMA TC ISR (barometer) sets; main loop reads at tick [3] */
extern volatile_flag_t baro_data_ready;

/* TIM2 update ISR sets; main loop reads at tick [14] (tick_wait) */
extern volatile_flag_t tick_flag;

#endif /* ISR_FLAGS_H */
