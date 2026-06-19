#include "algorithm/fdir.h"
#include "check.h"
#include <string.h>

/*
 * test_fdir.c — dev-PC unit tests for the FDIR module (Task 3, D050/D060/D063).
 *
 * METHODOLOGY (mirrors test_estimator.c). FDIR is tested in isolation: the
 * predicted_readings_t the gate consumes is built BY HAND here — including the
 * per-axis innovation variances — so the test sets each channel's chi-squared
 * exactly without ever running the estimator. Synthetic readings are constructed
 * directly; a deterministic LCG supplies replay-safe noise. Each test calls
 * fdir_init() first to clear FDIR's persistent latch/counter state.
 *
 * Severity model under test (D063): TIER-1 (admit) faults isolate IMMEDIATELY
 * and latch; TIER-2 (gate) faults withhold the measurement this tick and latch
 * only after FDIR_DEGRADE_COUNT consecutive failures; a latch clears ONLY via
 * fdir_init() (reset). The gyro cross-check is a DETECTOR ONLY (A1). FDIR is the
 * sole writer of health_flags_t and is restrict-only in the gate.
 *
 * Constants local to fdir.c (FDIR_*_STALENESS_US, FDIR_DEGRADE_COUNT, the
 * envelopes) are deliberately not visible here, so tests use values comfortably
 * past them (e.g. "very stale", "≥8 consecutive") rather than the exact knobs.
 * CHI2_THRESHOLD_2DOF and IMU_STALENESS_THRESHOLD_US are public and used exactly.
 */

/* g = standard gravity; a level static IMU reads specific force (0,0,-g). */
#define G_MSS    (9.80665f)
#define GATE_VAR (0.25f)        /* innovation variance per axis → chi2 = Σ r²/0.25 */

/* ---- synthetic builders ------------------------------------------------ */

/* A consistent, healthy, LEVEL reading set at timestamp t. */
static void make_level(imu_reading_t *i1, imu_reading_t *i2,
                       baro_reading_t *b, mag_reading_t *m, uint32_t t)
{
    memset(i1, 0, sizeof(*i1)); memset(i2, 0, sizeof(*i2));
    memset(b, 0, sizeof(*b));   memset(m, 0, sizeof(*m));
    i1->accel_mss[2] = -G_MSS;  i2->accel_mss[2] = -G_MSS;
    i1->timestamp_us = t;       i2->timestamp_us = t;
    i1->status = IMU_OK;        i2->status = IMU_OK;
    b->pressure_pa = 101325.0f; b->timestamp_us = t; b->status = BARO_OK;
    m->field_ut[0] = 40.0f; m->field_ut[1] = 1.5f; m->field_ut[2] = 38.0f;
    m->timestamp_us = t;        m->status = MAG_OK;
}

/* Predictions consistent with the level set; innov variance = var on every axis. */
static void make_pred(predicted_readings_t *p, float var, uint32_t t)
{
    memset(p, 0, sizeof(*p));
    p->imu1_accel_pred_mss[2] = -G_MSS; p->imu2_accel_pred_mss[2] = -G_MSS;
    p->mag_pred_ut[0] = 40.0f; p->mag_pred_ut[1] = 1.5f; p->mag_pred_ut[2] = 38.0f;
    for (int k = 0; k < 3; ++k) {
        p->imu1_accel_innov_var[k] = var;
        p->imu2_accel_innov_var[k] = var;
        p->mag_innov_var[k]        = var;
    }
    p->timestamp_us = t;
}

static void make_health(health_flags_t *h)
{
    memset(h, 0, sizeof(*h));
    h->imu1_healthy = true; h->imu2_healthy = true;
    h->baro_healthy = true; h->mag_healthy  = true;
}

/* Deterministic uniform noise in [-amp, +amp] (LCG → replay-safe). */
static uint32_t s_rng = 12345U;
static float noise(float amp)
{
    s_rng = (s_rng * 1103515245U) + 12345U;
    return amp * ((((float)((s_rng >> 16) & 0x7FFFU)) / 32768.0f) - 0.5f) * 2.0f;
}

/* ===== Admit — TIER-1 (hard, immediate latch) ========================== */

/* TEST-FDR-001/002/003 + BND-001: staleness per channel, with the IMU
 * exact-boundary (threshold, ±1 tick) since IMU_STALENESS_THRESHOLD_US is public. */
