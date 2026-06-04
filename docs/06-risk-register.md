# Risk Register

**Status:** SKELETON — to be populated as risks emerge (starting early Phase 1).
**Created:** 2026-05-11

The function of this register is to keep risks visible. Projects fail not because risks are unpredictable but because known risks are not acted on. This document is the place known risks live.

---

## Format

Each risk gets an ID, a description, a likelihood (Low/Medium/High), an impact (Low/Medium/High), a current status (Open/Mitigating/Closed), a mitigation plan, and a date of last review.

Example structure:

> **RISK-001 — Magnetometer unusable indoors**
> Magnetic interference from nearby metal and electronics could make magnetometer-based heading estimation unreliable during indoor bench testing.
> *Likelihood:* Medium. *Impact:* Medium.
> *Status:* Open.
> *Mitigation:* Characterize magnetometer noise in workspace before depending on it. Design estimator to operate without magnetometer if needed (fits the fault-tolerance theme).
> *Last reviewed:* [date]

---

## Initial seed risks (carried from Phase 0 — to be expanded)

- Procurement delays on specific components (IMU, LoRa modules, servos).
- Component compatibility issues across the avionics stack.
- Sensor calibration time underestimated.
- Magnetic interference indoors degrading magnetometer.
- Power-related issues (LiPo handling, voltage rails, brownouts during servo motion).
- Time slippage on the fault-tolerance work (highest-risk because it's the depth axis).
- Loss of motivation during long debugging slogs.
- Polytechnic institutional access not materializing.

## Airframe-side seed risks (added 2026-05-15 per D031)

- **Airframe CAD iteration creep.** Custom mechanical design with iteration is a classic schedule sink in real aerospace programs; explicit design-freeze discipline required after one or two revisions.
- **3D print failure modes.** Layer adhesion, warping, infill issues, large-print failures — each failed print consumes significant printer time and material.
- **Structural failure under demo-cycle perturbation loads.** Repeated bench perturbations may fatigue the printed structure over many demo and rehearsal cycles; coupon testing should precede full-airframe commitment.
- **Material / print-parameter selection risk.** Strength and stiffness vary significantly with filament family, print orientation, wall thickness, and infill pattern; wrong choices found late are expensive (full reprints).
- **Avionics–airframe integration mismatch.** Purpose-fit design only stays purpose-fit if avionics and airframe iteration cycles stay synchronized through their interface points (avionics bay dimensions, sensor mount positions, servo bracket geometry).
- **FDIR false positives from undiagnosed coupling.** Even with split rails per D033, integration may reveal unanticipated cross-channel noise (ground bounce, magnetic coupling, mechanical vibration coupling); budget for shakedown testing before relying on FDIR cleanliness for demos.

## Accepted risks — HIL removal (added 2026-06-04 per D051)

**R-HIL-01 — Closed-loop stability in degraded fin modes not analytically proven before hardware phase**
Removing the G1.5 gate eliminates the only pre-hardware tests (TEST-HIL-007, 007B, 007C) that would have proven `CTL_MODE_3FIN_REDUCED` and `CTL_MODE_2FIN_REDUCED` are dynamically stable with a realistic plant model. No pre-hardware gain/phase margin validation exists for the degraded mixing matrices.
*Likelihood:* Low (mixing matrices are analytically correct per CTL-006/CTL-007; stability margin is likely adequate). *Impact:* Medium (instability discovered late requires gain schedule tuning before demo).
*Status:* Open — accepted.
*Mitigation:* Validate closed-loop stability empirically during bench integration. If instability in a degraded mode is observed, tune the gain schedule on hardware before demo.
*Last reviewed:* 2026-06-04

**R-HIL-02 — FDIR fault detection latency validated only on physical hardware**
Removing TEST-HIL-004/005/006 eliminates the only pre-hardware tests that verified FDIR detects faults under dynamic operating conditions. The LR-3 detection latency bound will be verified in the first hardware integration run, not before.
*Likelihood:* Low (G1 FDIR unit tests validate detection logic in isolation; dynamic operation does not materially change the innovation gate math). *Impact:* Low (worst case: threshold requires tuning during hardware bring-up, which is expected calibration work).
*Status:* Open — accepted.
*Mitigation:* G1 FDIR unit tests validate detection logic in isolation; physical fault injection during bench demo validates end-to-end. Budget calibration time for FDIR threshold tuning during G2/G3.
*Last reviewed:* 2026-06-04

---

*Review the register at the start of each phase and after any significant procurement or integration milestone.*
