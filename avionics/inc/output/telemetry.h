#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "avionics_types.h"

/*
 * telemetry.h — downlink frame packer (TX ONLY)
 *
 * Layer: output. This module packs the per-tick state into a
 * telemetry_frame_t, computes the CRC-16/CCITT, and hands the frame to
 * sik_radio_tx_send(). It does NOT handle uplink — that lives in c2.h.
 *
 * Strict scope:
 *   - INPUT side only: TX path from MCU to ground station.
 *   - No RX, no protocol negotiation, no command parsing.
 *   - No retransmission logic; if the radio TX queue is full, the frame
 *     is dropped this tick and the next tick's frame becomes the next
 *     downlink. Frame_id increments unconditionally so the GCS can
 *     detect dropped frames.
 *
 * Frame format invariants (§4.4):
 *   - protocol_version is the first field. GCS rejects frames with a
 *     version other than AVIONICS_PROTOCOL_VERSION.
 *   - health_flags_t is unconditionally present. Health state is never
 *     omitted from a downlink frame, never optional.
 *   - fdir_gate_result_t and reset_cause are likewise unconditionally
 *     present (protocol v4, D063 / D053 A4/A6): the dashboard always sees the
 *     live chi²/gate/staleness/gyro-disagree signals and the last reboot cause.
 *   - timestamp_us is from TIM2 (D034).
 */

void telemetry_init(void);

/*
 * Pack the per-tick state into a telemetry_frame_t, compute CRC, and
 * dispatch via sik_radio_tx_send(). All input pointers must be non-NULL
 * — even isolated sensors produce a reading struct with an appropriate
 * status field (callers do not pass NULL for sensors here; that NULL
 * semantic applies only at the estimator boundary). gate carries the FDIR
 * innovation-gate outputs and reset_cause the classified last-reboot cause
 * (both downlinked unconditionally, protocol v4 — D063 / D053).
 */
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
);

#endif /* TELEMETRY_H */