static void test_admit_staleness(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    fdir_gate_result_t g;

    /* IMU1 stale past threshold → isolated; IMU2/baro fresh → healthy. */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i1.timestamp_us = t - (IMU_STALENESS_THRESHOLD_US + 1U);
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu1_healthy, "FDR-001 IMU1 stale isolated");
    CHECK(h.imu2_healthy,  "FDR-001 IMU2 fresh stays healthy");
    CHECK(g.imu1_stale_us == (IMU_STALENESS_THRESHOLD_US + 1U), "FDR-001 stale_us reported");

    /* IMU2 stale (symmetry, FDR-002). */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i2.timestamp_us = t - (IMU_STALENESS_THRESHOLD_US + 1U);
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu2_healthy, "FDR-002 IMU2 stale isolated");
    CHECK(h.imu1_healthy,  "FDR-002 IMU1 stays healthy");

    /* BND-001: age == threshold is NOT stale; threshold-1 not stale. */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i1.timestamp_us = t - IMU_STALENESS_THRESHOLD_US;          /* age == threshold */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(h.imu1_healthy, "BND-001 age==threshold not stale");
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i1.timestamp_us = t - (IMU_STALENESS_THRESHOLD_US - 1U);   /* age threshold-1 */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(h.imu1_healthy, "BND-001 age==threshold-1 not stale");

    /* FDR-003: baro very stale (its threshold is slower, value not visible). */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    b.timestamp_us = t - 1000000U;   /* 1 s old → far past any baro threshold */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.baro_healthy, "FDR-003 baro stale isolated");
    CHECK(h.imu1_healthy && h.imu2_healthy, "FDR-003 IMUs unaffected");
}

/* TEST-FDR-004/005/006: physical-envelope (saturation) faults isolate at once. */
static void test_admit_bounds(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    fdir_gate_result_t g;

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i1.accel_mss[0] = 200.0f;                         /* FDR-004 */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu1_healthy, "FDR-004 accel out of range isolated");
    CHECK(h.imu2_healthy,  "FDR-004 IMU2 unaffected");

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i1.gyro_rads[1] = 100.0f;                         /* FDR-005 (>~35 rad/s) */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu1_healthy, "FDR-005 gyro out of range isolated");

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    b.pressure_pa = 0.0f;                             /* FDR-006 low */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.baro_healthy, "FDR-006 pressure=0 isolated");

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    b.pressure_pa = 200000.0f;                        /* FDR-006 high */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.baro_healthy, "FDR-006 pressure=200k isolated");
}

/* The R1 hard exception: a non-finite sample never reaches the estimator. */
static void test_admit_nonfinite(void)
{
    const uint32_t t = 1000000U;
    const float nan_v = (float)NAN, inf_v = (float)INFINITY;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    fdir_gate_result_t g;

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i1.accel_mss[0] = nan_v;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu1_healthy, "NaN accel isolated immediately");

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i2.gyro_rads[2] = inf_v;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu2_healthy, "Inf gyro isolated immediately");

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    b.pressure_pa = nan_v;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.baro_healthy, "NaN pressure isolated immediately");

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    m.field_ut[1] = inf_v;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.mag_healthy, "Inf field isolated immediately");
}

/* Driver-reported status ≠ OK isolates even with otherwise-valid data. */
static void test_admit_status(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    fdir_gate_result_t g;

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i1.status = IMU_BUS_ERROR;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu1_healthy, "IMU bus error isolated");

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    m.status = MAG_TIMEOUT;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.mag_healthy, "mag timeout isolated");
}

/* TEST-FDR-016 + NULL handling on every channel (absent sensor → isolated). */
static void test_admit_null(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    fdir_gate_result_t g;

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    fdir_admit(&i1, &i2, NULL, &m, &h, &g);          /* FDR-016 NULL baro */
    CHECK(!h.baro_healthy, "FDR-016 NULL baro isolated, no crash");
    CHECK(h.imu1_healthy && h.imu2_healthy, "FDR-016 IMUs unaffected");

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    fdir_admit(NULL, NULL, &b, NULL, &h, &g);
    CHECK(!h.imu1_healthy && !h.imu2_healthy && !h.mag_healthy, "NULL imu/mag isolated");
    CHECK(g.imu1_stale_us == UINT32_MAX, "NULL channel stale_us = sentinel");
}

