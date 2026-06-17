#ifndef PLATFORM_H
#define PLATFORM_H

#include "avionics_types.h"

/*
 * platform.h — MCU clock config, peripheral init, IWDG watchdog, TIM2, NVIC
 *
 * Layer: hardware. Owns all bare-metal MCU configuration that must precede any
 * peripheral driver init. Owns TIM2 (the authoritative time source per D034),
 * the IWDG hardware watchdog, the NVIC interrupt priority scheme (D048), and the
 * boot-time reset-cause readout (D053 A4).
 *
 * Contract status: LOCKED (platform.h scrutiny session, 2026-06-17 — D048, D053).
 * The bodies in platform.c remain stubs (D045: tests before implementation); this
 * header is the locked contract those tests are authored against.
 *
 * ---------------------------------------------------------------------------
 * Watchdog discipline (D053 A1–A3):
 *   - IWDG only; WWDG is not used (D053 A1). IWDG is LSI-clocked, so it survives a
 *     hung clock-tree bring-up — the primary threat.
 *   - IWDG is started as the FIRST action in platform_init(), before clock or
 *     peripheral config, so any subsequent hang causes a reset rather than a hang.
 *   - Prescaler /32, reload RLR = 2937 → timeout BAND 2.00 s (fast LSI 47 kHz) /
 *     2.94 s (typ 32 kHz) / 5.53 s (slow 17 kHz). The fast corner is anchored at
 *     2.00 s because the shortest timeout governs false-trips. LSI tolerance
 *     (17/32/47 kHz) makes a single guaranteed number impossible with a fixed
 *     reload; the band is the honest spec.
 *   - NO kicks occur during the init sequence (a per-init kick would MASK a hang
 *     that strikes AFTER a module's kick). The whole init runs inside one window,
 *     budgeted to < 1.0 s = 50% of the fast-corner timeout (TEST-PLT-HW-005).
 *   - The FIRST kick is the boot-complete kick in main.c at the init→loop boundary
 *     (a known-good "all inits returned" assertion that also resets the window);
 *     thereafter the main loop kicks every iteration at tick [13].
 *
 * Reset-cause readout (D053 A4):
 *   platform_init() reads RCC_CSR early, classifies the cause, then clears the
 *   flags (RMVF) so the next boot reads clean. Exposed via platform_reset_cause().
 *
 * Fail-fast convention (D053 A7 / C2–C3):
 *   platform_init() (and every module _init()) returns void. Internal failures — a
 *   HAL call returning error, a bounded spin-wait timing out — fire ASSERT() (see
 *   fault.h), routing to system_safe_halt(). Reaching the boot-complete kick proves
 *   every init succeeded. The HSE-ready / PLL-lock waits are bounded (JPL Rule 2),
 *   never while(!RDY){}.
 *
 * NVIC interrupt priority scheme (D048):
 *   Grouping NVIC_PRIORITYGROUP_4 (4 preempt bits, 0 sub). Priorities:
 *       TIM2 update (loop tick) ........ 0  (highest)
 *       SPI1/SPI2 DMA-TC (IMU-1/2) ..... 1
 *       I2C DMA-TC (baro) .............. 1
 *       UART1 DMA-RX (radio) ........... 1
 *       SysTick (HAL only) ............. 15 (lowest)
 *   Priority does NOT provide boundary-variable consistency — that is structural
 *   (single word-aligned access + single-writer-per-variable). Priority allocates
 *   only latency/jitter: TIM2 highest protects the authoritative-clock timing;
 *   the DMA-TC bookkeeping tolerates sub-µs delay; SysTick never perturbs the loop.
 *   The ONE main-loop read-modify-write (clear a data_ready flag while snapshotting
 *   its double-buffer page index) is protected by a BASEPRI critical section:
 *       __set_BASEPRI(1u << (8u - __NVIC_PRIO_BITS));  // = 0x10: mask pri>=1 (DMA-TC); TIM2@0 stays live
 *       <snapshot page index; clear flag>              // microscopic
 *       __set_BASEPRI(0u);                             // then consume buffer OUTSIDE
 *   platform.c carries _Static_assert(__NVIC_PRIO_BITS == 4) so the shift cannot
 *   silently break. The IMU double-buffer page index is an ISR<->main crossing and
 *   is listed in isr_flags.h.
 *   >>> IMPLEMENTATION CAUTION: the BASEPRI shift, the mask semantics, and the
 *   "TIM2 stays live" property must be verified with extreme care and proven under
 *   TEST-PLT-HW-007 contention before this mechanism is trusted. <<<
 *
 * (platform_safe_state() is intentionally absent: a hardware-layer safe-state is
 *  delivered by the link-time system_safe_halt() in fault.h, not by this module —
 *  D053 A7.)
 */

