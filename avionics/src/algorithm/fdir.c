#include "algorithm/fdir.h"
#include "fault.h"           /* ASSERT */
#include <math.h>            /* isfinite, sqrtf, fabsf */
#include <stddef.h>          /* NULL */

/*
 * fdir.c — Fault Detection, Isolation, and Recovery. Design: D050 + D060 + D063.
 *
 * ROLE. FDIR is the SOLE writer of health_flags_t (architecture §4.4 invariant 1):
 * every other module reads health, none writes it. It runs in two passes that
 * bracket estimator_predict() (D050), and it NEVER imports the estimator — it
 * consumes the estimator's a-priori predictions as a predicted_readings_t VALUE
 * passed in by orchestration, keeping the dependency one-way and the health
 * authority uncontested.
 *
 *   fdir_admit()  — PRE-predict, TIER-1 (hard) checks. Absolute, model-free
 *                   verdicts that need no prediction: driver status, staleness,
 *                   physical-envelope/saturation, NaN/Inf, and the mag |B|
 *                   magnitude. A TIER-1 fault is physically impossible or a
 *                   broken sensor, so it isolates IMMEDIATELY and latches — no
 *                   benefit of the doubt. The gyro must clear admit before
 *                   estimator_predict() consumes it.
 *
 *   fdir_gate()   — POST-predict, TIER-2 (soft) checks. The innovation gate:
 *                   chi-squared of each correction measurement against its
 *                   predicted value. A TIER-2 fault is finite, in-range, fresh
 *                   but statistically suspect (a transient bump), so the
 *                   measurement is withheld THIS tick but the channel only
 *                   latches after FDIR_DEGRADE_COUNT consecutive failures.
 *                   RESTRICT-ONLY: it may close a channel, never re-admit one.
 *
 * SEVERITY MODEL (D063). Per channel FDIR holds a latch and a consecutive-fault
 * counter. health = (NOT latched) AND (no TIER-1 fault now) AND (gate open now).
 * TIER-1 latches at once; TIER-2 latches at the count; the IMU-vs-IMU gyro
 * cross-check is a DETECTOR ONLY and isolates nothing on its own (innocent
 * until proven guilty — a two-sensor disagreement cannot name the culprit; the
 * innovation gate does that when the fault also moves the accelerometer). A
 * latch clears ONLY via fdir_init() — i.e. a reset/reboot, the sole operator
 * intervention available (no soft re-admit command exists; that is future work).
 *
 * GATE STATISTIC (β, D063 amends D060). The gate cannot import P and is given no
 * innovation covariance to invert, so estimator_predict() exports the per-axis
 * a-priori innovation variances S_k = H_k P⁻ H_kᵀ + R alongside each prediction.
 * The gate forms chi2 = Σ_k (z_k − ẑ_k)² / S_k — a separate-from-filter
 * consistency test that is ADAPTIVE (S grows as covariance grows under
 * degradation, so the gate loosens correctly instead of false-tripping) and
 * needs NO matrix inversion, staying on the single-precision FPU (D062's win).
 * The accel/mag Jacobian H = [ĥ]ₓ is rank-2, so S already gives the radial
 * (magnitude) direction only R-variance and the two tangential directions P+R —
 * the 2-DOF direction structure falls out of the math (hence CHI2_THRESHOLD_2DOF).
 *
 * STALENESS REFERENCE. fdir_admit() is passed no clock and may not include a
 * hardware header (layer rule), so "now" is the NEWEST input timestamp this tick;
 * a channel's age is now − its timestamp. A wholly silent/dead bus is caught by
 * the status field, not this arithmetic. Ages use the wrap-safe (now − then) idiom.
 *
 * JPL POWER OF 10: fixed-size state, no heap, no recursion, bounded loops, ≥2
 * ASSERT per function, single-precision float only (isfinite/sqrtf/fabsf).
 */

/* =====================================================================
 * PROVISIONAL THRESHOLDS — D063.   *** MUST BE REPLACED ***
 * Order-of-magnitude placeholders so FDIR can run and be tested. The
 * statistical threshold comes from LR-3 (chi-squared design + detection
 * latency), the noise that feeds the gate from LR-2 (datasheet noise → R,
 * exported by the estimator), the |B|/dip reference from the LR-2 geomagnetic
 * model, and the envelope/debounce knobs from LR-3/CAL sensor characterisation.
 * Single source of truth: change them here, nowhere else. CHI2_THRESHOLD_2DOF
 * is the one shared constant and lives in avionics_types.h (the test plan and
 * BND-002 reference it by name).
 * ===================================================================== */
