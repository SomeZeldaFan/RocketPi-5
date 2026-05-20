#ifndef PLATFORM_H
#define PLATFORM_H

#include "avionics_types.h"

/*
 * platform.h — MCU clock config, peripheral init, IWDG watchdog, TIM2
 *
 * Layer: hardware. Owns all bare-metal MCU configuration that must precede
 * any peripheral driver init. Owns TIM2 (the authoritative time source per
 * D034) and the IWDG hardware watchdog.
 *
 * Watchdog discipline:
 *   - IWDG is started inside platform_init() BEFORE any peripheral init.
 *     If a peripheral init hangs (stuck SPI bus, unresponsive sensor), the
 *     watchdog times out and resets the MCU rather than hanging forever.
 *   - Timeout is set generously (~2–4 s) to cover the full init sequence.
 *   - Each module's _init() calls platform_watchdog_kick() on completion.
 *   - In steady state, the main loop kicks the watchdog every ~1 ms at
 *     tick [11]. The 2–4 s timeout becomes very conservative — fine.
 *
 * DEFERRED (see 02-current-state.md):
 *   - Watchdog architecture scrutiny: timeout value, kick placement,
 *     IWDG vs WWDG tradeoff, behavior on watchdog reset vs assert reset.
 *   - platform_safe_state() removed pending resolution of how the hardware
 *     layer can drive actuators safe without violating the layer rule
 *     (hardware layer cannot include output layer headers).
 */

/*
 * Start the platform. Must be called before any other _init() in the system.
 * Order of operations: enable IWDG with conservative timeout → configure
 * clocks → configure TIM2 → kick watchdog.
 */
void platform_init(void);

/*
 * Read the authoritative microsecond timestamp from TIM2 (D034).
 * Every sensor reading, telemetry frame, and event log is timestamped using
 * this function. No other clock source is authoritative.
 */
uint32_t platform_timer_us(void);

/*
 * Pet the IWDG watchdog. Callable from module _init() functions during
 * boot AND from the main loop tick [11] in steady state. Not exclusively
 * a main-loop call.
 */
void platform_watchdog_kick(void);

#endif /* PLATFORM_H */
