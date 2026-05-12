# Project Constraints & Context

**Version:** v0.5 (locked — supersedes v0.4)
**Last updated:** 2026-05-12
**Status:** Phase 0 complete.

---

## 1. Mission Need

Design, build, and validate a bench-top model rocketry GNC (Guidance, Navigation, and Control) suite demonstrating closed-loop attitude control with fault-tolerant sensor fusion under deliberate sensor compromise — within a 16-week window (May–July 2026) using a Raspberry Pi 5 as the development and ground station platform.

This is a *founding statement*, not a specification. Specifications are downstream.

---

## 2. Intended Audiences

- **Aerospace engineering faculty** — the primary technical audience. The project engages real aerospace subdisciplines (GNC, FDIR) and must hold up to domain scrutiny.
- **Current program professor** — issued the Pi 5 kit; expects a completed deliverable by July 2026. Scope and documentation must be legible to a software engineering background.
- **UAE aerospace and defense industry** — secondary audience for whom fault-tolerant GNC and GPS-denied operation analogues are directly relevant.
- **The builder** — technical growth and genuine understanding are first-class outcomes, not side effects.

**Implication:** the project engages at least one genuine aerospace concern at depth, not as a cosmetic framing over a generic embedded project. Domain authenticity is non-negotiable.

---

## 3. Resources

### 3.1 Hardware on hand

Raspberry Pi 5 kit: Pi 5, breadboard, LCD display, starter kit components, image/camera sensor, power supply, SD card, case.

### 3.2 Tools available

- Soldering iron (trivial to acquire).
- Multimeter (trivial to acquire).
- 3D printer (accessible via family).
- Development computer: Ryzen 7 9700X, RTX 4080 Super, 64 GB DDR5. This is a serious dev machine — high-fidelity simulation and ML workloads are first-class options.

### 3.3 Acquirable

Additional sensors, microcontrollers, flight controllers, RF modules, mounting structures, 3D-printed parts, etc., all in scope subject to budget.

### 3.4 Skills acquirable

3D printing, soldering, CAD, control theory, sensor fusion, embedded development — all in scope to learn during the project.

### 3.5 Budget

Soft ceiling **~$2,000 USD** across the project lifetime (excluding tools listed in 3.2). Contributions from monthly stipend (~AED1,000/month available) plus possible family contribution. Not a hard cap; specific expenditures justified case-by-case.

### 3.6 Time

**~16 weeks calendar** (May 11 – end of July 2026).

Estimated total work budget: **~400 hours** (350–450 conservative range).

- Baseline: ~20 hrs/week × 10 weeks ≈ 200 hours
- Vacation surge: ~45 hrs/week × 6 weeks ≈ 270 hours
- Haircut for life, exams, illness, motivation troughs.

This is *not* a casual side-project budget. It places ambitious work within reach but does not place a full CubeSat or anything requiring multi-month skill ramps within reach.

### 3.7 Workspace

- **Primary:** home.
- **Outdoor:** available.
- **Institutional:** Polytechnic labs potentially accessible (3D printing, CNC, electronics test equipment) via managing director. *Action: pursue formal access for project endorsement and lab use.*

---

## 4. Project Shape (LOCKED)

**Bench-only model rocketry GNC suite, avionics- and systems-centric, with high-fidelity simulation as parallel deliverable.**

The system is built to operational completeness on a test stand — IMU, sensor fusion, state estimation, control law, servo-driven aerodynamic control surfaces, ground station — but is **not flown**. Flight is excluded for procurement, regulatory, and logistical reasons (see §5).

The airframe (if used) is a commercial mid-power kit, modified to integrate the avionics bay and control surfaces. The airframe is the *vehicle for the engineering*, not the focus of it.

The Pi 5 serves as ground station and possibly companion computer (specific role decided in Phase 1).

A separate flight-critical microcontroller (likely STM32-class, decided in Phase 1) owns the real-time control loop.

---

## 5. Depth Axis (LOCKED): Fault-Tolerant Graceful Degradation

