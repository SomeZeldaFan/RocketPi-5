# Project Constraints & Context

**Version:** v0.2 (draft for review — supersedes v0.1)
**Last updated:** 2026-05-11
**Status:** Phase 0 nearly complete. Locks at v0.3 after review.

---

## 1. Mission Need

Build a first hardware project, in the aerospace domain, of sufficient depth and polish that it stands out among the work of peers and is presentable to future aerospace faculty as evidence of capability — given a Raspberry Pi 5 kit as the starting point and a 16-week window (May–July 2026).

This is a *founding statement*, not a specification. Specifications are downstream.

---

## 2. Stakeholders & Audiences

- **Primary judge:** future aerospace professors (audience not yet met; taste is inferred, not known).
- **Secondary judge:** current ISET (software engineering) professor who issued the kit and expects a deliverable in July.
- **Tertiary judge:** peers in current program.
- **Industry gateway aspiration:** UAE aerospace and defense employers (HALCON, EDGE, others) for whom the artifact could serve as evidence of capability.
- **Self:** the builder, whose technical growth and genuine interest are themselves outcomes of the project.

**Implication:** the project must read as serious to someone who knows aerospace, not merely as "impressive hardware project for a software student." This raises the bar on domain authenticity — the project must engage with at least one genuine aerospace concern, not be a generic embedded project with an aerospace skin.

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
- Live ground station UI showing telemetry and sensor health
- Onboard data logging (SD card)
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

## 9. Definition of "Impressive" (Operational Rubric)

Candidate scope additions and design decisions are scored against the following criteria. This is the rubric — re-read before any meaningful scope decision.

1. **Domain authenticity** — engages a real aerospace problem, not cosmetically. Fault-tolerant estimation is the project's primary authenticity claim.
2. **Technical depth** — at least one component requiring genuine learning beyond tutorial-level. The sensor fusion + fault detection pipeline carries this.
3. **Systems integration** — multiple subsystems with real interfaces. IMU, MCU, servos, telemetry radio, ground station, simulation — all must compose.
4. **Documentation & rigor** — written up as if for review. Requirements traced, decisions justified, results measured. *This is the cheapest multiplier for perceived seriousness.*
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

---

## 11. Open Questions

These are unresolved and need attention in Phase 1 or before:

1. **Math background not fully characterized.** Linear algebra, probability, calculus comfort levels remain undocumented. Will shape how heavily we lean on Kalman-family estimators vs. simpler complementary filters as the baseline.
2. **Specific microcontroller choice deferred to Phase 1.** STM32, Teensy, RP2040 all candidates. Selection depends on PWM channel needs, sensor interface needs, ecosystem maturity for safety-critical patterns.
3. **Specific IMU and barometer selection deferred to Phase 1.** BNO055 (with on-chip fusion — less educational but easier) vs. ICM-20948/MPU9250 (raw, you implement fusion — more educational, more work).
4. **Specific telemetry band and module deferred to Phase 1.** 433 MHz vs. 868 MHz; verify license-free status in UAE.
5. **Airframe presence question.** Open whether the test stand uses a real commercial airframe with motor mounted (more realistic, harder to fixture, requires commercial motor procurement which we've now flagged as restricted) or whether it's an avionics test rig that doesn't require an airframe at all. Decide in Phase 1.

---

## 12. Version History

- **v0.1** (2026-05-11): Initial strawman. Time budget estimated at 128 hours (incorrect — based on misread of "3 hours per week"). No domain locked.
- **v0.2** (2026-05-11): Time budget corrected to ~400 hours. Domain locked (model rocketry). Project shape locked (bench-only, GNC-focused, avionics-centric). Depth axis locked (fault-tolerant graceful degradation). Flight excluded. Demos redefined. Scope inclusions and exclusions enumerated. Locked principles enumerated. Open questions surfaced.
- **v0.3** (pending): Lock after review.
