#ifndef AVIONICS_TYPES_H
#define AVIONICS_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/*
 * avionics_types.h — canonical shared type definitions
 *
 * Single source of truth for all data types shared between avionics modules.
 * Depends only on stdint.h and stdbool.h. Every other module includes this.
 * No module defines public types outside this file.
 *
 * This file IS the architecture expressed in C types. Changes here propagate
 * to every module — review carefully before modifying anything.
 */

/* =========================================================================
 * ISR-SHARED FLAG TYPE
 * =========================================================================
 * Any variable that crosses the ISR/main-loop boundary must use this type.
 * The volatile qualifier prevents the compiler from caching the value in a
 * register — without it the main loop would never observe the ISR's write.
 * The type name is self-documenting: seeing volatile_flag_t in a declaration
 * immediately tells a reader this variable crosses the ISR/main-loop boundary.
 * All ISR-shared variables are enumerated in isr_flags.h.
 */
typedef volatile uint8_t volatile_flag_t;

/* =========================================================================
 * PROTOCOL VERSION
 * =========================================================================
 * Carried as the first field in every telemetry_frame_t and command_frame_t.
 * The ground station checks this on receipt and alerts the operator on a
 * mismatch before bad data propagates. Increment whenever frame layout changes.
 */
/* v2 (D054, 2026-06-17): mag_healthy added to health_flags_t — frame layout changed.
 * v3 (D062, 2026-06-18): attitude_estimate_t.covariance widened [6]→[9] (full
 * 9-state EKF error diagonal) — frame layout changed. */
#define AVIONICS_PROTOCOL_VERSION ((uint8_t)3U)

/* =========================================================================
 * SYSTEM CONSTANTS
 * =========================================================================
 * TBD values are pending literature review results. Do not substitute
 * placeholder values — wait for the designated LR to produce the number.
 */
#define AVIONICS_NUM_FINS        ((uint8_t)4U)
#define AVIONICS_BARO_RATE_HZ    ((uint32_t)50U)

/* LR-1 (2026-05-22, D052): 1000 Hz — derived from 10 Hz disturbance bandwidth × 5× margin,
 * Franklin sampling rule ωs/ωBW ≥ 20 (Ch. 11, Eq. 11.3). See docs/derivations/LR-1-loop-rate.md */
#define AVIONICS_LOOP_RATE_HZ        ((uint32_t)1000U)

/* LR-1 (2026-05-22, D052): 5 × loop period = 5 × 1000 μs = 5000 μs.
 * A reading older than 5 loop cycles is stale regardless of sensor status. */
#define IMU_STALENESS_THRESHOLD_US   ((uint32_t)5000U)

/* TBD — LR-3: chi-squared threshold for 2-DOF innovation gate at chosen confidence level */
/* #define CHI2_THRESHOLD_2DOF */

/* =========================================================================
 * STATUS ENUMS — SENSOR DRIVERS
 * =========================================================================
 * Every function that reads a sensor returns one of these. IMU_ISOLATED means
 * FDIR has flagged this sensor; the driver returns this to signal that upstream
 * callers should pass NULL to the estimator rather than this reading.
 */
typedef enum {
    IMU_OK,
    IMU_BUS_ERROR,
    IMU_TIMEOUT,
    IMU_STALE_DATA,
    IMU_OUT_OF_RANGE,
    IMU_ISOLATED
} imu_status_t;

typedef enum {
    BARO_OK,
    BARO_BUS_ERROR,
    BARO_TIMEOUT,
    BARO_STALE_DATA,
    BARO_OUT_OF_RANGE,
    BARO_ISOLATED
} baro_status_t;

/* Magnetometer (BMM150, D054) — dedicated I2C bus. Mirrors imu_status_t.
 * MAG_ISOLATED means FDIR has flagged the mag; upstream passes NULL to the
 * estimator rather than this reading, and the yaw channel grows covariance. */
typedef enum {
    MAG_OK,
    MAG_BUS_ERROR,
    MAG_TIMEOUT,
    MAG_STALE_DATA,
    MAG_OUT_OF_RANGE,
    MAG_ISOLATED
} mag_status_t;

/* =========================================================================
 * STATUS ENUMS — ALGORITHM MODULES
 * =========================================================================
 */
typedef enum {
    EST_MODE_DUAL_IMU,       /* both IMUs healthy — full innovation cross-check */
    EST_MODE_IMU1_ONLY,      /* IMU-2 isolated — estimator propagates on IMU-1 only */
    EST_MODE_IMU2_ONLY,      /* IMU-1 isolated — estimator propagates on IMU-2 only */
    EST_MODE_DEAD_RECKONING, /* both IMUs isolated — propagate with last known rates */
    EST_MODE_FAULT           /* unrecoverable estimator fault */
} estimator_mode_t;

