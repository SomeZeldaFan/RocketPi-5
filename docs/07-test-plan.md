# Test Plan / Test Log

**Status:** POPULATED — full test enumeration complete (2026-05-21). Tests are executed and their results logged as the project progresses through the gate structure below.
**Created:** 2026-05-11
**Last updated:** 2026-05-21

This document captures both the planned tests (what we will verify, how, and against what acceptance criteria) and the executed tests (when run, results, pass/fail, notes).

Each test is repeatable. If the procedure is not written down, the test is not a test — it's a one-off observation.

Per the locked principle in constraints §10.7 (D045): tests are written before the module they test is implemented. A test written after the code confirms the code as written; a test written against the contract verifies the contract.

---

## Format

Each test entry has a unique ID, a purpose, linked requirements/decisions, a procedure, acceptance criteria defined in advance, an apparatus list, and an execution log.

> **TEST-XXX-NNN — Short name**
> *Purpose:* What this test verifies.
> *Linked requirement(s):* REQ-XXX-NNN / Dxxx.
> *Procedure:* Step-by-step what to do.
> *Acceptance criteria:* What constitutes pass vs. fail, defined before the test is run.
> *Apparatus:* What you need to run the test.
> *Execution log:*
> - 2026-XX-XX: Run #1. Result: [pass/fail]. Notes: ...

---

## Gate Structure (non-negotiable)

Progression through the project is governed by gates. No gate may be bypassed. A single failing test blocks the gate. A test that has not been run counts as a failing test.

| Gate | Condition to pass | Unlocks |
|------|-------------------|---------|
| G0 | Build artifact tests pass (BLD-001–004) | Dev-PC unit tests |
| G1 | All dev-PC unit tests pass (PLT structural subset incl. NVIC priority-scheme check / FDR / EST / CTL / TEL / C2 / FSM) + boundary and edge case coverage (BND) + exhaustive FSM transition coverage + code coverage ≥ 90% branch coverage on all algorithm modules (COV) | Hardware peripheral bring-up |
| G2 | All per-peripheral bring-up tests pass (BMI/ICM/BAR/RAD/ACT) + PLT-HW tests pass (TIM2, watchdog, power-on sequencing, interrupt-preemption consistency) + power consumption baseline per rail (PWR) + bus contention tests (BUS) + per-peripheral failure injection (PFLT) + FDIR threshold calibration cross-reference (CAL) | Subsystem integration |
| G3 | All subsystem integration tests pass + end-to-end pipeline data integrity (PIP) + fault propagation completeness (FPP) + reboot and recovery to a verified known safe state (RBT) | Full system integration |
| G4 | All integration tests pass + timing margin quantified (TMG) + watchdog recovery state verified (WDR) + 24 h long-duration soak (SOK), gated by prior validation of the anomaly-detection script (SOK-000) + concurrent/simultaneous multi-fault injection (CFI) | Demo permitted |

---

# LAYER 0 — Build Artifact Validation (dev-PC, pre-flash gate — G0)

> **TEST-BLD-001 — cppcheck clean build**
> *Purpose:* No uninitialized variables, out-of-bounds pointer arithmetic, or NULL dereferences flagged by static analysis.
> *Linked requirement(s):* D019 (JPL Power of 10), D045.
> *Procedure:* Run `cppcheck --enable=all avionics/src/ avionics/inc/`.
> *Acceptance criteria:* Exit code 0, zero errors. Warnings treated as errors.
> *Apparatus:* Dev PC, cppcheck installed.
> *Execution log:* Not yet run.

> **TEST-BLD-002 — No heap allocation in map file**
> *Purpose:* JPL Rule 3 — no dynamic memory allocation anywhere in the build.
> *Linked requirement(s):* D019, D043.
> *Procedure:* Build the firmware; grep the linker `.map` file for `malloc`, `calloc`, `realloc`, `free`, `_sbrk`, and heap segment symbols.
> *Acceptance criteria:* Grep returns empty — no dynamic allocation symbols present.
> *Apparatus:* Dev PC, STM32CubeIDE or arm-none-eabi-gcc + linker map.
> *Execution log:* Not yet run.

> **TEST-BLD-003 — Worst-case stack depth within allocation**
> *Purpose:* Deterministic stack usage — no overflow possible in the call graph.
> *Linked requirement(s):* D019, D043.
> *Procedure:* Compute worst-case stack depth from the full call graph (`arm-none-eabi-nm` + manual trace, or an automated tool). Compare against the stack allocation in the linker script.
> *Acceptance criteria:* Worst-case depth ≤ 80% of the linker stack allocation, leaving explicit margin.
> *Apparatus:* Dev PC, linker script, call graph trace.
> *Execution log:* Not yet run.

> **TEST-BLD-004 — No undefined-behaviour flags (compiler warnings)**
> *Purpose:* Clean compile under a strict warning set.
> *Linked requirement(s):* D019.
> *Procedure:* Compile with `-Wall -Wextra -Werror -Wundef -Wshadow`.
> *Acceptance criteria:* Zero warnings. Every warning is a hard build failure.
> *Apparatus:* Dev PC, GCC dev-PC build config in `avionics/test/`.
> *Execution log:* Not yet run.

---

# LAYER 0 — Platform: G1 Structural Subset (dev-PC, no hardware — G1)

These verify platform-layer discipline by code review and static analysis. They need no hardware — only the source tree.

> **TEST-PLT-001 — Timestamp source exclusively `platform_timer_us()`**
> *Purpose:* No module reads TIM2 registers directly or uses any other time source.
> *Linked requirement(s):* REQ-SYS-008, D034.
> *Procedure:* Grep all `.c` files outside `platform.c` for direct TIM2 register access (`TIM2->CNT`, `TIM2->CCR*`) and alternative clock reads (`HAL_GetTick`, unwrapped `DWT->CYCCNT`).
> *Acceptance criteria:* Zero hits — only `platform.c` touches TIM2 hardware.
> *Apparatus:* Dev PC, grep.
> *Execution log:* Not yet run.