/*
 * Start the platform. Must be called before any other _init() in the system.
 * Order: enable IWDG (FIRST, band per D053 A2) → read+classify+clear RCC_CSR reset
 * cause (D053 A4) → configure clocks (bounded HSE/PLL waits) → configure TIM2
 * (1 MHz, D034) → set NVIC grouping + priorities (D048) → configure GPIO/peripheral
 * clocks. void + fail-fast: any failure fires ASSERT (fault.h). Does NOT kick the
 * watchdog — the first kick is the boot-complete kick in main.c.
 */
void platform_init(void);

/*
 * Read the authoritative microsecond timestamp from TIM2 (D034).
 * TIM2 is a free-running 32-bit counter clocked at 1 MHz, so the count IS the
 * microsecond timestamp. Derivation (C6): TIM2 is on APB1; at 168 MHz SYSCLK the
 * APB1 timer clock is 84 MHz (APB1 capped at 42 MHz, ×2 timer doubling — see
 * datasheets/STM32F4xx.pdf p.122 "Characteristics of TIMx connected to the APB1
 * domain"), so prescaler 83 gives 84 MHz / 84 = 1 MHz.
 *
 * Wrap-safe use (C4): the counter rolls over every 2^32 us ≈ 71.58 min — a known,
 * expected event, NOT a fault. Compute intervals as (now - then) in uint32_t
 * (correct for any interval < 71.58 min); NEVER order two absolute timestamps with
 * < or > (wrong across a rollover). A 64-bit total-elapsed time, if ever needed
 * (e.g. soak duration), is built on top of this primitive — not exposed here.
 *
 * JPL Rule 5 exemption: this is the hottest function in the system (called for
 * every timestamp) and the register read cannot fail, so it carries no assertions.
 */
uint32_t platform_timer_us(void);

/*
 * Pet the IWDG watchdog. Called from exactly two sites (D053 A3):
 *   - once at the boot-complete boundary in main.c (after all _init() return,
 *     before entering the loop), and
 *   - every main-loop iteration at tick [13].
 * NEVER from a module _init() — a per-init kick would mask a post-kick hang.
 * JPL Rule 5 exemption: a single key-register write with no meaningful precondition.
 */
void platform_watchdog_kick(void);

/*
 * Return the cause of the most recent reset, classified by platform_init() from
 * RCC_CSR at boot (D053 A4). The ordered classification (SOFTWARE → WATCHDOG →
 * POWER_ON → BROWNOUT → PIN-if-alone → UNKNOWN) returns the first specific match,
 * so RESET_PIN is reported only when PINRSTF is the sole flag set (NRST is
 * bidirectional, so PINRSTF co-sets on nearly every reset). WATCHDOG and BROWNOUT
 * are the fault causes that drive the post-reboot safe-state path (consumed by the
 * FSM/telemetry per D053 A6). The cause is held in RAM (no backup domain). Returns
 * RESET_UNKNOWN if no flag matched.
 */
reset_cause_t platform_reset_cause(void);

#endif /* PLATFORM_H */
