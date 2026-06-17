#ifndef MS5611_H
#define MS5611_H

#include "avionics_types.h"

/*
 * ms5611.h — barometer driver (MS5611-01BA03 via GY-63 module)
 *
 * Layer: hardware. The barometer sits alone on its own I2C bus (D026) — a
 * dissimilar sensor on a dissimilar bus, providing cross-modal sensor
 * diversity for FDIR (D027). An I2C hang on this bus loses the baro only,
 * never the IMUs.
 *
 * The MS5611 requires multiple I2C transactions per reading: trigger
 * pressure conversion → wait → read raw → trigger temperature conversion →
 * wait → read raw → apply compensation. The driver implements this as an
 * internal state machine across calls.
 *
 * Decimation: the main loop calls ms5611_service() every tick. The driver
 * tracks elapsed time and only emits a new reading at ~50 Hz
 * (AVIONICS_BARO_RATE_HZ). Most calls return without producing output.
 */

/*
 * Configure I2C, reset and load PROM calibration coefficients from the
 * MS5611. Must be called after platform_init(). Does NOT kick the watchdog
 * — no per-_init() kicks (D053 A3).
 */
void ms5611_init(void);

/*
 * Service the internal I2C state machine. On most ticks this returns without
 * filling out; on a decimation boundary it produces a fresh compensated
 * reading with altitude derived from pressure using a standard sea-level
 * reference. Returns BARO_OK when a new reading is produced; otherwise
 * returns a status describing the current state or fault condition.
 * Caller MUST check the status before using pressure_pa/altitude_m.
 */
baro_status_t ms5611_service(baro_reading_t *out);

#endif /* MS5611_H */