/* mag |B| magnitude band: the magnitude variant of the §5 disturbance. */
static void test_admit_mag_magnitude(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    fdir_gate_result_t g;

    /* In-band field passes. */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(h.mag_healthy, "in-band |B| healthy");

    /* |B| far too high (nearby metal pulling field up). */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    m.field_ut[0] = 200.0f; m.field_ut[1] = 0.0f; m.field_ut[2] = 0.0f;  /* |B|=200 */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.mag_healthy, "|B| too high isolated");

    /* |B| far too low. */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    m.field_ut[0] = 5.0f; m.field_ut[1] = 0.0f; m.field_ut[2] = 0.0f;    /* |B|=5 */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.mag_healthy, "|B| too low isolated");
}

/* now-reference: each channel in turn carries the newest timestamp (covers the
 * now-selection branches), and the all-equal case. */
static void test_now_reference(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    fdir_gate_result_t g;

    /* baro newest: a slightly-old IMU1 must read its age against baro's stamp. */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    b.timestamp_us = t + 10000U;            /* baro is newest */
    i1.timestamp_us = t;                     /* age = 10000 > threshold → stale */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu1_healthy, "now from baro: IMU1 measured stale vs newest");

    /* mag newest. */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    m.timestamp_us = t + 10000U;
    i2.timestamp_us = t;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu2_healthy, "now from mag: IMU2 measured stale vs newest");

    /* all equal: nothing stale. */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(h.imu1_healthy && h.imu2_healthy && h.baro_healthy && h.mag_healthy,
          "all-equal timestamps: all healthy");
}

/* A1: gyro cross-check is a DETECTOR ONLY — sets gyro_disagree, isolates nobody. */
static void test_gyro_crosscheck(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    fdir_gate_result_t g;

    /* Disagree beyond threshold → flag set, BOTH IMUs still healthy. */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i1.gyro_rads[2] = 0.5f;                  /* |Δω| = 0.5 > 0.10 */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(g.gyro_disagree, "A1 gyro disagreement detected");
    CHECK(h.imu1_healthy && h.imu2_healthy, "A1 disagreement isolates neither IMU");

    /* Agree → flag clear. */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!g.gyro_disagree, "A1 gyros agree → no flag");

    /* One IMU absent → cross-check skipped (no false flag). */
    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i1.gyro_rads[2] = 0.5f;
    fdir_admit(&i1, NULL, &b, &m, &h, &g);
    CHECK(!g.gyro_disagree, "A1 cross-check skipped when a gyro is absent");
}

/* ===== Gate — TIER-2 (soft, debounced) ================================= */

/* TEST-FDR-007: measurements consistent with predictions → no false positive. */
static void test_gate_consistent(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    predicted_readings_t p; fdir_gate_result_t g;

    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(g.imu1_gate_open && g.imu2_gate_open, "FDR-007 IMU gates open on agreement");
    CHECK(g.mag_gate_open, "FDR-007 mag gate open on agreement");
    CHECK(h.imu1_healthy && h.imu2_healthy && h.mag_healthy, "FDR-007 all stay healthy");
    CHECK(g.chi2_imu1 <= CHI2_THRESHOLD_2DOF, "FDR-007 chi2 below threshold");
}

/* TEST-FDR-008/009: one IMU's accel disagrees with the prediction → its gate
 * closes that tick and the other IMU is retained. */
static void test_gate_imu_outlier(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    predicted_readings_t p; fdir_gate_result_t g;

    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    i1.accel_mss[0] = 5.0f;          /* in-range but residual 5 → chi2 = 100 */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(!g.imu1_gate_open, "FDR-008 IMU1 gate closes on innovation outlier");
    CHECK(g.imu2_gate_open,  "FDR-008 IMU2 gate stays open");
    CHECK(!h.imu1_healthy,   "FDR-008 IMU1 measurement withheld");
    CHECK(h.imu2_healthy,    "FDR-008 IMU2 retained");
    CHECK(g.chi2_imu1 > CHI2_THRESHOLD_2DOF, "FDR-008 chi2 over threshold");

    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    i2.accel_mss[1] = 5.0f;          /* FDR-009 symmetric */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(!g.imu2_gate_open, "FDR-009 IMU2 gate closes");
    CHECK(g.imu1_gate_open,  "FDR-009 IMU1 gate stays open");
    CHECK(!h.imu2_healthy && h.imu1_healthy, "FDR-009 IMU2 isolated, IMU1 retained");
}

