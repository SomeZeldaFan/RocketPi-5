#ifndef ESTIMATOR_H
#define ESTIMATOR_H

#include "avionics_types.h"

/*
 * estimator.h — attitude state estimator (EKF or equivalent)
 *
 * Layer: algorithm. Consumes filtered sensor readings (post-FDIR), produces
 * an attitude_estimate_t with covariance and an explicit mode.
 *
 * NULL-pointer semantics for sensor inputs:
 *   - An isolated sensor is passed as NULL. The estimator handles degraded
 *     operation by reading which pointers are NULL and adjusting the
 *     process / measurement model accordingly. Zero-filled readings are NOT
 *     used to signal isolation — that is ambiguous; NULL is unambiguous.
 *
 * Mode transitions are entirely determined by which sensor pointers are
 * non-NULL plus the FDIR health flags:
 *   - both IMUs non-NULL  → EST_MODE_DUAL_IMU
 *   - IMU-1 only          → EST_MODE_IMU1_ONLY
 *   - IMU-2 only          → EST_MODE_IMU2_ONLY
 *   - both NULL           → EST_MODE_DEAD_RECKONING (propagate with last rates)
 *   - unrecoverable       → EST_MODE_FAULT
 *
 * Covariance grows as sensors are lost. The output's covariance field is
 * the system's numerically expressed confidence — downstream consumers
 * (control law, telemetry) propagate or display it; ignoring it is a
 * reviewer finding (§4.4 invariant 3).
 *
 * Per D035: the fusion algorithm is user-implemented (EKF or Madgwick/Mahony
 * equivalent). No proprietary on-chip fusion. JPL Power of 10 compliant.
 *
 * ============================================================================
 * WARNING — DEFERRED BOUNDARY ISSUE — DO NOT IMPLEMENT AGAINST CURRENT SIGNATURE
 * ============================================================================
 * The split between estimator_predict() and estimator_update() — required by
 * the FDIR/estimator boundary resolution (see fdir.h WARNING) — is not yet
 * expressed in this interface. The current single estimator_update() is a
 * placeholder for that split.
 *
 * Resolution pattern: two-phase tick.
 *   tick step A: estimator_predict()    → produces predicted_readings_t for FDIR
 *   tick step B: fdir_update()          → produces health_flags using predictions
 *   tick step C: estimator_update()     → posterior update with healthy sensors
 *
 * Tracked as an open task in docs/02-current-state.md. The signature below
 * is intentionally incomplete. Resolve the boundary before implementing.
 * ============================================================================
 */

void estimator_init(void);

/*
 * Reset the estimator to a known clean state. Required for HIL replay so
 * the same test case can be run repeatedly against the same firmware
 * instance without state contamination between runs.
 */
void estimator_reset(void);

/*
 * INCOMPLETE — single-phase signature will split into predict + update.
 * See WARNING above. Stub left intact to make the call graph link.
 *
 * imu1, imu2, baro may be NULL when the corresponding sensor is isolated.
 */
estimator_mode_t estimator_update(
    const imu_reading_t    *imu1,
    const imu_reading_t    *imu2,
    const baro_reading_t   *baro,
    const health_flags_t   *health,
    attitude_estimate_t    *out
);

#endif /* ESTIMATOR_H */
