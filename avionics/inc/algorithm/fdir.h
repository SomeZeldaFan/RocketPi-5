#ifndef FDIR_H
#define FDIR_H

#include "avionics_types.h"

/*
 * fdir.h — Fault Detection, Isolation, and Recovery
 *
 * Layer: algorithm. FDIR is the authoritative source of health_flags_t in
 * the system. It is the ONLY module that writes health flags; every other
 * module (estimator, control law, telemetry) reads them.
 *
 * What FDIR does:
 *   - Staleness watchdog: reads sensor timestamps, raises a health flag if
 *     a channel has not produced a fresh reading within its staleness budget.
 *   - Bounds check: range-checks accel, gyro, pressure values against their
 *     physically plausible envelopes.
 *   - Innovation gate (chi-squared cross-check between IMU-1 and IMU-2):
 *     statistically detects sensor disagreement and isolates the outlier.
 *   - Actuator health (future): detects actuator faults via servo feedback
 *     or response monitoring; not yet implemented.
 *
 * Architectural boundary (D024, §4.4):
 *   - FDIR sets health flags. Estimator reads them. Estimator NEVER sets
 *     health flags. No module crosses this boundary.
 *
 * ============================================================================
 * WARNING — DEFERRED BOUNDARY ISSUE — DO NOT IMPLEMENT AGAINST CURRENT SIGNATURE
 * ============================================================================
 * The innovation gate requires comparing each sensor reading against the EKF's
 * predicted measurement (residual = z - H * x_pred). The current fdir_update()
 * signature accepts only the raw sensor readings — it has no input for the
 * predicted measurements. Implementing the innovation gate against the
 * current signature would require importing from the estimator (violating
 * the algorithm-layer isolation rule) or a redesigned call sequence.
 *
 * Resolution pattern: two-phase tick.
 *   tick step A: estimator_predict()    → produces predicted_readings_t
 *   tick step B: fdir_update(actuals, predictions, health_out, gate_out)
 *   tick step C: estimator_update(actuals, health_out)  → posterior state
 *
 * Tracked as an open task in docs/02-current-state.md. The signature below
 * is intentionally incomplete. Resolve the boundary before implementing.
 * ============================================================================
 */

void fdir_init(void);

/*
 * INCOMPLETE — signature pending boundary resolution. See WARNING above.
 *
 * Stub accepts current sensor readings only. Real implementation will
 * additionally accept the estimator's predicted measurements.
 */
void fdir_update(
    const imu_reading_t  *imu1,
    const imu_reading_t  *imu2,
    const baro_reading_t *baro,
    health_flags_t       *health_out,
    fdir_gate_result_t   *gate_out
);

#endif /* FDIR_H */
