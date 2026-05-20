#include "orchestration/mode_fsm.h"

/*
 * mode_fsm.c — STUB
 *
 * Real implementation will:
 *   - Maintain a static current-mode variable, initially SYS_MODE_SAFE_HOLD
 *     (boot starts safe; FLIGHT or DEMO must be commanded explicitly).
 *   - On each _update() call:
 *       1. If cmd is non-NULL, evaluate the requested transition against
 *          the current health flags. Reject unsafe transitions (e.g.
 *          attempting to enter FLIGHT while EST_MODE_FAULT is active).
 *       2. If the transition is permitted, update current mode and queue
 *          an ack frame for telemetry (the ack mechanism is part of the
 *          telemetry/c2 protocol; details deferred to implementation).
 *       3. If health degrades to a point that the current mode is no longer
 *          safe, demote to SAFE_HOLD automatically (operator can re-enter
 *          FLIGHT/DEMO once health recovers).
 *
 * Per D020: FLIGHT mode applies real deflection limits; DEMO mode applies
 * relaxed limits for presentation legibility. Both are operator-selected;
 * SAFE_HOLD is automatic on health failure or boot.
 */

void mode_fsm_init(void)
{
    /* Real implementation: set current mode to SYS_MODE_SAFE_HOLD. */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}

system_mode_t mode_fsm_update(
    const command_frame_t *cmd,
    const health_flags_t  *health
)
{
    /* Real implementation: evaluate cmd against health, transition or
     * reject, return the active mode. */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    (void)cmd;
    (void)health;
    return SYS_MODE_SAFE_HOLD;
}

system_mode_t mode_fsm_get(void)
{
    /* Real implementation: return the static current-mode variable. */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    return SYS_MODE_SAFE_HOLD;
}
