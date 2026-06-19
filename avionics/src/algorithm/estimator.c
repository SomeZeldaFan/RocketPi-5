#include "algorithm/estimator.h"
#include "fault.h"           /* ASSERT */
#include <math.h>            /* sqrtf, isnan */
#include <stddef.h>          /* NULL */

/*
 * estimator.c — attitude estimator (error-state EKF). Design: D061 + D062.
 *
 * ALGORITHM. Multiplicative (error-state) EKF. The NOMINAL state is a unit
 * quaternion plus two gyro biases; the filter tracks a 9-element ERROR state
 *   δx = [ δθ(3) | δb1(3) | δb2(3) ]
 *        δθ  — small-angle attitude error (body axes), injected multiplicatively
 *        δb1 — IMU-1 gyro-bias error      δb2 — IMU-2 gyro-bias error
 * and a 9×9 error covariance P. Per-IMU biases (not one shared bias) are what
 * make true dual-IMU fusion honest: the gyro DIFFERENCE observes b1−b2 directly
 * (D061). P's diagonal is exported verbatim as attitude_estimate_t.covariance[9].
 *
 * Two phases per tick, bracketing the FDIR gate (D050):
 *   estimator_predict() — propagate nominal + grow P; export predicted_readings_t.
 *   estimator_update()  — sequential-scalar Kalman corrections on healthy channels.
 * Tick order: admit → predict → gate → update.
 *
 * CONVENTIONS (fixed here to prevent sign bugs):
 *   - Hamilton quaternion q = (w, x, y, z), body←nav rotation, unit norm.
 *   - NED navigation frame. Reference vectors: gravity g_nav = +Z (down),
 *     geomagnetic field B_nav from the bench-site model.
 *   - Units: rad, rad/s, m/s², µT, seconds. Single-precision float ONLY — the
 *     target FPU is single-precision; double would be soft-emulated (D062).
 *
 * JPL POWER OF 10: fixed-size matrices, no heap, no recursion, bounded loops,
 * ≥2 assertions per function. Numerical robustness against single precision is
 * explicit — Joseph-form covariance updates, symmetrization, a diagonal PD-floor
 * (P never silently reaches zero), quaternion renormalization every step.
 *
 * TUNING CONSTANTS (R, Q, P0, g_nav, B_nav) are PROVISIONAL — see the marked
 * block below; they MUST be replaced with LR-2 / geomagnetic-model values
 * before this code is considered final (D062).
 */

/* =====================================================================
 * PROVISIONAL TUNING CONSTANTS — D062.   *** MUST BE REPLACED ***
 * Order-of-magnitude placeholders so the filter can run for tests. Replace
 * with LR-2-derived noise (R*, Q*, P0*) and a geomagnetic-model field
 * (B_NAV_*) before this code is demo-final / ships. Single source of truth:
 * change them here, nowhere else.
 * ===================================================================== */
#define EST_P0_ATT_VAR  (1.0e-1f)  /* init attitude-error var   [rad^2]      */
#define EST_P0_BIAS_VAR (1.0e-4f)  /* init gyro-bias var        [(rad/s)^2]  */
#define EST_Q_ATT       (1.0e-6f)  /* attitude process noise / step          */
#define EST_Q_BIAS      (1.0e-9f)  /* gyro-bias random walk / step           */
#define EST_R_ACC       (4.0e-2f)  /* accel meas var / axis     [(m/s^2)^2]  */
#define EST_R_MAG       (2.5e-1f)  /* mag meas var / axis       [uT^2]       */
#define EST_R_DIFF      (1.0e-4f)  /* gyro-difference meas var  [(rad/s)^2]  */
#define EST_PD_FLOOR    (1.0e-9f)  /* covariance diagonal floor              */
#define EST_G_NAV_D     (9.80665f) /* gravity, NED +Down        [m/s^2]      */
#define EST_B_NAV_N     (40.0f)    /* geomag North (PLACEHOLDER) [uT]        */
#define EST_B_NAV_E     (1.5f)     /* geomag East  (PLACEHOLDER) [uT]        */
#define EST_B_NAV_D     (38.0f)    /* geomag Down  (PLACEHOLDER) [uT]        */

/* Nominal state: unit quaternion + per-IMU gyro biases (D061). The 9-element
 * error state and its covariance are reckoned ABOUT this nominal. */
