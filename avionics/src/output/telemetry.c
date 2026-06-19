#include "output/telemetry.h"
#include "hardware/sik_radio.h"
#include "hardware/platform.h"

/*
 * telemetry.c — STUB (TX side only)
 *
 * Real implementation will:
 *   - Maintain a monotonically increasing frame_id counter (incremented
 *     every call, regardless of whether the radio TX queue accepted the
 *     frame — the GCS uses gaps in frame_id to detect drops).
 *   - Pack the per-tick state into a telemetry_frame_t with
 *     protocol_version = AVIONICS_PROTOCOL_VERSION as the first field and
 *     timestamp_us = platform_timer_us() from the authoritative MCU clock.
 *   - Compute CRC-16/CCITT over the entire frame (except the crc16 field
 *     itself), store it in the trailing crc16 field.
 *   - Call sik_radio_tx_send(); on a queue-full return, log the drop
 *     (future telemetry counter) but do not block — drop and move on.
 *
 * Strict scope: this module is TX only. RX is c2's responsibility.
 */

void telemetry_init(void)
{
    /* Real implementation: reset frame_id to 0, clear staging buffer. */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}

void telemetry_pack_and_send(
    const imu_reading_t       *imu1,
    const imu_reading_t       *imu2,
    const baro_reading_t      *baro,
    const attitude_estimate_t *est,
    const actuator_cmd_t      *act,
    const health_flags_t      *health,
    const fdir_gate_result_t  *gate,
    reset_cause_t              reset_cause,
    system_mode_t              sys_mode
)
{
    /* Real implementation: build the telemetry_frame_t in a static staging
     * struct, copy the inputs in (incl. gate + reset_cause, protocol v4 / D063),
     * compute CRC, hand to sik_radio_tx_send(). */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    (void)imu1;
    (void)imu2;
    (void)baro;
    (void)est;
    (void)act;
    (void)health;
    (void)gate;
    (void)reset_cause;
    (void)sys_mode;
}
