#ifndef ICM42688P_H
#define ICM42688P_H

#include "avionics_types.h"

/*
 * icm42688p.h — IMU-2 SPI driver (TDK InvenSense ICM-42688-P)
 *
 * Layer: hardware. Heterogeneous to bmi160.h by design (D024) — different
 * silicon family, different register map, different temperature behaviour.
 * Common-mode failures cannot defeat the FDIR cross-check.
 *
 * Each IMU is on its own dedicated SPI bus (D026) — no bus sharing. A stuck
 * CS or hung transaction on this bus cannot affect IMU-1.
 *
 * Timing: a SPI2 DMA TC ISR sets imu2_data_ready when a new sample arrives.
 * icm42688p_read() consumes the latest filled double-buffer page.
 */

/*
 * Configure SPI2 + DMA, initialise ICM-42688-P registers (ODR, FS range,
 * AAF/UI filter), arm the data-ready interrupt, kick the platform watchdog.
 * Must be called after platform_init().
 */
void icm42688p_init(void);

/*
 * Consume the latest ICM-42688-P reading. Pairs the sample with a TIM2
 * timestamp. Returns IMU_OK on success or one of the imu_status_t failure
 * modes. Caller MUST check the status before using accel_mss/gyro_rads.
 */
imu_status_t icm42688p_read(imu_reading_t *out);

#endif /* ICM42688P_H */