typedef struct {
    float q[4];   /* Hamilton (w,x,y,z), body<-nav, unit norm */
    float b1[3];  /* IMU-1 gyro bias [rad/s] */
    float b2[3];  /* IMU-2 gyro bias [rad/s] */
} est_state_t;

static est_state_t s_nom;       /* nominal state */
static float       s_P[9][9];   /* 9x9 error covariance: [dtheta|db1|db2] */
static float       s_omega[3];  /* last blended body rate [rad/s] (dead-reckon hold) */

void estimator_init(void)
{
    estimator_reset();
    ASSERT(s_nom.q[0] > 0.5f);   /* identity quaternion seeded */
    ASSERT(s_P[0][0] > 0.0f);    /* covariance is positive */
}

void estimator_reset(void)
{
    /* Deterministic known state (TEST-EST-009/011): identity attitude, zero
     * bias, diagonal priors. Off-diagonal covariance starts at zero. */
    s_nom.q[0] = 1.0f; s_nom.q[1] = 0.0f; s_nom.q[2] = 0.0f; s_nom.q[3] = 0.0f;
    for (int i = 0; i < 3; ++i) { s_nom.b1[i] = 0.0f; s_nom.b2[i] = 0.0f; }

    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) { s_P[i][j] = 0.0f; }
    }
    s_P[0][0] = s_P[1][1] = s_P[2][2] = EST_P0_ATT_VAR;          /* attitude */
    for (int i = 3; i < 9; ++i) { s_P[i][i] = EST_P0_BIAS_VAR; } /* both biases */
    for (int i = 0; i < 3; ++i) { s_omega[i] = 0.0f; }          /* dead-reckon rate */

    ASSERT(s_P[0][0] > 0.0f);    /* attitude prior set */
    ASSERT(s_P[8][8] > 0.0f);    /* bias prior set */
}

/* =====================================================================
 * Fixed-size linear-algebra primitives. `static inline` — zero call cost
 * and no -Wunused warning while a helper's first consumer is still ahead.
 * vec3 operands are float[3]. Trivial leaf helpers, exempt from the JPL
 * Rule-5 assertion-density floor (audited in the hygiene pass).
 * ===================================================================== */
static inline void  v_add  (const float a[3], const float b[3], float o[3])
{ for (int i = 0; i < 3; ++i) { o[i] = a[i] + b[i]; } }
static inline void  v_sub  (const float a[3], const float b[3], float o[3])
{ for (int i = 0; i < 3; ++i) { o[i] = a[i] - b[i]; } }
static inline void  v_scale(const float a[3], float s, float o[3])
{ for (int i = 0; i < 3; ++i) { o[i] = a[i] * s; } }
static inline void  v_cross(const float a[3], const float b[3], float o[3])
{ o[0]=a[1]*b[2]-a[2]*b[1]; o[1]=a[2]*b[0]-a[0]*b[2]; o[2]=a[0]*b[1]-a[1]*b[0]; }

