# Test Plan / Test Log

**Status:** SKELETON — populated as tests are designed and run (starting Phase 2).
**Created:** 2026-05-11

This document captures both the planned tests (what we will verify, how, and against what acceptance criteria) and the executed tests (when run, results, pass/fail, notes).

Each test should be repeatable. If the procedure is not written down, the test is not a test — it's a one-off observation.

---

## Format

> **TEST-XXX-NNN — Short name**
> *Purpose:* What this test verifies.
> *Linked requirement(s):* REQ-XXX-NNN.
> *Procedure:* Step-by-step what to do.
> *Acceptance criteria:* What constitutes pass vs. fail.
> *Apparatus:* What you need to run the test.
>
> *Execution log:*
> - 2026-XX-XX: Run #1. Result: [pass/fail]. Notes: ...
> - 2026-XX-XX: Run #2. Result: [pass/fail]. Notes: ...

---

## Anticipated test categories

- Sensor characterization (IMU noise floor, magnetometer field map, barometer drift)
- Estimator nominal operation (attitude tracking accuracy on a tilt platform)
- Estimator dynamic response (step responses, settling times)
- Fault injection — single-sensor disconnection
- Fault injection — sensor disagreement (noise injection, bias injection)
- Fault injection — actuator loss (servo disable mid-run; verify control-allocation reconfiguration)
- Fault injection — compound (simultaneous sensor and actuator fault; verify both isolated, system continues)
- Control loop closed-loop response on bench
- Telemetry link characterization (range, packet loss, latency)
- Ground station UI verification
- Simulation validation (against analytical cases or against known reference flights)
- End-to-end integration test

## Structural test categories (added 2026-05-15 per D031)

- **Material coupon testing.** Print test specimens across candidate filaments / print orientations / wall thicknesses / infill patterns; characterize tensile and bending behavior; select print strategy before committing to a full airframe print.
- **Full-airframe load testing.** Once an airframe is fabricated, validate it withstands the expected bench perturbation load envelope (tens of newtons peak) with margin.
- **Demo-cycle fatigue testing.** Survive N perturbation cycles representative of demo and rehearsal usage without structural degradation; pick N based on expected demo count and rehearsal frequency with margin.
- **Avionics–airframe integration verification.** Sensor mount alignment within tolerance; servo bracket fit; cable routing clearance; gimbal mount alignment; antenna clearance from sensors.
- **Power rail isolation verification.** Measure clean rail voltage during worst-case servo slew events (all four fins commanded to opposite extremes simultaneously); confirm no sag below MCU brown-out threshold and no measurable noise coupling into sensor ADC readings.
- **Time authority verification.** Confirm MCU timestamps are monotonic across radio reconnection events, transport switches (radio ↔ wired), and Pi reboots.

---

*Populate as tests are designed.*