#define FDIR_ACCEL_ABS_MAX   (160.0f)   /* per-axis |accel| envelope ≈16 g  [m/s^2] */
#define FDIR_GYRO_ABS_MAX    (35.0f)    /* per-axis |gyro| envelope ≈2000 dps [rad/s] */
#define FDIR_PRESS_MIN_PA    (1000.0f)  /* min plausible pressure            [Pa]   */
#define FDIR_PRESS_MAX_PA    (120000.0f)/* max plausible pressure            [Pa]   */
/* Per-sensor staleness. IMUs run at the loop rate (5 ms = 5 ticks, the locked
 * IMU_STALENESS_THRESHOLD_US). The baro is decimated to AVIONICS_BARO_RATE_HZ
 * (50 Hz ≈ 20 ms period) so it needs a slower watchdog or a healthy reading
 * reads "stale" every tick; the mag rate awaits its task-7 driver. Provisional. */
#define FDIR_BARO_STALENESS_US ((uint32_t)100000U)  /* ≈5 × 20 ms baro period */
#define FDIR_MAG_STALENESS_US  ((uint32_t)100000U)  /* PLACEHOLDER until task-7 mag rate */
#define FDIR_B_EXPECTED_UT   (55.2f)    /* Earth-field magnitude (PLACEHOLDER) [uT] */
#define FDIR_B_TOL_FRAC      (0.30f)    /* |B| accepted band, ±fraction of expected */
#define FDIR_DIP_COS_EXP     (-0.6885f) /* cos∠(field, specific-force), attitude-invariant */
#define FDIR_DIP_COS_TOL     (0.10f)    /* dip cosine tolerance                     */
#define FDIR_GYRO_CROSS_MAX  (0.10f)    /* IMU1-vs-IMU2 gyro disagreement (detect)  [rad/s] */
#define FDIR_DEGRADE_COUNT   ((uint8_t)5U) /* consecutive TIER-2 faults → latch     */

/* =====================================================================
 * PERSISTENT STATE (D063). The four monitored channels share one index so the
 * per-channel latch/counter logic stays uniform. `latched` is the sticky
 * isolation, cleared ONLY by fdir_init() (a reset/reboot is the operator's
 * re-admit). `bad_count` counts CONSECUTIVE TIER-2 (soft, innovation) faults; it
 * resets on a clean tick and trips the latch at FDIR_DEGRADE_COUNT. TIER-1
 * (hard) faults latch directly and ignore the counter. This is the only state
 * FDIR keeps — admit/gate are otherwise pure functions of their inputs.
 * ===================================================================== */
enum { FDIR_CH_IMU1 = 0, FDIR_CH_IMU2, FDIR_CH_BARO, FDIR_CH_MAG, FDIR_NUM_CH };

typedef struct {
    bool    latched;
    uint8_t bad_count;
} fdir_channel_t;

static fdir_channel_t s_ch[FDIR_NUM_CH];

/* =====================================================================
 * Leaf predicates / vector ops. `static inline` — no -Wunused while a helper's
 * first consumer is still in a later section, and zero call cost. Trivial leaves,
 * exempt from the JPL Rule-5 assertion-density floor (audited in the hygiene pass).
 * ===================================================================== */
static inline bool vec3_finite(const float v[3])
{ return isfinite(v[0]) && isfinite(v[1]) && isfinite(v[2]); }

static inline bool vec3_exceeds(const float v[3], float lim)
{ return (fabsf(v[0]) > lim) || (fabsf(v[1]) > lim) || (fabsf(v[2]) > lim); }

/* Wrap-safe modular age: TIM2 rolls over at 2^32 µs ≈ 71.58 min, a known event,
 * so (now − then) on uint32_t is the correct elapsed-µs idiom (D053 cross-ref). */
static inline uint32_t age_us(uint32_t now, uint32_t then)
{ return now - then; }

static inline float vec3_dot(const float a[3], const float b[3])
{ return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]); }

static inline void vec3_sub(const float a[3], const float b[3], float o[3])
{ for (int i = 0; i < 3; ++i) { o[i] = a[i] - b[i]; } }

