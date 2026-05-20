#ifndef MODE_FSM_H
#define MODE_FSM_H

#include "avionics_types.h"

/*
 * mode_fsm.h — system operating mode state machine
 *
 * Layer: orchestration. Tracks the current system_mode_t (flight / demo /
 * safe hold) and validates mode transitions against system health.
 *
 * Inputs:
 *   - Parsed command_frame_t from c2 (may be NULL on ticks with no command)
 *   - Authoritative health_flags_t from FDIR
 *
 * Outputs:
 *   - The active system_mode_t (returned per-tick and queryable via _get)
 *   - Mode-change acknowledgments queued for downlink (D020 — bidirectional)
 *
 * Mode transition rules (per D020):
 *   - FLIGHT ↔ DEMO transitions are operator-initiated via C2 commands.
 *   - Any transition is rejected if system health makes it unsafe (e.g.
 *     attempting to enter FLIGHT mode while EST_MODE_FAULT is active).
 *   - SAFE_HOLD can be entered from any state; entering it is always allowed.
 *   - SAFE_HOLD → FLIGHT/DEMO requires healthy system state.
 */

void mode_fsm_init(void);

/*
 * Advance the FSM by one tick.
 *   cmd:    parsed command frame this tick, or NULL if none received.
 *   health: current authoritative health flags from FDIR.
 *
 * Returns the active mode for this tick (post-transition).
 */
system_mode_t mode_fsm_update(
    const command_frame_t *cmd,
    const health_flags_t  *health
);

/* Read the active mode without advancing the FSM. */
system_mode_t mode_fsm_get(void);

#endif /* MODE_FSM_H */
