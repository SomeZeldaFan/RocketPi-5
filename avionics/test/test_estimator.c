#include "algorithm/estimator.h"
#include "check.h"
#include <string.h>

/* ===== Synthetic input helpers =====================================
 * Build the sensor readings a STATIONARY, LEVEL vehicle produces: the
 * accelerometer reads specific force (-g, body-up at level), gyros read zero,
 * the magnetometer reads the reference field. These are exactly what the
 * estimator predicts at identity attitude, so a consistent stream must hold
 * the estimate level. Tilted/rotating generators arrive in F2. */
static void make_level(imu_reading_t *a, imu_reading_t *b,
                       mag_reading_t *m, uint32_t t)
{
    memset(a, 0, sizeof(*a));
    memset(b, 0, sizeof(*b));
    memset(m, 0, sizeof(*m));
    a->accel_mss[2] = -9.80665f;  b->accel_mss[2] = -9.80665f;
    a->timestamp_us = t;          b->timestamp_us = t;
    a->status = IMU_OK;           b->status = IMU_OK;
    m->field_ut[0] = 40.0f; m->field_ut[1] = 1.5f; m->field_ut[2] = 38.0f;
    m->timestamp_us = t; m->status = MAG_OK;
}

/* TEST-EST-001 (static convergence to level): fed consistent level data, the
 * estimate stays at (0,0,0) with bounded, positive covariance and DUAL_IMU. */
static void test_static_level(void)
{
    health_flags_t h;
    memset(&h, 0, sizeof(h));
    h.imu1_healthy = true; h.imu2_healthy = true; h.mag_healthy = true;

    imu_reading_t a, b; mag_reading_t m;
    predicted_readings_t pred; attitude_estimate_t est;
    memset(&est, 0, sizeof(est));

    estimator_reset();
    for (uint32_t t = 1; t <= 2000U; ++t) {
        make_level(&a, &b, &m, t);
        estimator_predict(&a, &b, &pred);
        estimator_update(&a, &b, &m, &h, &est);
    }
    CHECK_NEAR(est.roll_rad,  0.0f, 0.02f, "static: roll level");
    CHECK_NEAR(est.pitch_rad, 0.0f, 0.02f, "static: pitch level");
    CHECK_NEAR(est.yaw_rad,   0.0f, 0.02f, "static: yaw level");
    CHECK(est.covariance[0] > 0.0f, "static: attitude covariance positive");
    CHECK(est.mode == EST_MODE_DUAL_IMU, "static: DUAL_IMU mode");
}

/* ===== Independent attitude generator (F2) ==========================
 * Build sensor data for an ARBITRARY known attitude via a DCM derived from
 * Euler angles — a code path INDEPENDENT of the estimator's quaternion math,
 * so a sign error in the estimator cannot be masked by matching test data.
 * ZYX (yaw, pitch, roll) body-from-nav, the convention the estimator extracts. */
static void dcm_from_euler(float roll, float pitch, float yaw, float R[3][3])
{
    const float cr = cosf(roll),  sr = sinf(roll);
    const float cp = cosf(pitch), sp = sinf(pitch);
    const float cy = cosf(yaw),   sy = sinf(yaw);
    R[0][0]= cp*cy;            R[0][1]= cp*sy;            R[0][2]= -sp;
    R[1][0]= sr*sp*cy - cr*sy; R[1][1]= sr*sp*sy + cr*cy; R[1][2]= sr*cp;
    R[2][0]= cr*sp*cy + sr*sy; R[2][1]= cr*sp*sy - sr*cy; R[2][2]= cr*cp;
}
static void rot3(const float R[3][3], const float v[3], float o[3])
{
    for (int i = 0; i < 3; ++i) { o[i] = R[i][0]*v[0] + R[i][1]*v[1] + R[i][2]*v[2]; }
}
static void make_attitude(float roll, float pitch, float yaw, const float gyro[3],
                          imu_reading_t *a, imu_reading_t *b, mag_reading_t *m,
                          uint32_t t)
{
    const float g_nav[3] = {0.0f, 0.0f, -9.80665f};
    const float bmag[3]  = {40.0f, 1.5f, 38.0f};
    float R[3][3], acc[3], fld[3];
    dcm_from_euler(roll, pitch, yaw, R);
    rot3(R, g_nav, acc);
    rot3(R, bmag, fld);
    memset(a, 0, sizeof(*a)); memset(b, 0, sizeof(*b)); memset(m, 0, sizeof(*m));
    for (int i = 0; i < 3; ++i) {
        a->accel_mss[i] = acc[i]; b->accel_mss[i] = acc[i];
        a->gyro_rads[i] = gyro[i]; b->gyro_rads[i] = gyro[i];
        m->field_ut[i]  = fld[i];
    }
    a->timestamp_us = t; b->timestamp_us = t; a->status = IMU_OK; b->status = IMU_OK;
    m->timestamp_us = t; m->status = MAG_OK;
}

