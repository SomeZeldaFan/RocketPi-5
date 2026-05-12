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

---

*Review the register at the start of each phase and after any significant procurement or integration milestone.*