/* Quaternion: Hamilton (w,x,y,z), unit norm, encodes the body<-nav attitude. */
static inline void q_normalize(float q[4])
{
    float n = sqrtf(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    ASSERT(n > 1.0e-6f);                  /* refuse to normalize a null quat */
    for (int i = 0; i < 4; ++i) { q[i] /= n; }
}
static inline void q_mul(const float a[4], const float b[4], float o[4])
{
    o[0] = a[0]*b[0] - a[1]*b[1] - a[2]*b[2] - a[3]*b[3];
    o[1] = a[0]*b[1] + a[1]*b[0] + a[2]*b[3] - a[3]*b[2];
    o[2] = a[0]*b[2] - a[1]*b[3] + a[2]*b[0] + a[3]*b[1];
    o[3] = a[0]*b[3] + a[1]*b[2] - a[2]*b[1] + a[3]*b[0];
}
/* Express a nav-frame vector in body coords: v_b = v - 2w(u x v) + 2 u x(u x v),
 * u = (x,y,z). Passive transform by the attitude q. Verified in TEST-EST-003. */
static inline void q_rotate_n2b(const float q[4], const float v[3], float o[3])
{
    const float qv[3] = { q[1], q[2], q[3] };
    float t[3], uxt[3];
    v_cross(qv, v, t); v_scale(t, 2.0f, t);   /* t   = 2 (u x v) */
    v_cross(qv, t, uxt);                       /* uxt = u x t     */
    for (int i = 0; i < 3; ++i) { o[i] = v[i] - q[0]*t[i] + uxt[i]; }
}
/* Small rotation vector (rad) -> quaternion increment, renormalized. */
static inline void q_from_smallangle(const float dth[3], float o[4])
{
    o[0] = 1.0f; o[1] = 0.5f*dth[0]; o[2] = 0.5f*dth[1]; o[3] = 0.5f*dth[2];
    q_normalize(o);
}

/* =====================================================================
 * 3x3 skew + 9x9 matrix algebra for the error covariance.
 *
 * ALIASING HAZARD. mat9_mul and mat9_transpose write their output element by
 * element WHILE STILL READING their inputs, so the output MUST be a distinct
 * buffer from every input — squaring (A*A) or writing a product back over an
 * operand corrupts data still needed for later elements. Each guards this with
 * an assertion. The s_scr* matrices give predict/update non-aliasing scratch
 * destinations; the superloop is single-threaded and these calls never nest,
 * so sharing them is safe.
 *
 * 2D-array params are not const-qualified: C cannot implicitly convert
 * float(*)[9] to const float(*)[9] (nested-qualifier rule) and -Werror rejects
 * it; these functions still never write through their logical inputs.
 * ===================================================================== */
static float s_scrA[9][9];   /* matrix-product scratch (predict/update) */
static float s_scrB[9][9];
static float s_scrC[9][9];

static inline void m3_skew(const float v[3], float M[3][3])
{
    M[0][0]= 0.0f;  M[0][1]=-v[2];  M[0][2]= v[1];
    M[1][0]= v[2];  M[1][1]= 0.0f;  M[1][2]=-v[0];
    M[2][0]=-v[1];  M[2][1]= v[0];  M[2][2]= 0.0f;
}
static inline void mat9_mul(float A[9][9], float B[9][9], float C[9][9])
{
    ASSERT((const void *)C != (const void *)A);  /* output must not alias inputs */
    ASSERT((const void *)C != (const void *)B);
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            float s = 0.0f;
            for (int k = 0; k < 9; ++k) { s += A[i][k] * B[k][j]; }
            C[i][j] = s;
        }
    }
}
static inline void mat9_transpose(float A[9][9], float T[9][9])
{
    ASSERT((const void *)T != (const void *)A);  /* in-place transpose corrupts */
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) { T[i][j] = A[j][i]; }
    }
}
static inline void mat9_identity(float M[9][9])
{
    ASSERT((const void *)M != (const void *)0);
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) { M[i][j] = (i == j) ? 1.0f : 0.0f; }
    }
}

/* =====================================================================
 * Covariance hygiene for single precision. Forces P exactly symmetric and
 * imposes a positive diagonal floor. Float round-off accumulated over millions
 * of Joseph updates slowly breaks symmetry and can drive a variance to zero or
 * slightly negative; this backstop guarantees a usable, PD-ish P every tick and
 * is what makes TEST-EST-008 (no variance silently reaches zero) hold. Runs at
 * the end of predict (post-Q) and update (post-correction). (D062)
 * ===================================================================== */
static void cov_condition(void)
{
    for (int i = 0; i < 9; ++i) {
        for (int j = i + 1; j < 9; ++j) {
            const float avg = 0.5f * (s_P[i][j] + s_P[j][i]);
            s_P[i][j] = avg;
            s_P[j][i] = avg;
        }
        if (s_P[i][i] < EST_PD_FLOOR) { s_P[i][i] = EST_PD_FLOOR; }
    }
    ASSERT(s_P[0][0] >= EST_PD_FLOOR);
    ASSERT(s_P[8][8] >= EST_PD_FLOOR);
}

/* =====================================================================
 * A-priori innovation variance per axis, exported for fdir_gate (β, D063 — amends
 * D060). For a vector reference measurement the 1×9 Jacobian row of axis k is the
 * matching row of [h]ₓ with zero gyro-bias columns (D1), so
 *   S_k = H_k P⁻ H_kᵀ + R  reduces to the attitude-block quadratic form
 *   Σ_{i,j<3} [h]ₓ[k][i] · P[i][j] · [h]ₓ[k][j] + R,
 * evaluated against the just-propagated a-priori P (s_P here). FDIR then forms
 * chi2 = Σ_k (z_k − h_k)² / S_k — NO matrix inversion, so the gate is adaptive
 * (S grows with P under degradation) yet stays on the single-precision FPU
 * (D062's win). [h]ₓ is rank-2, so the radial/magnitude direction carries only R
 * while the two tangential directions carry P+R — the 2-DOF structure is implicit.
 * ===================================================================== */