/* TEST-EST-001 (convergence to a non-zero tilt): held at 5 deg roll, 10 deg
 * pitch, the filter must converge there from identity (gravity-observable). */
static void test_converge_tilt(void)
{
    health_flags_t h; memset(&h, 0, sizeof(h));
    h.imu1_healthy = true; h.imu2_healthy = true; h.mag_healthy = true;
    const float gyro0[3] = {0.0f, 0.0f, 0.0f};
    imu_reading_t a, b; mag_reading_t m;
    predicted_readings_t pred; attitude_estimate_t est; memset(&est, 0, sizeof(est));

    estimator_reset();
    for (uint32_t t = 1; t <= 2000U; ++t) {
        make_attitude(0.0873f, 0.1745f, 0.0f, gyro0, &a, &b, &m, t);  /* 5,10,0 deg */
        estimator_predict(&a, &b, &pred);
        estimator_update(&a, &b, &m, &h, &est);
    }
    CHECK_NEAR(est.roll_rad,  0.0873f, 0.02f, "tilt: roll -> 5 deg");
    CHECK_NEAR(est.pitch_rad, 0.1745f, 0.02f, "tilt: pitch -> 10 deg");
    CHECK_NEAR(est.yaw_rad,   0.0f,    0.05f, "tilt: yaw stays 0");
}

/* Held at 30 deg heading (roll=pitch=0): only the magnetometer can pull yaw
 * there, so this pins the yaw sign that the (yaw=0) tilt test could not. */
static void test_converge_yaw(void)
{
    health_flags_t h; memset(&h, 0, sizeof(h));
    h.imu1_healthy = true; h.imu2_healthy = true; h.mag_healthy = true;
    const float gyro0[3] = {0.0f, 0.0f, 0.0f};
    imu_reading_t a, b; mag_reading_t m;
    predicted_readings_t pred; attitude_estimate_t est; memset(&est, 0, sizeof(est));

    estimator_reset();
    for (uint32_t t = 1; t <= 3000U; ++t) {
        make_attitude(0.0f, 0.0f, 0.5236f, gyro0, &a, &b, &m, t);   /* yaw 30 deg */
        estimator_predict(&a, &b, &pred);
        estimator_update(&a, &b, &m, &h, &est);
    }
    CHECK_NEAR(est.yaw_rad,   0.5236f, 0.03f, "yaw -> 30 deg");
    CHECK_NEAR(est.roll_rad,  0.0f,    0.03f, "yaw test: roll stays 0");
    CHECK_NEAR(est.pitch_rad, 0.0f,    0.03f, "yaw test: pitch stays 0");
}

/* TEST-EST-003 (analytic, DCM-independent): hand-computed readings for a pure
 * 90 deg yaw. At roll=pitch=0, gravity is unchanged (0,0,-g); the field
 * (N,E,D)=(40,1.5,38) maps to body (1.5,-40,38) since body-x->east, body-y->
 * south. The estimator must read yaw = +pi/2 — verifies the generator too. */
static void test_analytic_yaw90(void)
{
    health_flags_t h; memset(&h, 0, sizeof(h));
    h.imu1_healthy = true; h.imu2_healthy = true; h.mag_healthy = true;
    imu_reading_t a, b; mag_reading_t m;
    predicted_readings_t pred; attitude_estimate_t est; memset(&est, 0, sizeof(est));

    estimator_reset();
    for (uint32_t t = 1; t <= 3000U; ++t) {
        memset(&a, 0, sizeof(a)); memset(&b, 0, sizeof(b)); memset(&m, 0, sizeof(m));
        a.accel_mss[2] = -9.80665f; b.accel_mss[2] = -9.80665f;
        a.status = IMU_OK; b.status = IMU_OK; a.timestamp_us = t; b.timestamp_us = t;
        m.field_ut[0] = 1.5f; m.field_ut[1] = -40.0f; m.field_ut[2] = 38.0f;
        m.status = MAG_OK; m.timestamp_us = t;
        estimator_predict(&a, &b, &pred);
        estimator_update(&a, &b, &m, &h, &est);
    }
    CHECK_NEAR(est.yaw_rad, 1.5708f, 0.03f, "analytic: yaw = +90 deg");
}

