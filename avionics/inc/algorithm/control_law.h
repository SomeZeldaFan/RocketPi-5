#ifndef CONTROL_LAW_H
#define CONTROL_LAW_H

#include "avionics_types.h"

/*
 * control_law.h — attitude control law and actuator allocation
 *
 * Layer: algorithm. Reads the attitude estimate plus health flags and
 * system mode, produces a per-fin deflection command. Reconfigures the
 * actuator mixing matrix when actuator faults are detected, falling
 * through the documented degradation chain:
 *   FULL_AUTHORITY  → 3FIN_REDUCED  → 2FIN_REDUCED  → SAFE_HOLD.
 *
 * D030: four fins in "+" configuration, independently actuated. Three
 * remaining fins can still provide 3-axis control after a single fin
 * failure — that is the whole point of the four-fin choice.
 *
 * Mode-aware behaviour:
 *   - SYS_MODE_FLIGHT:    real deflection limits (e.g. ±3°)
 *   - SYS_MODE_DEMO:      relaxed deflection limits for presentation
 *   - SYS_MODE_SAFE_HOLD: zero deflection
 * Mode limits are enforced downstream in actuators_write() — control_law
 * may command beyond the limit; the actuator layer hard-clamps.
 *
 * Covariance from the estimator should inform control-law authority — high
 * covariance (low confidence) should produce gentler corrections. The
 * specific blending is an implementation detail for the per-module scrutiny
 * session; the interface gives the control law everything it needs.
 */

void control_law_init(void);

/*
 * Compute the per-fin deflection command for this tick.
 *   est:        current attitude estimate with covariance and mode
 *   health:     authoritative health flags (from FDIR)
 *   sys_mode:   current system mode (from mode FSM)
 *   out:        filled with per-fin deflections and a control_mode_t
 *
 * Returns the control_mode_t the law selected this tick.
 */
control_mode_t control_law_update(
    const attitude_estimate_t *est,
    const health_flags_t      *health,
    system_mode_t              sys_mode,
    actuator_cmd_t            *out
);

#endif /* CONTROL_LAW_H */
