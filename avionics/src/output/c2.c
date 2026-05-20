#include "output/c2.h"
#include "hardware/sik_radio.h"

/*
 * c2.c — STUB (RX side only)
 *
 * Real implementation will:
 *   - Call sik_radio_rx_pending() to test for a complete buffered frame.
 *   - On true, call sik_radio_rx_read() to dequeue raw bytes into a local
 *     command_frame_t.
 *   - Validate protocol_version against AVIONICS_PROTOCOL_VERSION. On
 *     mismatch, silently discard (future iteration: surface as a fault).
 *   - Compute CRC-16/CCITT over the frame (excluding crc16 field) and
 *     compare against the trailing crc16 field. On mismatch, silently
 *     discard.
 *   - On a fully-validated frame, write to *out and return true.
 *
 * Strict scope: this module is RX only. TX is telemetry's responsibility.
 */

void c2_init(void)
{
    /* Real implementation: nothing to initialise beyond the underlying
     * sik_radio_init() (which is called separately by main). */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}

bool c2_receive(command_frame_t *out)
{
    /* Real implementation: drain one frame from sik_radio_rx_read(),
     * validate version and CRC, return true on success. */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    (void)out;
    return false;
}