/* TEST-EST-002 (steady-rotation tracking): a constant yaw rate about the
 * vertical. The estimate must follow the integrated heading and report the
 * body rate — the FIRST exercise of the quaternion propagation path, since
 * every prior test held gyro = 0. */
static void test_track_yaw_rate(void)
{
    health_flags_t h; memset(&h, 0, sizeof(h));
    h.imu1_healthy = true; h.imu2_healthy = true; h.mag_healthy = true;
    const float rate = 0.2f;                  /* rad/s about down (body z) */
    const float gyro[3] = {0.0f, 0.0f, rate};
    const float dt = 0.001f;
    imu_reading_t a, b; mag_reading_t m;
    predicted_readings_t pred; attitude_estimate_t est; memset(&est, 0, sizeof(est));

    estimator_reset();
    float yaw_true = 0.0f;
    for (uint32_t t = 1; t <= 2000U; ++t) {
        yaw_true += rate * dt;
        make_attitude(0.0f, 0.0f, yaw_true, gyro, &a, &b, &m, t);
        estimator_predict(&a, &b, &pred);
        estimator_update(&a, &b, &m, &h, &est);
    }
    CHECK_NEAR(est.yaw_rad,       yaw_true, 0.02f, "track: yaw follows rate");
    CHECK_NEAR(est.yaw_rate_rads, rate,     0.02f, "track: body yaw rate reported");
    CHECK_NEAR(est.roll_rad,      0.0f,     0.02f, "track: roll stays 0");
    CHECK_NEAR(est.pitch_rad,     0.0f,     0.02f, "track: pitch stays 0");
}

/* TEST-EST-004/005/006/012: degraded modes via the NULL contract. A channel
 * the gate isolated arrives NULL; the estimator reports the matching mode and
 * keeps running without crashing. */
static void test_degraded_modes(void)
{
    health_flags_t h; memset(&h, 0, sizeof(h));
    h.imu1_healthy = true; h.imu2_healthy = true; h.mag_healthy = true;
    const float g0[3] = {0.0f, 0.0f, 0.0f};
    imu_reading_t a, b; mag_reading_t m;
    predicted_readings_t pred; attitude_estimate_t est; memset(&est, 0, sizeof(est));

    /* IMU2 isolated -> IMU1_ONLY, still tracks level on IMU1 + mag. */
    estimator_reset();
    for (uint32_t t = 1; t <= 500U; ++t) {
        make_attitude(0.0f, 0.0f, 0.0f, g0, &a, &b, &m, t);
        estimator_predict(&a, NULL, &pred);
        estimator_update(&a, NULL, &m, &h, &est);
    }
    CHECK(est.mode == EST_MODE_IMU1_ONLY, "mode IMU1_ONLY when imu2 NULL");
    CHECK_NEAR(est.pitch_rad, 0.0f, 0.03f, "IMU1_ONLY stays level");

    /* IMU1 isolated -> IMU2_ONLY. */
    estimator_reset();
    for (uint32_t t = 1; t <= 500U; ++t) {
        make_attitude(0.0f, 0.0f, 0.0f, g0, &a, &b, &m, t);
        estimator_predict(NULL, &b, &pred);
        estimator_update(NULL, &b, &m, &h, &est);
    }
    CHECK(est.mode == EST_MODE_IMU2_ONLY, "mode IMU2_ONLY when imu1 NULL");

    /* Both isolated -> DEAD_RECKONING (propagate on last rate, no corrections). */
    estimator_reset();
    for (uint32_t t = 1; t <= 100U; ++t) {
        estimator_predict(NULL, NULL, &pred);
        estimator_update(NULL, NULL, NULL, &h, &est);
    }
    CHECK(est.mode == EST_MODE_DEAD_RECKONING, "mode DEAD_RECKONING when both NULL");
    CHECK(!isnan(est.yaw_rad), "DEAD_RECKONING produces finite output");
}

/* TEST-EST-007/008: covariance never silently reaches zero (the PD-floor holds
 * under sustained updates), and it GROWS when the sensor that bounds a state is
 * removed — here, yaw uncertainty climbs once the magnetometer is dropped. */