/* §5 magnetic-disturbance — the depth-axis headline. A single mag, no twin, is
 * still caught two ways: chi2 vs the IMU-predicted field (wrong DIRECTION) and the
 * model-free dip check (wrong angle to gravity). Sustained → mag isolated, IMUs
 * unaffected so yaw degrades gracefully. Also exercises the dip accel-reference
 * selection (IMU1 → IMU2 → none). */
static void test_gate_mag_disturbance(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    predicted_readings_t p; fdir_gate_result_t g;

    /* (a) Wrong-direction field, |B| still in band → chi2 fails. */
    fdir_init();
    for (int k = 0; k < 8; ++k) {
        make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
        m.field_ut[0] = 1.5f; m.field_ut[1] = -40.0f; m.field_ut[2] = 38.0f;  /* |B|≈55 */
        fdir_admit(&i1, &i2, &b, &m, &h, &g);
        fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    }
    CHECK(!g.mag_gate_open, "§5 disturbed-direction mag gate closed (chi2)");
    CHECK(!h.mag_healthy,   "§5 mag isolated after sustained disturbance");
    CHECK(h.imu1_healthy && h.imu2_healthy, "§5 IMUs unaffected → yaw degrades gracefully");

    /* (b) Dip-only failure: pred == meas (chi2 passes) but field aligned with
     * gravity (cos = -1, far from the expected dip) → dip check closes the gate. */
    fdir_init();
    for (int k = 0; k < 8; ++k) {
        make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
        m.field_ut[0] = 0.0f; m.field_ut[1] = 0.0f; m.field_ut[2] = 55.0f;
        p.mag_pred_ut[0] = 0.0f; p.mag_pred_ut[1] = 0.0f; p.mag_pred_ut[2] = 55.0f;
        fdir_admit(&i1, &i2, &b, &m, &h, &g);
        fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    }
    CHECK(!h.mag_healthy, "§5 dip-only failure isolates mag");

    /* (c) Dip falls back to IMU2 when IMU1 is isolated (one tick is enough to
     * close the gate). */
    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    i1.status = IMU_BUS_ERROR;                       /* isolate IMU1 (TIER-1) */
    m.field_ut[0] = 0.0f; m.field_ut[1] = 0.0f; m.field_ut[2] = 55.0f;
    p.mag_pred_ut[0] = 0.0f; p.mag_pred_ut[1] = 0.0f; p.mag_pred_ut[2] = 55.0f;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(!g.mag_gate_open, "§5 dip uses IMU2 reference when IMU1 isolated");

    /* (d) Both IMUs out → no accel reference; chi2-only gate still catches a
     * wrong-direction field. */
    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    i1.status = IMU_BUS_ERROR; i2.status = IMU_BUS_ERROR;
    m.field_ut[0] = 1.5f; m.field_ut[1] = -40.0f; m.field_ut[2] = 38.0f;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(!h.imu1_healthy && !h.imu2_healthy, "§5(d) both IMUs out");
    CHECK(!g.mag_gate_open, "§5(d) chi2-only mag gate (no accel ref) still closes");
}

/* TEST-BND-002: chi2 exactly at, just below, and just above the threshold. The
 * single-axis residual is sized so chi2 = (resid²/var) lands where intended. */
static void test_gate_boundary(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    predicted_readings_t p; fdir_gate_result_t g;

    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    i1.accel_mss[0] = sqrtf((CHI2_THRESHOLD_2DOF - 0.1f) * GATE_VAR);   /* chi2 just below */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(g.imu1_gate_open, "BND-002 chi2 just below threshold → open");

    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    i1.accel_mss[0] = sqrtf(CHI2_THRESHOLD_2DOF * GATE_VAR);            /* chi2 == threshold */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(g.imu1_gate_open, "BND-002 chi2 == threshold → open (<=)");

    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    i1.accel_mss[0] = sqrtf((CHI2_THRESHOLD_2DOF + 0.1f) * GATE_VAR);   /* chi2 just above */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(!g.imu1_gate_open, "BND-002 chi2 just above threshold → closed");
}

