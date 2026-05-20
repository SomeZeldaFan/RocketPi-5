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
 *     A hung peripheral init will then time out the watchdog and reset the
 *     MCU rather than hanging forever.
 *   - Hardware drivers next, in stable order. Each _init() pets the watchdog
 *     on completion (kick is inside the driver's _init, not here).
 *   - Algorithm modules next, then output, then orchestration.
 *   - Loop body executes the 12 tick steps in the architectural order.
 *     The order is load-bearing: FDIR runs before estimator so the estimator
 *     sees authoritative health flags; estimator before control law so the
 *     law sees the latest attitude; etc.
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
    static health_flags_t      health;
    static fdir_gate_result_t  gate;
    static attitude_estimate_t estimate;
    static actuator_cmd_t      cmd;
    static command_frame_t     c2_cmd;

    /* ------------------------- Main loop ------------------------- */
    for (;;) {
        /* [1] Consume latest BMI160 sample (DMA-filled). */
        (void)bmi160_read(&imu1);

        /* [2] Consume latest ICM-42688-P sample (DMA-filled). */
        (void)icm42688p_read(&imu2);

        /* [3] Service the barometer state machine (decimated to ~50 Hz internally). */
        (void)ms5611_service(&baro);

        /* [4] FDIR: staleness, bounds, innovation gate → health flags. */
        fdir_update(&imu1, &imu2, &baro, &health, &gate);

        /* [5] Estimator: predict+update with healthy sensors. NULL = isolated.
         *     Real implementation will use the health flags to choose NULL
         *     vs the actual pointer per sensor. */
        (void)estimator_update(&imu1, &imu2, &baro, &health, &estimate);

        /* [6] Control law: attitude error → deflection commands.
         *     Reads system mode from mode_fsm (set by previous tick's [10]). */
        (void)control_law_update(&estimate, &health, mode_fsm_get(), &cmd);

        /* [7] Actuators: clamp to mode limits, write PWM. */
        actuators_write(&cmd);

        /* [8] Telemetry: pack downlink frame, CRC, queue to SiK DMA TX. */
        telemetry_pack_and_send(&imu1, &imu2, &baro, &estimate, &cmd, &health, mode_fsm_get());

        /* [9] + [10] C2 receive and Mode FSM update. The parsed command is
         *           passed straight to mode_fsm; NULL if no frame this tick. */
        if (c2_receive(&c2_cmd)) {
            (void)mode_fsm_update(&c2_cmd, &health);
        } else {
            (void)mode_fsm_update((const command_frame_t *)0, &health);
        }

        /* [11] Watchdog kick — main-loop heartbeat. */
        platform_watchdog_kick();

        /* [12] Wait for next tick. Real implementation will spin on
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