typedef enum {
    CTL_MODE_FULL_AUTHORITY, /* all 4 fins healthy */
    CTL_MODE_3FIN_REDUCED,   /* one fin isolated — mixing matrix reconfigured */
    CTL_MODE_2FIN_REDUCED,   /* two fins isolated — limited authority */
    CTL_MODE_SAFE_HOLD       /* cannot stabilise — command zero deflection */
} control_mode_t;

typedef enum {
    SYS_MODE_FLIGHT,    /* real deflection limits enforced */
    SYS_MODE_DEMO,      /* relaxed deflection limits for presentation legibility */
    SYS_MODE_SAFE_HOLD  /* no actuation — fins at zero */
} system_mode_t;

typedef enum {
    CMD_SET_MODE_FLIGHT,
    CMD_SET_MODE_DEMO,
    CMD_SET_MODE_SAFE_HOLD,
    CMD_ACK_REQUEST,
    CMD_RESET_ESTIMATOR
} command_id_t;

/* =========================================================================
 * STATUS ENUMS — PLATFORM
 * =========================================================================
 * Reset cause classified by platform_init() from RCC_CSR at boot, then the
 * flags are cleared (RMVF) so the next boot reads clean. Read via
 * platform_reset_cause(). Classification is an ordered hierarchy — the first
 * matching specific flag wins — so RESET_PIN is reported only when the NRST
 * pin flag is the sole flag set (PINRSTF is set on nearly every reset because
 * NRST is bidirectional; it is informative only in isolation). WATCHDOG and
 * BROWNOUT are the fault causes that drive the post-reboot safe-state path
 * (D053 A4/A6). WWDG is not used (D053 A1) so no WWDG cause exists. RESET_LOW_POWER
 * is omitted — no low-power modes are used. RESET_UNKNOWN is the safe default. */
typedef enum {
    RESET_POWER_ON,   /* PORRSTF — cold power-on (clean) */
    RESET_WATCHDOG,   /* IWDGRSTF — independent watchdog fired (fault) */
    RESET_SOFTWARE,   /* SFTRSTF — deliberate software reset (clean) */
    RESET_BROWNOUT,   /* BORRSTF — supply dipped mid-run (fault) */
    RESET_PIN,        /* PINRSTF alone — external NRST (debugger/button) */
    RESET_UNKNOWN     /* no flag matched — safe default */
} reset_cause_t;

/* =========================================================================
 * DATA STRUCTS — SENSOR READINGS
 * =========================================================================
 * Every reading struct carries a timestamp (from TIM2, per D034) and a status.
 * Callers must check status before using any data field. A reading with a
 * non-OK status may contain stale, partial, or zeroed data — treat as invalid.
 *
 * accel_mss: m/s², body frame, [X, Y, Z]
 * gyro_rads: rad/s, body frame, [X, Y, Z]
 */
typedef struct {
    float         accel_mss[3];
    float         gyro_rads[3];
    uint32_t      timestamp_us;
    imu_status_t  status;
} imu_reading_t;

/*
 * pressure_pa:   absolute pressure in Pascals
 * temperature_c: temperature in Celsius (from MS5611 compensation)
 * altitude_m:    derived altitude using standard sea-level reference
 */
typedef struct {
    float          pressure_pa;
    float          temperature_c;
    float          altitude_m;
    uint32_t       timestamp_us;
    baro_status_t  status;
} baro_reading_t;

/*
 * field_ut:      magnetic field per axis [x,y,z], microtesla, body frame.
 * Bounds the yaw estimate (BMM150, D054). No driver yet — see hardware layer (task 7).
 */
typedef struct {
    float          field_ut[3];
    uint32_t       timestamp_us;
    mag_status_t   status;
} mag_reading_t;

/* =========================================================================
 * DATA STRUCTS — FDIR
 * =========================================================================
 * fdir_gate_result_t carries the raw innovation gate outputs. Uplink to the
 * ground station so the dashboard can show the chi-squared values live.
 *
 * chi2_imu1/2:       normalised chi-squared residual for each IMU this tick
 * imu1/2_gate_open:  true = reading accepted by the innovation gate
 * imu1/2_stale_us:   microseconds since the last valid reading on this channel
 *
 * chi2_mag:          mag innovation chi-squared (measured field vs mag_pred_ut).
 *                    The lone mag has no twin to cross-check; its gate is the
 *                    analytical innovation test vs the IMU-predicted field (D060).
 * mag_gate_open:     true = mag reading accepted by the innovation gate
 * mag_stale_us:      microseconds since the last valid mag reading
 */
