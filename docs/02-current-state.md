# Current State

**Last updated:** 2026-05-19

Live project status. Updated at the end of every work session.

---

## Where we are

**Phase:** Phase 1 — requirements first pass complete (D035–D036 logged). Hardware selection complete (D038–D042). Active tracks: airframe structural design (CAD), coding framework, and literature reviews. LR-2 and LR-3 now unblocked by hardware selection. LR-1 runs in parallel. Servos deferred pending LR-4 and CAD.

**Constraints doc:** v0.8 locked.
**Domain:** Locked — bench-only model rocketry GNC suite.
**Depth axis:** Locked — fault-tolerant graceful degradation under sensor compromise.
**Flight:** Excluded.
**System architecture:** Locked (D024–D034). Custom semi-monocoque airframe path adopted (D031); D011 retired.

Pi 5 bring-up complete (DietPi v10.3.3, kernel 6.18.29, SSH access confirmed, GPIO stack verified). VS Code Remote-SSH setup deferred. Avionics component selection complete (D038–D042). Coding framework now unblocked — MCU confirmed as STM32F407ZGT6; toolchain is STM32CubeIDE + HAL.

---

## What's done

- Phase 0 complete — project shape, depth axis, scope, and principles all locked.
- Constraints document locked at v0.8.
- Decisions log populated: D001–D036.
- Coding standard locked: NASA JPL Power of 10.
- Ground station scope defined: multi-pane dashboard, 3D attitude visualization, fin deflection view, bidirectional C2 link, demo/flight mode toggle.
- **Phase 1 architecture review complete** (per D023). Eleven structural decisions logged as D024–D034, covering: IMU configuration, real-time path ownership, sensor bus topology, barometer inclusion, telemetry radio termination, MCU↔Pi physical link, control surface count, test stand / airframe configuration, Pi 5 role precision, power architecture, and authoritative time source.
- **Hardware selection complete** (D038–D042). UAE 433 MHz band confirmed (TDRA). MCU: STM32F407ZGT6 core board. IMU-1: GY-BMI160 (Bosch BMI160). IMU-2: ICM-42688-P breakout (TDK InvenSense). Barometer: MS5611 via GY-63 module. Radio: HolyBro SiK V3 433 MHz pair. All components on breakout modules or core boards — no custom PCB required. ST-Link V2 and full tooling procured. LR-2, LR-3, and coding framework all unblocked.
- Architecture document populated with hardware architecture (§2) and airframe structural architecture (§3 skeleton).
- Risk register, test plan, and bibliography expanded with airframe-side entries.
- **Pi 5 bring-up complete.** DietPi v10.3.3 installed on kernel 6.18.29+rpt-rpi-2712. SSH access confirmed at 192.168.3.25 (wlan0). GPIO stack verified: lgpio 0.2.2 + gpiozero 2.0.1 on gpiochip0 (RP1). Hardware revision e04171.
- **Requirements document first pass complete** (docs/04-requirements.md). Sections 1–8 populated: system-level, estimation, control, FDIR, telemetry, ground station, power, documentation. Sections 9–11 (airframe, simulation, demonstration) deferred as stubs. Eight performance requirements carry [TBD] placeholders pending literature review (LR-1 through LR-4) — see in-progress below.

## What's in progress

- **Literature reviews LR-2 and LR-3.** Now unblocked by D040. LR-2: MEMS IMU estimation accuracy — run against BMI160 and ICM-42688-P datasheets (noise density, Allan deviation, bias instability); produces steady-state EKF error bound in degrees. LR-3: FDIR innovation gating — requires LR-1 result and IMU noise specs; chi-squared threshold and detection latency.
- **Coding framework and deliverables plan.** Now unblocked — MCU confirmed as STM32F407ZGT6. Produces: repo scaffolding, C module interface headers (avionics application layer), GCS project structure, build system skeleton (STM32CubeIDE + HAL), and phased coding deliverable list.
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

1. **LR-1 — Control loop rate** — independent of hardware; runs now. Produces a justified Hz recommendation to commit into REQ-SYS-011.
2. **LR-2 — MEMS IMU estimation accuracy** — unblocked by D040. Run against BMI160 and ICM-42688-P datasheets. Produces steady-state EKF error bound in degrees (unblocks REQ-EST-006, REQ-CTL-007).
3. **LR-3 — FDIR innovation gating** — unblocked by D040; requires LR-1 first. Produces detection latency bound in milliseconds (unblocks REQ-FDR-008).
4. **Coding framework and deliverables plan** — unblocked by hardware selection. Produces repo scaffolding, C module interface headers, GCS project structure, build system skeleton (STM32CubeIDE + HAL), and phased coding deliverable list.
5. **Begin airframe CAD** — first-pass body geometry, avionics bay dimensioned to selected components, fin pivot locations, gimbal mount integration. Unlocks LR-4. Run material coupon prints as structural design firms up.
6. **Pi 5: VS Code Remote-SSH setup** — install Remote-SSH extension in VS Code, configure ~/.ssh/config entry for the Pi, connect and verify Pi filesystem is accessible from the laptop.
7. **Compose the formal system block diagram** — referenced as deferred in `docs/05-architecture.md` §1. Choose diagramming methodology appropriate for the documentation standard.
8. **Servo finalisation** — after LR-4 and CAD provide torque and size requirements.
