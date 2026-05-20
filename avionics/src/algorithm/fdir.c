#include "algorithm/fdir.h"

/*
 * fdir.c — STUB
 *
 * ============================================================================
 * WARNING — DEFERRED BOUNDARY ISSUE — DO NOT IMPLEMENT AGAINST CURRENT SIGNATURE
 * ============================================================================
 * The fdir_update() signature in this file is INCOMPLETE. The innovation gate
 * needs predicted measurements from the estimator's prediction step (residual
 * = z - H*x_pred), which are not provided by the current interface.
 *
 * Resolving the boundary requires a two-phase tick:
 *   tick A: estimator_predict()    → predicted_readings_t
 *   tick B: fdir_update(actuals, predictions, health_out, gate_out)
 *   tick C: estimator_update(actuals, health_out)
 *
 * Tracked as an open task in docs/02-current-state.md.
 * Do not begin implementing FDIR logic against the current signature.
 * ============================================================================
 *
 * When the boundary is resolved, this module will implement:
 *   - Staleness watchdog: per-sensor "last seen" timestamp check against
 *     IMU_STALENESS_THRESHOLD_US (TBD from LR-1).
 *   - Bounds check: range-check accel, gyro, pressure values against the
 *     physically plausible envelope; trip per-channel health flag on violation.
 *   - Innovation gate: chi-squared cross-check between IMU-1 / IMU-2 against
 *     the EKF prediction, threshold CHI2_THRESHOLD_2DOF (TBD from LR-3).
 *   - Health flag propagation: write health_flags_t, write fdir_gate_result_t.
 */

void fdir_init(void)
{
    /* Real implementation: clear internal staleness tracking, set all health
     * flags to true initially (any sensor must prove unhealthy to be flagged). */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}

void fdir_update(
    const imu_reading_t  *imu1,
    const imu_reading_t  *imu2,
    const baro_reading_t *baro,
    health_flags_t       *health_out,
    fdir_gate_result_t   *gate_out
)
{
    /* SIGNATURE INTENTIONALLY INCOMPLETE — see WARNING at top of file.
     * Real implementation deferred until the FDIR/estimator boundary task
     * is closed. */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    (void)imu1;
    (void)imu2;
    (void)baro;
    (void)health_out;
    (void)gate_out;
}
