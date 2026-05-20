#include "algorithm/estimator.h"

/*
 * estimator.c — STUB
 *
 * ============================================================================
 * WARNING — DEFERRED BOUNDARY ISSUE — DO NOT IMPLEMENT AGAINST CURRENT SIGNATURE
 * ============================================================================
 * The single estimator_update() exposed below is a placeholder. The real
 * interface will split into estimator_predict() and estimator_update() to
 * allow FDIR access to predicted measurements without violating the algorithm
 * layer's isolation rule.
 *
 *   tick A: estimator_predict()    → produces predicted_readings_t for FDIR
 *   tick B: fdir_update(actuals, predictions, health_out)
 *   tick C: estimator_update(actuals, health_out)
 *
 * Tracked as an open task in docs/02-current-state.md.
 * Do not begin implementing the EKF against the current signature.
 * ============================================================================
 *
 * When the boundary is resolved, this module will implement (D035):
 *   - State vector definition (typically: quaternion + body rates, or
 *     Euler angles + rates).
 *   - EKF predict step: propagate state through the system model using
 *     gyro measurements; propagate covariance with process noise Q.
 *   - EKF update step: for each healthy sensor, compute innovation, apply
 *     Kalman update with measurement noise R derived from IMU datasheet
 *     specs (LR-2).
 *   - Degraded modes: with NULL sensor pointers, widen covariance, reduce
 *     update gain, transition mode enum accordingly.
 *   - Reset: clear all state and covariance to known initial values.
 *
 * Alternative implementation: Madgwick or Mahony complementary filter (per
 * D035 "or equivalent"). Decision on specific algorithm deferred to
 * implementation session.
 */

void estimator_init(void)
{
    /* Real implementation: zero state vector, set initial covariance to
     * reflect "no knowledge" priors, set mode to EST_MODE_DUAL_IMU. */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}

void estimator_reset(void)
{
    /* Real implementation: identical to estimator_init() effects on state
     * and covariance, used during HIL replay between test cases. */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}

estimator_mode_t estimator_update(
    const imu_reading_t    *imu1,
    const imu_reading_t    *imu2,
    const baro_reading_t   *baro,
    const health_flags_t   *health,
    attitude_estimate_t    *out
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
    (void)health;
    (void)out;
    return EST_MODE_FAULT;
}