static inline float vec3_norm(const float v[3])
{ return sqrtf(vec3_dot(v, v)); }

void fdir_init(void)
{
    /* Clear every isolation latch and debounce counter: each channel boots
     * trusted (a sensor must PROVE unhealthy to be flagged). This is the ONLY
     * site that clears a latch, so a reset is the operator's sole re-admit path. */
    for (int i = 0; i < FDIR_NUM_CH; ++i) {
        s_ch[i].latched   = false;
        s_ch[i].bad_count = 0U;
    }
    ASSERT(!s_ch[FDIR_CH_IMU1].latched);              /* all channels admitted */
    ASSERT(s_ch[FDIR_NUM_CH - 1].bad_count == 0U);    /* counters cleared      */
}

/* =====================================================================
 * TIER-1 (admit, hard) predicates. Each returns true if the channel is
 * physically impossible or sensor-broken THIS tick — NULL/absent, driver
 * status ≠ OK, stale, non-finite, or out of the physical envelope (the mag
 * also checks |B| magnitude, its lone model-free invariant, D060). A NULL
 * channel yields the UINT32_MAX stale sentinel. These are absolute: no model,
 * no twin, no debounce — a true verdict latches immediately (apply_tier1).
 * ===================================================================== */
static bool imu_tier1_fault(const imu_reading_t *r, uint32_t now, uint32_t *stale_us)
{
    ASSERT(stale_us != NULL);
    ASSERT(FDIR_ACCEL_ABS_MAX > 0.0f);
    ASSERT(FDIR_GYRO_ABS_MAX > 0.0f);
    if (r == NULL) { *stale_us = UINT32_MAX; return true; }
    *stale_us = age_us(now, r->timestamp_us);
    return (r->status != IMU_OK)
        || (*stale_us > IMU_STALENESS_THRESHOLD_US)
        || !vec3_finite(r->accel_mss) || !vec3_finite(r->gyro_rads)
        || vec3_exceeds(r->accel_mss, FDIR_ACCEL_ABS_MAX)
        || vec3_exceeds(r->gyro_rads, FDIR_GYRO_ABS_MAX);
}

static bool baro_tier1_fault(const baro_reading_t *r, uint32_t now, uint32_t *stale_us)
{
    ASSERT(stale_us != NULL);
    ASSERT(FDIR_PRESS_MAX_PA > FDIR_PRESS_MIN_PA);
    if (r == NULL) { *stale_us = UINT32_MAX; return true; }
    *stale_us = age_us(now, r->timestamp_us);
    return (r->status != BARO_OK)
        || (*stale_us > FDIR_BARO_STALENESS_US)
        || !isfinite(r->pressure_pa)
        || (r->pressure_pa < FDIR_PRESS_MIN_PA) || (r->pressure_pa > FDIR_PRESS_MAX_PA);
}

static bool mag_tier1_fault(const mag_reading_t *r, uint32_t now, uint32_t *stale_us)
{
    ASSERT(stale_us != NULL);
    ASSERT(FDIR_B_TOL_FRAC < 1.0f);
    if (r == NULL) { *stale_us = UINT32_MAX; return true; }
    *stale_us = age_us(now, r->timestamp_us);
    if ((r->status != MAG_OK) || (*stale_us > FDIR_MAG_STALENESS_US)
        || !vec3_finite(r->field_ut)) { return true; }
    const float bmag = vec3_norm(r->field_ut);   /* model-free |B| invariant */
    return (bmag < (FDIR_B_EXPECTED_UT * (1.0f - FDIR_B_TOL_FRAC)))
        || (bmag > (FDIR_B_EXPECTED_UT * (1.0f + FDIR_B_TOL_FRAC)));
}

/* Apply a TIER-1 verdict to one channel: a hard fault latches at once (D063);
 * health this tick is simply "not latched" (a prior latch keeps it isolated). */
