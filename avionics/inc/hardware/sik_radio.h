#ifndef SIK_RADIO_H
#define SIK_RADIO_H

#include "avionics_types.h"

/*
 * sik_radio.h — raw UART+DMA transport for the HolyBro SiK V3 radio
 *
 * Layer: hardware. This module owns ONLY the byte transport — UART
 * configuration, DMA-driven RX ring buffer, DMA-driven TX queue. It does
 * NOT understand frames, CRCs, or protocol semantics. Those live in
 * telemetry.c (TX side) and c2.c (RX side).
 *
 * Per D028: the radio terminates at the MCU on a UART with DMA. No comms
 * coprocessor. The protocol stack lives in MCU firmware, JPL-Power-of-10
 * compliant. At LoRa-class data rates the DMA path makes CPU overhead
 * effectively zero.
 *
 * Per D029: an additional wired UART carries the same protocol for dev/debug
 * and as a demo fallback. The application layer is transport-agnostic;
 * runtime selection between radio and wired transports happens above this
 * module (specifically inside telemetry/c2 in this scaffold; transport
 * abstraction is a future refactor if the wired channel earns its keep).
 */

/* Configure UART1 + DMA RX and TX, start the RX ring buffer. */
void sik_radio_init(void);

/* True if the RX ring buffer contains at least one complete-looking frame. */
bool sik_radio_rx_pending(void);

/*
 * Drain one frame from the RX ring buffer into *out. Returns true on a
 * complete read, false if no full frame is available this call. CRC
 * validation is NOT performed here — that is c2's responsibility.
 */
bool sik_radio_rx_read(command_frame_t *out);

/*
 * Queue a downlink frame for DMA TX. Returns true if successfully queued,
 * false if the TX path is full (caller should drop or retry next tick).
 */
bool sik_radio_tx_send(const telemetry_frame_t *frame);

#endif /* SIK_RADIO_H */
