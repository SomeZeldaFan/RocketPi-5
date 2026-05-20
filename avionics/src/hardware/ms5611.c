#include "hardware/ms5611.h"
#include "hardware/platform.h"
#include "isr_flags.h"

/*
 * ms5611.c — STUB
 *
 * Real implementation will:
 *   - Configure I2C with DMA streams.
 *   - Reset the MS5611, read its 6-word PROM calibration coefficients.
 *   - Implement an internal state machine to interleave pressure and
 *     temperature conversions: trigger D1 → wait OSR-dependent time →
 *     read D1 → trigger D2 → wait → read D2 → apply MS5611 compensation
 *     polynomial → derive altitude from compensated pressure using a
 *     standard sea-level reference (1013.25 hPa).
 *   - Decimate output to AVIONICS_BARO_RATE_HZ (50 Hz nominal).
 */

/*
 * Owning definition for the ISR/main-loop boundary flag declared in
 * isr_flags.h. The volatile qualifier comes from the volatile_flag_t typedef.
 */
volatile_flag_t baro_data_ready;  /* zero-initialised by C startup; see isr_flags.h */

void ms5611_init(void)
{
    /*
     * Real implementation: configure I2C+DMA, reset the MS5611, read PROM
     * calibration words, store them statically, kick the watchdog.
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}

baro_status_t ms5611_service(baro_reading_t *out)
{
    /*
     * Real implementation: advance the internal I2C state machine. Most
     * ticks: return without producing output. On decimation boundary:
     * compute compensated pressure and altitude, fill *out with timestamp
     * from platform_timer_us(), return BARO_OK.
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    (void)out;
    return BARO_TIMEOUT;
}