static bool apply_tier1(int ch, bool fault_now)
{
    ASSERT(ch >= 0);
    ASSERT(ch < FDIR_NUM_CH);
    if (fault_now) { s_ch[ch].latched = true; }
    const bool healthy = !s_ch[ch].latched;
    if (fault_now) { ASSERT(!healthy); }   /* a hard fault cannot leave it healthy */
    return healthy;
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
    ASSERT(health_out != NULL);
    ASSERT(gate_out   != NULL);

    /* Tick reference: the newest timestamp among the present readings. No clock
     * is passed in and FDIR may not include a hardware header (layer rule), so a
     * channel's age is measured against the freshest sibling; a wholly silent bus
     * is caught by its status field, not this arithmetic (A2). */
    uint32_t now = 0U;
    if ((imu1 != NULL) && (imu1->timestamp_us > now)) { now = imu1->timestamp_us; }
    if ((imu2 != NULL) && (imu2->timestamp_us > now)) { now = imu2->timestamp_us; }
    if ((baro != NULL) && (baro->timestamp_us > now)) { now = baro->timestamp_us; }
    if ((mag  != NULL) && (mag->timestamp_us  > now)) { now = mag->timestamp_us;  }

    /* TIER-1 (hard) verdicts → immediate latch → preliminary health. Each
     * predicate also writes that channel's staleness into gate_out (baro has no
     * staleness field in fdir_gate_result_t, so it lands in a local). */
    uint32_t baro_stale;   /* computed for the verdict; no baro field in gate_out */
    health_out->imu1_healthy = apply_tier1(FDIR_CH_IMU1,
        imu_tier1_fault(imu1, now, &gate_out->imu1_stale_us));
    health_out->imu2_healthy = apply_tier1(FDIR_CH_IMU2,
        imu_tier1_fault(imu2, now, &gate_out->imu2_stale_us));
    health_out->baro_healthy = apply_tier1(FDIR_CH_BARO,
        baro_tier1_fault(baro, now, &baro_stale));
    health_out->mag_healthy  = apply_tier1(FDIR_CH_MAG,
        mag_tier1_fault(mag,  now, &gate_out->mag_stale_us));

    /* Gyro cross-check — DETECTOR ONLY (A1/D063). Two sensors cannot name the
     * culprit, so this isolates NOTHING: it raises a soft operator caution and
     * lets the innovation gate isolate the IMU whose accel also moves. Skipped
     * unless both gyros are present and finite (a non-finite gyro already
     * latched the channel TIER-1, and feeding it here would be meaningless). */
    bool disagree = false;
    if ((imu1 != NULL) && (imu2 != NULL)
        && vec3_finite(imu1->gyro_rads) && vec3_finite(imu2->gyro_rads)) {
        float d[3];
        vec3_sub(imu1->gyro_rads, imu2->gyro_rads, d);
        disagree = vec3_exceeds(d, FDIR_GYRO_CROSS_MAX);
    }
    gate_out->gyro_disagree = disagree;
}

/* =====================================================================
 * TIER-2 (gate, soft) helpers.
 * ===================================================================== */

/* Normalised innovation chi-squared for a vector channel: Σ_k (z_k − pred_k)²/S_k
 * using the estimator-exported a-priori variances (β, D063). No matrix inverse. */
static float chi2_vec(const float meas[3], const float pred[3], const float var[3])
{
    float chi2 = 0.0f;
    for (int k = 0; k < 3; ++k) {
        ASSERT(var[k] > 0.0f);              /* estimator guarantees S_k > 0 */
        const float resid = meas[k] - pred[k];
        chi2 += (resid * resid) / var[k];
    }
    ASSERT(isfinite(chi2));
    return chi2;
}

/* Model-free dip check: the angle between the field and the specific-force
 * (gravity reaction) is attitude-INVARIANT, so cos∠(B, a) must match the known
 * Earth dip whatever the orientation — this catches a disturbance that rotates
 * the field without changing |B|, which the chi2-vs-prediction test could miss
 * if the model itself were being pulled. Needs a healthy accel reference; a
 * degenerate (near-zero) vector cannot define an angle, so it does not gate. */
static bool dip_ok(const float field[3], const float accel[3])
{
    const float nb = vec3_norm(field);
    const float na = vec3_norm(accel);
    ASSERT(isfinite(nb));
    ASSERT(isfinite(na));
    const float denom = nb * na;
    if (denom < 1.0e-6f) { return true; }   /* degenerate vector → no angle to test */
    const float c = vec3_dot(field, accel) / denom;
    ASSERT(isfinite(c));
    return fabsf(c - FDIR_DIP_COS_EXP) <= FDIR_DIP_COS_TOL;
}

