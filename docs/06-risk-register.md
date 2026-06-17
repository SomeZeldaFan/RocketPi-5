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

## Sensor / estimation risks (added 2026-06-14 per D050)

**R-YAW-01 — Yaw unobservable with 6-axis IMUs (no magnetometer)**
The selected IMUs (BMI160, ICM-42688-P) are 6-axis (accelerometer + gyroscope). Gravity, sensed by the accelerometer, constrains roll and pitch but provides no absolute yaw/heading reference, so yaw is observable only through gyro integration and drifts without bound — the accelerometer correction that bounds roll/pitch cannot bound yaw. Surfaced during the FDIR/estimator boundary work (D050). Note: constraints §5 lists a "magnetometer disturbance" failure mode that the current hardware cannot exhibit — the scope statement and the selected hardware need reconciling.
*Likelihood:* High (a property of the hardware, not a contingency). *Impact:* Medium (roll/pitch — the primary bench-perturbation axes — remain well-estimated; the yaw estimate degrades over time).
*Status:* Closed (D054, 2026-06-17) — option (a) taken: magnetometer (Bosch BMM150, dedicated I2C bus) added. Absolute yaw reference restored → yaw is now an observable, bounded EKF state; the constraints §5 magnetic-disturbance failure mode is now achievable, suiting the depth axis.
*Mitigation (realized):* BMM150 on its own I2C bus; `mag_status_t` / `mag_reading_t` / `mag_healthy` added to the canonical types; protocol version bumped to 2. The mag becomes its own innovation-gated channel in FDIR. RISK-001 (mag unusable indoors) remains the operative residual and stays open under its own entry.
*Last reviewed:* 2026-06-17

## Airframe joint / assembly risks (added 2026-06-17 per D055 / D056)

**R-AF-01 — Stringer-bore coaxiality across the joint**
The four Ø5 stringer bores in mating sections are printed in two separate parts; if they are not coaxial within print + slip-fit tolerance, four rigid 200 mm aluminium stringers cannot be inserted across the joint, or will force misalignment.
*Likelihood:* — *Impact:* —
*Status:* **Closed (D058, 2026-06-17)** — the aluminium stringers were removed entirely; the bolted flange carries bench loads with ample margin. The risk no longer exists. Residual radial-centring (now loose) is tracked instead as the exploratory centring-spigot TODO (airframe doc §12.2), not a risk.
*Last reviewed:* 2026-06-17

**R-AF-02 — Heat-set insert depth inconsistency**
Heat-set inserts can sink to inconsistent depths in a plain pilot hole, giving uneven clamp behaviour across 8 bolts × ~6 joints.
*Likelihood:* Low (mitigated by design). *Impact:* Low–Medium.
*Status:* Mitigating.
*Mitigation:* Stepped pilot (D055 / airframe doc §5.2) — the diameter-step shoulder is a hard bottom datum so every insert seats at the same depth. Confirm on the test coupon.
*Last reviewed:* 2026-06-17

**R-AF-03 — Mass, print time, and filament cost of thick-wall sections**
A 280 mm section is a large print. The original 12 mm wall was ~1.7 L of wall volume per section; ~6 sections implied roughly a spool and many print-hours each.
*Likelihood:* Medium. *Impact:* Low–Medium (schedule + filament budget).
*Status:* **Mitigating / downgraded (D058, 2026-06-17).** Wall thinned 12 → 6 mm and OD 174 → 162 (wall volume cut well over half), and the first article is a minimal stack (avionics / one empty / aft-most / cosmetic mount) rather than ~6 sections. New limiting failure mode is local skin denting (low at 6 mm) — covered by the wall coupon (airframe doc §12.14).
*Mitigation:* Coupon-first to avoid scrapping full sections; add empty sections for length only after the minimal stack is proven; add a printed ring stiffener only if a coupon dents.
*Last reviewed:* 2026-06-17

**R-AF-04 — Canard control coupling / stability**
Forward canards (D056) contribute pitch/yaw moments ahead of the CG and can couple with body modes; the degraded-mode mixing matrices (REQ-CTL-005) must remain stable on canards. On the static bench the dynamic risk is limited, but the control-law tuning assumptions move from aft fins to canards.
*Likelihood:* Low–Medium. *Impact:* Medium.
*Status:* Open.
*Mitigation:* Validate closed-loop stability (nominal and 3-of-4) empirically during bench integration; pulls on the same shakedown budget as R-HIL-01.
*Last reviewed:* 2026-06-17

**R-AF-05 — Joint assembly slog**
Each joint is 8 heat-set inserts + 8 bolts (no stringers, D058), every insert a chance to go in crooked. Reduced substantially by the minimal first stack (~2–3 joints, not ~6) and by removing the 24 stringers entirely. Uncharacterised tedium is a planning failure (Constraints §10.1).
*Likelihood:* Low–Medium. *Impact:* Low.
*Status:* Open — accepted (downgraded D058).
*Mitigation:* Budget the assembly time; consider an insert-installation jig; standardise the joint so the process is repeatable.
*Last reviewed:* 2026-06-17

---

*Review the register at the start of each phase and after any significant procurement or integration milestone.*
