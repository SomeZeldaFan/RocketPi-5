# Current State

**Last updated:** 2026-06-14

Live project status. Updated at the end of every work session.

---

## Where we are

**Phase:** Phase 1 — requirements first pass complete (D035–D036 logged). Hardware selection complete (D038–D042). Coding framework scaffold complete (D043, 2026-05-20): software architecture §4 locked, avionics directory layered scaffold created, all module headers + stub `.c` files in place. Test plan fully populated (2026-05-21): `docs/07-test-plan.md` carries the complete test enumeration and the G0–G4 gate structure; validation-before-implementation locked as a principle (D045, §10.7). Active tracks: per-module header scrutiny (next), airframe structural design (CAD), literature reviews. LR-2 and LR-3 now unblocked by hardware selection. LR-1 runs in parallel. Servos deferred pending CAD.

**Constraints doc:** v1.1 locked.
**Domain:** Locked — bench-only model rocketry GNC suite.
**Depth axis:** Locked — fault-tolerant graceful degradation under sensor compromise.
**Flight:** Excluded.
**System architecture:** Locked (D024–D034). Custom semi-monocoque airframe path adopted (D031); D011 retired.
**Software architecture:** Locked (D043). Bare-metal superloop with DMA + ISRs. Layered scaffold (hardware / algorithm / output / orchestration) in `avionics/` — see `docs/05-architecture.md §4`.

Pi 5 bring-up complete (DietPi v10.3.3, kernel 6.18.29, SSH access confirmed, GPIO stack verified). VS Code Remote-SSH setup deferred. Avionics component selection complete (D038–D042). Coding framework scaffold complete (2026-05-20) — MCU is STM32F407ZGT6; toolchain is STM32CubeIDE + HAL.

---

## What's done

- Phase 0 complete — project shape, depth axis, scope, and principles all locked.
- Constraints document locked at v1.1 (v1.0: §4 updated for Phase 1 decisions; v1.1: §10.7 validation-before-implementation principle added).
- Decisions log populated: D001–D052 (D048 reserved — NVIC priority scheme, to be logged in the `platform.h` scrutiny session; D050 logged 2026-06-14 — FDIR/estimator boundary).
- Coding standard locked: NASA JPL Power of 10.
- **Test plan fully populated (2026-05-21)** — `docs/07-test-plan.md` carries the complete test enumeration (build artifact, platform, all five sensor/peripheral bring-ups, FDIR/estimator/control-law/telemetry/C2/FSM unit tests, code coverage, boundary cases, exhaustive FSM coverage, power, bus contention, per-peripheral and concurrent fault injection, pipeline integrity, fault propagation, reboot recovery, timing margin, watchdog recovery, soak) and the non-bypassable G0–G4 gate structure. Tests are written before the modules they test (D045).
- **D044–D049 logged.** D044: airframe reinforcement boundary revised. D045: test protocol and gate structure. D046: actuator fault detection — operator-asserted (no servo feedback path). D047: loop overrun policy — warn, halt on 3 consecutive. D049: FSM responds to sensing health only, not actuator health.
- Ground station scope defined: multi-pane dashboard, 3D attitude visualization, fin deflection view, bidirectional C2 link, demo/flight mode toggle.
- **Phase 1 architecture review complete** (per D023). Eleven structural decisions logged as D024–D034, covering: IMU configuration, real-time path ownership, sensor bus topology, barometer inclusion, telemetry radio termination, MCU↔Pi physical link, control surface count, test stand / airframe configuration, Pi 5 role precision, power architecture, and authoritative time source.
- **Hardware selection complete** (D038–D042). UAE 433 MHz band confirmed (TDRA). MCU: STM32F407ZGT6 core board. IMU-1: GY-BMI160 (Bosch BMI160). IMU-2: ICM-42688-P breakout (TDK InvenSense). Barometer: MS5611 via GY-63 module. Radio: HolyBro SiK V3 433 MHz pair. All components on breakout modules or core boards — no custom PCB required. ST-Link V2 and full tooling procured. LR-2, LR-3, and coding framework all unblocked.
- Architecture document populated with hardware architecture (§2) and airframe structural architecture (§3 skeleton).
- Risk register, test plan, and bibliography expanded with airframe-side entries.
- **Pi 5 bring-up complete.** DietPi v10.3.3 installed on kernel 6.18.29+rpt-rpi-2712. SSH access confirmed at 192.168.3.25 (wlan0). GPIO stack verified: lgpio 0.2.2 + gpiozero 2.0.1 on gpiochip0 (RP1). Hardware revision e04171.
- **Requirements document first pass complete** (docs/04-requirements.md). Sections 1–8 populated: system-level, estimation, control, FDIR, telemetry, ground station, power, documentation. Sections 9–10 (airframe, demonstration) deferred as stubs. Eight performance requirements carry [TBD] placeholders pending literature review (LR-1 through LR-3) — see in-progress below.
- **D043 logged (2026-05-20)** — scheduling model: bare-metal superloop with DMA + ISRs. FreeRTOS explicitly rejected. Five-reason rationale documented; alternatives considered.
- **Software architecture §4 written and locked (2026-05-20)** — `docs/05-architecture.md §4`. Module decomposition, scheduling model with 12-step tick sequence, ISR table with volatile discipline, canonical type definitions, fault propagation chain with four invariants, module interface contracts, test harness strategy. GCS architecture explicitly deferred to its own session.
- **Avionics scaffold complete (2026-05-20)** — layered directory structure (`hardware/`, `algorithm/`, `output/`, `orchestration/`). `avionics_types.h` (canonical types, `volatile_flag_t`, `AVIONICS_PROTOCOL_VERSION`), `isr_flags.h` (manifest of all ISR/main-loop boundary crossings), 12 module headers with real declarations + comment contracts, 12 stub `.c` files with PLACEHOLDER RETURN comments and zero functional code, `main.c` superloop skeleton expressing the 12-step tick sequence, `stm32f4xx_it.c` and `stm32f4xx_hal_msp.c` HAL stub files, 4 dev-PC test harness stubs.
- **LR-1 complete (2026-05-22, D052)** — `AVIONICS_LOOP_RATE_HZ` = 1000 Hz locked; `IMU_STALENESS_THRESHOLD_US` = 5000 μs. Derived from 10 Hz disturbance bandwidth × 5× margin × Franklin Eq. (11.3) sampling rule. REQ-SYS-011, REQ-EST-002, REQ-CTL-008 resolved. LR-3 unblocked. Full write-up: `docs/derivations/LR-1-loop-rate.md`.
- **FDIR/estimator boundary resolved (2026-06-14, D050)** — two-phase estimator (`estimator_predict`/`estimator_update`) + two-phase FDIR (`fdir_admit`/`fdir_gate`), interleaved admit→predict→gate→update; new `predicted_readings_t` carries the estimator's predicted measurements to the gate as data (no estimator import). FDIR writes health in two passes (admit preliminary, gate restrict-only). Architecture §4.1/§4.2/§4.4/§4.5 updated and signatures locked. Unblocks the EKF and FDIR innovation-gate implementations and TEST-EST-011.

