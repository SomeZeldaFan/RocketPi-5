#include "algorithm/fdir.h"

/*
 * fdir.c — STUB
 *
 * Two-pass FDIR (D050): fdir_admit() runs the absolute checks before
 * estimator_predict(); fdir_gate() runs the innovation gate after it. FDIR is
 * the sole writer of health_flags_t and never imports the estimator — it
 * consumes the estimator's predictions as a predicted_readings_t value.
 *
 * When implemented, this module will provide:
 *   fdir_admit():
 *     - staleness watchdog: per-sensor timestamp age vs IMU_STALENESS_THRESHOLD_US
 *     - bounds check: accel / gyro / pressure vs the physical envelope, NaN, sat
 *     - gyro cross-check: IMU-1 vs IMU-2 gyro agreement (no prediction needed)
 *     - writes the preliminary health verdict + staleness fields of the result
 *   fdir_gate():
 *     - innovation gate: chi-squared test of residual = z - h(x_pred) against
 *       CHI2_THRESHOLD_2DOF (TBD from LR-3); isolates the outlier
 *     - restrict-only on health (never resurrects an admitted-out channel)
 *     - writes the chi-squared / gate-open fields of the result
 */

void fdir_init(void)
{
    /* Real implementation: clear staleness tracking; set all health flags true
     * initially (a sensor must prove unhealthy to be flagged). */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}

void fdir_admit(
    const imu_reading_t  *imu1,
    const imu_reading_t  *imu2,
    const baro_reading_t *baro,
    const mag_reading_t  *mag,
    health_flags_t       *health_out,
    fdir_gate_result_t   *gate_out
)
{
    /* Real implementation: absolute checks (staleness, bounds, gyro-vs-gyro)
     * → preliminary health_out + staleness fields of gate_out.
     * mag (D060): absolute checks only — no twin to cross-check; staleness,
     * NaN/sat, |B| field-magnitude vs the known Earth-field magnitude. */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    (void)imu1;
    (void)imu2;
    (void)baro;
    (void)mag;
    (void)health_out;
    (void)gate_out;
}

void fdir_gate(
    const imu_reading_t        *imu1,
    const imu_reading_t        *imu2,
    const baro_reading_t       *baro,
    const mag_reading_t        *mag,
    const predicted_readings_t *predictions,
    health_flags_t             *health_inout,
    fdir_gate_result_t         *gate_out
)
{
    /* Real implementation: innovation gate of each reading vs its prediction
     * → restrict health_inout; write chi-squared / gate-open fields.
     * mag (D060): chi2_mag = residual(measured, predictions->mag_pred_ut) χ²
     * plus dip-angle check — analytical cross-check, no second mag. */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    (void)imu1;
    (void)imu2;
    (void)baro;
    (void)mag;
    (void)predictions;
    (void)health_inout;
    (void)gate_out;
}
