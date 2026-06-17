#include "output/actuators.h"
#include "hardware/platform.h"

/*
 * actuators.c — STUB
 *
 * Real implementation will:
 *   - Configure four PWM channels on the appropriate timer (TIM3 or TIM4),
 *     50 Hz frame with 1.0–2.0 ms pulse range mapped to ±max-deflection.
 *   - actuators_write(): hard-clamp each deflection to the mode-appropriate
 *     limit (FLIGHT vs DEMO), convert radians to pulse width, write the
 *     timer compare registers.
 *   - actuators_safe(): write the neutral 1.5 ms pulse to all four channels
 *     (zero deflection). Always callable, idempotent.
 *
 * Mode-dependent deflection limits (TBD — implementation session):
 *   - SYS_MODE_FLIGHT:    ±3° (or LR-4-derived limit)
 *   - SYS_MODE_DEMO:      ±15° (relaxed for presentation)
 *   - SYS_MODE_SAFE_HOLD: 0° (no actuation)
 *
 * System mode is read from mode_fsm_get() inside actuators_write so the
 * mode-dependent clamp is enforced at the lowest level, not by any
 * upstream caller.
 */

void actuators_init(void)
{
    /* Real implementation: configure PWM peripheral, set all channels to
     * neutral (zero deflection), start PWM output. No watchdog kick (D053 A3). */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}

void actuators_write(const actuator_cmd_t *cmd)
{
    /* Real implementation: for each fin 0..3, clamp cmd->deflection_rad[i]
     * to the active mode limit, convert to pulse width, write to PWM
     * compare register. */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    (void)cmd;
}

void actuators_safe(void)
{
    /* Real implementation: write neutral pulse to all four channels. */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}
