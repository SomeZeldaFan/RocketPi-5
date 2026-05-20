#ifndef C2_H
#define C2_H

#include "avionics_types.h"

/*
 * c2.h — command-and-control uplink parser (RX ONLY)
 *
 * Layer: output. This module drains the SiK RX ring buffer, validates
 * the CRC, and deserialises one command_frame_t. It does NOT touch the
 * TX path — that lives in telemetry.h. The TX/RX split is enforced by
 * keeping the modules separate and never granting one access to the
 * other's transport entry points.
 *
 * Strict scope:
 *   - OUTPUT side only on the protocol layer: RX path from ground station
 *     to MCU. (The "output" layer label comes from the architectural layer
 *     hierarchy; conceptually it's the "uplink protocol input".)
 *   - No TX, no telemetry packing, no frame production.
 *   - Returns a single parsed command_frame_t per call; if multiple frames
 *     are buffered, the next call drains the next one.
 *
 * Protocol validation:
 *   - protocol_version must match AVIONICS_PROTOCOL_VERSION; otherwise
 *     the frame is silently discarded (a future iteration may surface
 *     this as a fault flag to the GCS).
 *   - CRC-16/CCITT must validate; otherwise the frame is silently discarded.
 *   - Both of the above failure modes will be addressed in the per-module
 *     scrutiny session; the current interface returns a simple boolean.
 */

void c2_init(void);

/*
 * Attempt to deserialise one command frame from the SiK RX ring buffer.
 * Returns true and fills *out on a complete, version-matched, CRC-valid
 * frame. Returns false if no such frame is available this tick.
 */
bool c2_receive(command_frame_t *out);

#endif /* C2_H */
