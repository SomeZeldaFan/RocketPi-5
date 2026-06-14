#ifndef ESTIMATOR_H
#define ESTIMATOR_H

#include "avionics_types.h"

/*
 * estimator.h — attitude state estimator (EKF or equivalent)
 *
 * Layer: algorithm. Maintains the attitude belief in two phases per tick
 * (D050), bracketing the FDIR innovation gate:
 *
 *   estimator_predict()  — propagate the a-priori state forward using the
 *                          ADMITTED gyro, and export the predicted measurements
 *                          (predicted_readings_t) the gate needs. Runs before
 *                          fdir_gate(); needs no health verdict of its own.
 *   estimator_update()   — correct (Kalman update) using only the sensors FDIR
 *                          flagged healthy; produces the posterior estimate.
 *
 * NULL-pointer semantics for sensor inputs:
 *   - An isolated sensor is passed as NULL (never zero-filled data — NULL is
 *     unambiguous). In _predict(), a gyro isolated by fdir_admit() is NULL; in
 *     _update(), any channel isolated by fdir_gate() is NULL.
 *
 * Mode transitions are determined by which sensor pointers are non-NULL plus
 * the FDIR health flags:
 *   - both IMUs non-NULL  → EST_MODE_DUAL_IMU
 *   - IMU-1 only          → EST_MODE_IMU1_ONLY
 *   - IMU-2 only          → EST_MODE_IMU2_ONLY
 *   - both NULL           → EST_MODE_DEAD_RECKONING (propagate with last rates)
 *   - unrecoverable       → EST_MODE_FAULT
 *
 * Covariance grows as sensors are lost; the output's covariance field is the
 * system's numerically expressed confidence — downstream consumers propagate
 * or display it; ignoring it is a reviewer finding (§4.4 invariant 3).
 *
 * Per D035: the fusion algorithm is user-implemented (EKF or Madgwick/Mahony
 * equivalent). No proprietary on-chip fusion. JPL Power of 10 compliant.
 */

void estimator_init(void);

/*
 * Reset the estimator to a known clean state. Used between deterministic
 * replay runs so the same input sequence reproduces the same output.
 */
void estimator_reset(void);

/*
 * Predict phase. Propagate the a-priori state using the admitted gyro and write
 * the predicted measurements to *predictions_out for fdir_gate(). imu1 / imu2
 * may be NULL when fdir_admit() isolated that gyro. Baro is not an input: it
 * drives no propagation (it is a correction measurement only).
 */
void estimator_predict(
    const imu_reading_t  *imu1,
    const imu_reading_t  *imu2,
    predicted_readings_t *predictions_out
);

/*
 * Correct phase. Kalman update using only healthy channels; imu1, imu2, baro
 * are NULL when isolated. Fills *out (attitude, covariance, mode) and returns
 * the estimator_mode_t selected this tick.
 */
estimator_mode_t estimator_update(
    const imu_reading_t  *imu1,
    const imu_reading_t  *imu2,
    const baro_reading_t *baro,
    const health_flags_t *health,
    attitude_estimate_t  *out
);

#endif /* ESTIMATOR_H */