## What's in progress

- **Literature reviews LR-2 and LR-3.** LR-2: MEMS IMU estimation accuracy — run against BMI160 and ICM-42688-P datasheets (noise density, Allan deviation, bias instability); produces steady-state EKF error bound in degrees. Unblocked by D040. LR-3: FDIR innovation gating — chi-squared threshold and detection latency. Unblocked by D040 + LR-1 (D052, 1000 Hz).
- **Per-module header scrutiny sessions.** With the scaffold in place, each module header gets its own dedicated session to scrutinise the interface contract, type-check signatures against the fault propagation invariants, and lock the safe default returns currently marked as PLACEHOLDER. Headers do not move from scaffold to implementation-ready until they pass their scrutiny session.
- **Airframe: structural design.** Begin CAD work — first-pass body geometry, avionics bay (dimensioned to selected components), fin pivot locations. Material/print-parameter coupon testing to follow before committing to full airframe prints.
- **Pi 5 bring-up: VS Code Remote-SSH.** Final bring-up item — connect VS Code on dev laptop to Pi via Remote-SSH extension. Deferred from 2026-05-16 session.

### Literature review tasks — required before TBD requirements can be committed

These tasks must be completed before the indicated requirements in docs/04-requirements.md can have their numerical values set. Each produces a written summary and a justified number recommendation. **Sequencing matters — see dependency notes.**

- **LR-1 — Control loop rate** — **COMPLETE (2026-05-22, D052).** Result: 1000 Hz. See `docs/derivations/LR-1-loop-rate.md`.

- **LR-2 — MEMS IMU estimation accuracy** *(unblocks REQ-EST-006, REQ-CTL-007)*
  **Requires IMU selection first.** Runs on real datasheet noise specs (noise density, Allan deviation, bias instability) for the selected chips — not class-level estimates. Determine what steady-state attitude accuracy is achievable with the chosen dual-IMU EKF configuration. Produce a justified RMS error bound in degrees.

- **LR-3 — FDIR innovation gating design** *(unblocks REQ-FDR-008)*
  **Unblocked — LR-1 complete (D052, 1000 Hz) and IMUs selected (D040).** Chi-squared consistency test design for dual-IMU cross-check. Window size and threshold selection — uses the loop rate (1000 Hz) and IMU noise specs (datasheets) as inputs. Characterise the tradeoff between false-positive rate and detection latency. Produce a justified detection latency bound in milliseconds.


### Deferred — must be tracked, not lost

These items were explicitly deferred during the 2026-05-20 coding framework session. Each has a flagged comment in the relevant source file so the issue is not lost during downstream implementation.

- **Watchdog architecture scrutiny** — `avionics/inc/hardware/platform.h` flags this. The IWDG timeout value, the placement of `platform_watchdog_kick()` calls within each module `_init()`, the IWDG vs WWDG choice, and the safe-state behaviour on watchdog reset vs. assert reset all need a dedicated review. Sign-off required before the platform module is implemented for real.