static void innov_var(const float h[3], float r, float S_out[3])
{
    ASSERT(r > 0.0f);
    float Hatt[3][3];
    m3_skew(h, Hatt);
    for (int k = 0; k < 3; ++k) {
        float s = 0.0f;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) { s += Hatt[k][i] * s_P[i][j] * Hatt[k][j]; }
        }
        S_out[k] = s + r;     /* quadratic form ≥ 0 (P PSD) plus r > 0 */
    }
    ASSERT(S_out[0] > 0.0f);  /* innovation variance strictly positive */
    ASSERT(!isnan(S_out[2]));
}

void estimator_predict(
    const imu_reading_t  *imu1,
    const imu_reading_t  *imu2,
    predicted_readings_t *predictions_out
)
{
    ASSERT(predictions_out != NULL);
    const float dt = 1.0f / (float)AVIONICS_LOOP_RATE_HZ;   /* 1 ms (D052) */

    /* Bias-corrected body rate. Both gyros healthy → equal-weight blend (which
     * IS the inverse-variance blend under the provisional equal R); one gyro →
     * that one; neither (both isolated by admit) → hold the last rate, i.e.
     * dead reckoning (D062). NULL pointer == channel isolated. */
    float w1[3] = {0.0f}, w2[3] = {0.0f};
    const bool h1 = (imu1 != NULL);
    const bool h2 = (imu2 != NULL);
    if (h1) { v_sub(imu1->gyro_rads, s_nom.b1, w1); }
    if (h2) { v_sub(imu2->gyro_rads, s_nom.b2, w2); }

    if (h1 && h2) { v_add(w1, w2, s_omega); v_scale(s_omega, 0.5f, s_omega); }
    else if (h1)  { v_scale(w1, 1.0f, s_omega); }
    else if (h2)  { v_scale(w2, 1.0f, s_omega); }
    /* else: keep s_omega from the last tick — dead reckoning. */

    /* Propagate the nominal quaternion: q <- q (X) dq(omega*dt), renormalized.
     * Body-referenced rate post-multiplies (q is the body's orientation in nav,
     * Hamilton). Verified against the analytic case in TEST-EST-003. */
    float dth[3], dq[4], qnew[4];
    v_scale(s_omega, dt, dth);
    q_from_smallangle(dth, dq);
    q_mul(s_nom.q, dq, qnew);
    for (int i = 0; i < 4; ++i) { s_nom.q[i] = qnew[i]; }
    q_normalize(s_nom.q);

    ASSERT(!isnan(s_nom.q[0]));    /* propagation produced a finite quaternion */

    /* ---- Covariance propagation: P <- F P F^T + Q --------------------
     * Discrete error-state transition F. Attitude error rotates with the body
     * rate (I - [w dt]x) and is driven by the gyro-bias errors that leak into
     * the blended rate; the biases themselves are random walks (identity).
     * The bias->attitude leak weight matches each gyro's blend weight, so an
     * isolated IMU's bias neither feeds attitude nor takes process noise
     * (frozen, D062): c=0.5 when both healthy, 1.0 when sole, 0.0 when out. */
    float F[9][9];
    mat9_identity(F);
    {
        float wdt[3], sk[3][3];
        v_scale(s_omega, dt, wdt);
        m3_skew(wdt, sk);                                     /* [w dt]x        */
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) { F[i][j] -= sk[i][j]; }   /* I - [w dt]x */
        }
        const float c1 = h1 ? (h2 ? 0.5f : 1.0f) : 0.0f;
        const float c2 = h2 ? (h1 ? 0.5f : 1.0f) : 0.0f;
        for (int i = 0; i < 3; ++i) {
            F[i][3 + i] = -c1 * dt;      /* d(theta)/d(b1) */
            F[i][6 + i] = -c2 * dt;      /* d(theta)/d(b2) */
        }
    }
    mat9_mul(F, s_P, s_scrA);            /* scrA = F P              */
    mat9_transpose(F, s_scrB);           /* scrB = F^T              */
    mat9_mul(s_scrA, s_scrB, s_P);       /* P    = (F P) F^T        */

    /* Process noise Q (diagonal). A frozen IMU's bias gets no random walk. */
    for (int i = 0; i < 3; ++i) { s_P[i][i]     += EST_Q_ATT; }
    for (int i = 0; i < 3; ++i) { s_P[3 + i][3 + i] += h1 ? EST_Q_BIAS : 0.0f; }
    for (int i = 0; i < 3; ++i) { s_P[6 + i][6 + i] += h2 ? EST_Q_BIAS : 0.0f; }

    cov_condition();                     /* symmetrize + PD-floor (E1) */
    ASSERT(s_P[0][0] > 0.0f);            /* attitude variance stays positive */
    ASSERT(!isnan(s_P[8][8]));

    /* ---- Predicted measurements for fdir_gate (D060) -----------------
     * Reference vectors rotated into the predicted body frame; the same h(x)
     * the update forms its innovations against, snapshotted here for the gate.
     * Accelerometer convention: a static IMU reads the SPECIFIC FORCE, i.e. the
     * reaction to gravity = -g (points "up"). The exact sign is tied to the IMU
     * axis convention, confirmed at bring-up and pinned by TEST-EST-001. Both
     * IMUs are co-located/aligned, so their predicted readings are identical. */
    const float g_react[3] = {0.0f, 0.0f, -EST_G_NAV_D};      /* -gravity */
    const float b_nav[3]   = {EST_B_NAV_N, EST_B_NAV_E, EST_B_NAV_D};
    float a_pred[3];
    q_rotate_n2b(s_nom.q, g_react, a_pred);
    for (int i = 0; i < 3; ++i) {
        predictions_out->imu1_accel_pred_mss[i] = a_pred[i];
        predictions_out->imu2_accel_pred_mss[i] = a_pred[i];
    }
    q_rotate_n2b(s_nom.q, b_nav, predictions_out->mag_pred_ut);

    /* A-priori innovation variances for the gate (β, D063). s_P is the a-priori
     * covariance just propagated above. Both IMUs are co-located → identical
     * accel variance (per-IMU R is a future refinement), so compute once + copy. */
    innov_var(a_pred, EST_R_ACC, predictions_out->imu1_accel_innov_var);
    for (int i = 0; i < 3; ++i) {
        predictions_out->imu2_accel_innov_var[i] = predictions_out->imu1_accel_innov_var[i];
    }
    innov_var(predictions_out->mag_pred_ut, EST_R_MAG, predictions_out->mag_innov_var);

    predictions_out->timestamp_us = h1 ? imu1->timestamp_us :
                                    (h2 ? imu2->timestamp_us : 0U);

    ASSERT(!isnan(a_pred[2]));
    ASSERT(!isnan(predictions_out->mag_pred_ut[0]));
}

