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
- **Avionics–airframe integration mismatch.** Purpose-fit design only stays purpose-fit if avionics and airframe iteration cycles stay synchronized through their interface points (avionics bay dimensions, sensor mount positions, servo bracket geometry, gimbal mount).
- **FDIR false positives from undiagnosed coupling.** Even with split rails per D033, integration may reveal unanticipated cross-channel noise (ground bounce, magnetic coupling, mechanical vibration coupling); budget for shakedown testing before relying on FDIR cleanliness for demos.

---

*Review the register at the start of each phase and after any significant procurement or integration milestone.*