/* Apply a TIER-2 verdict to one channel (caller guarantees it is currently
 * healthy). Restrict-only with debounce (D063): a clean tick resets the
 * consecutive count; a failed tick increments it and latches at
 * FDIR_DEGRADE_COUNT. Health this tick = (NOT latched) AND (gate open now), so a
 * single soft outlier is withheld immediately but only sustained badness condemns
 * the channel — and a latch never clears here (only fdir_init does). */
static bool gate_apply(int ch, bool gate_open_now)
{
    ASSERT(ch >= 0);
    ASSERT(ch < FDIR_NUM_CH);
    if (gate_open_now) {
        s_ch[ch].bad_count = 0U;            /* clean tick resets the consecutive count */
    } else {
        /* Once latched the gate stops calling this channel, so bad_count freezes
         * at FDIR_DEGRADE_COUNT and never overflows — no separate guard needed. */
        s_ch[ch].bad_count = (uint8_t)(s_ch[ch].bad_count + 1U);
        if (s_ch[ch].bad_count >= FDIR_DEGRADE_COUNT) { s_ch[ch].latched = true; }
    }
    const bool healthy = (!s_ch[ch].latched) && gate_open_now;
    if (s_ch[ch].latched) { ASSERT(!healthy); }   /* latched ⇒ unhealthy */
    return healthy;
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
    ASSERT(predictions  != NULL);
    ASSERT(health_inout != NULL);

    /* Baro carries no prediction (correction-only, D050/D062), so there is no
     * innovation to gate — baro health stays exactly as admit set it. */
    (void)baro;

    /* IMU-1 accelerometer innovation gate (TIER-2). Only a channel admit left
     * healthy AND with a reading present is gated; an isolated channel keeps
     * gate_open=false and its health untouched (restrict-only). */
    if (health_inout->imu1_healthy && (imu1 != NULL)) {
        const float chi2 = chi2_vec(imu1->accel_mss, predictions->imu1_accel_pred_mss,
                                    predictions->imu1_accel_innov_var);
        const bool open = (chi2 <= CHI2_THRESHOLD_2DOF);
        gate_out->chi2_imu1 = chi2;
        gate_out->imu1_gate_open = open;
        health_inout->imu1_healthy = gate_apply(FDIR_CH_IMU1, open);
    } else {
        gate_out->chi2_imu1 = 0.0f;
        gate_out->imu1_gate_open = false;
    }

    /* IMU-2 accelerometer innovation gate (TIER-2) — symmetric. */
    if (health_inout->imu2_healthy && (imu2 != NULL)) {
        const float chi2 = chi2_vec(imu2->accel_mss, predictions->imu2_accel_pred_mss,
                                    predictions->imu2_accel_innov_var);
        const bool open = (chi2 <= CHI2_THRESHOLD_2DOF);
        gate_out->chi2_imu2 = chi2;
        gate_out->imu2_gate_open = open;
        health_inout->imu2_healthy = gate_apply(FDIR_CH_IMU2, open);
    } else {
        gate_out->chi2_imu2 = 0.0f;
        gate_out->imu2_gate_open = false;
    }

    /* Magnetometer innovation gate (TIER-2): chi2 vs the predicted field AND the
     * model-free dip check. The dip needs a healthy accel reference (prefer IMU-1,
     * else IMU-2); with neither, the chi2 test alone decides. This pair is what
     * makes the §5 magnetic-disturbance mode detectable with a single sensor (D060). */
    if (health_inout->mag_healthy && (mag != NULL)) {
        const float chi2 = chi2_vec(mag->field_ut, predictions->mag_pred_ut,
                                    predictions->mag_innov_var);
        const imu_reading_t *aref =
            (health_inout->imu1_healthy && (imu1 != NULL)) ? imu1 :
            ((health_inout->imu2_healthy && (imu2 != NULL)) ? imu2 : NULL);
        const bool dip = (aref != NULL) ? dip_ok(mag->field_ut, aref->accel_mss) : true;
        const bool open = (chi2 <= CHI2_THRESHOLD_2DOF) && dip;
        gate_out->chi2_mag = chi2;
        gate_out->mag_gate_open = open;
        health_inout->mag_healthy = gate_apply(FDIR_CH_MAG, open);
    } else {
        gate_out->chi2_mag = 0.0f;
        gate_out->mag_gate_open = false;
    }
}