/* =====================================================================
 * Measurement model for the vector references (accelerometer, magnetometer).
 * Produces the predicted body-frame reading h(x) = q_rotate_n2b(q, ref) and
 * its 3x3 attitude Jacobian. Under the q = q_hat (X) dq error convention (the
 * one D6 injects), a fixed nav vector's body coordinates perturb to first order
 * as  f ≈ h + [h]x dtheta , so the attitude Jacobian block is exactly [h]x
 * (skew of the prediction). The gyro-bias Jacobian columns are zero — gravity
 * and the geomagnetic field do not depend on gyro bias. The bias columns are
 * therefore omitted here and treated as zero by the scalar update (D2).
 * ===================================================================== */
static void meas_model(const float q[4], const float ref[3],
                       float h[3], float Hatt[3][3])
{
    ASSERT(ref != NULL);
    ASSERT(h != NULL);
    q_rotate_n2b(q, ref, h);     /* predicted body-frame measurement */
    m3_skew(h, Hatt);            /* attitude Jacobian block = [h]x    */
}

/* =====================================================================
 * Sequential scalar Kalman update (Joseph form). Processes ONE scalar
 * measurement: innov = z - h(nominal), H = its 1x9 Jacobian row, r = noise
 * variance. Accumulates the error-state correction into dx (injected once in
 * D6) and shrinks P. The effective innovation subtracts H*dx so measurements
 * already applied this tick are accounted for at the FIXED linearization point
 * (no re-rotation between the tick's few scalar updates). Because each
 * measurement is scalar, S is a scalar and there is NO matrix inversion — the
 * JPL/single-precision robustness win (D062). Covariance uses the Joseph form
 * (I-KH)P(I-KH)^T + rKK^T, kept as explicit matrix algebra for auditability;
 * the rank-1 optimization is deferred to profiling.
 * ===================================================================== */
