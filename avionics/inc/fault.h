#ifndef FAULT_H
#define FAULT_H

/*
 * fault.h — foundation: system-wide fault-halt primitive + ASSERT macro
 *
 * Layer: foundation (below all others). Includes nothing upward, so ANY layer
 * may include it without violating the layer rule. The declaration here is
 * resolved at LINK time against the definition in the orchestration layer
 * (orchestration/fault.c) — the inversion-of-control that lets a hardware-layer
 * assertion drive the actuators (an output-layer concern) to a safe state
 * without the hardware layer ever including output/actuators.h. (D053 A7 / D048.)
 *
 * system_safe_halt():
 *   The single sink for every detected-fault halt. Defined in
 *   orchestration/fault.c, where it may legally include output/actuators.h.
 *   Contract: drive the actuators to their safe (zero-deflection) state, disable
 *   interrupts, and spin. It does NOT reset — the independent IWDG (D053 A1/A2)
 *   then times out and the MCU reboots, surfacing the event as a WATCHDOG reset
 *   cause (D053 A4) so assertion failures and hardfaults funnel through one path.
 *   It never returns. Must be boot-stage-safe: callable before actuators_init(),
 *   where actuators_safe() is a no-op because the fin PWM pins are still high-Z
 *   (and therefore already safe). No function pointers (JPL Power-of-10 Rule 9).
 */
void system_safe_halt(void);

/*
 * ASSERT(cond) — project assertion (JPL Power-of-10 Rule 5).
 *
 * On a failed condition the system takes an explicit recovery action — it does
 * NOT continue and is NOT compiled out in any build. The recovery action is the
 * fail-fast safe halt above (D053 A7; the void fail-fast init convention, C2/C3).
 * A failure means an invariant is broken and continuing is more dangerous than
 * halting safe.
 *
 * The macro carries no file/line capture — system_safe_halt() takes no arguments
 * (locked signature). Post-mortem identification of the failing assertion is the
 * deferred hardfault-snapshot work (D053 A5), not this macro.
 */
#define ASSERT(cond)                 \
    do {                             \
        if (!(cond)) {               \
            system_safe_halt();      \
        }                            \
    } while (0)

#endif /* FAULT_H */
