#include "hardware/sik_radio.h"
#include "hardware/platform.h"

/*
 * sik_radio.c — STUB
 *
 * Real implementation will:
 *   - Configure UART1 with circular-DMA RX into a statically allocated ring
 *     buffer (size TBD; bounded — JPL Rule 3 forbids dynamic allocation).
 *   - Configure UART1 TX with DMA on a per-frame basis (single-shot DMA
 *     transfer per telemetry frame).
 *   - Provide a simple frame delimiter / length-prefixed framing on top
 *     of the SiK transparent serial bridge so that RX can identify frame
 *     boundaries in the byte stream.
 *
 * The ring buffer write pointer is advanced by the UART RX DMA hardware /
 * ISR; the read pointer is advanced by sik_radio_rx_read(). The pair of
 * pointers crosses the ISR/main-loop boundary but is fully encapsulated
 * inside this module — the main loop never reads them directly, which is
 * why they are not exposed in isr_flags.h.
 */

void sik_radio_init(void)
{
    /*
     * Real implementation: configure UART1+DMA circular RX, prepare TX DMA
     * stream (not started until first sik_radio_tx_send), kick watchdog.
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}

bool sik_radio_rx_pending(void)
{
    /*
     * Real implementation: compare RX ring buffer write pointer (advanced
     * by DMA/ISR) against read pointer. Return true if at least one full
     * frame is buffered.
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    return false;
}

bool sik_radio_rx_read(command_frame_t *out)
{
    /*
     * Real implementation: copy the next framed packet bytes from the RX
     * ring buffer into a local staging area, deserialise into *out. CRC
     * validation is NOT done here — c2 handles that.
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    (void)out;
    return false;
}

bool sik_radio_tx_send(const telemetry_frame_t *frame)
{
    /*
     * Real implementation: copy the frame into a statically allocated TX
     * staging buffer (if not already in DMA transfer), kick off DMA TX,
     * return true. If a TX is already in flight, return false so the caller
     * knows the frame was dropped this tick.
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    (void)frame;
    return false;
}
