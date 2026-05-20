/*
 * stm32f4xx_hal_msp.c — STUB
 *
 * HAL MCU Support Package (MSP) init/deinit callbacks live here. These
 * callbacks are invoked by HAL during peripheral init to configure GPIO
 * alternate functions, peripheral clocks, and DMA streams.
 *
 * Real implementation will provide:
 *
 *   HAL_MspInit() — top-level: enable SYSCFG and PWR clocks, set NVIC
 *                   priority grouping.
 *
 *   HAL_SPI_MspInit() — for each SPI peripheral (SPI1, SPI2):
 *                   - enable SPI clock
 *                   - enable GPIO clocks for SCK, MISO, MOSI, CS pins
 *                   - configure GPIO alternate function
 *                   - enable DMA controller clock
 *                   - configure DMA streams for TX and RX
 *                   - set NVIC priorities for DMA streams
 *
 *   HAL_I2C_MspInit() — for I2C1 (barometer):
 *                   - enable I2C clock, configure GPIO alt function
 *                   - configure DMA stream
 *                   - set NVIC priority
 *
 *   HAL_UART_MspInit() — for UART1 (SiK radio):
 *                   - enable UART clock, configure GPIO alt function for
 *                     TX and RX
 *                   - configure circular DMA RX stream and one-shot DMA
 *                     TX stream
 *                   - set NVIC priority
 *
 *   HAL_TIM_Base_MspInit() — for TIM2 (authoritative microsecond timer):
 *                   - enable TIM2 clock
 *                   - configure as 32-bit free-running counter at 1 MHz
 *
 *   Corresponding HAL_*_MspDeInit() functions for graceful teardown
 *   (used during peripheral reconfiguration / module reset paths).
 *
 * No register writes appear in this stub — implementation deferred to the
 * per-module scrutiny session for this file.
 */
