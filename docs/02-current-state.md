# Current State

**Last updated:** 2026-05-18

Live project status. Updated at the end of every work session.

---

## Where we are

**Phase:** Phase 1 — requirements first pass complete (D035–D036 logged). Active tracks: hardware component selection (MCU, IMUs, barometer, radio, servos) and airframe structural design (CAD). Literature reviews LR-2 and LR-3 follow hardware selection; LR-1 can run in parallel now.

**Constraints doc:** v0.8 locked.
**Domain:** Locked — bench-only model rocketry GNC suite.
**Depth axis:** Locked — fault-tolerant graceful degradation under sensor compromise.
**Flight:** Excluded.
**System architecture:** Locked (D024–D034). Custom semi-monocoque airframe path adopted (D031); D011 retired.

Pi 5 bring-up complete (DietPi v10.3.3, kernel 6.18.29, SSH access confirmed, GPIO stack verified). VS Code Remote-SSH setup deferred. Avionics component selection in progress (D037).

---

## What's done

- Phase 0 complete — project shape, depth axis, scope, and principles all locked.
- Constraints document locked at v0.8.
- Decisions log populated: D001–D036.
- Coding standard locked: NASA JPL Power of 10.
- Ground station scope defined: multi-pane dashboard, 3D attitude visualization, fin deflection view, bidirectional C2 link, demo/flight mode toggle.
- **Phase 1 architecture review complete** (per D023). Eleven structural decisions logged as D024–D034, covering: IMU configuration, real-time path ownership, sensor bus topology, barometer inclusion, telemetry radio termination, MCU↔Pi physical link, control surface count, test stand / airframe configuration, Pi 5 role precision, power architecture, and authoritative time source.
- Architecture document populated with hardware architecture (§2) and airframe structural architecture (§3 skeleton).
- Risk register, test plan, and bibliography expanded with airframe-side entries.
- **Pi 5 bring-up complete.** DietPi v10.3.3 installed on kernel 6.18.29+rpt-rpi-2712. SSH access confirmed at 192.168.3.25 (wlan0). GPIO stack verified: lgpio 0.2.2 + gpiozero 2.0.1 on gpiochip0 (RP1). Hardware revision e04171.
- **Requirements document first pass complete** (docs/04-requirements.md). Sections 1–8 populated: system-level, estimation, control, FDIR, telemetry, ground station, power, documentation. Sections 9–11 (airframe, simulation, demonstration) deferred as stubs. Eight performance requirements carry [TBD] placeholders pending literature review (LR-1 through LR-4) — see in-progress below.

## What's in progress

- **Hardware component selection.** MCU (STM32F4-class+), IMUs (two heterogeneous SPI raw-output chips, per D024/D035), barometer (I2C). UAE procurability is a binding input per constraints §10.4. IMU selection unblocks LR-2. Radio selection pending UAE frequency regulatory check (TDRA). Servos shortlisted to metal-gear + position-feedback; finalised after LR-4 and CAD.
- **Airframe: structural design.** Begin CAD work — first-pass body geometry, avionics bay (dimensioned to selected components), fin pivot locations, gimbal mount integration. Material/print-parameter coupon testing to follow before committing to full airframe prints.
- **Pi 5 bring-up: VS Code Remote-SSH.** Final bring-up item — connect VS Code on dev laptop to Pi via Remote-SSH extension. Deferred from 2026-05-16 session.

### Literature review tasks — required before TBD requirements can be committed

These tasks must be completed before the indicated requirements in docs/04-requirements.md can have their numerical values set. Each produces a written summary and a justified number recommendation. **Sequencing matters — see dependency notes.**

- **LR-1 — Control loop rate** *(unblocks REQ-SYS-011, REQ-EST-002, REQ-CTL-008)*
  Independent — can run now, no hardware dependency. Sampling theory applied to control systems. Determine the expected closed-loop bandwidth of a fin-stabilised rocket body on a bench gimbal. Derive the minimum control loop rate from that bandwidth using the Nyquist criterion and engineering margin. Produce a justified Hz recommendation.

- **LR-2 — MEMS IMU estimation accuracy** *(unblocks REQ-EST-006, REQ-CTL-007)*
  **Requires IMU selection first.** Runs on real datasheet noise specs (noise density, Allan deviation, bias instability) for the selected chips — not class-level estimates. Determine what steady-state attitude accuracy is achievable with the chosen dual-IMU EKF configuration. Produce a justified RMS error bound in degrees.

- **LR-3 — FDIR innovation gating design** *(unblocks REQ-FDR-008)*
  **Requires both LR-1 and IMU selection.** Chi-squared consistency test design for dual-IMU cross-check. Window size and threshold selection — requires the loop rate (LR-1) and IMU noise specs (datasheets) as inputs. Characterise the tradeoff between false-positive rate and detection latency. Produce a justified detection latency bound in milliseconds.

- **LR-4 — Vehicle dynamics** *(unblocks REQ-CTL-003, REQ-CTL-004, REQ-CTL-006)*
  **Requires airframe CAD geometry.** Pitch/yaw/roll moments of inertia, fin effectiveness coefficients, achievable control bandwidth and settling time. Run in parallel with early airframe design — geometric inputs (body diameter, fin area, moment arms) come from the CAD model.

### Coding framework and deliverables plan

Follows hardware selection. Cannot be written without knowing the specific MCU — the toolchain, HAL structure, and build system are MCU-dependent. Once hardware is confirmed, this task produces: repo scaffolding, C module interface definitions (avionics application layer), GCS project structure, build system skeleton, and a phased coding deliverable list.

## What's blocked

Nothing blocked.

## Next concrete tasks

1. **Hardware component selection** — MCU (STM32F4-class+), IMUs (two heterogeneous SPI raw-output chips), barometer (I2C). Verify UAE procurability per constraints §10.4. IMU selection immediately unblocks LR-2. Radio selection pending TDRA frequency regulatory check. Servos: shortlist metal-gear + position-feedback candidates; finalise after LR-4 and CAD.
2. **LR-1 — Control loop rate** — independent of hardware; run in parallel with task 1. Produces a justified Hz recommendation to commit into REQ-SYS-011.
3. **Coding framework and deliverables plan** — follows hardware selection. Produces repo scaffolding, C module interface headers, GCS project structure, build system skeleton, and phased coding deliverable list.
4. **Begin airframe CAD** — first-pass body geometry, avionics bay dimensioned to selected components, fin pivot locations, gimbal mount integration. Unlocks LR-4. Run material coupon prints as structural design firms up.
5. **Pi 5: VS Code Remote-SSH setup** — install Remote-SSH extension in VS Code, configure ~/.ssh/config entry for the Pi, connect and verify Pi filesystem is accessible from the laptop.
6. **Compose the formal system block diagram** — referenced as deferred in `docs/05-architecture.md` §1. Choose diagramming methodology appropriate for the documentation standard.
7. **Radio and servo finalisation** — radio after TDRA frequency check; servos after LR-4 and CAD provide torque and size requirements.