> **TEST-PLT-002 — Watchdog kicked only at boot-complete and tick [13]; never in any `_init()`**
> *Purpose:* Watchdog kick placement matches D053 A3 — no `_init()` kicks the watchdog (a per-init kick would mask a hang that strikes *after* that module's kick); the kick fires once at the boot-complete boundary in `main.c` (after all `_init()` return, before the loop) and then every main-loop iteration at tick [13].
> *Linked requirement(s):* D043; D053 A3; `platform.h` watchdog discipline.
> *Procedure:* Grep all `_init()` function bodies for `platform_watchdog_kick()` (expect none). Confirm exactly one boot-complete call in `main.c` before `for(;;)`, and the tick [13] call inside the loop.
> *Acceptance criteria:* Zero `_init()` bodies call the kick; `main.c` has exactly one boot-complete kick before the loop and the tick [13] kick inside it; no other call sites (kick not buried in algorithm code).
> *Apparatus:* Dev PC, grep.
> *Execution log:* Not yet run.

> **TEST-PLT-003 — Init sequence order in `main.c` matches architecture**
> *Purpose:* `platform_init()` first (IWDG before any peripheral), hardware drivers next, algorithm, output, orchestration last.
> *Linked requirement(s):* §4.1, D043.
> *Procedure:* Read the `main.c` init sequence.
> *Acceptance criteria:* Order is exactly `platform_init` → `bmi160_init` → `icm42688p_init` → `ms5611_init` → `sik_radio_init` → `fdir_init` → `estimator_init` → `control_law_init` → `actuators_init` → `telemetry_init` → `c2_init` → `mode_fsm_init`. No reordering, no gaps.
> *Apparatus:* Dev PC, code review.
> *Execution log:* Not yet run.

> **TEST-PLT-004 — `volatile_flag_t` used for all ISR/main-loop boundary crossings**
> *Purpose:* Every ISR-shared variable uses the canonical type; no ISR-shared variable exists outside `isr_flags.h`.
> *Linked requirement(s):* §4.3, `isr_flags.h` contract.
> *Procedure:* Grep all `.c`/`.h` files for the `volatile` keyword outside `avionics_types.h` (typedef) and `isr_flags.h` + its owning `.c` definitions.
> *Acceptance criteria:* Zero hits — all volatility expressed through `volatile_flag_t`, no scattered bare `volatile` declarations.
> *Apparatus:* Dev PC, grep.
> *Execution log:* Not yet run.

> **TEST-PLT-005 — NVIC interrupt priority scheme matches the documented decision**
> *Purpose:* NVIC priority assignments in `platform.c` match the scheme documented in D048; the scheme is explicitly documented, not implicit.
> *Linked requirement(s):* D048 (must be logged first — `platform.h` scrutiny session).
> *Procedure:* Read every `NVIC_SetPriority` (or HAL equivalent) call in `platform.c`. Cross-reference each against the D048 scheme. Confirm the scheme is documented in a `platform.c`/`platform.h` comment block and in `docs/05-architecture.md §4`.
> *Acceptance criteria:* Every priority value matches the D048 scheme. The TIM2 tick ISR and DMA TC ISRs have a defined relative ordering with a written rationale. No interrupt has an undocumented or inconsistent priority.
> *Apparatus:* Dev PC, code review, D048 decision text.
> *Execution log:* Not yet run. **Unblocked 2026-06-17** — D048 logged (`platform.h` scrutiny session).

---

# LAYER 0 — Platform: Hardware Subset (STM32F407ZGT6 required — G2)

> **TEST-PLT-HW-001 — TIM2 monotonicity**
> *Purpose:* `platform_timer_us()` never returns a value ≤ a previous call.
> *Linked requirement(s):* REQ-SYS-008, D034.
> *Procedure:* Call `platform_timer_us()` 10,000 times consecutively; record all values.
> *Acceptance criteria:* All 10,000 values strictly increasing. Rollover at UINT32_MAX is a documented known event (verified separately in TEST-SOK-002), not a failure.
> *Apparatus:* STM32F407ZGT6, ST-Link, wired UART.
> *Execution log:* Not yet run.

> **TEST-PLT-HW-002 — TIM2 resolution and accuracy**
> *Purpose:* Microsecond resolution; drift < 100 ppm against an external reference.
> *Linked requirement(s):* REQ-SYS-008, D034.
> *Procedure:* Measure 1,000,000 μs by TIM2; compare against an oscilloscope or stopwatch reference.
> *Acceptance criteria:* Within ±100 μs of the reference.
> *Apparatus:* STM32F407ZGT6, oscilloscope or precision reference.
> *Execution log:* Not yet run.

> **TEST-PLT-HW-003 — Watchdog fires on missed kick**
> *Purpose:* IWDG resets the MCU when `platform_watchdog_kick()` is not called within the timeout.
> *Linked requirement(s):* `platform.h` watchdog discipline.
> *Procedure:* Modify firmware to skip the kick at tick [13]. Power on and observe.
> *Acceptance criteria:* MCU resets within the configured timeout (2.00–5.53 s band — fast/slow LSI corners, D053 A2). IWDG reset flag set in RCC_CSR on next boot, classified `RESET_WATCHDOG` by `platform_reset_cause()`, readable over wired UART.
> *Apparatus:* STM32F407ZGT6, modified firmware, wired UART.
> *Execution log:* Not yet run.

> **TEST-PLT-HW-004 — Watchdog fires on hung peripheral init**
> *Purpose:* A hung `_init()` call cannot hold the MCU indefinitely.
> *Linked requirement(s):* `platform.h` watchdog discipline.
> *Procedure:* Insert an infinite loop anywhere inside `bmi160_init()`. Power on. (Per D053 A3 there are no per-`_init()` kicks — the single init window must catch a hang anywhere in the sequence.)
> *Acceptance criteria:* MCU resets within the configured timeout (2.00–5.53 s band).
> *Apparatus:* STM32F407ZGT6, modified firmware.
> *Execution log:* Not yet run.

> **TEST-PLT-HW-005 — Full init sequence completes within watchdog window**
> *Purpose:* All 12 `_init()` calls complete before the watchdog would trip on a clean boot.
> *Linked requirement(s):* D043.
> *Procedure:* Timestamp `platform_init()` entry and `mode_fsm_init()` return using TIM2; print elapsed time over wired UART.
> *Acceptance criteria:* Elapsed time < 50% of the fast-corner (worst-case shortest) timeout = **< 1.0 s** (D053 A2). If exceeded, increase RLR.
> *Apparatus:* STM32F407ZGT6, wired UART, terminal.
> *Execution log:* Not yet run.

> **TEST-PLT-HW-006 — Rail A / Rail B power-on sequencing**
> *Purpose:* The clean rail (MCU, sensors) is stable before the servo rail energises.
> *Linked requirement(s):* D033.
> *Procedure:* Scope Rail A and Rail B simultaneously through a power-on cycle.
> *Acceptance criteria:* Rail A regulated and stable for ≥ 100 ms before Rail B energises. No servo twitch during Rail B power-on.
> *Apparatus:* STM32F407ZGT6, bench PSU, oscilloscope (dual probe).
> *Execution log:* Not yet run.

> **TEST-PLT-HW-007 — Interrupt preemption: ISR/main-loop boundary consistency under simultaneous firing**
> *Purpose:* When the TIM2 tick ISR fires while a DMA TC ISR is mid-execution (or vice versa), `isr_flags.h` boundary variables and double-buffer pointers are left consistent for the main loop.
> *Linked requirement(s):* D048, `isr_flags.h` contract.
> *Procedure:* Instrument the firmware to deliberately align a TIM2 update with a DMA TC event so the two ISRs contend, per the D048 priority scheme. Run ≥ 10,000 simultaneous-firing events; on every main-loop read, check every `isr_flags.h` variable and every double-buffer pointer.
> *Acceptance criteria:* Every `isr_flags.h` variable holds a valid value (0 or 1, no torn intermediate); every double-buffer pointer points to a fully-written page. Zero inconsistencies across all 10,000 events — a single inconsistency is a fail.
> *Apparatus:* STM32F407ZGT6, logic analyser, instrumented firmware, automated consistency checker.
> *Execution log:* Not yet run. **Unblocked 2026-06-17** — D048 logged. (Frames the expected pass as confirming run-to-completion + single-writer ownership + the BASEPRI guard, not "priority = atomicity".)

---

# LAYER 1 — BMI160 Sensor Bring-Up & Characterisation (G2)

> **TEST-BMI-001 — Device ID (WHOAMI)**
> *Purpose:* Genuine Bosch BMI160 connected and communicating.
> *Linked requirement(s):* D040.
> *Procedure:* SPI-read register 0x00 (chip_id) 10 times consecutively.
> *Acceptance criteria:* Returns exactly 0xD1 on all 10 reads. Any other value: fail, investigate counterfeit.
> *Apparatus:* STM32F407ZGT6, BMI160 on SPI1.
> *Execution log:* Not yet run.

> **TEST-BMI-002 — SPI integrity across clock rates**
> *Purpose:* SPI bus wiring is clean — no glitches, no bit errors.
> *Linked requirement(s):* D026, D040.
> *Procedure:* Read chip_id 1,000 times at 1 MHz, at 5 MHz, and at the configured operational rate.
> *Acceptance criteria:* Correct chip_id on every read at every rate — zero errors.
> *Apparatus:* STM32F407ZGT6, logic analyser on SPI1.
> *Execution log:* Not yet run.

> **TEST-BMI-003 — Noise floor vs. datasheet**
> *Purpose:* Sensor is genuine and undamaged; noise density within datasheet spec. Produces the characterised noise values used by TEST-CAL-001.
> *Linked requirement(s):* D040; feeds LR-3, TEST-FDR-015, TEST-CAL-001.
> *Procedure:* 60-second static test on a level surface at room temperature; log raw data; compute RMS noise in Python on the dev PC.
> *Acceptance criteria:* Gyro RMS noise ≤ 0.1 °/s; accel RMS noise ≤ 1.5 mg. Values logged and kept on file.
> *Apparatus:* STM32F407ZGT6, level surface, logged data, Python analysis.
> *Execution log:* Not yet run.

> **TEST-BMI-004 — Bias stability (static)**
> *Purpose:* Gyro bias within expected range; not a saturated or stuck sensor.
> *Linked requirement(s):* D040; feeds TEST-CAL-001.
> *Procedure:* 10-minute static test on a level surface; log to UART; compute per-axis means.
> *Acceptance criteria:* Mean gyro on each axis within ±3 °/s of zero; mean accel Z within ±0.05 g of 1g, X and Y within ±0.05 g of 0.
> *Apparatus:* STM32F407ZGT6, level surface, UART log.
> *Execution log:* Not yet run.

> **TEST-BMI-005 — Output range under known rotation**
> *Purpose:* Axes oriented correctly, scaling correct.
> *Linked requirement(s):* D040.
> *Procedure:* Rotate the sensor 90° about each axis, one axis at a time.
> *Acceptance criteria:* Accel reads ±1g on the expected axis; gyro reads sign-correct rotation; cross-axis coupling < 5%.
> *Apparatus:* STM32F407ZGT6, inclinometer or known-good reference surface.
> *Execution log:* Not yet run.

> **TEST-BMI-006 — Data-ready interrupt at configured ODR**
> *Purpose:* `imu1_data_ready` ISR flag fires at the expected sample rate.
> *Linked requirement(s):* D040, `isr_flags.h`.
> *Procedure:* Count data-ready interrupts over 1 second.
> *Acceptance criteria:* Rate within ±2% of the configured ODR.
> *Apparatus:* STM32F407ZGT6, logic analyser or GPIO toggle + scope.
> *Execution log:* Not yet run.

> **TEST-BMI-007 — DMA double-buffer integrity**
> *Purpose:* No partial or torn reads — timestamp and sensor data from the same sample.
> *Linked requirement(s):* D043, `bmi160.h` contract.
> *Procedure:* Run 10,000 reads; check timestamp monotonicity and sample-to-sample data consistency.
> *Acceptance criteria:* Timestamp strictly increasing; no impossible jumps between consecutive samples (no torn buffer reads).
> *Apparatus:* STM32F407ZGT6, logged output.
> *Execution log:* Not yet run.

> **TEST-BMI-008 — `IMU_BUS_ERROR` on forced SPI fault**
> *Purpose:* Driver returns `IMU_BUS_ERROR`, not garbage, when SPI is disrupted.
> *Linked requirement(s):* `avionics_types.h` (`imu_status_t`), `bmi160.h` contract.
> *Procedure:* Disconnect MISO mid-run.
> *Acceptance criteria:* `bmi160_read()` returns `IMU_BUS_ERROR` within one read cycle. System does not crash.
> *Apparatus:* STM32F407ZGT6, jumper wire.
> *Execution log:* Not yet run.

> **TEST-BMI-009 — `IMU_STALE_DATA` on timeout**
> *Purpose:* Staleness detection triggers within the `IMU_STALENESS_THRESHOLD_US` window.
> *Linked requirement(s):* `avionics_types.h`, `bmi160.h` contract.
> *Procedure:* Stop the DMA stream; wait longer than the threshold; call `bmi160_read()`; resume the stream.
> *Acceptance criteria:* Returns `IMU_STALE_DATA` after the threshold; recovers to `IMU_OK` after the stream resumes.
> *Apparatus:* STM32F407ZGT6, modified firmware.
> *Execution log:* Not yet run.

> **TEST-BMI-010 — `IMU_OUT_OF_RANGE` on saturated values**
> *Purpose:* The driver range-check catches physically implausible values.
> *Linked requirement(s):* `avionics_types.h`, `bmi160.h` contract.
> *Procedure:* Inject raw register values corresponding to > 100 m/s² acceleration via the wired UART debug path.
> *Acceptance criteria:* Driver returns `IMU_OUT_OF_RANGE`; does not propagate garbage to the estimator.
> *Apparatus:* STM32F407ZGT6, dev-PC wired UART injection.
> *Execution log:* Not yet run.

---

# LAYER 1 — ICM-42688-P Sensor Bring-Up & Characterisation (G2)

> **TEST-ICM-001 — Device ID (WHO_AM_I)**
> *Purpose:* Genuine TDK InvenSense ICM-42688-P.
> *Linked requirement(s):* D040.
> *Procedure:* SPI-read the WHO_AM_I register 10 times consecutively.
> *Acceptance criteria:* Returns exactly 0x47 on all 10 reads.
> *Apparatus:* STM32F407ZGT6, ICM-42688-P on SPI2.
> *Execution log:* Not yet run.

> **TEST-ICM-002 — SPI integrity across clock rates**
> *Purpose:* SPI2 bus wiring is clean — no bit errors.
> *Linked requirement(s):* D026, D040.
> *Procedure:* Read WHO_AM_I 1,000 times at 1 MHz, 5 MHz, and the operational rate.
> *Acceptance criteria:* Correct value on every read at every rate — zero errors.
> *Apparatus:* STM32F407ZGT6, logic analyser on SPI2.
> *Execution log:* Not yet run.

> **TEST-ICM-003 — Noise floor vs. datasheet**
> *Purpose:* Sensor genuine and undamaged. Produces characterised noise values for TEST-CAL-001.
> *Linked requirement(s):* D040; feeds LR-3, TEST-FDR-015, TEST-CAL-001.
> *Procedure:* 60-second static test, same methodology as TEST-BMI-003.
> *Acceptance criteria:* Gyro noise density ≤ 0.004 °/s/√Hz (datasheet spec 0.0028; 40% margin). Values logged.
> *Apparatus:* STM32F407ZGT6, level surface, Python analysis.
> *Execution log:* Not yet run.

> **TEST-ICM-004 — Bias stability (static)**
> *Purpose:* Gyro bias within expected range. Feeds TEST-CAL-001.
> *Linked requirement(s):* D040.
> *Procedure:* Same procedure as TEST-BMI-004, on the ICM-42688-P.
> *Acceptance criteria:* Same thresholds as TEST-BMI-004.
> *Apparatus:* STM32F407ZGT6, level surface, UART log.
> *Execution log:* Not yet run.

> **TEST-ICM-005 — Output range under known rotation**
> *Purpose:* Axes oriented correctly, scaling correct.
> *Linked requirement(s):* D040.
> *Procedure:* Same procedure as TEST-BMI-005, on the ICM-42688-P.
> *Acceptance criteria:* Same thresholds as TEST-BMI-005.
> *Apparatus:* STM32F407ZGT6, inclinometer.
> *Execution log:* Not yet run.

> **TEST-ICM-006 — Data-ready interrupt at configured ODR**
> *Purpose:* `imu2_data_ready` ISR flag fires at the expected sample rate.
> *Linked requirement(s):* D040, `isr_flags.h`.
> *Procedure:* Count data-ready interrupts over 1 second.
> *Acceptance criteria:* Rate within ±2% of the configured ODR.
> *Apparatus:* STM32F407ZGT6, logic analyser or GPIO toggle + scope.
> *Execution log:* Not yet run.

> **TEST-ICM-007 — DMA double-buffer integrity**
> *Purpose:* No partial or torn reads on SPI2.
> *Linked requirement(s):* D043, `icm42688p.h` contract.
> *Procedure:* Same procedure as TEST-BMI-007, on SPI2.
> *Acceptance criteria:* Same as TEST-BMI-007.
> *Apparatus:* STM32F407ZGT6, logged output.
> *Execution log:* Not yet run.

> **TEST-ICM-008 — `IMU_BUS_ERROR` on forced SPI fault**
> *Purpose:* Driver returns `IMU_BUS_ERROR` on SPI2 disruption; SPI1 unaffected.
> *Linked requirement(s):* D026, `icm42688p.h` contract.
> *Procedure:* Disconnect SPI2 MISO mid-run while logging both IMU channels.
> *Acceptance criteria:* `icm42688p_read()` returns `IMU_BUS_ERROR` within one read cycle; BMI160 on SPI1 continues producing valid data; no crash.
> *Apparatus:* STM32F407ZGT6, jumper wire.
> *Execution log:* Not yet run.

> **TEST-ICM-009 — `IMU_STALE_DATA` on timeout**
> *Purpose:* Staleness detection triggers within the threshold window on the ICM-42688-P.
> *Linked requirement(s):* `icm42688p.h` contract.
> *Procedure:* Same procedure as TEST-BMI-009, on the ICM-42688-P.
> *Acceptance criteria:* Same as TEST-BMI-009.
> *Apparatus:* STM32F407ZGT6, modified firmware.
> *Execution log:* Not yet run.

> **TEST-ICM-010 — Bus isolation: SPI2 fault does not affect SPI1**
> *Purpose:* D026 — physically separate buses; a fault on one cannot corrupt the other.
> *Linked requirement(s):* D024, D026.
> *Procedure:* Force SPI2 MISO to ground while logging both IMU channels simultaneously.
> *Acceptance criteria:* BMI160 on SPI1 continues producing valid data; no cross-contamination of buffers; no shared fault propagation.
> *Apparatus:* STM32F407ZGT6, jumper wire on SPI2 MISO, dual-channel logging.
> *Execution log:* Not yet run.

---

# LAYER 1 — MS5611 Barometer Bring-Up & Characterisation (G2)

> **TEST-BAR-001 — I2C address scan**
> *Purpose:* MS5611 present and responding at the expected I2C address.
> *Linked requirement(s):* D026, D041.
> *Procedure:* Run an I2C address scan on the barometer bus.
> *Acceptance criteria:* Exactly 0x77 found (PS pin high); no other devices on the bus.
> *Apparatus:* STM32F407ZGT6, I2C bus, logic analyser.
> *Execution log:* Not yet run.

> **TEST-BAR-002 — PROM calibration read and CRC**
> *Purpose:* All 6 factory calibration coefficients loaded correctly; internal CRC valid.
> *Linked requirement(s):* D041, `ms5611.h` contract.
> *Procedure:* Read all 6 PROM coefficients 10 times; compute the MS5611 CRC-4.
> *Acceptance criteria:* All 6 coefficients non-zero; CRC-4 passes; identical values on all 10 reads.
> *Apparatus:* STM32F407ZGT6, I2C.
> *Execution log:* Not yet run.

> **TEST-BAR-003 — Pressure accuracy vs. reference**
> *Purpose:* Compensated pressure output is accurate.
> *Linked requirement(s):* D027, D041.
> *Procedure:* Compare MS5611 output against a known local pressure (weather station or phone barometer cross-reference).
> *Acceptance criteria:* Within ±2 hPa of the reference. Values logged.
> *Apparatus:* STM32F407ZGT6, reference pressure source.
> *Execution log:* Not yet run.

> **TEST-BAR-004 — Temperature compensation accuracy**
> *Purpose:* Temperature channel working and compensation applied.
> *Linked requirement(s):* D041, `ms5611.h` contract.
> *Procedure:* Compare MS5611 temperature output against an independent thermometer.
> *Acceptance criteria:* Within ±2°C of ambient.
> *Apparatus:* STM32F407ZGT6, thermometer.
> *Execution log:* Not yet run.

> **TEST-BAR-005 — Altitude derivation at known reference**
> *Purpose:* `altitude_m` correctly derived from pressure using a standard sea-level reference.
> *Linked requirement(s):* D027, `ms5611.h` contract.
> *Procedure:* At a location of known elevation, compare computed altitude against the looked-up elevation.
> *Acceptance criteria:* Within ±10 m.
> *Apparatus:* STM32F407ZGT6, location with known elevation.
> *Execution log:* Not yet run.

> **TEST-BAR-006 — Decimation at `AVIONICS_BARO_RATE_HZ`**
> *Purpose:* Driver decimates correctly — a new reading at ~50 Hz, not every tick.
> *Linked requirement(s):* `ms5611.h` contract, `AVIONICS_BARO_RATE_HZ`.
> *Procedure:* Count `BARO_OK` returns over 1 second.
> *Acceptance criteria:* Count = 50 ± 2; all other ticks return a non-`BARO_OK` status indicating no new data.
> *Apparatus:* STM32F407ZGT6, logged output.
> *Execution log:* Not yet run.

> **TEST-BAR-007 — I2C bus hang recovery**
> *Purpose:* An I2C hang on the baro bus does not lock up the main loop.
> *Linked requirement(s):* D026.
> *Procedure:* Force an I2C NACK (pull SDA low); then remove the fault.
> *Acceptance criteria:* `ms5611_service()` returns `BARO_BUS_ERROR`; main loop continues; IMUs unaffected; baro recovers to `BARO_OK` within 1 second after the fault is removed.
> *Apparatus:* STM32F407ZGT6, jumper wire on I2C SDA.
> *Execution log:* Not yet run.

> **TEST-BAR-008 — `BARO_STALE_DATA` detection**
> *Purpose:* Staleness detection triggers within the threshold window.
> *Linked requirement(s):* `ms5611.h` contract.
> *Procedure:* Stop the I2C state machine; wait beyond the threshold.
> *Acceptance criteria:* Returns `BARO_STALE_DATA` within the threshold; recovers when serviced again.
> *Apparatus:* STM32F407ZGT6, modified firmware.
> *Execution log:* Not yet run.

---

# LAYER 1 — SiK Radio Bring-Up (G2)

> **TEST-RAD-001 — UART loopback at operational baud rate**
> *Purpose:* UART wiring and baud configuration correct, zero bit errors.
> *Linked requirement(s):* D028, D042, `sik_radio.h` contract.
> *Procedure:* Transmit 10,000 bytes over a wired loopback (TX→RX) on UART1.
> *Acceptance criteria:* Zero bit errors, zero framing errors.
> *Apparatus:* STM32F407ZGT6, jumper-wire loopback on UART1.
> *Execution log:* Not yet run.

> **TEST-RAD-002 — DMA TX queue full behaviour**
> *Purpose:* `sik_radio_tx_send()` returns `false` (not a silent drop) when the TX queue is full.
> *Linked requirement(s):* `sik_radio.h` contract.
> *Procedure:* Fill the TX queue beyond capacity.
> *Acceptance criteria:* Function returns `false`; no buffer overflow; no crash.
> *Apparatus:* STM32F407ZGT6, modified firmware.
> *Execution log:* Not yet run.

> **TEST-RAD-003 — DMA RX ring buffer integrity**
> *Purpose:* Ring buffer pointer advances correctly; no data loss under sustained receive.
> *Linked requirement(s):* D028, `sik_radio.h` contract.
> *Procedure:* Inject 10,000 bytes at the operational baud rate.
> *Acceptance criteria:* All bytes received in order with zero loss.
> *Apparatus:* STM32F407ZGT6, PC serial sender.
> *Execution log:* Not yet run.

> **TEST-RAD-004 — Air-to-ground link packet loss**
> *Purpose:* Radio link works at bench range with acceptable packet loss.
> *Linked requirement(s):* D042.
> *Procedure:* Transmit 1,000 telemetry frames over the radio pair at bench range (< 5 m).
> *Acceptance criteria:* Packet loss < 1%.
> *Apparatus:* STM32F407ZGT6 + HolyBro SiK air module, HolyBro ground module on Pi.
> *Execution log:* Not yet run.

> **TEST-RAD-005 — Bidirectional simultaneous operation**
> *Purpose:* MCU can TX telemetry and RX commands simultaneously without corruption.
> *Linked requirement(s):* D020, D028.
> *Procedure:* Run MCU TX at telemetry rate while the ground station sends commands.
> *Acceptance criteria:* Zero frame corruption in either direction.
> *Apparatus:* Full hardware setup, Pi ground station.
> *Execution log:* Not yet run.

> **TEST-RAD-006 — Wired fallback transport**
> *Purpose:* The wired UART carries the same protocol as the radio.
> *Linked requirement(s):* D029.
> *Procedure:* Disconnect the radio; connect the wired UART to the Pi.
> *Acceptance criteria:* Identical frame format received; no protocol changes required.
> *Apparatus:* STM32F407ZGT6, USB-serial adapter, Pi.
> *Execution log:* Not yet run.

---

# LAYER 1 — Actuators Bring-Up (hardware — G2)

> **TEST-ACT-001 — PWM frequency on all 4 channels**
> *Purpose:* PWM output at the correct frequency for the selected servos.
> *Linked requirement(s):* D030, `actuators.h` contract.
> *Procedure:* Scope each of the 4 PWM channels.
> *Acceptance criteria:* Frequency within ±1% of target on every channel.
> *Apparatus:* STM32F407ZGT6, oscilloscope.
> *Execution log:* Not yet run.

> **TEST-ACT-002 — Pulse width range matches servo spec**
> *Purpose:* Min and max PWM pulse widths correspond to the full fin deflection range.
> *Linked requirement(s):* `actuators.h` contract.
> *Procedure:* Command minimum and maximum deflection; measure the pulse widths.
> *Acceptance criteria:* Min ≈ 1000 μs, max ≈ 2000 μs (servo-dependent), each within ±10 μs of the expected value.
> *Apparatus:* STM32F407ZGT6, oscilloscope.
> *Execution log:* Not yet run.

> **TEST-ACT-003 — `actuators_safe()` zeroes all fins**
> *Purpose:* The safe call always drives all fins to zero deflection regardless of prior command.
> *Linked requirement(s):* `actuators.h` contract.
> *Procedure:* Command arbitrary deflections on all fins, then call `actuators_safe()`; scope all 4 channels.
> *Acceptance criteria:* All 4 channels at zero-deflection pulse width within one PWM cycle.
> *Apparatus:* STM32F407ZGT6, 4-channel oscilloscope.
> *Execution log:* Not yet run.

> **TEST-ACT-004 — Hard clamp in flight mode**
> *Purpose:* `actuators_write()` clamps commands exceeding flight-mode limits.
> *Linked requirement(s):* D020, REQ-CTL-005, `actuators.h` contract.
> *Procedure:* Command a deflection of 3× the flight limit in `SYS_MODE_FLIGHT`; measure the PWM.
> *Acceptance criteria:* Measured pulse width corresponds to exactly the flight limit, not the commanded value.
> *Apparatus:* STM32F407ZGT6, oscilloscope.
> *Execution log:* Not yet run.

> **TEST-ACT-005 — Hard clamp in demo mode**
> *Purpose:* Relaxed demo-mode limits enforced by the actuator layer.
> *Linked requirement(s):* D020, `actuators.h` contract.
> *Procedure:* Same as TEST-ACT-004 with `SYS_MODE_DEMO` active.
> *Acceptance criteria:* Measured pulse width corresponds to exactly the demo limit (larger than flight, but clamped).
> *Apparatus:* STM32F407ZGT6, oscilloscope.
> *Execution log:* Not yet run.

> **TEST-ACT-006 — Rail A voltage during max servo slew**
> *Purpose:* D033 — servo current draw does not brown out the MCU or contaminate sensor rails.
> *Linked requirement(s):* D033.
> *Procedure:* Command all 4 servos to simultaneous hard slew (max rate, opposite extremes); scope Rail A and the servo rail.
> *Acceptance criteria:* Rail A stays above the MCU brownout threshold (2.7 V for STM32F4) throughout; noise on Rail A < 50 mV peak-to-peak.
> *Apparatus:* STM32F407ZGT6, 4 servos, bench PSU, dual-probe oscilloscope.
> *Execution log:* Not yet run.

> **TEST-ACT-007 — Mechanical fin throw matches command**
> *Purpose:* Servo horn geometry and linkage produce the commanded angle at the fin.
> *Linked requirement(s):* D030; feeds the control law actuator model.
> *Procedure:* Command 5°, 10°, 15°, and max deflection on each fin; measure actual fin angle with a digital inclinometer.
> *Acceptance criteria:* Error < ±1° at each point for all 4 fins.
> *Apparatus:* Assembled airframe section, digital inclinometer.
> *Execution log:* Not yet run.

> **TEST-ACT-008 — Per-fin geometric zero calibration**
> *Purpose:* A zero-deflection command produces a geometrically neutral fin position.
> *Linked requirement(s):* D030.
> *Procedure:* Command 0 rad on each fin; measure with an inclinometer.
> *Acceptance criteria:* Each fin within ±0.5° of geometric neutral.
> *Apparatus:* Assembled airframe, digital inclinometer.
> *Execution log:* Not yet run.

---

# LAYER 2 — FDIR Unit Tests (dev-PC test harness — G1)

> **TEST-FDR-001 — Staleness detection: IMU1**
> *Purpose:* `fdir_admit()` sets `imu1_healthy = false` when the IMU1 timestamp is stale.
> *Linked requirement(s):* REQ-FDR-001, `fdir.h` contract.
> *Procedure:* Inject an `imu_reading_t` with `timestamp_us` = (current_time − IMU_STALENESS_THRESHOLD_US − 1).
> *Acceptance criteria:* `health_out.imu1_healthy == false`.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-002 — Staleness detection: IMU2**
> *Purpose:* Same as TEST-FDR-001 for IMU2.
> *Linked requirement(s):* REQ-FDR-001, `fdir.h` contract.
> *Procedure:* Inject a stale IMU2 reading past the threshold.
> *Acceptance criteria:* `health_out.imu2_healthy == false`.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-003 — Staleness detection: baro**
> *Purpose:* Same methodology for the barometer channel.
> *Linked requirement(s):* `fdir.h` contract.
> *Procedure:* Inject a stale baro reading past the threshold.
> *Acceptance criteria:* `health_out.baro_healthy == false`.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-004 — Bounds check: accel out of range**
> *Purpose:* Physically implausible acceleration is flagged.
> *Linked requirement(s):* `fdir.h` contract.
> *Procedure:* Inject `accel_mss[0] = 200` m/s² (far beyond physical limit).
> *Acceptance criteria:* `imu1_healthy == false`; the value is not propagated to the estimator.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-005 — Bounds check: gyro out of range**
> *Purpose:* Physically implausible angular rate is flagged.
> *Linked requirement(s):* `fdir.h` contract.
> *Procedure:* Inject `gyro_rads` beyond the physical limit.
> *Acceptance criteria:* `imu1_healthy == false`; value not propagated.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-006 — Bounds check: baro out of range**
> *Purpose:* Implausible pressure is flagged.
> *Linked requirement(s):* `fdir.h` contract.
> *Procedure:* Inject `pressure_pa = 0` and `pressure_pa = 200,000`.
> *Acceptance criteria:* `baro_healthy == false` for both.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-007 — Innovation gate: both IMUs agree**
> *Purpose:* No false positive when sensors agree. Exercises `fdir_gate()` (post-predict).
> *Linked requirement(s):* REQ-FDR-008, D024, D050; requires `CHI2_THRESHOLD_2DOF` from LR-3.
> *Procedure:* Call `fdir_gate()` with IMU1/IMU2 readings consistent within noise and matching `predicted_readings_t` (chi-squared < `CHI2_THRESHOLD_2DOF`); preliminary health all-healthy.
> *Acceptance criteria:* `imu1_gate_open == true`, `imu2_gate_open == true`; both remain healthy.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-008 — Innovation gate: IMU1 is outlier**
> *Purpose:* The outlier IMU is isolated; the healthy IMU is retained. Exercises `fdir_gate()`.
> *Linked requirement(s):* REQ-FDR-008, D024, D050.
> *Procedure:* Call `fdir_gate()` with an IMU1 reading whose residual vs the prediction exceeds the threshold while IMU2 agrees.
> *Acceptance criteria:* `imu1_gate_open == false`, `imu2_gate_open == true`; `imu1_healthy == false`, `imu2_healthy == true`.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-009 — Innovation gate: IMU2 is outlier**
> *Purpose:* Symmetric to TEST-FDR-008 — IMU2 flagged, IMU1 retained. Exercises `fdir_gate()`.
> *Linked requirement(s):* REQ-FDR-008, D024, D050.
> *Procedure:* Call `fdir_gate()` with an IMU2 reading whose residual vs the prediction exceeds the threshold while IMU1 agrees.
> *Acceptance criteria:* `imu2_gate_open == false`, `imu1_gate_open == true`; `imu2_healthy == false`, `imu1_healthy == true`.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-010 — Detection latency within LR-3 bound**
> *Purpose:* FDIR detects and isolates a fault within the specified detection latency, whether caught at `fdir_admit()` (stale/out-of-range) or `fdir_gate()` (innovation).
> *Linked requirement(s):* REQ-FDR-008, D050; requires the latency bound from LR-3.
> *Procedure:* Inject a fault at tick N; measure the tick at which isolation occurs across the admit→predict→gate sequence.
> *Acceptance criteria:* `imu1_healthy == false` appears by tick N + (detection_latency_bound / tick_period). Pass only if within the bound.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-011 — Recovery semantics (REFRAMED by D063: latch-until-reset)**
> *Purpose:* Under the D063 severity model isolation is STICKY: a transient (sub-count) TIER-2 outlier withholds the measurement for the tick and then recovers on clean data, but a *latched* channel (any TIER-1 fault, or `FDIR_DEGRADE_COUNT` consecutive TIER-2 failures) does NOT auto-recover — it clears only via `fdir_init()` (a reset/reboot, the operator re-admit path).
> *Linked requirement(s):* `fdir.h` contract, D063.
> *Procedure:* (a) inject one TIER-2 outlier then clean data → channel recovers; (b) inject a sustained fault until latched, then clean data → channel stays isolated; (c) call `fdir_init()` → channel re-admitted.
> *Acceptance criteria:* (a) `imu1_healthy` true again after the single spike; (b) `imu1_healthy` stays false on clean data once latched; (c) `imu1_healthy` true after `fdir_init()`.
> *Apparatus:* Dev-PC test harness (`test_severity_latch`, `test_init_clears`).
> *Execution log:* Run 2026-06-19 (Task 3) — PASS.

> **TEST-FDR-012 — Dual IMU fault → dead reckoning path**
> *Purpose:* Both IMUs faulted simultaneously does not crash the system.
> *Linked requirement(s):* D024, `estimator.h` contract.
> *Procedure:* Inject both IMU readings as stale or out of range.
> *Acceptance criteria:* `imu1_healthy == false`, `imu2_healthy == false`; no assert, no crash; estimator transitions to `EST_MODE_DEAD_RECKONING` when called with both health flags false.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-013 — Baro fault isolated: IMUs unaffected**
> *Purpose:* A fault on one bus does not propagate to others (D026 guarantee, verified in software).
> *Linked requirement(s):* D026.
> *Procedure:* Inject a baro out-of-range reading with clean IMU readings.
> *Acceptance criteria:* `baro_healthy == false`; `imu1_healthy` and `imu2_healthy` unchanged (true).
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-014 — Health flag authority (code review test)**
> *Purpose:* No module other than `fdir.c` writes to any field of `health_flags_t`.
> *Linked requirement(s):* §4.4, `fdir.h` contract.
> *Procedure:* Grep all `.c` files outside `fdir.c` for writes to `health_flags_t` members.
> *Acceptance criteria:* Zero results.
> *Apparatus:* Dev PC, grep.
> *Execution log:* Not yet run.

> **TEST-FDR-015 — False positive rate on clean data**
> *Purpose:* `CHI2_THRESHOLD_2DOF` is not too tight — the gate does not trigger on genuine sensor noise.
> *Linked requirement(s):* REQ-FDR-008; depends on TEST-BMI-003/TEST-ICM-003 characterised noise.
> *Procedure:* Run 10,000 ticks with clean synthetic IMU data drawn from the characterised noise distributions.
> *Acceptance criteria:* False positive rate (gate closed on clean data) < 5%.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FDR-016 — NULL baro input handled**
> *Purpose:* The function is safe when the baro pointer is NULL (isolated sensor).
> *Linked requirement(s):* `fdir.h` contract.
> *Procedure:* Call `fdir_admit()` with NULL for baro.
> *Acceptance criteria:* No crash, no assert; `baro_healthy` set to false.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Run 2026-06-19 (Task 3) — PASS.

> **TEST-FDR-017 — Two-tier severity / debounce (D063)**
> *Purpose:* TIER-1 (hard) faults isolate immediately; TIER-2 (soft, innovation) faults latch only after `FDIR_DEGRADE_COUNT` consecutive failures; NaN/Inf is always an immediate isolate (never reaches the estimator).
> *Linked requirement(s):* D063, `fdir.h` contract.
> *Procedure:* Single envelope/NaN sample (TIER-1) → immediate isolate; single innovation outlier (TIER-2) → withheld but not latched; `FDIR_DEGRADE_COUNT` consecutive outliers → latched.
> *Acceptance criteria:* matches the tier rules above (`test_severity_latch`, `test_admit_nonfinite`).
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Run 2026-06-19 (Task 3) — PASS.

> **TEST-FDR-018 — Magnetometer disturbance detection (§5, D060/D063)**
> *Purpose:* The single mag, with no twin, is caught two ways: the innovation gate vs the IMU-predicted field (wrong direction) and the model-free dip-angle check (wrong angle to gravity); sustained disturbance isolates the mag while the IMUs stay healthy (yaw degrades gracefully). Also the `|B|` magnitude variant in admit.
> *Linked requirement(s):* Constraints §5, D060, D063, REQ-FDR-008.
> *Procedure:* (a) `|B|` out of band → admit isolates; (b) in-band wrong-direction field → gate chi2 isolates; (c) `pred == meas` but field aligned with gravity → dip check isolates; exercises the IMU1→IMU2→none accel-reference fallback.
> *Acceptance criteria:* mag isolated in each case; `imu1/2_healthy` unaffected (`test_admit_mag_magnitude`, `test_gate_mag_disturbance`).
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Run 2026-06-19 (Task 3) — PASS.

> **TEST-FDR-019 — Gyro cross-check is detector-only; restrict-only gate (A1, D063)**
> *Purpose:* An IMU1-vs-IMU2 gyro disagreement sets `gate_out.gyro_disagree` but isolates NEITHER IMU (innocent until proven guilty); the gate is restrict-only and never re-admits a channel admit isolated.
> *Linked requirement(s):* D063 (A1), §4.4 invariant 5, `fdir.h` contract.
> *Procedure:* Disagreeing gyros → flag set, both IMUs healthy; admit-isolated channel fed a perfect reading through the gate → stays isolated.
> *Acceptance criteria:* `gyro_disagree == true` with both IMUs healthy; gate never sets a health flag true (`test_gyro_crosscheck`, `test_restrict_only`).
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Run 2026-06-19 (Task 3) — PASS.

---

# LAYER 2 — Estimator Unit Tests (dev-PC test harness — G1)

> **TEST-EST-001 — Static convergence to level**
> *Purpose:* Estimator converges to roll=0, pitch=0 from an arbitrary initial condition under static inputs.
> *Linked requirement(s):* REQ-EST-004, D035.
> *Procedure:* Inject static IMU readings (gravity on Z, zero gyro) from an initial condition of roll=30°.
> *Acceptance criteria:* Converges to |roll| < 1°, |pitch| < 1° within 200 ticks.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-EST-002 — Pure single-axis rotation tracking**
> *Purpose:* Estimator tracks a known rotation rate accurately.
> *Linked requirement(s):* REQ-EST-006 (from LR-2).
> *Procedure:* Inject a constant gyro rate of 30 °/s about Z; after 1 second, read the estimated yaw.
> *Acceptance criteria:* Estimated yaw = 30° ± the REQ-EST-006 bound.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-EST-003 — Analytical quaternion case**
> *Purpose:* A known 90° rotation about Z produces the correct output — the hand-verifiable baseline.
> *Linked requirement(s):* D035.
> *Procedure:* Inject a perfect 90°-about-Z rotation (synthetic, no noise).
> *Acceptance criteria:* Output roll=0°, pitch=0°, yaw=90° ± 0.1°.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-EST-004 — `EST_MODE_IMU1_ONLY` when IMU2 isolated**
> *Purpose:* A NULL pointer for IMU2 triggers the correct degraded mode.
> *Linked requirement(s):* `estimator.h` contract.
> *Procedure:* Call `estimator_update()` with NULL for imu2.
> *Acceptance criteria:* Return value `EST_MODE_IMU1_ONLY`; estimator continues producing valid output; no crash, no NULL dereference.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-EST-005 — `EST_MODE_IMU2_ONLY` when IMU1 isolated**
> *Purpose:* Symmetric to TEST-EST-004.
> *Linked requirement(s):* `estimator.h` contract.
> *Procedure:* Call `estimator_update()` with NULL for imu1.
> *Acceptance criteria:* Return value `EST_MODE_IMU2_ONLY`; valid output; no crash.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-EST-006 — `EST_MODE_DEAD_RECKONING` when both isolated**
> *Purpose:* Both IMUs NULL drops to dead reckoning without crash.
> *Linked requirement(s):* `estimator.h` contract.
> *Procedure:* Call `estimator_update()` with NULL for both IMUs.
> *Acceptance criteria:* Return `EST_MODE_DEAD_RECKONING`; covariance grows each tick; output still populated from the last known state.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-EST-007 — Covariance grows under sensor loss**
> *Purpose:* §4.4 invariant 3 — covariance reflects actual uncertainty.
> *Linked requirement(s):* §4.4.
> *Procedure:* Establish a converged dual-IMU estimate; remove IMU2 (NULL); run 100 ticks.
> *Acceptance criteria:* All `covariance[0..5]` larger after 100 ticks than at the transition point.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-EST-008 — Covariance never zero-initialised silently**
> *Purpose:* The output covariance field is always meaningfully populated.
> *Linked requirement(s):* §4.4.
> *Procedure:* Run the estimator from cold init; inspect `out.covariance` on every tick.
> *Acceptance criteria:* `out.covariance[i] > 0` for all i on every tick from tick 1 onward.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-EST-009 — `estimator_reset()` clears state**
> *Purpose:* Reset returns the estimator to initial conditions.
> *Linked requirement(s):* `estimator.h` contract.
> *Procedure:* Run to convergence; call `estimator_reset()`; inspect internal state.
> *Acceptance criteria:* Covariance back to initial (large) values; mode back to initial; state indistinguishable from a fresh `estimator_init()`.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-EST-010 — Deterministic replay**
> *Purpose:* The same input sequence always produces the same output — no hidden non-determinism.
> *Linked requirement(s):* Firmware correctness gate — deterministic replay is a required property of the control-path firmware.
> *Procedure:* Record N ticks of sensor input; replay twice with `estimator_reset()` between.
> *Acceptance criteria:* Both output sequences bit-identical.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-EST-011 — Staged tick sequence: admit → predict → gate → update (D050)**
> *Purpose:* The `fdir_admit()` → `estimator_predict()` → `fdir_gate()` → `estimator_update()` sequence produces the correct posterior.
> *Linked requirement(s):* D050 (FDIR/estimator boundary).
> *Procedure:* Run the staged tick; `fdir_admit()` sets preliminary health; pass `estimator_predict()`'s `predicted_readings_t` into `fdir_gate()`; pass only healthy-flagged sensors (NULL for isolated) into `estimator_update()`.
> *Acceptance criteria:* The gate computes residual = measured − predicted; `fdir_gate()` only restricts health (never resurrects an admitted-out channel); full sequence produces output equivalent to the analytical case TEST-EST-003.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-EST-012 — `EST_MODE_FAULT` propagation**
> *Purpose:* Downstream consumers handle `EST_MODE_FAULT` gracefully.
> *Linked requirement(s):* `estimator.h` contract, §4.4.
> *Procedure:* Force the estimator into `EST_MODE_FAULT`; run a full-stack tick.
> *Acceptance criteria:* Control law returns `CTL_MODE_SAFE_HOLD`; actuators go to zero; telemetry frame still sends with the fault mode encoded; no assert.
> *Apparatus:* Dev-PC test harness (full-stack).
> *Execution log:* Not yet run.

---

# LAYER 2 — Control Law Unit Tests (dev-PC test harness — G1)

> **TEST-CTL-001 — Zero attitude error → zero deflection**
> *Purpose:* No spurious output when the system is at target.
> *Linked requirement(s):* `control_law.h` contract.
> *Procedure:* Inject a level attitude estimate (zero angles, zero rates, nominal covariance).
> *Acceptance criteria:* All four `deflection_rad[i]` = 0.0 ± floating-point epsilon.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-CTL-002 — Non-zero error → correct direction command**
> *Purpose:* The control law produces a corrective command in the right direction.
> *Linked requirement(s):* `control_law.h` contract.
> *Procedure:* Inject a 10° pitch error (nose up); inspect the per-fin commands.
> *Acceptance criteria:* The pitch-correcting fins receive corrective (not destabilising) deflection; direction verified against the expected mixer output.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-CTL-003 — Deflection clamp in flight mode**
> *Purpose:* `actuators_write()` enforces flight-mode limits.
> *Linked requirement(s):* REQ-CTL-005, D020.
> *Procedure:* Inject an attitude error large enough that the unclamped command would exceed flight limits; measure the PWM.
> *Acceptance criteria:* Measured PWM does not exceed the flight limit. (Control law may output any value; the actuator layer clamps.)
> *Apparatus:* STM32F407ZGT6 + actuators, oscilloscope.
> *Execution log:* Not yet run.

> **TEST-CTL-004 — Relaxed limits in demo mode**
> *Purpose:* Demo-mode relaxed limits applied.
> *Linked requirement(s):* D020.
> *Procedure:* Repeat TEST-CTL-003 input with `SYS_MODE_DEMO` active.
> *Acceptance criteria:* PWM shows a larger deflection than TEST-CTL-003 but within the demo limit.
> *Apparatus:* STM32F407ZGT6 + actuators, oscilloscope.
> *Execution log:* Not yet run.

> **TEST-CTL-005 — `SYS_MODE_SAFE_HOLD` → zero deflection always**
> *Purpose:* Safe hold overrides any attitude error.
> *Linked requirement(s):* `control_law.h` contract.
> *Procedure:* Inject an arbitrary attitude error with mode `SYS_MODE_SAFE_HOLD`.
> *Acceptance criteria:* All four deflections = zero, always.
> *Apparatus:* Dev-PC test harness + hardware verification.
> *Execution log:* Not yet run.

> **TEST-CTL-006 — `CTL_MODE_3FIN_REDUCED`: single fin fault**
> *Purpose:* D030 — the mixing matrix reconfigures after a single fin loss.
> *Linked requirement(s):* D030.
> *Procedure:* Set `health.actuator_healthy[0] = false`; call `control_law_update()`; inspect output and verify the mixing matrix math analytically.
> *Acceptance criteria:* Returns `CTL_MODE_3FIN_REDUCED`; fin 0 command = 0; the remaining 3 fins receive redistributed commands producing net corrective torque.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-CTL-007 — `CTL_MODE_2FIN_REDUCED`: two-fin fault**
> *Purpose:* The mixing matrix reconfigures for two-fin loss.
> *Linked requirement(s):* D030.
> *Procedure:* Set `actuator_healthy[0] = false`, `actuator_healthy[1] = false`; call `control_law_update()`.
> *Acceptance criteria:* Returns `CTL_MODE_2FIN_REDUCED`; both faulted fins commanded to 0; the remaining two receive limited-authority corrective commands.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-CTL-008 — `CTL_MODE_SAFE_HOLD` on 3+ fin faults**
> *Purpose:* Three or more fin faults drop to safe hold.
> *Linked requirement(s):* D030.
> *Procedure:* Set 3 or more fins unhealthy; call `control_law_update()`.
> *Acceptance criteria:* Returns `CTL_MODE_SAFE_HOLD`; all fins commanded to 0.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-CTL-009 — High covariance → reduced authority**
> *Purpose:* The control law respects estimator confidence.
> *Linked requirement(s):* §4, `control_law.h` contract.
> *Procedure:* Inject an estimate with covariance 100× nominal; compare command magnitude against the nominal-covariance case for the same attitude error.
> *Acceptance criteria:* Command magnitude ≤ 50% of the nominal-covariance command; direction confirmed (more uncertainty → gentler command).
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-CTL-010 — Closed-loop stability on free-body vehicle dynamics plant model**
> *Purpose:* The control law is actually stable with the expected vehicle dynamics.
> *Linked requirement(s):* REQ-CTL-003, REQ-CTL-004.
> *Procedure:* Implement a simplified free-body vehicle dynamics plant model from inertia estimates derived during airframe CAD; close the loop on the dev PC; apply a 10° step.
> *Acceptance criteria:* Step response settles within the REQ-CTL-004 bound without oscillation; gain margin > 6 dB, phase margin > 45° from the Bode plot.
> *Apparatus:* Dev PC, Python plant model.
> *Execution log:* Not yet run. NOTE — depends on airframe CAD geometry for inertia estimates.

> **TEST-CTL-011 — Return value matches internal mode selection**
> *Purpose:* `control_law_update()` return value is always consistent with `out->mode`.
> *Linked requirement(s):* `control_law.h` contract.
> *Procedure:* Run all health-flag combinations; compare the return value against `out->mode`.
> *Acceptance criteria:* They agree on every combination — no case where return and `out->mode` differ.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

---

# LAYER 2 — Telemetry Unit Tests (dev-PC + hardware — G1/G2)

> **TEST-TEL-001 — Protocol version in first byte**
> *Purpose:* Frame format invariant — the version field is always first.
> *Linked requirement(s):* §4.4, `AVIONICS_PROTOCOL_VERSION`.
> *Procedure:* Pack 1,000 consecutive frames; inspect raw bytes.
> *Acceptance criteria:* `bytes[0] == AVIONICS_PROTOCOL_VERSION` on every frame.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-TEL-002 — CRC-16/CCITT correctness**
> *Purpose:* CRC computed correctly.
> *Linked requirement(s):* `telemetry.h` contract.
> *Procedure:* Pack a known frame; compute the expected CRC with an independent Python reference; then corrupt one byte.
> *Acceptance criteria:* Computed CRC matches the reference; after the byte corruption, the CRC check fails.
> *Apparatus:* Dev-PC test harness + Python reference.
> *Execution log:* Not yet run.

> **TEST-TEL-003 — CRC covers entire frame**
> *Purpose:* No byte lies outside CRC coverage.
> *Linked requirement(s):* `telemetry.h` contract.
> *Procedure:* Flip each byte of the frame, one at a time, recomputing the CRC each time.
> *Acceptance criteria:* Every single byte flip changes the CRC. Any byte that does not is a fail.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-TEL-004 — Frame ID monotonically increments**
> *Purpose:* `frame_id` increments by 1 per frame so the GCS can detect drops.
> *Linked requirement(s):* `telemetry.h` contract.
> *Procedure:* Send 1,000 frames; inspect `frame_id` on the GCS.
> *Acceptance criteria:* `frame_id` increments by exactly 1 each frame; GCS reports zero gaps.
> *Apparatus:* STM32F407ZGT6 + radio + Pi GCS.
> *Execution log:* Not yet run.

> **TEST-TEL-005 — Health flags unconditionally present**
> *Purpose:* §4.4 invariant — health is never omitted from a downlink frame.
> *Linked requirement(s):* §4.4.
> *Procedure:* Inspect 1,000 frames across various sensor states.
> *Acceptance criteria:* The `health` field is explicitly populated in every frame, including the all-healthy case (never a zero-struct by omission).
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-TEL-006 — Timestamp monotonicity in frames**
> *Purpose:* `timestamp_us` never regresses across frames.
> *Linked requirement(s):* D034.
> *Procedure:* Log 1,000 received frames; compare each `timestamp_us` against the previous.
> *Acceptance criteria:* Every frame's `timestamp_us` ≥ the previous frame's. No backward jump.
> *Apparatus:* STM32F407ZGT6 + radio + Pi log.
> *Execution log:* Not yet run.

> **TEST-TEL-007 — TX queue full returns false, no crash**
> *Purpose:* A full TX queue is handled without silent corruption.
> *Linked requirement(s):* `telemetry.h` / `sik_radio.h` contracts.
> *Procedure:* Fill the TX queue.
> *Acceptance criteria:* `sik_radio_tx_send()` returns false; main loop continues; no silent data corruption.
> *Apparatus:* STM32F407ZGT6, modified firmware.
> *Execution log:* Not yet run.

> **TEST-TEL-008 — Overrun counter present in telemetry frame (D047)**
> *Purpose:* The D047 overrun warning path is observable in the telemetry frame itself, not only in wired UART debug output (debug output is stripped in release builds — a frame field is the durable, release-build-safe channel).
> *Linked requirement(s):* D047.
> *Procedure:* With the D047 warning path active, inject 1 then 2 consecutive overruns; inspect the captured telemetry frame's `overrun_count` field; then inject an on-time tick.
> *Acceptance criteria:* `overrun_count` is populated with the correct count during overruns and resets to 0 on a clean on-time tick. The field is present in every frame regardless of overrun state — never conditionally populated, never omitted.
> *Apparatus:* Dev-PC test harness + hardware verification.
> *Execution log:* Not yet run. NOTE — depends on the `overrun_count` field being added to `telemetry_frame_t`.

---

# LAYER 2 — C2 Unit Tests (dev-PC + hardware — G1/G2)

> **TEST-C2-001 — Valid command frame parses correctly**
> *Purpose:* A well-formed command frame is parsed correctly.
> *Linked requirement(s):* `c2.h` contract.
> *Procedure:* Inject a correctly-formed, version-matched, CRC-valid `command_frame_t` into the RX buffer.
> *Acceptance criteria:* `c2_receive()` returns true and fills `*out`; all fields match the injected values.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-C2-002 — Wrong protocol version silently discarded**
> *Purpose:* A version mismatch is rejected cleanly.
> *Linked requirement(s):* `c2.h` contract, `AVIONICS_PROTOCOL_VERSION`.
> *Procedure:* Inject a frame with `protocol_version` ≠ `AVIONICS_PROTOCOL_VERSION`.
> *Acceptance criteria:* `c2_receive()` returns false; no crash; no partial struct filled.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-C2-003 — CRC failure silently discarded**
> *Purpose:* A corrupted frame is rejected.
> *Linked requirement(s):* `c2.h` contract.
> *Procedure:* Inject a frame with one byte corrupted.
> *Acceptance criteria:* `c2_receive()` returns false; no crash.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-C2-004 — Partial frame: no crash, no partial parse**
> *Purpose:* An incomplete frame never produces a partial parse.
> *Linked requirement(s):* `c2.h` contract.
> *Procedure:* Inject 1, 2, 3, … N−1 bytes of a valid frame (never completing it).
> *Acceptance criteria:* `c2_receive()` returns false on every call; no crash, no memory corruption.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-C2-005 — All command_id_t values parse correctly**
> *Purpose:* Every defined command parses and produces the correct command field.
> *Linked requirement(s):* `c2.h` contract, `command_id_t`.
> *Procedure:* Inject one valid frame for each defined `command_id_t` value (including `CMD_FAULT_ACTUATOR` once added).
> *Acceptance criteria:* All parse correctly with the correct command field.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-C2-006 — `CMD_RESET_ESTIMATOR` triggers reset end-to-end**
> *Purpose:* The reset command reaches `estimator_reset()`.
> *Linked requirement(s):* `c2.h` contract, `command_id_t`.
> *Procedure:* Inject `CMD_RESET_ESTIMATOR` via radio; observe `estimator_reset()` invocation (UART debug trace or telemetry state change).
> *Acceptance criteria:* `estimator_reset()` is called on the MCU.
> *Apparatus:* Full hardware setup + Pi GCS.
> *Execution log:* Not yet run.

> **TEST-C2-007 — Mode command acknowledgment in downlink**
> *Purpose:* A mode command is acknowledged in the downlink.
> *Linked requirement(s):* D020.
> *Procedure:* Inject `CMD_SET_MODE_DEMO`; inspect the next downlink frame.
> *Acceptance criteria:* The next frame's `sys_mode` field is `SYS_MODE_DEMO`; the GCS dashboard reflects the active mode and acknowledgment.
> *Apparatus:* Full hardware setup + Pi GCS.
> *Execution log:* Not yet run.

> **TEST-C2-008 — `CMD_FAULT_ACTUATOR_i` sets actuator health flag end-to-end**
> *Purpose:* The operator-asserted actuator fault path (D046).
> *Linked requirement(s):* D046; depends on a `CMD_FAULT_ACTUATOR` family (set + clear, per actuator index) added to `command_id_t`.
> *Procedure:* Inject `CMD_FAULT_ACTUATOR_0` from the GCS; inspect the next telemetry frame; inject the clear variant; repeat for all four actuator indices and an out-of-range index.
> *Acceptance criteria:* `actuator_healthy[0] = false` after the set, `= true` after the clear; correct behaviour on all four indices; no crash on any index value, including out-of-range.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run. NOTE — depends on the `CMD_FAULT_ACTUATOR` family being added to `command_id_t`.

---

# LAYER 2 — Mode FSM Unit Tests (dev-PC — G1)

> **TEST-FSM-001 — FLIGHT → DEMO with healthy system**
> *Purpose:* An operator-initiated FLIGHT→DEMO transition is accepted when healthy.
> *Linked requirement(s):* D020, `mode_fsm.h` contract.
> *Procedure:* Initial mode `SYS_MODE_FLIGHT`, healthy `health_flags_t`; inject `CMD_SET_MODE_DEMO`.
> *Acceptance criteria:* `mode_fsm_update()` returns `SYS_MODE_DEMO`; `mode_fsm_get()` agrees.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FSM-002 — DEMO → FLIGHT with healthy system**
> *Purpose:* Symmetric to TEST-FSM-001.
> *Linked requirement(s):* D020, `mode_fsm.h` contract.
> *Procedure:* Initial mode `SYS_MODE_DEMO`, healthy; inject `CMD_SET_MODE_FLIGHT`.
> *Acceptance criteria:* Returns `SYS_MODE_FLIGHT`; `mode_fsm_get()` agrees.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FSM-003 — Any state → SAFE_HOLD always accepted**
> *Purpose:* Entering safe hold is always allowed.
> *Linked requirement(s):* `mode_fsm.h` contract.
> *Procedure:* From each of FLIGHT, DEMO, SAFE_HOLD, healthy and unhealthy: inject `CMD_SET_MODE_SAFE_HOLD`.
> *Acceptance criteria:* Always transitions to `SYS_MODE_SAFE_HOLD`.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FSM-004 — FLIGHT rejected when estimator faulted**
> *Purpose:* An unsafe transition is rejected.
> *Linked requirement(s):* D049, `mode_fsm.h` contract.
> *Procedure:* Set `health` to indicate `EST_MODE_FAULT`; from SAFE_HOLD, inject `CMD_SET_MODE_FLIGHT`.
> *Acceptance criteria:* Mode remains SAFE_HOLD; the transition is rejected.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FSM-005 — SAFE_HOLD → FLIGHT requires healthy state**
> *Purpose:* Recovery from safe hold requires a healthy system.
> *Linked requirement(s):* D049, `mode_fsm.h` contract.
> *Procedure:* Unhealthy system, inject `CMD_SET_MODE_FLIGHT` (expect reject); restore healthy state, inject again (expect accept).
> *Acceptance criteria:* Rejected while unhealthy; accepted once healthy.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FSM-006 — NULL command: no state change, no crash**
> *Purpose:* A tick with no command is safe.
> *Linked requirement(s):* `mode_fsm.h` contract.
> *Procedure:* Call `mode_fsm_update()` with NULL for cmd.
> *Acceptance criteria:* Mode unchanged from the previous tick; no null-pointer crash.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-FSM-007 — `mode_fsm_get()` consistency with `mode_fsm_update()` return**
> *Purpose:* The query and the update agree.
> *Linked requirement(s):* `mode_fsm.h` contract.
> *Procedure:* Run 1,000 ticks with random command/health combinations; compare each `mode_fsm_update()` return against the immediately following `mode_fsm_get()`.
> *Acceptance criteria:* They are equal on every tick.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

---

# LAYER 3 — Full System Integration (G3)

> **TEST-INT-001 — Clean boot: no hardfault, no watchdog trip**
> *Purpose:* The full firmware boots cleanly.
> *Linked requirement(s):* D043.
> *Procedure:* Power on the MCU with full firmware; observe the GCS.
> *Acceptance criteria:* All 12 `_init()` calls complete; watchdog not tripped; no hardfault vector entered; telemetry frames arrive on the GCS within 2 seconds of power-on.
> *Apparatus:* Full hardware setup.
> *Execution log:* Not yet run.

> **TEST-INT-002 — First valid tick: all outputs populated**
> *Purpose:* The first tick produces meaningful output on every channel.
> *Linked requirement(s):* §4.2.
> *Procedure:* Inspect the first received telemetry frame.
> *Acceptance criteria:* Valid `imu_reading_t`, `baro_reading_t`, `attitude_estimate_t`, `actuator_cmd_t`, `health_flags_t` — all with meaningful, non-zero-default fields.
> *Apparatus:* Full hardware setup + GCS.
> *Execution log:* Not yet run.

> **TEST-INT-003 — WCET: main loop within tick budget**
> *Purpose:* The loop executes within `1 / AVIONICS_LOOP_RATE_HZ`.
> *Linked requirement(s):* REQ-SYS-011, D043.
> *Procedure:* Toggle a GPIO at loop entry and exit; scope worst-case execution time under deliberate stress (4 servos slewing, both IMUs active, radio TX/RX busy) for 60 seconds.
> *Acceptance criteria:* Worst-case execution time ≤ 80% of the tick period.
> *Apparatus:* STM32F407ZGT6, oscilloscope, deliberate stress load.
> *Execution log:* Not yet run.

> **TEST-INT-004 — Loop overrun policy: warning path and halt path (D047)**
> *Purpose:* The D047 loop overrun policy is implemented exactly — transient overrun warns, sustained overrun halts.
> *Linked requirement(s):* D047.
> *Procedure (warning path):* Artificially delay 1 then 2 consecutive ticks beyond the period; then inject an on-time tick.
> *Procedure (halt path):* Artificially delay 3 consecutive ticks beyond the period.
> *Acceptance criteria (warning path):* Each overrun increments the counter and emits a telemetry-frame warning; the system does NOT halt; an on-time tick resets the consecutive counter to zero (confirmed in telemetry).
> *Acceptance criteria (halt path):* On the 3rd consecutive overrun the MCU asserts halt and calls `actuators_safe()` (all fins to zero, confirmed by scope); no silent tick-skipping at any point.
> *Apparatus:* STM32F407ZGT6, modified firmware, wired UART, oscilloscope.
> *Execution log:* Not yet run.

> **TEST-INT-005 — Extended run: no stack overflow**
> *Purpose:* No memory corruption over a multi-hour run.
> *Linked requirement(s):* D019, TEST-BLD-003.
> *Procedure:* Run the full firmware for 4 hours continuously; inspect the stack canary / linker guard page at the end.
> *Acceptance criteria:* Stack canary intact; no memory corruption; telemetry frame content still valid.
> *Apparatus:* Full hardware setup, automated GCS log analysis.
> *Execution log:* Not yet run.

> **TEST-INT-006 — Live fault injection: IMU1 disconnect**
> *Purpose:* The system degrades gracefully mid-run.
> *Linked requirement(s):* D013, D024.
> *Procedure:* With the system running and stabilising, physically disconnect IMU1.
> *Acceptance criteria:* Within the LR-3 detection latency bound, `health.imu1_healthy = false` appears in telemetry; estimator transitions to `EST_MODE_IMU1_ONLY`; control law continues outputting valid commands; no crash; IMU2 unaffected.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-INT-007 — Live fault injection: IMU2 disconnect**
> *Purpose:* Symmetric to TEST-INT-006.
> *Linked requirement(s):* D013, D024.
> *Procedure:* Physically disconnect IMU2 mid-run.
> *Acceptance criteria:* `health.imu2_healthy = false`; estimator transitions to `EST_MODE_IMU2_ONLY`; control law continues; no crash; IMU1 unaffected.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-INT-008 — Live fault injection: both IMUs disconnected**
> *Purpose:* Total inertial sensing loss is handled.
> *Linked requirement(s):* D024, `estimator.h` contract.
> *Procedure:* Disconnect both IMUs (sequentially or simultaneously).
> *Acceptance criteria:* System enters `EST_MODE_DEAD_RECKONING`; covariance growth visible in telemetry; control law enters `CTL_MODE_SAFE_HOLD` (or reduced authority per implementation); no crash.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-INT-009 — Live fault injection: baro disconnect**
> *Purpose:* Barometer loss does not affect inertial estimation.
> *Linked requirement(s):* D026, D027.
> *Procedure:* Disconnect the baro mid-run.
> *Acceptance criteria:* `baro_healthy = false` in telemetry; IMU estimation continues unaffected; no bus-hang propagation.
> *Apparatus:* Full hardware setup.
> *Execution log:* Not yet run.

> **TEST-INT-010 — Operator-asserted actuator fault: control law reconfiguration**
> *Purpose:* The control law reconfigures on an operator-asserted actuator fault.
> *Linked requirement(s):* D046, D030.
> *Procedure:* With the system running, the operator sends `CMD_FAULT_ACTUATOR_0` from the GCS.
> *Acceptance criteria:* `actuator_healthy[0] = false` in telemetry; control law transitions to `CTL_MODE_3FIN_REDUCED`; fin 0 commanded to 0; remaining fins receive reconfigured mixing; GCS shows `CTL_MODE_3FIN_REDUCED`.
> *Acceptance note:* Per D046, servo faults are operator-asserted — there is no feedback path from servo to MCU, so the MCU cannot autonomously detect a servo fault. This test verifies the control law response to an asserted fault, not a detection mechanism. There is no physical disconnection step.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-INT-011 — Compound fault: IMU1 fault + operator-asserted actuator fault**
> *Purpose:* A genuine autonomously-detected sensor fault and an operator-asserted actuator fault handled together.
> *Linked requirement(s):* D024, D030, D046.
> *Procedure:* Physically disconnect IMU1 and have the operator send `CMD_FAULT_ACTUATOR_0` from the GCS at the same time.
> *Acceptance criteria:* IMU1 fault autonomously detected (`imu1_healthy = false`); actuator fault operator-asserted (`actuator_healthy[0] = false`); estimator enters `EST_MODE_IMU1_ONLY`; control law enters `CTL_MODE_3FIN_REDUCED`; system continues stabilising with degraded but bounded accuracy; GCS shows both fault flags; no crash.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-INT-012 — Radio disconnect: MCU continues operating**
> *Purpose:* D025 — a Pi-side failure cannot affect the MCU control loop.
> *Linked requirement(s):* D025.
> *Procedure:* Power off the ground radio module mid-run.
> *Acceptance criteria:* MCU continues executing the control loop; `radio_healthy = false` in subsequent radio-recovered frames; `actuators_write()` continues at the same rate; no safe-state trigger from radio loss alone.
> *Apparatus:* Full hardware setup.
> *Execution log:* Not yet run.

> **TEST-INT-013 — Wired fallback transport: same frames**
> *Purpose:* The wired transport carries the identical protocol.
> *Linked requirement(s):* D029.
> *Procedure:* Plug in the wired USB-serial link.
> *Acceptance criteria:* The same telemetry frames are received over the wired link as over radio; protocol identical; no reconfiguration needed.
> *Apparatus:* Full hardware setup, USB-serial adapter.
> *Execution log:* Not yet run.

> **TEST-INT-014 — Mode command end-to-end: FLIGHT → DEMO**
> *Purpose:* A mode command round-trips end-to-end with a visible hardware effect.
> *Linked requirement(s):* D020.
> *Procedure:* From the GCS, send `CMD_SET_MODE_DEMO`; observe telemetry and scope.
> *Acceptance criteria:* Within 2 telemetry frames `sys_mode = SYS_MODE_DEMO`; on scope the fin deflection limit is visibly larger for the same disturbance; GCS shows the acknowledged mode.
> *Apparatus:* Full hardware setup, GCS, oscilloscope.
> *Execution log:* Not yet run.

> **TEST-INT-015 — `CMD_RESET_ESTIMATOR` end-to-end**
> *Purpose:* The reset command works end-to-end on real hardware.
> *Linked requirement(s):* `c2.h` / `estimator.h` contracts.
> *Procedure:* Send `CMD_RESET_ESTIMATOR` from the GCS during converged operation.
> *Acceptance criteria:* The estimator resets (covariance jumps to its initial value, visible in telemetry) and re-converges within the expected time.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

---

# LAYER 3 — Timing & Time Authority (G3)

> **TEST-TIM-001 — TIM2 monotonicity across radio reconnect**
> *Purpose:* MCU timestamps survive a radio reconnect event.
> *Linked requirement(s):* REQ-SYS-008, D034.
> *Procedure:* Disconnect and reconnect the ground radio; inspect telemetry timestamps across the event.
> *Acceptance criteria:* MCU timestamps never regress or jump backward across the reconnect.
> *Apparatus:* Full hardware setup, GCS log.
> *Execution log:* Not yet run.

> **TEST-TIM-002 — TIM2 monotonicity across transport switch**
> *Purpose:* Timestamps survive a radio↔wired transport switch.
> *Linked requirement(s):* D029, D034.
> *Procedure:* Switch from radio to wired transport and back; inspect timestamps.
> *Acceptance criteria:* Timestamps continuous and monotonic across the switch.
> *Apparatus:* Full hardware setup, GCS log.
> *Execution log:* Not yet run.

> **TEST-TIM-003 — TIM2 monotonicity across Pi reboot**
> *Purpose:* MCU time is unaffected by a ground-station reboot.
> *Linked requirement(s):* D025, D034.
> *Procedure:* Reboot the Pi mid-run.
> *Acceptance criteria:* MCU timestamps continue uninterrupted and monotonic; the GCS re-connects and interprets the timestamp stream without an anchor reset.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-TIM-004 — IMU timestamp delta consistency**
> *Purpose:* IMU readings are evenly spaced in time.
> *Linked requirement(s):* D034.
> *Procedure:* Log `timestamp_us` from 1,000 consecutive IMU readings; compute deltas.
> *Acceptance criteria:* Consecutive delta = 1/ODR ± 5%; no outliers > 2× the expected interval except at known ISR preemption boundaries.
> *Apparatus:* Full hardware setup, UART log analysis.
> *Execution log:* Not yet run.

---

# LAYER 3 — Watchdog (hardware — G3)

> **TEST-WDG-001 — Watchdog fires: missed kick in main loop**
> *Purpose:* TEST-PLT-HW-003 confirmed in the full-firmware context.
> *Linked requirement(s):* `platform.h` watchdog discipline.
> *Procedure:* Remove the kick from tick [13] in the full firmware build.
> *Acceptance criteria:* MCU resets within the configured timeout (2.00–5.53 s band).
> *Apparatus:* STM32F407ZGT6, modified firmware.
> *Execution log:* Not yet run.

> **TEST-WDG-002 — Watchdog fires: hung init**
> *Purpose:* TEST-PLT-HW-004 confirmed in the full-firmware build.
> *Linked requirement(s):* `platform.h` watchdog discipline.
> *Procedure:* Insert an infinite loop anywhere inside a `_init()` (there is no per-`_init()` kick — D053 A3).
> *Acceptance criteria:* MCU resets within the configured timeout (2.00–5.53 s band).
> *Apparatus:* STM32F407ZGT6, modified firmware.
> *Execution log:* Not yet run.

> **TEST-WDG-003 — Safe state on watchdog reset**
> *Purpose:* After a watchdog (fault) reset, the fins reach the safe (zero-deflection) state before the first control-law output, and the reset cause is surfaced to the operator. The brief high-Z fin window during the reset itself is explicitly accepted (D053 A6).
> *Linked requirement(s):* `actuators.h` contract; D053 A4/A6.
> *Procedure:* Trigger a watchdog reset; scope all servo channels through the reboot; capture the GCS.
> *Acceptance criteria:* Fins reach zero-deflection safe before the control loop's first actuation output (tick [9]); `platform_reset_cause()` returns `RESET_WATCHDOG`, surfaced to the GCS ("reset occurred → fins to safe state") and logged. The high-Z interval during the reset transient is accepted, not a failure (D053 A6).
> *Apparatus:* STM32F407ZGT6, oscilloscope.
> *Execution log:* Not yet run.

---

# LAYER 4 — Structural (airframe)

> **TEST-STR-001 — Material coupon tensile test**
> *Purpose:* Characterise tensile strength across candidate filaments and print parameters; select a print strategy.
> *Linked requirement(s):* D031, D044.
> *Procedure:* Print tensile specimens for each candidate filament (PETG, ABS/ASA, CF-filled variant) at candidate wall thickness and infill; test each.
> *Acceptance criteria:* Selected print strategy has a documented tensile margin over expected bench loads.
> *Apparatus:* 3D printer, tensile test rig.
> *Execution log:* Not yet run.

> **TEST-STR-002 — Material coupon bending test**
> *Purpose:* Characterise bending behaviour; confirm the candidate material is not brittle at fin pivot mounts.
> *Linked requirement(s):* D031, D044.
> *Procedure:* Bending-load characterisation on the TEST-STR-001 specimens.
> *Acceptance criteria:* Candidate material is not brittle under the bending loads expected at fin pivot mounts.
> *Apparatus:* 3D printer, bending test rig.
> *Execution log:* Not yet run.

> **TEST-STR-003 — Print orientation comparison**
> *Purpose:* Quantify anisotropy to inform print orientation per airframe section.
> *Linked requirement(s):* D031.
> *Procedure:* Print the same specimen in vertical and horizontal orientation; test both.
> *Acceptance criteria:* Anisotropy factor documented; final print orientation decided for each airframe section.
> *Apparatus:* 3D printer, test rig.
> *Execution log:* Not yet run.

> **TEST-STR-004 — Full airframe static load test**
> *Purpose:* Validate the assembled airframe withstands the bench load envelope with margin.
> *Linked requirement(s):* D031; load envelope from airframe CAD dynamics analysis.
> *Procedure:* Apply the rated perturbation force (from airframe CAD dynamics analysis) + 2× safety factor at the airframe body (representative of hand-held handling loads) and at the fin attachment points.
> *Acceptance criteria:* Zero permanent deformation, zero cracking, zero joint loosening.
> *Apparatus:* Assembled airframe, calibrated force applicator, calipers.
> *Execution log:* Not yet run.

> **TEST-STR-005 — Demo cycle fatigue test**
> *Purpose:* The airframe survives representative demo/rehearsal usage without degradation.
> *Linked requirement(s):* D031.
> *Procedure:* Apply N perturbation cycles (N = expected demo count × 10 for margin; N defined before the test); inspect joints, skin, and fin attachments after.
> *Acceptance criteria:* Zero structural degradation.
> *Apparatus:* Assembled airframe, cyclic load rig.
> *Execution log:* Not yet run.

> **TEST-STR-006 — Dimensional verification (all critical dims)**
> *Purpose:* Every critical printed dimension matches the CAD model.
> *Linked requirement(s):* D031.
> *Procedure:* Measure every critical dimension with digital calipers post-print, against the CAD model. (Critical dimensions enumerated once CAD is complete.)
> *Acceptance criteria:* Every dimension within tolerance. Any out-of-tolerance dimension → re-print; no improvised shimming.
> *Apparatus:* Digital calipers, CAD model.
> *Execution log:* Not yet run.

> **TEST-STR-007 — Avionics bay fit check**
> *Purpose:* Every module fits the avionics bay without forcing.
> *Linked requirement(s):* D031, D038–D042.
> *Procedure:* Drop each module (STM32F407ZGT6 board, BMI160, ICM-42688-P, MS5611, SiK air module) into the avionics bay.
> *Acceptance criteria:* Every module seats without forcing; cable routing clearances confirmed; lid closes fully.
> *Apparatus:* Assembled avionics bay, all hardware modules.
> *Execution log:* Not yet run.

> **TEST-STR-008 — Fin alignment verification**
> *Purpose:* Fins are aligned so the control law does not fight a phantom disturbance.
> *Linked requirement(s):* D030.
> *Procedure:* Measure each fin's cant angle in the assembled article with a digital inclinometer.
> *Acceptance criteria:* All four fins within ±0.5° of geometric neutral cant angle.
> *Apparatus:* Assembled airframe, digital inclinometer.
> *Execution log:* Not yet run.

> **TEST-STR-009 — Servo horn geometry verification**
> *Purpose:* Commanded deflection matches actual fin deflection across the range.
> *Linked requirement(s):* D030; control law actuator model.
> *Procedure:* For each fin, command 5°, 10°, and maximum deflection; measure actual fin angle.
> *Acceptance criteria:* Commanded vs. measured matches the control law actuator model within ±1° at each point.
> *Apparatus:* Assembled airframe, digital inclinometer, commanding firmware.
> *Execution log:* Not yet run.

> **TEST-STR-010 — Mass properties verification**
> *Purpose:* Assembled mass and CG match the CAD model within a justified tolerance.
> *Linked requirement(s):* D031; CG threshold from airframe CAD dynamics analysis.
> *Procedure:* Weigh the assembled article (without servo rail power); measure CG location on a balance fixture; compare both against CAD.
> *Acceptance criteria:* Mass within ±5% of the CAD estimate. CG deviation acceptance threshold = **[TBD — pending airframe CAD and dynamics analysis]**. The threshold must NOT be an arbitrary number: the airframe CAD dynamics analysis must include a CG sensitivity analysis answering how much CG deviation degrades stability margins to their minimum acceptable values; that number becomes the threshold, with its derivation chain documented. The earlier 10 mm placeholder is withdrawn and must not be used until that analysis derives or replaces it.
> *Apparatus:* Assembled airframe, scale, CG balance fixture.
> *Execution log:* Not yet run. BLOCKED on threshold — requires CG sensitivity analysis from airframe CAD dynamics work.

> **TEST-STR-011 — Cable routing clearance**
> *Purpose:* No cable contacts a moving part across the full fin deflection range.
> *Linked requirement(s):* D031.
> *Procedure:* Manually command full deflection on all four fins simultaneously; visually inspect each cable segment.
> *Acceptance criteria:* No cable contacts a servo horn, linkage rod, or rotating part anywhere in the full deflection range; all cables secured.
> *Apparatus:* Assembled airframe, commanding firmware.
> *Execution log:* Not yet run.

---

# LAYER 4 — Ground Station (GCS) Verification (G3/G4)

> **TEST-GCS-001 — Protocol version rejection**
> *Purpose:* The GCS rejects a version-mismatched frame rather than displaying garbage.
> *Linked requirement(s):* §4.4, `AVIONICS_PROTOCOL_VERSION`.
> *Procedure:* Inject a frame with a wrong version over the wired link.
> *Acceptance criteria:* The GCS alerts the operator; it does not silently display garbage data.
> *Apparatus:* Pi GCS, wired UART, Python frame injector.
> *Execution log:* Not yet run.

> **TEST-GCS-002 — Live telemetry chart accuracy**
> *Purpose:* The dashboard displays attitude accurately.
> *Linked requirement(s):* Constraints §7 (ground station scope).
> *Procedure:* Place the article level, then at a known 45° tilt; read the dashboard.
> *Acceptance criteria:* Level shows roll=0, pitch=0 within ±1°; 45° tilt shows ~45°.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-GCS-003 — Sensor health status indicators**
> *Purpose:* Health indicators track fault state promptly.
> *Linked requirement(s):* Constraints §7.
> *Procedure:* Inject a fault (disconnect IMU1); watch the dashboard health indicator.
> *Acceptance criteria:* The corresponding indicator changes state within 2 displayed frames.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-GCS-004 — 3D attitude visualisation tracks estimator output**
> *Purpose:* The 3D body view correctly reflects estimator output.
> *Linked requirement(s):* Constraints §7.
> *Procedure:* Rotate the article by hand; observe the 3D visualization.
> *Acceptance criteria:* The 3D body moves in correspondence; lag ≤ 200 ms; rotation axes correct (not mirrored or transposed).
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-GCS-005 — Fin deflection view shows actual commanded angles**
> *Purpose:* The fin view reflects real commanded deflections.
> *Linked requirement(s):* Constraints §7.
> *Procedure:* Command known deflection angles via fault injection / debug command; read the GCS fin view.
> *Acceptance criteria:* The fin view shows the correct angles within display resolution.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-GCS-006 — Mode toggle: FLIGHT ↔ DEMO command round-trip**
> *Purpose:* The mode toggle works from the GCS UI with a visible hardware effect.
> *Linked requirement(s):* D020.
> *Procedure:* Press DEMO on the GCS; observe the acknowledgment frame and the scope.
> *Acceptance criteria:* Acknowledgment appears in the next frame; the fin deflection limit changes on the oscilloscope.
> *Apparatus:* Full hardware setup, GCS, oscilloscope.
> *Execution log:* Not yet run.

> **TEST-GCS-007 — Dropped frame detection and display**
> *Purpose:* The GCS detects and surfaces dropped frames.
> *Linked requirement(s):* `telemetry.h` contract (frame_id).
> *Procedure:* Modify firmware to drop 5 consecutive MCU TX frames.
> *Acceptance criteria:* The GCS detects the `frame_id` sequence gap and logs/displays the gap count.
> *Apparatus:* Full hardware setup, modified firmware.
> *Execution log:* Not yet run.

---

# G1 ADDITIONS — Code Coverage (dev-PC — G1)

> **TEST-COV-001 — Branch coverage measurement on algorithm modules**
> *Purpose:* No algorithm module ships to hardware bring-up with untested branches.
> *Linked requirement(s):* D045.
> *Procedure:* Run the full dev-PC test harness under `gcov` (GCC `--coverage`); generate a coverage report; archive it dated.
> *Acceptance criteria:* Branch coverage on `fdir.c`, `estimator.c`, `control_law.c`, `telemetry.c`, `c2.c`, `mode_fsm.c` each ≥ 90%.
> *Apparatus:* Dev PC, GCC with `--coverage`, gcov/lcov.
> *Execution log:* Not yet run.

> **TEST-COV-002 — Uncovered branch audit**
> *Purpose:* Every uncovered branch is justified or covered.
> *Linked requirement(s):* D045.
> *Procedure:* For every branch below the threshold, either prove it unreachable by construction (and document) or add a covering test.
> *Acceptance criteria:* No uncovered branch is silently accepted — each is either documented as unreachable or newly covered.
> *Apparatus:* Dev PC, gcov output, manual review.
> *Execution log:* Not yet run.

---

# G1 ADDITIONS — Boundary and Edge Case Coverage (dev-PC — G1)

> **TEST-BND-001 — FDIR: exact-boundary staleness (threshold ± 1 tick)**
> *Purpose:* Staleness fires at exactly the threshold — not one tick early or late.
> *Linked requirement(s):* `fdir.h` contract.
> *Procedure:* Inject timestamp = (now − IMU_STALENESS_THRESHOLD_US), then = (now − IMU_STALENESS_THRESHOLD_US − 1).
> *Acceptance criteria:* At the threshold, `imu1_healthy` still true; one tick past it, `imu1_healthy` false. Both boundaries tested explicitly.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-BND-002 — FDIR: chi-squared exactly at threshold**
> *Purpose:* The innovation gate fires consistently at the threshold boundary.
> *Linked requirement(s):* REQ-FDR-008.
> *Procedure:* Inject chi-squared = `CHI2_THRESHOLD_2DOF` exactly, then threshold − ε, then threshold + ε.
> *Acceptance criteria:* Gate behaviour at the boundary is defined and consistent with the implementation's chosen convention (strict `<` vs `≤`).
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-BND-003 — Estimator: convergence from extreme initial condition**
> *Purpose:* The estimator does not diverge or produce NaN from a worst-case initial state.
> *Linked requirement(s):* `estimator.h` contract.
> *Procedure:* Initialise the estimator at roll=179°, pitch=89°, yaw=359° (near gimbal lock); run 500 ticks with clean IMU data.
> *Acceptance criteria:* No NaN in any output; no assert; converges monotonically.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-BND-004 — Estimator: zero-rate input (gyro = 0 exactly)**
> *Purpose:* Identically-zero input causes no division-by-zero or NaN.
> *Linked requirement(s):* `estimator.h` contract.
> *Procedure:* Inject `gyro_rads = {0.0, 0.0, 0.0}`; run 100 ticks.
> *Acceptance criteria:* No NaN in any output field; attitude unchanged from initial.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-BND-005 — Control law: deflection command at exactly the limit**
> *Purpose:* The exact flight-limit boundary is handled consistently.
> *Linked requirement(s):* REQ-CTL-005.
> *Procedure:* Command exactly ±FLIGHT_LIMIT_RAD, then ±FLIGHT_LIMIT_RAD + ε; measure PWM.
> *Acceptance criteria:* At the limit, output is exactly the limit (not limit − ε); past the limit, output is clamped to the limit.
> *Apparatus:* STM32F407ZGT6, oscilloscope.
> *Execution log:* Not yet run.

> **TEST-BND-006 — Telemetry: frame_id rollover at UINT32_MAX**
> *Purpose:* `frame_id` wraps cleanly without a false GCS gap alarm.
> *Linked requirement(s):* `telemetry.h` contract.
> *Procedure:* Initialise `frame_id` to 0xFFFFFFFE; send 4 frames.
> *Acceptance criteria:* Sequence is ...FFFE, FFFF, 0000, 0001; the GCS handles the rollover with no false 4-billion-frame gap alarm.
> *Apparatus:* Dev-PC test harness + GCS.
> *Execution log:* Not yet run.

> **TEST-BND-007 — C2: maximum-length RX buffer fill**
> *Purpose:* No overflow when the RX ring buffer is exactly full.
> *Linked requirement(s):* `c2.h` / `sik_radio.h` contracts.
> *Procedure:* Fill the RX ring buffer to capacity with valid bytes, then inject one more byte.
> *Acceptance criteria:* The buffer drops the byte cleanly or wraps — no memory corruption; `c2_receive()` returns valid frames for the bytes that fit.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-BND-008 — Mode FSM: all invalid command_id_t values**
> *Purpose:* The FSM does not transition on an unrecognised command.
> *Linked requirement(s):* `mode_fsm.h` contract.
> *Procedure:* Inject command frames with `command_id` set to every value not defined in `command_id_t` (e.g. 0xFF).
> *Acceptance criteria:* FSM mode unchanged; no crash; no assert.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

> **TEST-BND-009 — UINT32_MAX timestamp in imu_reading_t**
> *Purpose:* Staleness arithmetic handles the maximum timestamp value without integer-overflow misbehaviour.
> *Linked requirement(s):* `fdir.h` contract, D034.
> *Procedure:* Inject `timestamp_us = UINT32_MAX`.
> *Acceptance criteria:* The staleness subtraction does not wrap to a spuriously small value; staleness is correctly detected.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

---

# G1 ADDITIONS — Exhaustive FSM Transition Coverage (dev-PC — G1)

> **TEST-FSM-EXH-001 — All reachable (state × input) pairs enumerated**
> *Purpose:* Every reachable FSM (state × input × sensing-health) combination has a defined, tested outcome.
> *Linked requirement(s):* D049, `mode_fsm.h` contract.
> *Procedure:* Build a coverage matrix of {SYS_MODE_FLIGHT, SYS_MODE_DEMO, SYS_MODE_SAFE_HOLD} × {CMD_SET_MODE_FLIGHT, CMD_SET_MODE_DEMO, CMD_SET_MODE_SAFE_HOLD, CMD_ACK_REQUEST, CMD_RESET_ESTIMATOR, NULL} × {healthy, imu1_fault, imu2_fault, both_imu_fault, est_fault} = 90 cells; test each.
> *Acceptance criteria:* Every cell has a pre-defined expected output and a recorded result. No cell blank.
> *Scope note (D049):* The health dimension contains sensing-health states only. Actuator-fault states are deliberately excluded — per D049 the FSM has no dependency on `actuator_healthy[i]`; actuator health is handled entirely by the control law's degradation chain. The matrix is complete at 90 cells. This exclusion is documented in the matrix header so a reviewer sees it as a deliberate decision, not an omission. If D049 is revisited, this matrix expands.
> *Apparatus:* Dev-PC test harness; coverage matrix document appended to the lab notebook.
> *Execution log:* Not yet run.

> **TEST-FSM-EXH-002 — No unintended state retention across resets**
> *Purpose:* After `estimator_reset()`, the FSM does not retain a mode that depended on now-reset estimator state.
> *Linked requirement(s):* `mode_fsm.h` / `estimator.h` contracts.
> *Procedure:* Enter SYS_MODE_FLIGHT with a healthy estimator; inject an estimator fault mid-run (forces safe hold); send `CMD_RESET_ESTIMATOR`.
> *Acceptance criteria:* After reset the system is in a clean state — it cannot re-enter FLIGHT until the estimator reconverges.
> *Apparatus:* Dev-PC test harness.
> *Execution log:* Not yet run.

---

# G2 ADDITIONS — Power Consumption Baseline (G2)

> **TEST-PWR-001 — Rail A idle current baseline**
> *Purpose:* The MCU + sensors draw the expected current on the clean rail.
> *Linked requirement(s):* D033.
> *Procedure:* Measure Rail A current with all sensors initialised and the main loop running, no servo activity; record the mean.
> *Acceptance criteria:* Mean current within ±20% of the sum of datasheet typical currents for STM32F407 + BMI160 + ICM-42688-P + MS5611 + SiK air module.
> *Apparatus:* Bench PSU with current measurement, full hardware setup.
> *Execution log:* Not yet run.

> **TEST-PWR-002 — Rail A current during peak sensor activity**
> *Purpose:* No unexpected current spike during simultaneous bus DMA activity.
> *Linked requirement(s):* D033.
> *Procedure:* Trigger simultaneous SPI1 + SPI2 + I2C transactions; measure Rail A current and voltage.
> *Acceptance criteria:* Peak current does not sag Rail A below 3.1 V.
> *Apparatus:* Bench PSU, oscilloscope (current probe or low-side shunt on Rail A).
> *Execution log:* Not yet run.

> **TEST-PWR-003 — Rail B current during max servo slew**
> *Purpose:* Measure peak servo-rail current to size the PSU.
> *Linked requirement(s):* D033.
> *Procedure:* Command all 4 servos to simultaneous hard slew; measure peak Rail B current.
> *Acceptance criteria:* Peak current documented; the chosen PSU/regulator is rated for ≥ 1.5× this peak.
> *Apparatus:* Bench PSU, current probe on Rail B.
> *Execution log:* Not yet run.

> **TEST-PWR-004 — Rail isolation under Rail B fault**
> *Purpose:* A Rail B fault does not collapse Rail A.
> *Linked requirement(s):* D033.
> *Procedure:* Force a Rail B overcurrent (short through a low-resistance load); measure Rail A voltage throughout.
> *Acceptance criteria:* Rail A stays within ±3% of nominal; the MCU continues running (confirmed by continued telemetry).
> *Apparatus:* Bench PSU, oscilloscope, deliberate Rail B load.
> *Execution log:* Not yet run.

---

# G2 ADDITIONS — Bus Contention Testing (G2)

> **TEST-BUS-001 — SPI1 + SPI2 simultaneous transaction**
> *Purpose:* Simultaneous DMA transfers on both SPI buses produce no data corruption.
> *Linked requirement(s):* D026.
> *Procedure:* Trigger BMI160 (SPI1 DMA) and ICM-42688-P (SPI2 DMA) reads simultaneously for 10,000 iterations, with device-ID reads interspersed.
> *Acceptance criteria:* Zero corrupted samples on either channel; interspersed device-ID reads always correct.
> *Apparatus:* STM32F407ZGT6, logic analyser on both SPI buses.
> *Execution log:* Not yet run.

> **TEST-BUS-002 — SPI + I2C simultaneous operation**
> *Purpose:* I2C barometer activity does not interfere with SPI IMU reads.
> *Linked requirement(s):* D026.
> *Procedure:* Run the main loop at full rate with the baro and both IMUs active for 60 seconds.
> *Acceptance criteria:* Zero `IMU_BUS_ERROR` or `BARO_BUS_ERROR` returns; no timestamp anomalies.
> *Apparatus:* STM32F407ZGT6, full sensor suite.
> *Execution log:* Not yet run.

> **TEST-BUS-003 — UART DMA TX + SPI DMA simultaneous**
> *Purpose:* Telemetry TX does not starve or corrupt SPI DMA transfers.
> *Linked requirement(s):* D028.
> *Procedure:* Run the full main loop with radio TX active at maximum frame rate; count IMU read errors over 1,000 ticks.
> *Acceptance criteria:* IMU read error rate = zero.
> *Apparatus:* STM32F407ZGT6, full hardware setup.
> *Execution log:* Not yet run.

> **TEST-BUS-004 — I2C bus recovery after NACK under load**
> *Purpose:* I2C recovers from a mid-transaction NACK while SPI buses are active.
> *Linked requirement(s):* D026.
> *Procedure:* Inject an I2C NACK during simultaneous SPI activity; then remove the fault.
> *Acceptance criteria:* `ms5611_service()` returns `BARO_BUS_ERROR`; IMU channels unaffected; I2C recovers within 1 second; no shared DMA-controller corruption.
> *Apparatus:* STM32F407ZGT6, jumper wire on I2C SDA, logic analyser.
> *Execution log:* Not yet run.

---

# G2 ADDITIONS — Per-Peripheral Failure Injection (G2)

These confirm the fault detection path fires correctly at the hardware level — not just in the dev-PC harness. Each must be run on real hardware before G2 closes.

> **TEST-PFLT-001 — BMI160 SPI disconnect: FDIR fires on hardware**
> *Purpose:* FDIR detects a real BMI160 fault on hardware.
> *Linked requirement(s):* D024, REQ-FDR-008.
> *Procedure:* With the full firmware running, physically disconnect BMI160 SPI MISO.
> *Acceptance criteria:* `imu1_healthy = false` appears in live telemetry within the detection latency bound; estimator transitions to `EST_MODE_IMU1_ONLY`; no crash.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-PFLT-002 — ICM-42688-P SPI disconnect: FDIR fires on hardware**
> *Purpose:* Symmetric to TEST-PFLT-001 for SPI2.
> *Linked requirement(s):* D024, REQ-FDR-008.
> *Procedure:* Physically disconnect ICM-42688-P SPI MISO.
> *Acceptance criteria:* `imu2_healthy = false` within the latency bound; estimator transitions to `EST_MODE_IMU2_ONLY`; no crash.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-PFLT-003 — MS5611 I2C disconnect: FDIR fires on hardware**
> *Purpose:* FDIR detects a real baro fault on hardware.
> *Linked requirement(s):* D026, D027.
> *Procedure:* Disconnect the baro I2C link.
> *Acceptance criteria:* `baro_healthy = false` in telemetry; IMUs unaffected.
> *Apparatus:* Full hardware setup.
> *Execution log:* Not yet run.

> **TEST-PFLT-004 — SiK radio disconnect: `radio_healthy` drops on hardware**
> *Purpose:* Radio loss is flagged; the control loop is unaffected.
> *Linked requirement(s):* D025, D028.
> *Procedure:* Power off the air module; monitor wired UART telemetry.
> *Acceptance criteria:* `radio_healthy = false` in the next radio-recovered frames; MCU control loop uninterrupted (wired telemetry continues at full rate).
> *Apparatus:* Full hardware setup, wired UART monitor.
> *Execution log:* Not yet run.

> **TEST-PFLT-005 — Operator-asserted actuator fault: system response**
> *Purpose:* The system responds correctly to an operator-asserted actuator fault on hardware.
> *Linked requirement(s):* D046, D030.
> *Procedure:* With the full firmware running, the operator sends `CMD_FAULT_ACTUATOR_0` from the GCS.
> *Acceptance criteria:* `actuator_healthy[0] = false` in telemetry; control law enters `CTL_MODE_3FIN_REDUCED`; no crash.
> *Acceptance note:* Autonomous servo fault detection is out of scope (D046) — no feedback path exists from servo to MCU. This test verifies correct system response to a manually asserted fault, not a detection mechanism. There is no physical disconnection step.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run. NOTE — depends on the `CMD_FAULT_ACTUATOR` family in `command_id_t`.

---

# G2 ADDITIONS — Calibration Application Verification (G2)

> **TEST-CAL-001 — Characterised sensor noise actually drives the configured FDIR thresholds**
> *Purpose:* The noise/bias values measured in TEST-BMI-003/004 and TEST-ICM-003/004 were actually used to set the firmware constants — not left at arbitrary hardcoded defaults. Without this, TEST-FDR-015 can pass on synthetic data while the firmware threshold was set from a different assumed noise level, and FDIR then fails on real hardware noise.
> *Linked requirement(s):* REQ-FDR-008; depends on LR-3 derivation formula.
> *Procedure:* Take the characterised noise values on file from BMI-003/ICM-003; apply the documented derivation formula (LR-3 for `CHI2_THRESHOLD_2DOF`; documented margin factors for bounds-check limits and `IMU_STALENESS_THRESHOLD_US`); compare against the constants compiled into the firmware build (read from the build artifact, not the source comment).
> *Acceptance criteria:* The formula's output equals the compiled-in constant for each threshold. The characterised values, the formula, and the resulting constant are recorded in the lab notebook as one linked chain. A mismatch is a finding.
> *Apparatus:* Dev PC, characterisation data on file, firmware build artifact, LR-3 derivation formula.
> *Execution log:* Not yet run. NOTE — depends on LR-3.

---

# G3 ADDITIONS — End-to-End Pipeline Data Integrity (G3)

> **TEST-PIP-001 — Synthetic input traced through every pipeline stage**
> *Purpose:* A known synthetic input produces a correct, verifiable output at every intermediate stage and at the final actuator command.
> *Linked requirement(s):* §4.2.
> *Procedure:* Apply a precisely known physical input (e.g. orient the test stand to a known attitude); capture at each stage — (1) raw `imu_reading_t`, (2) `fdir_gate_result_t`, (3) `attitude_estimate_t`, (4) `actuator_cmd_t`, (5) telemetry frame on the GCS.
> *Acceptance criteria:* Each intermediate value is consistent with the known input and the stage's documented contract. Any inconsistent intermediate value is a bug, not a calibration issue.
> *Apparatus:* Full hardware setup, wired UART tap for intermediate logging, GCS.
> *Execution log:* Not yet run.

> **TEST-PIP-002 — Pipeline latency: input to actuator command**
> *Purpose:* Total latency from sensor sample to actuator command is within the tick budget and documented.
> *Linked requirement(s):* REQ-SYS-011.
> *Procedure:* Toggle a GPIO when the IMU DMA transfer completes (step 1) and again when `actuators_write()` is called (step 9); measure the scope delta.
> *Acceptance criteria:* Delta < one tick period; exact measured latency documented (becomes the stated system latency in the architecture document).
> *Apparatus:* STM32F407ZGT6, oscilloscope (two GPIO probes).
> *Execution log:* Not yet run.

> **TEST-PIP-003 — Pipeline integrity under degraded mode**
> *Purpose:* In a degraded mode (`EST_MODE_IMU1_ONLY`), the pipeline still produces correct, verifiable outputs at every stage.
> *Linked requirement(s):* §4.2, §4.4.
> *Procedure:* Inject an IMU1 fault (per TEST-PFLT-001); with the system in `EST_MODE_IMU1_ONLY`, repeat the TEST-PIP-001 synthetic-input trace using only IMU2 data.
> *Acceptance criteria:* Each stage's output is consistent with single-IMU operation; degraded mode is visible at every stage (health flags propagate through the full pipeline).
> *Apparatus:* Full hardware setup.
> *Execution log:* Not yet run.

---

# G3 ADDITIONS — Fault Propagation Completeness (G3)

> **TEST-FPP-001 — Fault propagation trace: IMU1 fault → all downstream effects**
> *Purpose:* A single injected fault propagates through every layer that should respond, and not into layers that should not.
> *Linked requirement(s):* D024, §4.4.
> *Procedure:* Inject an IMU1 fault; confirm each link of the chain — (1) `bmi160_read()` returns `IMU_BUS_ERROR`, (2) `fdir_admit()` sets `imu1_healthy = false`, (3) `estimator_update()` receives NULL for imu1 and enters `EST_MODE_IMU1_ONLY`, (4) `control_law_update()` operates on the health flags, (5) `telemetry_pack_and_send()` includes `imu1_healthy = false`, (6) the GCS dashboard shows the IMU1 fault indicator.
> *Acceptance criteria:* Every link confirmed, not inferred.
> *Apparatus:* Full hardware setup, intermediate UART logging, GCS.
> *Execution log:* Not yet run.

> **TEST-FPP-002 — Fault propagation trace: operator-asserted actuator fault → control law reconfiguration**
> *Purpose:* An operator-asserted actuator fault propagates correctly. Per D046 the chain begins one step later than for sensor faults — there is no autonomous detection event.
> *Linked requirement(s):* D046, D030.
> *Procedure:* Confirm each link — (1) operator sends `CMD_FAULT_ACTUATOR_0` from the GCS, (2) `c2_receive()` parses it, (3) `actuator_healthy[0] = false` is set, (4) `control_law_update()` returns `CTL_MODE_3FIN_REDUCED` with the mixing matrix reconfigured, (5) `telemetry_pack_and_send()` includes the new control mode, (6) the GCS shows the mode change.
> *Acceptance criteria:* Every link confirmed, not inferred.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-FPP-003 — Fault does not propagate to unrelated subsystems**
> *Purpose:* A fault in one subsystem does not affect a subsystem with no dependency on it.
> *Linked requirement(s):* D026.
> *Procedure:* Inject an IMU1 fault; verify each unaffected subsystem explicitly.
> *Acceptance criteria:* Baro readings continue uninterrupted (`baro_healthy` unchanged); servo PWM for fins 1–3 continues; radio telemetry continues. Each verified, not assumed.
> *Apparatus:* Full hardware setup.
> *Execution log:* Not yet run.

---

# G3 ADDITIONS — Reboot and Recovery (G3)

> **TEST-RBT-001 — Power-cycle to verified known safe state**
> *Purpose:* After any power cycle, the system boots into a known safe state before any control authority is established.
> *Linked requirement(s):* `actuators.h` / `mode_fsm.h` / `estimator.h` contracts.
> *Procedure:* Power-cycle the system with servos in an arbitrary prior deflection position; inspect the first telemetry frame and scope the servo PWM through the reboot.
> *Acceptance criteria:* Before the main loop's first tick — (1) `actuators_safe()` has run (all fins at zero, scope-confirmed); (2) all health flags initialised healthy (no stale fault state from a prior session); (3) system mode initialised to `SYS_MODE_SAFE_HOLD`; (4) estimator initialised but not yet converged (covariance at its initial large value).
> *Apparatus:* STM32F407ZGT6, oscilloscope, GCS.
> *Execution log:* Not yet run.

> **TEST-RBT-002 — Watchdog-reset to verified known safe state**
> *Purpose:* A watchdog reset recovers to the same known safe state as a clean boot, and the MCU knows it was a watchdog reset.
> *Linked requirement(s):* `platform.h` watchdog discipline.
> *Procedure:* Trigger a watchdog reset; apply the TEST-RBT-001 checks; additionally read RCC_CSR.
> *Acceptance criteria:* All TEST-RBT-001 safe-state criteria hold; the IWDG reset flag in RCC_CSR is set and readable (the MCU distinguishes a watchdog reset from a power-on reset).
> *Apparatus:* STM32F407ZGT6, oscilloscope, modified firmware to force a WD reset.
> *Execution log:* Not yet run.

> **TEST-RBT-003 — Assert-halt to verified known safe state**
> *Purpose:* A firmware assertion drives the fins safe before halting.
> *Linked requirement(s):* D019 (JPL Rule 5), `actuators.h` contract.
> *Procedure:* Inject a condition that triggers the assert handler; scope the servo channels.
> *Acceptance criteria:* `actuators_safe()` is called before the halt; no servo motion after the safe state is established; the MCU does not continue executing the main loop in a corrupted state.
> *Apparatus:* STM32F407ZGT6, oscilloscope, modified firmware.
> *Execution log:* Not yet run.

---

# G4 ADDITIONS — Timing Margin Quantification (G4)

> **TEST-TMG-001 — Worst-case execution time measured and margin documented**
> *Purpose:* The system has explicit, measured timing margin — not assumed margin.
> *Linked requirement(s):* REQ-SYS-011, D043.
> *Procedure:* Run TEST-INT-003 (WCET under deliberate stress); record WCET_measured; compute margin_us = tick_period_us − WCET_measured_us; record in the lab notebook.
> *Acceptance criteria:* margin_us ≥ 20% of tick_period_us. If margin < 20%, the loop rate must be re-evaluated or tick steps optimised before G4 closes.
> *Apparatus:* STM32F407ZGT6, oscilloscope (GPIO toggle method from TEST-INT-003).
> *Execution log:* Not yet run.

> **TEST-TMG-002 — Timing margin under thermal stress**
> *Purpose:* WCET does not increase meaningfully as the avionics bay heats up.
> *Linked requirement(s):* REQ-SYS-011.
> *Procedure:* Run the TEST-TMG-001 WCET measurement after 2 hours of continuous operation with the avionics bay closed; compare against the cold-start WCET.
> *Acceptance criteria:* Delta < 5% of the tick period. A larger delta triggers investigation of thermal throttling or clock-stability effects.
> *Apparatus:* STM32F407ZGT6, oscilloscope, assembled avionics bay, 2-hour run.
> *Execution log:* Not yet run.

---

# G4 ADDITIONS — Watchdog Recovery State Verification (G4)

> **TEST-WDR-001 — Exact system state after watchdog reset: documented and confirmed**
> *Purpose:* There is no ambiguity about what state the system is in after a watchdog reset.
> *Linked requirement(s):* `platform.h` watchdog discipline; TEST-RBT-002.
> *Procedure:* Trigger a watchdog reset; capture the first telemetry frame after reboot; document every field — sys_mode, all health flags, estimator mode, all actuator commands, covariance values.
> *Acceptance criteria:* The captured frame matches the expected known-safe initial state exactly. Any deviation is a bug. The documented "watchdog recovery state signature" is recorded in the lab notebook as the reference for all future watchdog recovery tests.
> *Apparatus:* STM32F407ZGT6, GCS (wired), modified firmware.
> *Execution log:* Not yet run.

> **TEST-WDR-002 — Watchdog recovery state is identical to clean power-on state**
> *Purpose:* Watchdog recovery is indistinguishable from a clean boot — no residual pre-reset state leaks through reset.
> *Linked requirement(s):* TEST-RBT-001, TEST-WDR-001.
> *Procedure:* Diff the watchdog-recovery first-frame (TEST-WDR-001) against the clean power-on first-frame (TEST-RBT-001).
> *Acceptance criteria:* Every field identical. Any differing field indicates residual state leakage through reset (e.g. retained RAM) and must be diagnosed and fixed.
> *Apparatus:* STM32F407ZGT6, GCS (wired), diff of two captured telemetry frames.
> *Execution log:* Not yet run.

---

# G4 ADDITIONS — Long-Duration Soak Test (G4)

> **TEST-SOK-000 — Anomaly-detection script validation (gates SOK-001)**
> *Purpose:* The automated GCS log analysis script that SOK-001 depends on actually detects the failure categories it claims to. An unvalidated script that silently misses a fault class would let a 24-hour soak pass while proving nothing — this test is the calibration check for the test equipment.
> *Linked requirement(s):* D045.
> *Procedure:* Feed the script synthetic telemetry logs with known embedded faults — at minimum: (1) a NaN in an attitude field, (2) a stuck (non-incrementing) timestamp, (3) a spurious FDIR health-flag transition, (4) a frame-ID gap, (5) a stack-canary failure indicator. Also feed a clean log.
> *Acceptance criteria:* The script flags every injected fault type with zero misses, and reports zero anomalies on the clean log (no false positives). SOK-001 may not run until this test passes.
> *Apparatus:* Dev PC, anomaly-detection script, hand-crafted synthetic telemetry logs.
> *Execution log:* Not yet run.

> **TEST-SOK-001 — 24-hour continuous operation: no degradation**
> *Purpose:* The system does not degrade over extended operation.
> *Linked requirement(s):* D019; TEST-INT-005.
> *Precondition:* TEST-SOK-000 has passed — the anomaly-detection script is validated. SOK-001 may not run until then.
> *Procedure:* Run the full firmware with all sensors active and radio TX active for 24 continuous hours; analyse the log with the validated script.
> *Acceptance criteria:* At end — (1) stack canary intact; (2) telemetry content still valid (no NaN, no stuck values); (3) FDIR health flags stable (no spurious faults developing); (4) control law output still zero for a level article (estimator still converged); (5) the MCU has not reset.
> *Apparatus:* Full hardware setup, automated GCS log with the validated anomaly-detection script.
> *Execution log:* Not yet run.

> **TEST-SOK-002 — Timestamp rollover at UINT32_MAX**
> *Purpose:* `platform_timer_us()` wraps at UINT32_MAX without a system fault, without corrupting FDIR staleness logic, and without the GCS misinterpreting the timestamp stream.
> *Linked requirement(s):* D034.
> *Procedure:* Initialise the TIM2 counter to UINT32_MAX − 1,000 (via modified firmware preload); run through the rollover.
> *Acceptance criteria:* FDIR staleness arithmetic produces no spurious fault on rollover; the GCS handles the rollover with no false gap detection; the system continues operating normally.
> *Apparatus:* STM32F407ZGT6, modified firmware (TIM2 counter preload), GCS.
> *Execution log:* Not yet run.

> **TEST-SOK-003 — Thermal characterisation during soak**
> *Purpose:* Thermal behaviour is characterised and within the safe operating range.
> *Linked requirement(s):* REQ-SYS-011 (feeds TEST-TMG-002); FDIR threshold refinement.
> *Procedure:* During TEST-SOK-001, log MCU die temperature (STM32 internal sensor via ADC) and the MS5611 temperature channel (avionics-bay ambient) every 10 minutes; plot temperature vs. time.
> *Acceptance criteria:* (1) MCU die temperature stays below 85°C; (2) the avionics bay reaches thermal equilibrium within 2 hours and stabilises (no runaway); (3) IMU bias drift vs. temperature is characterised (gyro bias vs. bay temperature plotted — feeds FDIR threshold refinement).
> *Apparatus:* Full hardware setup with avionics bay closed, automated logging, Python analysis.
> *Execution log:* Not yet run.

---

# G4 ADDITIONS — Concurrent and Simultaneous Fault Injection (G4)

These go beyond the singular fault injection of G3. Each injects two or more independent faults at the same instant and verifies that every fault is detected and isolated independently, and that faults do not mask each other.

> **TEST-CFI-001 — Simultaneous IMU1 fault + IMU2 fault**
> *Purpose:* Two independent sensor faults injected simultaneously are both detected.
> *Linked requirement(s):* D024.
> *Procedure:* Physically disconnect both IMU SPI buses at the same instant.
> *Acceptance criteria:* Both `imu1_healthy = false` and `imu2_healthy = false` appear within the detection latency bound; system enters `EST_MODE_DEAD_RECKONING`; covariance grows; no crash; neither fault masks the other.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-CFI-002 — Simultaneous IMU1 fault + operator-asserted actuator fault**
> *Purpose:* An autonomously-detected sensor fault and an operator-asserted actuator fault occurring together are both handled independently.
> *Linked requirement(s):* D024, D046, D030.
> *Procedure:* Physically disconnect IMU1 SPI and have the operator send `CMD_FAULT_ACTUATOR_0` from the GCS at the same instant.
> *Acceptance criteria:* `imu1_healthy = false` (autonomously detected) AND `actuator_healthy[0] = false` (operator-asserted, D046) both appear in telemetry; estimator enters `EST_MODE_IMU1_ONLY`; control law enters `CTL_MODE_3FIN_REDUCED`; both fault indicators visible on the GCS simultaneously; system continues stabilising with degraded but bounded capability.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-CFI-003 — Simultaneous IMU1 + IMU2 + baro fault (triple sensor fault)**
> *Purpose:* Catastrophic sensor failure — all sensing channels lost simultaneously.
> *Linked requirement(s):* D024, D026.
> *Procedure:* Disconnect all three sensor buses at the same instant.
> *Acceptance criteria:* All three health flags false; system enters `EST_MODE_DEAD_RECKONING` (or `EST_MODE_FAULT` per implementation); control law enters `CTL_MODE_SAFE_HOLD`; all fins go to zero; no crash; telemetry continues; the GCS shows all sensor faults simultaneously.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

> **TEST-CFI-004 — Simultaneous IMU fault + radio fault**
> *Purpose:* A sensor fault and a communication fault together do not cause the sensor fault to be missed.
> *Linked requirement(s):* D025, D029.
> *Procedure:* Disconnect IMU1 and power off the ground radio simultaneously; monitor the wired UART; then reconnect the radio.
> *Acceptance criteria:* The MCU continues operating via wired UART; `imu1_healthy = false` appears in wired telemetry within the detection latency bound; after the radio reconnects, the next radio frame also shows `imu1_healthy = false` — the fault is not lost during the radio outage.
> *Apparatus:* Full hardware setup, wired UART monitor, GCS.
> *Execution log:* Not yet run.

> **TEST-CFI-005 — All four actuators faulted simultaneously (operator-asserted)**
> *Purpose:* Total actuator loss drives the system to `CTL_MODE_SAFE_HOLD` and zero deflection, not an undefined mixed state.
> *Linked requirement(s):* D030, D046.
> *Procedure:* The operator sends `CMD_FAULT_ACTUATOR_0` through `CMD_FAULT_ACTUATOR_3` from the GCS in the same command burst.
> *Acceptance criteria:* All four `actuator_healthy[i] = false`; control law enters `CTL_MODE_SAFE_HOLD`; all four fins commanded to 0 (scope-confirmed, zero-deflection pulse width on all channels); no crash.
> *Apparatus:* Full hardware setup, 4-channel oscilloscope, GCS.
> *Execution log:* Not yet run.

> **TEST-CFI-006 — Compound fault during mode transition**
> *Purpose:* A fault injected at the exact moment a mode command is processed does not corrupt FSM state.
> *Linked requirement(s):* `mode_fsm.h` contract.
> *Procedure:* Send `CMD_SET_MODE_DEMO` and simultaneously disconnect IMU1.
> *Acceptance criteria:* The mode transition either completes or is rejected — but the FSM is in a definite, consistent state afterward; `mode_fsm_get()` returns a valid `system_mode_t`; no stuck transition, no undefined mode value.
> *Apparatus:* Full hardware setup, GCS.
> *Execution log:* Not yet run.

---

## Pre-implementation blockers (tracked)

These external dependencies must be completed before the tests they block can run. Ordering matters — each is listed with the earliest gate it blocks:

- **LR-3 (FDIR innovation gating)** — produces the `CHI2_THRESHOLD_2DOF` derivation formula and detection-latency bound. Blocks TEST-CAL-001, TEST-FDR-007/010/015. Must complete before G1.
- ~~**D048 (NVIC interrupt priority scheme)** — `platform.h` scrutiny-session deliverable. Blocks TEST-PLT-005 (G1) and TEST-PLT-HW-007 (G2).~~ **RESOLVED 2026-06-17** — D048 logged; TEST-PLT-005 and TEST-PLT-HW-007 unblocked.
- **`CMD_FAULT_ACTUATOR` family in `command_id_t`** — blocks TEST-C2-008 / PFLT-005 / INT-010 / FPP-002 / CFI-002 / CFI-005. Must land before G2 closes.
- **`overrun_count` field in `telemetry_frame_t`** — blocks TEST-TEL-008 and the frame-verified TEST-INT-004 warning path. Bundle with the D047 implementation work.
