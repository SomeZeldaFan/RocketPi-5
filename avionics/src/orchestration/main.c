#include "avionics_types.h"
#include "isr_flags.h"
#include "hardware/platform.h"
#include "hardware/bmi160.h"
#include "hardware/icm42688p.h"
#include "hardware/ms5611.h"
#include "hardware/sik_radio.h"
#include "algorithm/fdir.h"
#include "algorithm/estimator.h"
#include "algorithm/control_law.h"
#include "output/actuators.h"
#include "output/telemetry.h"
#include "output/c2.h"
#include "orchestration/mode_fsm.h"

/*
 * main.c — STUB
 *
 * Top-of-system superloop skeleton (D043). Every step here is either an
 * init call before the loop, or a numbered step inside the loop that
 * corresponds 1:1 to the §4.2 architectural tick sequence.
 *
 * NO functional logic. Comments describe intent.
 *
 * The structure is intentional and the order is the architecture:
 *   - platform_init() FIRST so IWDG is running before any peripheral init.
 *     A hang ANYWHERE in init then trips the single watchdog window and resets
 *     the MCU rather than hanging forever (D053 A3).
 *   - Hardware drivers next, in stable order. NO _init() kicks the watchdog — the
 *     whole init sequence runs inside one window (budgeted < 1.0 s), so a hang
 *     after any module's kick cannot be masked (D053 A3).
 *   - Algorithm modules next, then output, then orchestration.
 *   - One boot-complete watchdog kick fires after all _init() return, before the
 *     loop — a known-good "init succeeded" assertion that also resets the window.
 *   - Loop body executes the 14 tick steps in the architectural order (D050).
 *     The order is load-bearing: FDIR admission runs before predict so a bad
 *     gyro cannot poison the prediction; the gate runs after predict so it has
 *     the predicted measurements; estimator correct runs after the gate so it
 *     sees the final health flags; control law after that; etc.
 */

int main(void)
{
    /* ------- Boot-time init sequence (IWDG starts inside platform_init) ----- */
    platform_init();   /* clock, TIM2, IWDG: must be first */
    bmi160_init();
    icm42688p_init();
    ms5611_init();
    sik_radio_init();
    fdir_init();
    estimator_init();
    control_law_init();
    actuators_init();
    telemetry_init();
    c2_init();
    mode_fsm_init();

    /* Storage for the per-tick data flow. All static — JPL Rule 3. */
    static imu_reading_t       imu1;
    static imu_reading_t       imu2;
    static baro_reading_t      baro;
    static mag_reading_t       mag;
    static health_flags_t      health;
    static fdir_gate_result_t  gate;
    static predicted_readings_t pred;
    static attitude_estimate_t estimate;
    static actuator_cmd_t      cmd;
    static command_frame_t     c2_cmd;

    /* Boot complete: all _init() returned without hanging and all per-tick state
     * is established (statics are zero-initialised at load). Kick the watchdog once
     * here — a known-good "init succeeded, entering loop" assertion that also resets
     * the IWDG window so the first loop iteration gets a full window (D053 A3). A
     * fault reset cause (platform_reset_cause()) drives an initial SAFE_HOLD here in
     * a later session — FSM/telemetry scrutiny (D053 A6). */
    platform_watchdog_kick();

    /* ------------------------- Main loop ------------------------- */
    for (;;) {
        /* [1] Consume latest BMI160 sample (DMA-filled). */
        (void)bmi160_read(&imu1);

        /* [2] Consume latest ICM-42688-P sample (DMA-filled). */
        (void)icm42688p_read(&imu2);

        /* [3] Service the barometer state machine (decimated to ~50 Hz internally). */
        (void)ms5611_service(&baro);

        /* [3b] Magnetometer read — driver lands in task 7 (D060). Until then `mag`
         *      holds its zero-initialised static; admit/gate/update carry it. */

        /* [4] FDIR admission: absolute checks (staleness, bounds, gyro-vs-gyro,
         *     mag |B|) → preliminary health flags. Vets the gyro before predict. */
        fdir_admit(&imu1, &imu2, &baro, &mag, &health, &gate);

        /* [5] Estimator predict: propagate on the admitted gyro and export the
         *     predicted measurements for the gate. NULL for a gyro the
         *     admission pass rejected (post-admit verdict). */
        const imu_reading_t *pre_imu1 = health.imu1_healthy ? &imu1 : (const imu_reading_t *)0;
        const imu_reading_t *pre_imu2 = health.imu2_healthy ? &imu2 : (const imu_reading_t *)0;
        estimator_predict(pre_imu1, pre_imu2, &pred);

        /* [6] FDIR gate: innovation gate of each reading vs its prediction
         *     → final health flags (restrict-only). */
        fdir_gate(&imu1, &imu2, &baro, &mag, &pred, &health, &gate);

        /* [7] Estimator correct: Kalman update on healthy channels only.
         *     NULL = isolated (post-gate verdict). */
        const imu_reading_t  *est_imu1 = health.imu1_healthy ? &imu1 : (const imu_reading_t *)0;
        const imu_reading_t  *est_imu2 = health.imu2_healthy ? &imu2 : (const imu_reading_t *)0;
        const mag_reading_t  *est_mag  = health.mag_healthy  ? &mag  : (const mag_reading_t *)0;
        /* Baro is not an estimator input (D062) — altitude is unobservable in the
         * attitude-only state; FDIR still consumes baro for health monitoring. */
        (void)estimator_update(est_imu1, est_imu2, est_mag, &health, &estimate);

        /* [8] Control law: attitude error → deflection commands.
         *     Reads system mode from mode_fsm (set by previous tick's [12]). */
        (void)control_law_update(&estimate, &health, mode_fsm_get(), &cmd);

        /* [9] Actuators: clamp to mode limits, write PWM. */
        actuators_write(&cmd);

        /* [10] Telemetry: pack downlink frame, CRC, queue to SiK DMA TX. */
        telemetry_pack_and_send(&imu1, &imu2, &baro, &estimate, &cmd, &health, mode_fsm_get());

        /* [11] + [12] C2 receive and Mode FSM update. The parsed command is
         *            passed straight to mode_fsm; NULL if no frame this tick. */
        if (c2_receive(&c2_cmd)) {
            (void)mode_fsm_update(&c2_cmd, &health);
        } else {
            (void)mode_fsm_update((const command_frame_t *)0, &health);
        }

        /* [13] Watchdog kick — main-loop heartbeat. */
        platform_watchdog_kick();

        /* [14] Wait for next tick. Real implementation will spin on
         *      tick_flag (set by TIM2 update ISR), clear it atomically,
         *      and check for overrun (TIM2 elapsed > loop period since
         *      previous tick entry → assert and halt to safe state).
         *      tick_flag clear is essential scaffolding — unavoidable. */
        while (tick_flag == 0U) {
            /* spin until ISR fires */
        }
        tick_flag = 0U;
    }

    /* Unreachable. */
    return 0;
}
