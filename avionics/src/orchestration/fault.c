#include "fault.h"
#include "output/actuators.h"

/*
 * fault.c — STUB (orchestration layer)
 *
 * Defines system_safe_halt(), declared in the foundation header fault.h. Lives in
 * the orchestration layer because that layer legitimately sees both the hardware
 * and output layers — this is how a hardware- or algorithm-layer ASSERT drives the
 * actuators (output layer) safe without those layers including output/actuators.h
 * (D053 A7 / D048).
 *
 * Standalone translation unit (NOT in main.c): architecture §4.6 has test_main.c
 * REPLACE main.c in dev-PC test builds, so a handler defined in main.c would be an
 * undefined-symbol link error in every algorithm-module test that trips ASSERT.
 * fault.c links into both the real and the test builds.
 *
 * Real implementation:
 *   - actuators_safe()  — drive all fins to zero deflection. Boot-stage-safe: if
 *     called before actuators_init() it is a no-op (the fin PWM pins are still
 *     high-Z and therefore already safe).
 *   - __disable_irq()   — stop all interrupt activity.
 *   - for (;;) { }      — spin. It does NOT reset; the independent IWDG (D053 A1/A2)
 *     then fires and the reboot surfaces a WATCHDOG reset cause (D053 A4), so
 *     assertion failures and hardfaults funnel through one path.
 *   Never returns.
 */

void system_safe_halt(void)
{
    /*
     * Real implementation: actuators_safe(); __disable_irq(); for(;;){}.
     * See header above. No functional code in the stub beyond the spin that
     * preserves the never-returns contract.
     */

    /* PLACEHOLDER — NOT CONFIRMED SAFE. The real body drives the actuators safe and
     * disables interrupts BEFORE spinning. Spinning here (rather than returning) is
     * deliberate: system_safe_halt() must never return to a failed ASSERT. */
    for (;;) {
        /* spin until the IWDG resets the MCU */
    }
}