static void test_covariance(void)
{
    health_flags_t h; memset(&h, 0, sizeof(h));
    h.imu1_healthy = true; h.imu2_healthy = true; h.mag_healthy = true;
    const float g0[3] = {0.0f, 0.0f, 0.0f};
    imu_reading_t a, b; mag_reading_t m;
    predicted_readings_t pred; attitude_estimate_t est; memset(&est, 0, sizeof(est));

    estimator_reset();
    for (uint32_t t = 1; t <= 2000U; ++t) {
        make_attitude(0.0f, 0.0f, 0.0f, g0, &a, &b, &m, t);
        estimator_predict(&a, &b, &pred);
        estimator_update(&a, &b, &m, &h, &est);
    }
    for (int i = 0; i < 9; ++i) {
        CHECK(est.covariance[i] > 0.0f, "cov diagonal positive after convergence");
    }
    const float yaw_cov_bounded = est.covariance[2];

    /* Drop the magnetometer: yaw now has no correction and must grow. */
    for (uint32_t t = 2001; t <= 4000U; ++t) {
        make_attitude(0.0f, 0.0f, 0.0f, g0, &a, &b, &m, t);
        estimator_predict(&a, &b, &pred);
        estimator_update(&a, &b, NULL, &h, &est);
    }
    CHECK(est.covariance[2] > yaw_cov_bounded, "yaw covariance grows without mag");
}

/* TEST-EST-009/010/011: reset returns a known clean state, so the SAME input
 * sequence reproduces the SAME output bit-for-bit (deterministic replay), and a
 * reset wipes prior state (no cross-run contamination). */
static void test_reset_replay(void)
{
    health_flags_t h; memset(&h, 0, sizeof(h));
    h.imu1_healthy = true; h.imu2_healthy = true; h.mag_healthy = true;
    const float g0[3] = {0.0f, 0.0f, 0.0f};
    imu_reading_t a, b; mag_reading_t m;
    predicted_readings_t pred; attitude_estimate_t e1, e2;
    memset(&e1, 0, sizeof(e1)); memset(&e2, 0, sizeof(e2));

    /* Run a tilt scenario twice with a reset between -> outputs must be identical. */
    estimator_reset();
    for (uint32_t t = 1; t <= 1000U; ++t) {
        make_attitude(0.0873f, 0.1745f, 0.0f, g0, &a, &b, &m, t);
        estimator_predict(&a, &b, &pred); estimator_update(&a, &b, &m, &h, &e1);
    }
    estimator_reset();
    for (uint32_t t = 1; t <= 1000U; ++t) {
        make_attitude(0.0873f, 0.1745f, 0.0f, g0, &a, &b, &m, t);
        estimator_predict(&a, &b, &pred); estimator_update(&a, &b, &m, &h, &e2);
    }
    CHECK(e1.roll_rad == e2.roll_rad && e1.pitch_rad == e2.pitch_rad &&
          e1.yaw_rad == e2.yaw_rad, "replay: identical attitude (deterministic)");
    CHECK(e1.covariance[0] == e2.covariance[0], "replay: identical covariance");

    /* Leave a 30 deg yaw, reset, run one level tick -> reset must wipe it. */
    estimator_reset();
    for (uint32_t t = 1; t <= 2000U; ++t) {
        make_attitude(0.0f, 0.0f, 0.5236f, g0, &a, &b, &m, t);
        estimator_predict(&a, &b, &pred); estimator_update(&a, &b, &m, &h, &e1);
    }
    estimator_reset();
    make_attitude(0.0f, 0.0f, 0.0f, g0, &a, &b, &m, 1);
    estimator_predict(&a, &b, &pred); estimator_update(&a, &b, &m, &h, &e1);
    CHECK_NEAR(e1.yaw_rad, 0.0f, 0.05f, "reset wipes prior yaw");
}

/* The D054 headline: a constant gyro yaw-bias is held in check by the
 * magnetometer, but drifts unbounded once the mag is gone. The truth is level
 * the whole time; the gyro lies by 0.05 rad/s in yaw. Only the mag differs
 * between the two runs, so the contrast IS the magnetometer's value. */