static void scalar_update(float P[9][9], float dx[9],
                          const float H[9], float innov, float r)
{
    ASSERT(r > 0.0f);
    float PHt[9];
    for (int i = 0; i < 9; ++i) {
        float s = 0.0f;
        for (int j = 0; j < 9; ++j) { s += P[i][j] * H[j]; }
        PHt[i] = s;
    }
    float S = r;
    for (int i = 0; i < 9; ++i) { S += H[i] * PHt[i]; }
    ASSERT(S > 0.0f);                        /* innovation variance positive */

    float K[9], Hdx = 0.0f;
    for (int i = 0; i < 9; ++i) { K[i] = PHt[i] / S; }
    for (int i = 0; i < 9; ++i) { Hdx += H[i] * dx[i]; }
    const float y = innov - Hdx;             /* effective innovation */
    for (int i = 0; i < 9; ++i) { dx[i] += K[i] * y; }

    for (int i = 0; i < 9; ++i) {            /* s_scrC = I - K H */
        for (int j = 0; j < 9; ++j) {
            s_scrC[i][j] = ((i == j) ? 1.0f : 0.0f) - K[i] * H[j];
        }
    }
    mat9_mul(s_scrC, P, s_scrA);             /* (I-KH) P             */
    mat9_transpose(s_scrC, s_scrB);          /* (I-KH)^T             */
    mat9_mul(s_scrA, s_scrB, P);             /* (I-KH)P(I-KH)^T      */
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) { P[i][j] += r * K[i] * K[j]; }
    }
}

/* Apply a 3-axis vector measurement (accelerometer or magnetometer) as three
 * sequential scalar updates against the nominal. ref = nav-frame reference,
 * meas = measured body vector, r = per-axis noise variance. Each axis's 1x9
 * Jacobian row is the matching row of [h]x with zero gyro-bias columns (D1). */
static void apply_vec_update(const float meas[3], const float ref[3],
                             float dx[9], float r)
{
    ASSERT(meas != NULL);
    ASSERT(dx != NULL);
    float h[3], Hatt[3][3];
    meas_model(s_nom.q, ref, h, Hatt);
    for (int k = 0; k < 3; ++k) {
        float H[9] = {0.0f};
        for (int c = 0; c < 3; ++c) { H[c] = Hatt[k][c]; }
        scalar_update(s_P, dx, H, meas[k] - h[k], r);
    }
}