- **NVIC interrupt priority scheme (D048)** — the relative NVIC priorities of the TIM2 tick ISR and the DMA transfer-complete ISRs must be explicitly defined and documented, so a higher-priority interrupt cannot preempt a lower one mid-update and leave an `isr_flags.h` boundary variable inconsistent. To be decided and logged as D048 during the `platform.h` scrutiny session. TEST-PLT-005 (G1) and TEST-PLT-HW-007 (G2) cannot be authored or run until D048 exists.

- **`CMD_FAULT_ACTUATOR` family (code dependency)** — `command_id_t` in `avionics/inc/avionics_types.h` needs a `CMD_FAULT_ACTUATOR` set/clear family, per actuator index, to support the operator-asserted actuator fault path (D046). Blocks TEST-C2-008, TEST-PFLT-005, TEST-INT-010, TEST-FPP-002, TEST-CFI-002, TEST-CFI-005. Must land before G2 closes.

- **`overrun_count` telemetry field (code dependency)** — `telemetry_frame_t` in `avionics/inc/avionics_types.h` needs an `overrun_count` field, present in every frame, to expose the D047 loop-overrun warning path in a release-build-safe way. Blocks TEST-TEL-008 and the frame-verified TEST-INT-004 warning path. Bundle with the D047 implementation work.

- **Build system** — no Makefile, no STM32CubeIDE `.ioc`, no dev-PC GCC configuration produced this session. The avionics scaffold compiles in principle but has not been validated against any toolchain yet. Build system is the first blocker before any compilation can be verified.

- **`platform_safe_state()` layer violation** — function removed from `platform.h` (was in earlier draft of the plan). Hardware layer cannot include output layer headers; the question of how a hardware-layer assertion drives the actuators to a safe state is unresolved. Options: HAL register write that bypasses `actuators.c`; callback registered at init; assert handler in `main.c` calls `actuators_safe()` directly before halting. Decision needed before any path that fires `platform_safe_state()` is implemented.

- **Yaw observability — no magnetometer (2026-06-14)** — the selected IMUs (BMI160, ICM-42688-P) are 6-axis (accel + gyro); gravity pins roll/pitch but gives no yaw reference, so heading drifts on the gyro alone (unbounded). Constraints §5 lists a "magnetometer disturbance" failure mode the current hardware cannot exhibit — reconcile. Options: add a magnetometer (reopens the mag-disturbance failure mode, which suits the depth axis) or scope yaw to best-effort gyro-only. Logged as R-YAW-01. Decision deferred.

## What's blocked

Nothing blocked.

## Next concrete tasks

1. **Per-module header scrutiny sessions** — one session per header, deep review of contract, NULL semantics, safe defaults, and JPL compliance. Order suggested: `platform.h` (resolve watchdog scrutiny AND log D048 — the NVIC interrupt priority scheme — in the same session; TEST-PLT-005 and TEST-PLT-HW-007 depend on D048) → `bmi160.h` / `icm42688p.h` → `ms5611.h` → `sik_radio.h` → `actuators.h` → `telemetry.h` / `c2.h` → `mode_fsm.h` → `control_law.h` → algorithm-layer last (`fdir.h` / `estimator.h`, now that the boundary is closed under D050).
2. **Build system session** — first blocker before any compilation. Produces: STM32CubeIDE project `.ioc` configured for STM32F407ZGT6, Makefile or CMake configuration that picks up the layered `avionics/inc` and `avionics/src` tree, dev-PC GCC configuration for the `avionics/test/` harnesses.
3. **GCS session** — Python ground station decomposition deliberately deferred from the 2026-05-20 architecture session. Mirrors MCU decomposition: transport / protocol / parser / state model / dashboard / attitude viz / command sender / frame logger. Must implement runtime check of `protocol_version` field on every received frame.
4. **LR-2 — MEMS IMU estimation accuracy** — unblocked by D040. Run against BMI160 and ICM-42688-P datasheets. Produces steady-state EKF error bound in degrees (unblocks REQ-EST-006, REQ-CTL-007).
5. **LR-3 — FDIR innovation gating** — unblocked by D040 + LR-1 (D052, 1000 Hz). Produces detection latency bound and `CHI2_THRESHOLD_2DOF` value (unblocks REQ-FDR-008).
6. **Begin airframe CAD** — first-pass body geometry, avionics bay dimensioned to selected components, fin pivot locations. Run material coupon prints as structural design firms up.
7. **Pi 5: VS Code Remote-SSH setup** — install Remote-SSH extension in VS Code, configure ~/.ssh/config entry for the Pi, connect and verify Pi filesystem is accessible from the laptop.
8. **Compose the formal system block diagram** — referenced as deferred in `docs/05-architecture.md` §1. Choose diagramming methodology appropriate for the documentation standard.
9. **Servo finalisation** — after CAD provides torque and size requirements.