static void test_yaw_drift_demo(void)
{
    health_flags_t h; memset(&h, 0, sizeof(h));
    h.imu1_healthy = true; h.imu2_healthy = true; h.mag_healthy = true;
    const float gyro_bias[3] = {0.0f, 0.0f, 0.05f};   /* yaw-rate bias [rad/s] */
    imu_reading_t a, b; mag_reading_t m;
    predicted_readings_t pred; attitude_estimate_t est; memset(&est, 0, sizeof(est));

    /* Run A: bias present, mag HEALTHY -> mag bounds yaw. */
    estimator_reset();
    for (uint32_t t = 1; t <= 5000U; ++t) {
        make_attitude(0.0f, 0.0f, 0.0f, gyro_bias, &a, &b, &m, t);
        estimator_predict(&a, &b, &pred);
        estimator_update(&a, &b, &m, &h, &est);
    }
    const float yaw_with_mag = fabsf(est.yaw_rad);

    /* Run B: same bias, mag DROPPED -> yaw drifts on the uncorrected bias. */
    estimator_reset();
    for (uint32_t t = 1; t <= 3000U; ++t) {
        make_attitude(0.0f, 0.0f, 0.0f, gyro_bias, &a, &b, &m, t);
        estimator_predict(&a, &b, &pred);
        estimator_update(&a, &b, NULL, &h, &est);
    }
    const float yaw_without_mag = fabsf(est.yaw_rad);

    printf("  [yaw drift] with mag = %.4f rad, without = %.4f rad\n",
           (double)yaw_with_mag, (double)yaw_without_mag);
    CHECK(yaw_with_mag < 0.05f, "yaw bounded with mag (< ~3 deg)");
    CHECK(yaw_without_mag > 0.10f, "yaw drifts without mag (> ~6 deg)");
    CHECK(yaw_without_mag > 3.0f * yaw_with_mag, "mag dramatically curbs yaw drift");
}

/* D061: the differential-bias pseudo-measurement makes the per-IMU gyro biases
 * observable. With opposite yaw-axis biases the blend cancels (attitude stays
 * true), and the difference signal drives IMU-1's z-bias variance below what a
 * single IMU (no difference, yaw-bias seen only weakly via the mag) reaches. */
static void test_bias_observability(void)
{
    health_flags_t h; memset(&h, 0, sizeof(h));
    h.imu1_healthy = true; h.imu2_healthy = true; h.mag_healthy = true;
    const float zero[3] = {0.0f, 0.0f, 0.0f};
    imu_reading_t a, b; mag_reading_t m;
    predicted_readings_t pred; attitude_estimate_t est; memset(&est, 0, sizeof(est));

    estimator_reset();
    for (uint32_t t = 1; t <= 5000U; ++t) {
        make_attitude(0.0f, 0.0f, 0.0f, zero, &a, &b, &m, t);
        a.gyro_rads[2] = 0.03f; b.gyro_rads[2] = -0.03f;   /* opposite yaw bias */
        estimator_predict(&a, &b, &pred);
        estimator_update(&a, &b, &m, &h, &est);
    }
    const float bz_dual  = est.covariance[5];      /* IMU-1 z-bias error variance */
    const float yaw_dual = fabsf(est.yaw_rad);

    estimator_reset();
    for (uint32_t t = 1; t <= 5000U; ++t) {
        make_attitude(0.0f, 0.0f, 0.0f, zero, &a, &b, &m, t);
        a.gyro_rads[2] = 0.03f;
        estimator_predict(&a, NULL, &pred);
        estimator_update(&a, NULL, &m, &h, &est);
    }
    const float bz_single = est.covariance[5];

    printf("  [bias obs] IMU1 z-bias var: dual=%.2e single=%.2e; dual yaw=%.4f\n",
           (double)bz_dual, (double)bz_single, (double)yaw_dual);
    CHECK(yaw_dual < 0.02f, "attitude true despite per-IMU bias disagreement");
    CHECK(bz_dual < bz_single, "differential makes per-IMU z-bias more observable");
}

/* Deterministic uniform noise in [-amp, +amp] (LCG -> replay-safe). */
static uint32_t s_rng = 12345U;
static float noise(float amp)
{
    s_rng = s_rng * 1103515245U + 12345U;
    return amp * (((float)((s_rng >> 16) & 0x7FFFU) / 32768.0f) - 0.5f) * 2.0f;
}

/* F10 (stress): with realistic noise on every channel, the estimate converges
 * to and stays near the held truth (5,10,30 deg). With provisional R this is a
 * structural check (bounded, not divergent), not a tuned-accuracy claim. */
