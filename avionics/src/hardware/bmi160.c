#include "hardware/bmi160.h"
#include "hardware/platform.h"
#include "isr_flags.h"

/*
 * bmi160.c — STUB
 *
 * Real implementation will:
 *   - Configure SPI1 with DMA streams for TX and RX.
 *   - Reset the BMI160, verify the chip ID register, configure ODR, range,
 *     and the data-ready interrupt source.
 *   - Arm an EXTI line for the data-ready pin.
 *   - On data-ready, kick off a DMA-driven multi-byte register read into a
 *     double buffer. SPI1 DMA TC ISR sets imu1_data_ready and swaps pages.
 *   - bmi160_read() copies the latest filled page into *out, applies scaling,
 *     attaches platform_timer_us() timestamp, returns IMU_OK or a fault.
 */

/*
 * Owning definition for the ISR/main-loop boundary flag declared in
 * isr_flags.h. The volatile qualifier comes from the volatile_flag_t typedef.
 */
volatile_flag_t imu1_data_ready;  /* zero-initialised by C startup; see isr_flags.h */

void bmi160_init(void)
{
    /*
     * Real implementation: configure SPI1+DMA, init BMI160 registers,
     * arm interrupts. Does NOT kick the watchdog (no per-_init() kicks, D053 A3).
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}

imu_status_t bmi160_read(imu_reading_t *out)
{
    /*
     * Real implementation: check imu1_data_ready; if set, atomically clear
     * it, scale the raw accel/gyro counts to SI units, attach a timestamp
     * from platform_timer_us(), return IMU_OK. If no fresh data, return
     * IMU_STALE_DATA. If DMA reported an error, return IMU_BUS_ERROR.
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    (void)out;
    return IMU_TIMEOUT;
}