/* ===== Severity / latch semantics (D063) =============================== */

/* TIER-1 latches at once and persists; TIER-2 withholds a single outlier but
 * does NOT latch (recovers); a sustained TIER-2 fault latches and never
 * auto-recovers (TEST-FDR-011 reframed). */
static void test_severity_latch(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    predicted_readings_t p; fdir_gate_result_t g;

    /* TIER-1 single sample → immediate latch, stays isolated on clean data. */
    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); i1.accel_mss[0] = 200.0f;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu1_healthy, "TIER-1 single sample isolates immediately");
    make_level(&i1, &i2, &b, &m, t); make_health(&h);          /* clean again */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu1_healthy, "TIER-1 latched: stays isolated on clean data");

    /* TIER-2 single outlier → withheld this tick, NOT latched → recovers. */
    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    i1.accel_mss[0] = 5.0f;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(!h.imu1_healthy, "TIER-2 single outlier withheld this tick");
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(h.imu1_healthy, "TIER-2 single spike NOT latched → recovers next clean tick");

    /* TIER-2 sustained (>= the count) → latched → no auto-recovery. */
    fdir_init();
    for (int k = 0; k < 8; ++k) {
        make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
        i1.accel_mss[0] = 5.0f;
        fdir_admit(&i1, &i2, &b, &m, &h, &g);
        fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    }
    CHECK(!h.imu1_healthy, "TIER-2 sustained → latched");
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(!h.imu1_healthy, "FDR-011 latched channel does NOT auto-recover (clean data)");
}

/* fdir_init() clears latches — the reset/operator re-admit path. */
static void test_init_clears(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    predicted_readings_t p; fdir_gate_result_t g;

    fdir_init();
    for (int k = 0; k < 8; ++k) {
        make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
        i1.accel_mss[0] = 5.0f;
        fdir_admit(&i1, &i2, &b, &m, &h, &g);
        fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    }
    CHECK(!h.imu1_healthy, "latched before reset");

    fdir_init();                                              /* operator reset */
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(h.imu1_healthy, "fdir_init clears latch → channel re-admitted");
}

/* Restrict-only: the gate never re-admits a channel admit isolated, even with a
 * perfect reading. */
static void test_restrict_only(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    predicted_readings_t p; fdir_gate_result_t g;

    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    i1.timestamp_us = t - (IMU_STALENESS_THRESHOLD_US + 1U);   /* TIER-1 isolate */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu1_healthy, "restrict: admit isolated IMU1");
    i1.timestamp_us = t;                                       /* now a perfect reading */
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(!h.imu1_healthy, "restrict-only: gate never re-admits an isolated channel");
    CHECK(!g.imu1_gate_open, "restrict: isolated channel gate_open stays false");
}

/* TEST-FDR-010: detection latency. TIER-1 isolates within one tick; TIER-2 within
 * the (provisional, LR-3) consecutive-count bound — here proven ≤ 8 ticks. */
static void test_detection_latency(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    predicted_readings_t p; fdir_gate_result_t g;

    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h);
    b.pressure_pa = 0.0f;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.baro_healthy, "FDR-010 TIER-1 fault isolated within 1 tick");

    fdir_init();
    for (int k = 0; k < 8; ++k) {
        make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
        i1.accel_mss[0] = 5.0f;
        fdir_admit(&i1, &i2, &b, &m, &h, &g);
        fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    }
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(!h.imu1_healthy, "FDR-010 TIER-2 fault latched within the latency bound (<=8 ticks)");
}

/* TEST-FDR-015: false-positive rate on clean noisy data stays well under 5%. */
static void test_false_positive(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    predicted_readings_t p; fdir_gate_result_t g;
    const int N = 10000;
    int closures = 0;

    fdir_init();
    s_rng = 777U;
    for (int k = 0; k < N; ++k) {
        make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
        for (int a = 0; a < 3; ++a) {
            i1.accel_mss[a] += noise(0.05f); i2.accel_mss[a] += noise(0.05f);
            m.field_ut[a]   += noise(0.2f);
        }
        fdir_admit(&i1, &i2, &b, &m, &h, &g);
        fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
        if (!g.imu1_gate_open || !g.imu2_gate_open || !g.mag_gate_open) { ++closures; }
    }
    const float fp = (float)closures / (float)N;
    printf("  [fp] gate false-closure rate = %.4f over %d ticks\n", (double)fp, N);
    CHECK(fp < 0.05f, "FDR-015 false-positive rate < 5% on clean noisy data");
}