static void test_noise_robustness(void)
{
    health_flags_t h; memset(&h, 0, sizeof(h));
    h.imu1_healthy = true; h.imu2_healthy = true; h.mag_healthy = true;
    const float zero[3] = {0.0f, 0.0f, 0.0f};
    imu_reading_t a, b; mag_reading_t m;
    predicted_readings_t pred; attitude_estimate_t est; memset(&est, 0, sizeof(est));

    s_rng = 12345U;
    estimator_reset();
    float max_pitch_err = 0.0f;
    for (uint32_t t = 1; t <= 6000U; ++t) {
        make_attitude(0.0873f, 0.1745f, 0.5236f, zero, &a, &b, &m, t);
        for (int i = 0; i < 3; ++i) {
            a.accel_mss[i] += noise(0.2f);  b.accel_mss[i] += noise(0.2f);
            a.gyro_rads[i] += noise(0.01f); b.gyro_rads[i] += noise(0.01f);
            m.field_ut[i]  += noise(1.0f);
        }
        estimator_predict(&a, &b, &pred);
        estimator_update(&a, &b, &m, &h, &est);
        if (t > 2000U) {
            const float e = fabsf(est.pitch_rad - 0.1745f);
            if (e > max_pitch_err) { max_pitch_err = e; }
        }
    }
    printf("  [noise] roll=%.4f pitch=%.4f yaw=%.4f; maxPitchErr=%.4f\n",
           (double)est.roll_rad, (double)est.pitch_rad, (double)est.yaw_rad,
           (double)max_pitch_err);
    CHECK_NEAR(est.roll_rad,  0.0873f, 0.05f, "noisy: roll near truth");
    CHECK_NEAR(est.pitch_rad, 0.1745f, 0.05f, "noisy: pitch near truth");
    CHECK_NEAR(est.yaw_rad,   0.5236f, 0.10f, "noisy: yaw near truth");
    CHECK(max_pitch_err < 0.10f, "noisy: pitch error bounded post-convergence");
}

/* Coverage: estimator_init (the rest of the suite uses estimator_reset), the
 * both-IMU-isolated-but-mag-present dead-reckoning timestamp path, and the
 * near-vertical asinf clamp that guards the Euler extraction against a NaN. */
static void test_init_and_edges(void)
{
    health_flags_t h; memset(&h, 0, sizeof(h));
    h.imu1_healthy = true; h.imu2_healthy = true; h.mag_healthy = true;
    const float g0[3] = {0.0f, 0.0f, 0.0f};
    imu_reading_t a, b; mag_reading_t m;
    predicted_readings_t pred; attitude_estimate_t est; memset(&est, 0, sizeof(est));

    /* estimator_init path. */
    estimator_init();
    make_attitude(0.0f, 0.0f, 0.0f, g0, &a, &b, &m, 1);
    estimator_predict(&a, &b, &pred);
    estimator_update(&a, &b, &m, &h, &est);
    CHECK_NEAR(est.roll_rad, 0.0f, 0.05f, "init: level after estimator_init");

    /* Both IMUs isolated, mag still present -> DEAD_RECKONING, mag timestamp. */
    estimator_reset();
    for (uint32_t t = 1; t <= 50U; ++t) {
        make_attitude(0.0f, 0.0f, 0.0f, g0, &a, &b, &m, t);
        estimator_predict(NULL, NULL, &pred);
        estimator_update(NULL, NULL, &m, &h, &est);
    }
    CHECK(est.mode == EST_MODE_DEAD_RECKONING, "DEAD_RECKONING with mag present");

    /* Near-vertical pitch + noise (both signs): the asinf-domain clamp must keep
     * pitch finite when rounding pushes sin(pitch) past +/-1. */
    s_rng = 999U;
    const float pitches[2] = {1.5707f, -1.5707f};   /* ~ +90 and -90 deg */
    for (int s = 0; s < 2; ++s) {
        estimator_reset();
        for (uint32_t t = 1; t <= 1500U; ++t) {
            make_attitude(0.0f, pitches[s], 0.0f, g0, &a, &b, &m, t);
            for (int i = 0; i < 3; ++i) {
                a.accel_mss[i] += noise(0.3f); b.accel_mss[i] += noise(0.3f);
            }
            estimator_predict(&a, &b, &pred);
            estimator_update(&a, &b, &m, &h, &est);
            CHECK(!isnan(est.pitch_rad), "near-vertical: pitch finite (asinf clamp)");
        }
    }
}

int main(void)
{
    test_static_level();
    test_converge_tilt();
    test_converge_yaw();
    test_analytic_yaw90();
    test_track_yaw_rate();
    test_degraded_modes();
    test_covariance();
    test_reset_replay();
    test_yaw_drift_demo();
    test_bias_observability();
    test_noise_robustness();
    test_init_and_edges();
    return CHECK_REPORT("estimator");
}
