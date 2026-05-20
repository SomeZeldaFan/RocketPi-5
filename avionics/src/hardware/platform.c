#include "hardware/platform.h"

/*
 * platform.c — STUB
 *
 * No functional code. All function bodies contain a comment describing the
 * real implementation plus a PLACEHOLDER RETURN where applicable.
 *
 * Real implementation will:
 *   - Configure system clock tree (HSE → PLL → SYSCLK at 168 MHz).
 *   - Enable IWDG with a generous (~2–4 s) timeout BEFORE peripheral init.
 *   - Configure TIM2 as a free-running 32-bit microsecond counter (D034).
 *   - Configure NVIC priorities, GPIO clocks, peripheral clocks.
 *   - Kick the watchdog before returning.
 */

void platform_init(void)
{
    /*
     * Real implementation: see header. The IWDG must be started first so
     * that any subsequent hang in clock or peripheral configuration causes
     * a watchdog reset rather than an indefinite hang.
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}

uint32_t platform_timer_us(void)
{
    /*
     * Real implementation: read TIM2->CNT and return it. TIM2 is configured
     * in platform_init() to tick at 1 MHz so the count is directly the
     * microsecond timestamp. This is the authoritative system clock (D034).
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
    return 0U;
}

void platform_watchdog_kick(void)
{
    /*
     * Real implementation: write the IWDG key register to refresh the
     * countdown. Called from module _init() functions during boot and
     * from the main loop tick [11] in steady state.
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     * Safe default will be defined and justified during the
     * per-module scrutiny session for this file.
     */
}
