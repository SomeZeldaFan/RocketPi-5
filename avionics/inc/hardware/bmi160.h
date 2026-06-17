#ifndef BMI160_H
#define BMI160_H

#include "avionics_types.h"

/*
 * bmi160.h — IMU-1 SPI driver (Bosch BMI160)
 *
 * Layer: hardware. Wraps the BMI160 on dedicated SPI bus with DMA. The driver
 * configures the chip at init time, starts the DMA stream, and exposes a
 * single read function that consumes the latest filled DMA buffer.
 *
 * The driver is the ONLY module that knows it is a BMI160. Upstream callers
 * see an imu_reading_t with raw accel/gyro in SI units and a TIM2 timestamp.
 * Swapping the chip later means changing this file; no other module changes.
 *
 * Timing: a SPI1 DMA TC ISR sets imu1_data_ready when a new sample arrives.
 * bmi160_read() consumes the latest filled double-buffer page.
 */

/*
 * Configure SPI1 + DMA, initialise BMI160 registers (ODR, range, filtering),
 * arm the data-ready interrupt. Must be called after platform_init(). Does
 * NOT kick the watchdog — there are no per-_init() kicks (D053 A3).
 */
void bmi160_init(void);

/*
 * Consume the latest BMI160 reading. Pairs the sample with a TIM2 timestamp
 * captured at the moment of read. Returns IMU_OK if data is valid, otherwise
 * one of the documented imu_status_t failure modes. Caller MUST check the
 * status before using accel_mss/gyro_rads.
 */
imu_status_t bmi160_read(imu_reading_t *out);

#endif /* BMI160_H */
