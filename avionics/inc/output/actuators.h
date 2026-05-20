#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "avionics_types.h"

/*
 * actuators.h — PWM driver for the four control-surface servos
 *
 * Layer: output. Takes a per-fin deflection command from the control law
 * and writes the PWM duty cycle to the corresponding timer channel. Hard-
 * clamps deflections to the limits imposed by the current system_mode_t
 * (flight vs demo) before writing — the hardware never sees a command
 * that exceeds the configured envelope.
 *
 * D030: four fins in "+" configuration, each on its own PWM channel.
 * D033: servos draw from the high-current Rail B; this module assumes
 *       Rail A (clean rail) is stable for the MCU PWM peripheral itself.
 *       Power sequencing is platform_init()'s responsibility.
 *
 * The actuators module exposes a "safe" entry that drives all fins to
 * zero deflection regardless of input — used during boot, during faults
 * before the control law produces its first output, and during safe-hold.
 */

void actuators_init(void);

/*
 * Write per-fin PWM deflections from cmd. Hard-clamps to mode-appropriate
 * limits before writing. Caller does not need to clamp upstream — that is
 * this module's responsibility.
 */
void actuators_write(const actuator_cmd_t *cmd);

/*
 * Drive all four fins to zero deflection. Always callable, no preconditions
 * other than actuators_init() having run.
 */
void actuators_safe(void);

#endif /* ACTUATORS_H */
