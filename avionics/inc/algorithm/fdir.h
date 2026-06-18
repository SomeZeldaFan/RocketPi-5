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
 * Two-pass design (D050), interleaved around estimator_predict():
 *
 *   fdir_admit()  — PRE-predict. Absolute checks that need no model
 *                   prediction: staleness watchdog, bounds/NaN/saturation, and
 *                   the direct IMU-1 vs IMU-2 gyro cross-check. Writes the
 *                   PRELIMINARY health verdict. Runs before estimator_predict()
 *                   so a faulted gyro is caught before it drives the prediction.
 *
 *   fdir_gate()   — POST-predict. The innovation gate: a chi-squared test of
 *                   each correction measurement against the estimator's
 *                   predicted measurement (residual = z - h(x_pred)), threshold
 *                   CHI2_THRESHOLD_2DOF (TBD from LR-3). Restrict-only: it may
 *                   mark a channel unhealthy but never resurrects one that
 *                   admission already isolated.
 *
 * Architectural boundary (D024, D050, §4.4):
 *   - FDIR sets health flags; the estimator reads them and NEVER sets them.
 *   - FDIR consumes the estimator's predictions as a predicted_readings_t
 *     VALUE (passed in by orchestration); it never imports the estimator. This
 *     keeps the dependency one-way and FDIR the clean single authority.
 */

void fdir_init(void);

/*
 * Pass 1 — admission control (pre-predict). Absolute checks only:
 *   - staleness:  now - reading.timestamp_us > IMU_STALENESS_THRESHOLD_US
 *   - bounds:     accel / gyro / pressure outside the physical envelope, NaN, sat
 *   - gyro cross: IMU-1 vs IMU-2 gyro disagreement (needs no prediction)
 *   - mag:        the lone mag has no twin → no cross-check here; absolute checks
 *                 only (staleness, NaN/sat, |B| field-magnitude vs the known
 *                 Earth-field magnitude — attitude-independent, needs no model). (D060)
 * Writes the preliminary verdict to *health_out and the staleness fields of
 * *gate_out. The gyro must clear this pass before estimator_predict() uses it.
 */
void fdir_admit(
    const imu_reading_t  *imu1,
    const imu_reading_t  *imu2,
    const baro_reading_t *baro,
    const mag_reading_t  *mag,
    health_flags_t       *health_out,
    fdir_gate_result_t   *gate_out
);

/*
 * Pass 2 — innovation gate (post-predict). Forms residual = measured -
 * predicted for each correction measurement and runs the chi-squared test
 * against CHI2_THRESHOLD_2DOF, isolating the outlier. RESTRICT-ONLY on
 * *health_inout: may clear a health flag, never sets one true. Fills the
 * chi-squared / gate-open fields of *gate_out. predictions comes from
 * estimator_predict(); FDIR does not import the estimator. The mag is gated
 * analytically against predictions->mag_pred_ut (no second mag): chi2_mag plus
 * a dip-angle check — this is what makes the §5 mag-disturbance mode detectable
 * with a single sensor (D060).
 */
void fdir_gate(
    const imu_reading_t        *imu1,
    const imu_reading_t        *imu2,
    const baro_reading_t       *baro,
    const mag_reading_t        *mag,
    const predicted_readings_t *predictions,
    health_flags_t             *health_inout,
    fdir_gate_result_t         *gate_out
);

#endif /* FDIR_H */