/* TEST-FDR-013 / D026: a baro fault does not propagate to the IMUs or mag. */
static void test_baro_isolation(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    fdir_gate_result_t g;

    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h);
    b.pressure_pa = 0.0f;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.baro_healthy, "FDR-013 baro isolated");
    CHECK(h.imu1_healthy && h.imu2_healthy && h.mag_healthy,
          "FDR-013 no cross-channel propagation");
}

/* ===== Coverage edges (compound-condition operands) ==================== */

/* Operands the headline tests only exercise one way: baro driver-status fault,
 * mag staleness with status OK, the imu1 now-loop edge, and a non-finite imu1
 * gyro reaching the (skipped) cross-check. */
static void test_admit_coverage_edges(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    fdir_gate_result_t g;

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    b.status = BARO_BUS_ERROR;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.baro_healthy, "baro bus error isolated (status operand)");

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    m.timestamp_us = t - 1000000U;          /* status OK, but stale */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.mag_healthy, "mag stale (status OK) isolated (staleness operand)");

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i1.timestamp_us = 0U;                    /* not the newest → now-loop edge */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu1_healthy, "imu1 ts=0 vs fresh siblings → stale");
    CHECK(h.imu2_healthy,  "imu2 unaffected");

    fdir_init(); make_level(&i1, &i2, &b, &m, t); make_health(&h);
    i1.gyro_rads[0] = (float)NAN;            /* cross-check must skip, not flag */
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    CHECK(!h.imu1_healthy,  "imu1 NaN gyro isolated (TIER-1)");
    CHECK(!g.gyro_disagree, "cross-check skipped on non-finite imu1 gyro");
}

/* Degenerate dip reference: a zero accel (with zero prediction so the IMU passes
 * its own gate) is chosen as aref, exercising the |a|≈0 skip in dip_ok. */
static void test_gate_degenerate_ref(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    predicted_readings_t p; fdir_gate_result_t g;

    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    i1.accel_mss[2] = 0.0f; p.imu1_accel_pred_mss[2] = 0.0f;
    fdir_admit(&i1, &i2, &b, &m, &h, &g);
    fdir_gate(&i1, &i2, &b, &m, &p, &h, &g);
    CHECK(h.imu1_healthy, "zero-accel IMU1 passes its zero-prediction gate");
    CHECK(h.mag_healthy,  "dip skipped on degenerate accel reference");
}

/* B5: the gate is NULL-safe even if a channel is flagged healthy but absent (an
 * inconsistent state admit prevents — here forced to drive the defensive paths). */
static void test_gate_null_safety(void)
{
    const uint32_t t = 1000000U;
    health_flags_t h; imu_reading_t i1, i2; baro_reading_t b; mag_reading_t m;
    predicted_readings_t p; fdir_gate_result_t g;

    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    fdir_gate(NULL, NULL, &b, &m, &p, &h, &g);     /* IMUs absent, mag present */
    CHECK(!g.imu1_gate_open && !g.imu2_gate_open, "NULL IMUs → gates reported closed");
    CHECK(g.mag_gate_open, "mag still gated (aref falls through to none → chi2-only)");

    fdir_init();
    make_level(&i1, &i2, &b, &m, t); make_health(&h); make_pred(&p, GATE_VAR, t);
    fdir_gate(&i1, &i2, &b, NULL, &p, &h, &g);     /* mag absent */
    CHECK(!g.mag_gate_open, "NULL mag → mag gate reported closed");
}

int main(void)
{
    test_admit_staleness();
    test_admit_bounds();
    test_admit_nonfinite();
    test_admit_status();
    test_admit_null();
    test_admit_mag_magnitude();
    test_now_reference();
    test_gyro_crosscheck();
    test_gate_consistent();
    test_gate_imu_outlier();
    test_gate_mag_disturbance();
    test_gate_boundary();
    test_severity_latch();
    test_init_clears();
    test_restrict_only();
    test_detection_latency();
    test_false_positive();
    test_baro_isolation();
    test_admit_coverage_edges();
    test_gate_degenerate_ref();
    test_gate_null_safety();
    return CHECK_REPORT("fdir");
}