Beyond nominal closed-loop control, the system demonstrates **robust operation under deliberate sensor compromise.**

Failure modes characterized in scope:

- Magnetometer disturbance (nearby metal, magnetic interference)
- IMU sensor degradation or loss
- Sensor disagreement (innovation gating)
- Sensor disconnection

The system detects sensor inconsistency, isolates the faulty channel, and continues operating with degraded but bounded accuracy. This is the project's *distinctive depth axis* and what separates it from "student built a control system."

Robustness is **architected from day one**, not retrofitted (see §10).

This direction is selected because:

- It is honest to the bench setup (sensor robustness exists for stationary and moving vehicles alike).
- It maps to a real aerospace subdiscipline (FDIR — Fault Detection, Isolation, and Recovery).
- It is genuinely defense-relevant (jamming, spoofing, adversarial sensor environments).
- It does not force a project-identity reframe.

---

## 6. Demos (deliverables for July presentation)

### Demo 1 — Live bench demo (PRIMARY)

Live demonstration of control surfaces responding to physical perturbation of the test stand. Includes nominal operation and live demonstration of fault tolerance: disconnect a sensor mid-operation, watch the estimator degrade gracefully and report sensor health.

### Demo 2 — DROPPED

Real flight with telemetry. Excluded because of no-flight decision.

### Demo 3 — REDEFINED: High-fidelity simulation as standalone deliverable

RocketPy (or equivalent) simulation of the vehicle in hypothetical flight, with the actual avionics software running against simulated sensor inputs (hardware-in-the-loop). The deliverable is *not* simulation-vs-actual comparison (no actual flight data exists), but rather: simulated flight envelope, expected vehicle behavior, predicted control system performance, and stress-test results under simulated sensor failures during flight regimes.

---

## 7. Scope: Inclusions

- IMU-based attitude estimation (sensor fusion, state estimator)
- Closed-loop control law driving servo-actuated aerodynamic control surfaces
- Sensor health monitoring and fault detection logic
- Graceful degradation under sensor compromise
- Telemetry downlink to Pi 5 ground station
- **Live ground station UI** running on the Pi 5 (1024×600 touch display, or external monitor). Multi-pane dashboard showing: live telemetry charts, sensor health status and confidence indicators, the system's own estimate of data accuracy and veracity, a real-time 3D attitude visualization of the rocket body driven by the estimator output, and a separate zoomed view of the fin assembly showing actuated control surface deflections as they occur.
- High-fidelity simulation of vehicle in hypothetical flight
- Post-test analysis tooling (Python, on dev machine)
- Documentation: requirements, architecture, decisions, test results, post-project writeup

## 8. Scope: Exclusions

- **No flight.** Static / bench testing only.
- **No custom airframe design.** Commercial kit only, modified for integration.
- **No propulsion work.** Commercial motors only (and only as static prop, not fired).
- **No TVC (thrust vectoring control)** on any flight-intended article.
- **No projectiles, no kinetic payload, no weaponization** of any kind, in any framing.
- **No live motor firing.** Static fire tests are out of scope.
- **No multi-stage anything.** Single article, single configuration.
- **No GCAA permission-seeking** for this project. Closed; will not be revisited.
- **No swarm, no multi-vehicle.** Single article.
- **No autonomous mission planning.** This is a control system project, not an autonomy project.

---

## 9. Engineering Quality Rubric

Candidate scope additions and design decisions are evaluated against the following criteria. Re-read before any meaningful scope decision.

