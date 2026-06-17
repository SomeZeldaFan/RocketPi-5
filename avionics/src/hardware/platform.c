#include "hardware/platform.h"
#include "fault.h"

/*
 * platform.c — STUB
 *
 * No functional code. Bodies describe the real implementation (locked to D048 +
 * D053 in the platform.h scrutiny session, 2026-06-17) plus a PLACEHOLDER RETURN
 * where applicable. The contract in platform.h is authoritative.
 *
 * Real implementation will:
 *   - Start IWDG FIRST (prescaler /32, RLR = 2937 → 2.00/2.94/5.53 s band, D053 A2)
 *     so a hang in any subsequent step resets rather than hangs (D053 A1/A3).
 *   - Read RCC_CSR, classify the reset cause (ordered: SFTRSTF → IWDGRSTF → PORRSTF
 *     → BORRSTF → PINRSTF-if-alone → UNKNOWN), store it, then clear via RMVF (D053 A4).
 *   - Configure the system clock tree (HSE → PLL → SYSCLK 168 MHz) with BOUNDED
 *     HSE-ready / PLL-lock waits (JPL Rule 2 — never while(!RDY){}).
 *   - Configure TIM2 as a free-running 32-bit 1 MHz counter: APB1 timer clock 84 MHz,
 *     prescaler 83 → 1 MHz → count == µs (D034; derivation in platform.h C6).
 *   - Set NVIC grouping NVIC_PRIORITYGROUP_4 and the D048 priorities (TIM2=0,
 *     DMA-TC=1, SysTick=15), then GPIO/peripheral clocks.
 *   - Check every HAL return (JPL Rule 7); on any failure, ASSERT() (fault.h) →
 *     system_safe_halt(). void + fail-fast (D053 A7 / C2–C3).
 *   - NOT kick the watchdog here — the first kick is the boot-complete kick in main.c.
 *
 * Real implementation MUST also carry, alongside the BASEPRI critical-section macro
 * the drivers use (D048):
 *     _Static_assert(__NVIC_PRIO_BITS == 4, "BASEPRI mask shift assumes 4 prio bits");
 * so the `1u << (8u - __NVIC_PRIO_BITS)` mask cannot silently break. (Deferred to
 * implementation — __NVIC_PRIO_BITS comes from the CMSIS device header, not yet in
 * the build.)
 */

void platform_init(void)
{
    /*
     * Real implementation: see platform.h and the header above. IWDG is started
     * first so any subsequent hang (clock config, peripheral init) causes a
     * watchdog reset rather than an indefinite hang.
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     */
}

uint32_t platform_timer_us(void)
{
    /*
     * Real implementation: read TIM2->CNT and return it. TIM2 is configured in
     * platform_init() to tick at 1 MHz so the count is directly the microsecond
     * timestamp (D034). The read cannot fail — there is no degraded-mode return to
     * design, and (per JPL Rule 5 exemption, platform.h) no assertion. Callers use
     * (now - then) for wrap-safe intervals; never order absolute timestamps (C4).
     */

    /* PLACEHOLDER RETURN — stub artifact, NOT a safe default.
     * 0U is what the stub returns; the real function returns TIM2->CNT. Because a
     * constant 0 makes every dt zero (breaking monotonicity, TEST-PLT-HW-001, and
     * every estimator dt), this stub MUST NOT be linked into any timing-dependent
     * path. The placeholder is removed at implementation, not "defaulted."
     */
    return 0U;
}

void platform_watchdog_kick(void)
{
    /*
     * Real implementation: write the IWDG key register to refresh the countdown.
     * Called from exactly two sites (D053 A3): once at the boot-complete boundary
     * in main.c (after all _init() return, before the loop), and every main-loop
     * iteration at tick [13]. NEVER from a module _init() — a per-init kick would
     * mask a hang that strikes after that module's kick.
     */

    /* PLACEHOLDER RETURN — NOT CONFIRMED SAFE.
     * This value has not been reviewed for correctness or safety.
     * Do not rely on this stub's output for any system behavior.
     */
}

reset_cause_t platform_reset_cause(void)
{
    /*
     * Real implementation: return the cause classified and stored by platform_init()
     * from RCC_CSR at boot (D053 A4). Held in a module-static RAM variable (no backup
     * domain); the hardware flags were cleared via RMVF after the read.
     */

    /* PLACEHOLDER RETURN — RESET_UNKNOWN is the genuine safe default ("we don't
     * know"). The real function returns the classified cause.
     */
    return RESET_UNKNOWN;
}
