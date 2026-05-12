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
- Control loop closed-loop response on bench
- Telemetry link characterization (range, packet loss, latency)
- Ground station UI verification
- Simulation validation (against analytical cases or against known reference flights)
- End-to-end integration test

---

*Populate as tests are designed.*