1. **Domain authenticity** — engages a real aerospace problem, not cosmetically. Fault-tolerant estimation is the project's primary authenticity claim.
2. **Technical depth** — at least one component requiring genuine learning beyond tutorial-level. The sensor fusion + fault detection pipeline carries this.
3. **Systems integration** — multiple subsystems with real interfaces. IMU, MCU, servos, telemetry radio, ground station, simulation — all must compose.
4. **Documentation & rigor** — written up as if for review. Requirements traced, decisions justified, results measured.
5. **Demonstrability** — non-builder can appreciate the result in under five minutes (Demo 1 satisfies this).
6. **Originality of choice, not invention** — the specific framing (bench-only, fault-tolerant, GNC-focused, defense-adjacent) is uncommon. Not novel research; genuinely uncommon at student level.
7. **Slog-to-learning ratio** — concepts and tasks should be net learning-dense. Mechanical tedium is acceptable when budgeted; uncharacterized tedium is a planning failure.

---

## 10. Locked Principles

### 10.1 Slog is estimated explicitly

Every concept and major task is evaluated for the ratio of learning-dense work to mechanical-tedium work. Slog phases are planned for, not encountered. We do not psych ourselves up about tedium; we put it in the schedule. (Adapted from how mature engineering organizations plan integration and test campaigns — slog is called schedule margin in industry.)

### 10.2 Architect for robustness from day one

Fault tolerance is not Phase 2. It shapes the architecture from the first commit. The first version of any estimator may be a trivial nominal implementation, but it exposes the *interfaces* fault tolerance needs: sensor health channels, innovation tracking, confidence bounds, graceful degradation paths. Retrofitting robustness is 2–3× the cost of designing for it. We will not pay that cost.

### 10.3 Decisions go in the log

Every architectural or scope decision is logged with date, decision, alternatives considered, and rationale. We do not relitigate decisions; we read the log.

### 10.4 Procurement reality is a hard constraint

The trade space is bounded by what can actually be acquired in the UAE on this timeline and budget. Theoretical availability is not availability.

### 10.5 Authoritative sources for regulatory questions

When regulatory questions arise, the regulator (or its published rules) is the source of truth — not inference, not e-commerce restrictions, not forum posts. (Pattern established when contacting GCAA early in Phase 0.)

### 10.6 JPL Power of 10 coding standard

All control-path and flight-critical code abides by the NASA JPL Power of 10 rules. This is a non-negotiable quality floor, not a stretch goal.

---

## 11. Open Questions

These are unresolved. They'll be answered when the project reaches them.

1. **Specific microcontroller choice.** STM32, Teensy, RP2040 all candidates. Selection depends on PWM channel needs, sensor interface needs, ecosystem maturity for safety-critical patterns.
2. **Specific IMU and barometer selection.** BNO055 (with on-chip fusion — less educational but easier) vs. ICM-20948/MPU9250 (raw, you implement fusion — more educational, more work).
3. **Specific telemetry band and module.** 433 MHz vs. 868 MHz; verify license-free status in UAE.
4. **Airframe presence question.** Open whether the test stand uses a real commercial airframe or a pure avionics test rig.
5. **Simulation validation standard.** NASA-STD-7009 is a candidate framework for documenting simulation credibility (Demo 3). Defer until simulation scope is defined.

---

## 12. Version History

- **v0.1** (2026-05-11): Initial strawman. Time budget estimated at 128 hours (incorrect — based on misread of "3 hours per week"). No domain locked.
- **v0.2** (2026-05-11): Time budget corrected to ~400 hours. Domain locked (model rocketry). Project shape locked (bench-only, GNC-focused, avionics-centric). Depth axis locked (fault-tolerant graceful degradation). Flight excluded. Demos redefined. Scope inclusions and exclusions enumerated. Locked principles enumerated. Open questions surfaced.
- **v0.3** (2026-05-12): Public-facing reframe — mission statement depersonalized, stakeholder framing neutralized (§2 renamed to Intended Audiences), §9 renamed to Engineering Quality Rubric, JPL Power of 10 added as §10.6.
- **v0.4** (2026-05-12): Scope update — SD card onboard logging removed (Pi 5 handles all logging over telemetry); ground station UI description expanded to reflect multi-pane dashboard with 3D attitude visualization and fin deflection view.
- **v0.5** (2026-05-12): Open questions reframed as timeless (removed phase-gate language); NASA-STD-7009 added as a deferred candidate for simulation validation.