estimator_mode_t estimator_update(
    const imu_reading_t  *imu1,
    const imu_reading_t  *imu2,
    const mag_reading_t  *mag,
    const health_flags_t *health,
    attitude_estimate_t  *out
)
{
    ASSERT(out != NULL);
    ASSERT(health != NULL);

    /* Accumulated error-state correction, injected into the nominal once in D6.
     * Every update applies to P and dx at the FIXED linearization point (the
     * nominal from predict). A NULL channel is one the gate isolated: it is
     * simply skipped, so the covariance reduction it would have contributed
     * never happens and that direction's uncertainty keeps growing — graceful
     * degradation, not a special case. */
    float dx[9] = {0.0f};

    /* --- Accelerometer updates: bound roll & pitch (gravity reference) --- */
    const float g_react[3] = {0.0f, 0.0f, -EST_G_NAV_D};
    if (imu1 != NULL) { apply_vec_update(imu1->accel_mss, g_react, dx, EST_R_ACC); }
    if (imu2 != NULL) { apply_vec_update(imu2->accel_mss, g_react, dx, EST_R_ACC); }

    /* --- Magnetometer update: bound yaw -------------------------------
     * The field is the ONLY absolute heading reference (gravity cannot resolve
     * rotation about the vertical). With the mag healthy, yaw is observed and
     * its covariance shrinks; with it isolated (NULL), yaw is corrected by
     * nothing and drifts on gyro bias — the "yaw bounded with mag, drifts
     * without" demonstration (D054/D060). A magnetic disturbance is caught by
     * fdir_gate and arrives here as NULL, so only trustworthy field data is
     * fused. (Full-vector update; a horizontal-only projection that confines
     * the mag strictly to yaw is a possible refinement.) */
    const float b_nav[3] = {EST_B_NAV_N, EST_B_NAV_E, EST_B_NAV_D};
    if (mag != NULL) { apply_vec_update(mag->field_ut, b_nav, dx, EST_R_MAG); }

    /* --- Differential-bias pseudo-measurement (D061) ------------------
     * Both gyros measure the SAME true rate, so their bias-corrected rates must
     * agree: d = (g1 - b1) - (g2 - b2) ~ 0. The residual d is, to first order,
     * (db1 - db2) + noise, so it observes the bias DIFFERENCE directly — which
     * is what makes the two per-IMU biases separable (attitude updates alone
     * see only their blended sum). Jacobian H = [0 | +I | -I]; the measurement
     * is "the difference should be zero", so the innovation is d itself. Noise
     * is the sum of both gyro variances (EST_R_DIFF). Requires BOTH gyros.
     * NOTE: this reads RAW gyro data, not FDIR's verdict — same quantity the
     * admit gyro-cross-check looks at, different purpose, no D050 violation. */
    if ((imu1 != NULL) && (imu2 != NULL)) {
        for (int k = 0; k < 3; ++k) {
            const float d = (imu1->gyro_rads[k] - s_nom.b1[k])
                          - (imu2->gyro_rads[k] - s_nom.b2[k]);
            float H[9] = {0.0f};
            H[3 + k] =  1.0f;    /* d(d)/d(db1) = +1 */
            H[6 + k] = -1.0f;    /* d(d)/d(db2) = -1 */
            scalar_update(s_P, dx, H, d, EST_R_DIFF);
        }
    }

    cov_condition();   /* symmetrize + PD-floor after the corrections (E1) */

    /* --- Inject the accumulated correction into the nominal -----------
     * dx = [dtheta | db1 | db2]. Attitude is injected MULTIPLICATIVELY,
     * q = q_hat (X) dq(dtheta), so the quaternion stays unit-norm and the
     * convention matches F (C2) and the Jacobians (D1). Biases add directly. */
    {
        float dq[4], qn[4];
        q_from_smallangle(&dx[0], dq);
        q_mul(s_nom.q, dq, qn);
        for (int i = 0; i < 4; ++i) { s_nom.q[i] = qn[i]; }
        q_normalize(s_nom.q);
        for (int i = 0; i < 3; ++i) {
            s_nom.b1[i] += dx[3 + i];
            s_nom.b2[i] += dx[6 + i];
        }
    }

    /* --- Degraded mode from channel availability (D062) --------------- */
    estimator_mode_t mode;
    if      ((imu1 != NULL) && (imu2 != NULL)) { mode = EST_MODE_DUAL_IMU; }
    else if  (imu1 != NULL)                    { mode = EST_MODE_IMU1_ONLY; }
    else if  (imu2 != NULL)                    { mode = EST_MODE_IMU2_ONLY; }
    else                                       { mode = EST_MODE_DEAD_RECKONING; }

    /* --- Output: Euler (ZYX) from the quaternion, body rates, P diagonal ---
     * roll/pitch/yaw extracted from the Hamilton quaternion; pitch arg clamped
     * so single-precision rounding cannot push asinf out of domain. Rates are
     * BODY angular rates (p,q,r) = the bias-corrected blend, NOT Euler deriva-
     * tives (a convention the control law is built to). covariance[9] is the
     * P diagonal verbatim. Euler signs are pinned by TEST-EST-001/003. */
    const float *q = s_nom.q;
    float sp = 2.0f * (q[0]*q[2] - q[3]*q[1]);                 /* sin(pitch) */
    sp = (sp > 1.0f) ? 1.0f : ((sp < -1.0f) ? -1.0f : sp);
    out->roll_rad  = atan2f(2.0f*(q[0]*q[1] + q[2]*q[3]), 1.0f - 2.0f*(q[1]*q[1] + q[2]*q[2]));
    out->pitch_rad = asinf(sp);
    out->yaw_rad   = atan2f(2.0f*(q[0]*q[3] + q[1]*q[2]), 1.0f - 2.0f*(q[2]*q[2] + q[3]*q[3]));
    out->roll_rate_rads  = s_omega[0];
    out->pitch_rate_rads = s_omega[1];
    out->yaw_rate_rads   = s_omega[2];
    for (int i = 0; i < 9; ++i) { out->covariance[i] = s_P[i][i]; }
    out->mode = mode;
    out->timestamp_us = (imu1 != NULL) ? imu1->timestamp_us :
                        (imu2 != NULL) ? imu2->timestamp_us :
                        (mag  != NULL) ? mag->timestamp_us  : 0U;

    ASSERT(!isnan(out->yaw_rad));
    ASSERT(!isnan(out->covariance[0]));
    return mode;
}