typedef struct {
    float    chi2_imu1;
    float    chi2_imu2;
    float    chi2_mag;
    bool     imu1_gate_open;
    bool     imu2_gate_open;
    bool     mag_gate_open;
    uint32_t imu1_stale_us;
    uint32_t imu2_stale_us;
    uint32_t mag_stale_us;
} fdir_gate_result_t;

/*
 * predicted_readings_t — the estimator's a-priori predicted measurements,
 * produced by estimator_predict() and consumed by fdir_gate() for the
 * innovation gate. Crosses the algorithm-layer boundary as a VALUE: FDIR reads
 * this struct, it never imports the estimator. Per-IMU predicted accelerometer
 * vector = gravity direction rotated into the body frame given the predicted
 * orientation; the gate forms residual = measured - predicted. Baro is
 * correction-only this cycle — no predicted altitude field. (D050)
 *
 * mag_pred_ut: predicted Earth field in the body frame — the nav-frame
 * reference field rotated by the predicted orientation. Source for the mag
 * innovation residual in fdir_gate (D060). Single mag → one prediction.
 */
typedef struct {
    float    imu1_accel_pred_mss[3];
    float    imu2_accel_pred_mss[3];
    float    mag_pred_ut[3];
    uint32_t timestamp_us;
} predicted_readings_t;

/*
 * health_flags_t — authoritative per-channel health state.
 * Written exclusively by FDIR. Read by estimator, control law, telemetry.
 * No module other than FDIR sets these fields.
 */
typedef struct {
    bool imu1_healthy;
    bool imu2_healthy;
    bool baro_healthy;
    bool mag_healthy;
    bool actuator_healthy[4]; /* per-fin: index matches fin number 0..3 */
    bool radio_healthy;
    bool wired_healthy;
} health_flags_t;

/* =========================================================================
 * DATA STRUCTS — ESTIMATOR OUTPUT
 * =========================================================================
 * covariance[9]: diagonal of the 9-state EKF error covariance P (D061, D062) —
 *   [0..2] attitude-error variances        [δθ_roll, δθ_pitch, δθ_yaw]
 *   [3..5] IMU-1 gyro-bias-error variances [δb1_x, δb1_y, δb1_z]
 *   [6..8] IMU-2 gyro-bias-error variances [δb2_x, δb2_y, δb2_z]
 * Grows as sensors are isolated. Mandatory in every output — a consumer that
 * ignores covariance ignores the system's self-reported confidence. The two
 * bias triplets also expose the per-IMU drift and the differential-bias fault
 * signal (D061) to the ground station.
 */
typedef struct {
    float            roll_rad;
    float            pitch_rad;
    float            yaw_rad;
    float            roll_rate_rads;
    float            pitch_rate_rads;
    float            yaw_rate_rads;
    float            covariance[9];
    estimator_mode_t mode;
    uint32_t         timestamp_us;
} attitude_estimate_t;

/* =========================================================================
 * DATA STRUCTS — CONTROL AND ACTUATION
 * =========================================================================
 * deflection_rad[4]: commanded fin deflection in radians, per-fin.
 * Index matches fin number 0..3. Hard-clamped by actuators_write()
 * to the limits imposed by the current system_mode_t.
 */
typedef struct {
    float         deflection_rad[4];
    uint32_t      timestamp_us;
    control_mode_t mode;
} actuator_cmd_t;

/* =========================================================================
 * DATA STRUCTS — TELEMETRY FRAMES
 * =========================================================================
 * telemetry_frame_t: downlink frame from MCU to ground station.
 * command_frame_t:   uplink frame from ground station to MCU.
 *
 * protocol_version is ALWAYS the first field. The ground station reads it
 * before deserialising anything else and refuses to process a frame whose
 * version does not match AVIONICS_PROTOCOL_VERSION.
 *
 * health_flags_t is unconditionally present in every downlink frame.
 * Health state is never omitted, never optional.
 */
typedef struct {
    uint8_t           protocol_version;
    uint32_t          frame_id;
    uint32_t          timestamp_us;
    imu_reading_t     imu1;
    imu_reading_t     imu2;
    baro_reading_t    baro;
    attitude_estimate_t estimate;
    actuator_cmd_t    actuators;
    health_flags_t    health;
    system_mode_t     sys_mode;
    uint16_t          crc16;
} telemetry_frame_t;

typedef struct {
    uint8_t      protocol_version;
    uint32_t     cmd_seq;
    command_id_t command;
    uint32_t     timestamp_us;
    uint16_t     crc16;
} command_frame_t;

#endif /* AVIONICS_TYPES_H */
