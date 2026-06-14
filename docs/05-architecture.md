# Architecture

**Status:** Hardware and structural architecture established (D024–D034). Software architecture locked (D043, 2026-05-20). Ground station section pending.
**Created:** 2026-05-11
**Last updated:** 2026-05-20

This document captures the "how" of the system. Component breakdown, data flow, hardware block diagram, software module structure, interface definitions between subsystems.

The architecture is designed to support fault tolerance from day one (see Constraints §10.2). Interfaces between estimator, controller, sensor abstraction, and health monitor are defined such that fault-tolerance logic can be added incrementally without restructuring.

---

## 1. System Block Diagram

**Deferred.** Formal system block diagram to be produced using proper diagramming methodology appropriate for the project's documentation standard. The nodes, interfaces, and architectural boundaries described in §2 are the authoritative system concept; the diagram visualizes them, it does not define them.

---

## 2. Hardware Architecture

### 2.1 System concept

Three operational nodes and one offline node:

**Test stand (bench).** The custom semi-monocoque airframe (per §3), housing the avionics bay. Hand-held or bench-rested during demo operation. The airframe is a deliberate structural engineering deliverable — not packaging.

**Avionics bay (inside airframe).** MCU, dual heterogeneous IMUs, barometer, radio module, four servos driving four independently-actuated fins. The MCU owns the entire real-time path: sensor read, fusion, FDIR, control law, actuator command, mode FSM, and time authority. Nothing running on Linux is in the control loop (per D025).

**Ground station.** Pi 5 (DietPi) with display and radio module. Bounded role (per D032): telemetry reception, logging, dashboard UI, C2 command transmission. Not in the real-time loop. Loss of the Pi does not affect MCU operation.

**Dev PC (offline).** Ryzen 7 9700X, RTX 4080 Super, 64 GB DDR5. Firmware development environment, post-test analysis host. Not part of the deployed system. Connected to the Pi via SSH; connected to the MCU via wired USB-serial.

### 2.2 Sensor stack

**Dual heterogeneous IMU on isolated SPI buses.** Two IMUs from different chip families (different manufacturer / different sensor family), each on its own SPI chip-select line on a shared SPI bus. SPI's per-device chip-select gives hardware-enforced fault isolation: one IMU pulling its pins into bad states cannot reach the other IMU. Heterogeneous chips prevent common-mode silicon failures from defeating the cross-check.

- IMU-1 (chip A): SPI, CS1.
- IMU-2 (chip B, different family from A): SPI, CS2.

Specific IMU part selection is deferred to the component-selection phase, bounded by Constraints §11.2.

**Barometer on dedicated I2C.** A single barometric pressure sensor alone on its own I2C peripheral. Pressure is a dissimilar measurement axis (pressure vs. inertial; membrane deflection vs. mass-on-spring), giving FDIR a third sensor type immune to the failure modes that fool inertial sensors. Enables the bench cross-modal demo (tap the stand → IMUs see acceleration, baro sees no altitude change).

- Barometer: I2C, sole device on its bus.

A bus-level hang on the barometer's I2C only loses the barometer, which is an acceptable single-channel loss — the IMU cross-check remains intact.

### 2.3 Compute roles

**MCU (TBD class — STM32F4 or above).** Owns the entire real-time path. Responsibilities:

- Sensor read (dual IMU via SPI, barometer via I2C).
- State estimation (sensor fusion of dual-IMU + baro).
- FDIR (innovation gating between IMUs, dissimilar-sensor cross-check with baro, per-sensor health channels, mixing-matrix reconfiguration on actuator faults).
- Control law computation.
- Actuator command (four PWM channels to servos).
- Mode FSM (demo/flight mode, nominal/degraded/safe states).
- Time authority (hardware monotonic timer, microsecond resolution — per D034).
- Telemetry framing and transmission (over radio or wired transport).
- Command reception, validation, and acknowledgment.

All MCU code is JPL Power of 10 compliant per D019.

**Pi 5 (DietPi).** Ground station only. Responsibilities (per D032):

- Telemetry reception over the active transport (radio or wired).
- Logging of all received frames to local storage.
- Dashboard UI rendering (multi-pane: live telemetry, sensor health, 3D attitude visualization, fin deflection view — per Constraints §7).
- C2 command transmission (mode toggle and any future C2 expansion).

The Pi runs no estimation, FDIR, or control-path logic. It uses MCU-provided timestamps for all data operations.

**Dev PC.** Offline node. Firmware development and flashing; post-test analysis tooling.

### 2.4 Actuator stack

**Four servo-actuated aerodynamic fins, "+" configuration** (per D030). Fins at 90° spacing, aligned with the principal body axes. Each fin is independently actuated by its own servo on its own PWM channel from the MCU. Independent actuation enables actuator-loss fault tolerance: losing one fin allows the control allocator to redistribute commanded moments across the remaining three with degraded but bounded authority.

- 4× hobby-grade metal-gear servos (metal gears non-negotiable — the actuator-jam demo deliberately stalls a servo; plastic gears strip on stall).
- 4× PWM channels from the MCU.

Specific servo selection deferred to component-selection phase. Position-feedback variants preferred where cheaply available, for direct detection of stuck/jammed actuators.

### 2.5 Power architecture

**Split rails, common ground** (per D033). The clean rail's voltage is invariant to servo activity, by construction.

- **Rail A (clean):** Regulated low-noise 3.3V / 5V supplying MCU, IMU-1, IMU-2, barometer, and radio module. Low current (well under 500 mA continuous). LDO or clean switching regulator with downstream LDO; bulk decoupling at the regulator and local decoupling at each IC.
- **Rail B (servo):** 5–6V high-current supplying the four servos. Sized for 6–8 A peak (worst-case all-four-servos slewing simultaneously). Stiff switching regulator.
- **Common ground plane.** Both rails share ground; only the positive supplies are separated.

Power-on sequencing: Rail A brought up first (MCU initializes and asserts commanded servo positions), then Rail B (avoids servo twitching to random positions before being commanded). Simple sequencer in the regulator wiring or via a small RC delay on Rail B's enable line.

Optional power monitoring: ADC channels on the MCU sample both rail voltages, giving the FDIR module visibility into power state ("is the clean rail at nominal, or sagging?"). Cheap and aligned with the project's depth axis.

### 2.6 Telemetry and comms

**Radio link (primary operational link).** Matched radio module pair (specific module TBD — LoRa-class). MCU-side module connected via UART with DMA-driven ring buffer (per D028). Pi-side module connected via UART/USB. Bidirectional: downlink (telemetry frames), uplink (C2 commands). JPL Power of 10 discipline ensures the radio driver cannot stall the control loop — bounded buffers, bounded handlers, no dynamic allocation.

**Wired link (dev/debug/backup channel).** USB-serial between MCU and Pi (per D029). Used for firmware flashing, high-bandwidth log dumps, pre-demo subsystem checks, and as a fallback if the radio fails during a demo.

**Same protocol on both transports.** A transport abstraction layer in MCU firmware sits above one driver per transport (radio UART, wired UART/USB). Above the transport layer sits the framing layer (packet boundaries, CRC). Above that sits the application layer (telemetry frame generation, command dispatch). The application layer is transport-agnostic — same packet format, same CRC, same command structure regardless of which transport delivered the bytes. The Pi-side ground station software has a symmetric transport abstraction.

No dedicated comms coprocessor (per D028). At LoRa data rates and JPL discipline, the radio driver coexists safely with the control loop on the MCU.

### 2.7 Time architecture

**MCU hardware monotonic timer is the sole time authority** (per D034). A dedicated MCU timer runs at microsecond resolution from power-up. Every sensor reading is paired with the system timestamp at the moment of read completion. Every telemetry frame carries an MCU timestamp in its header. The Pi inherits MCU time for all logging, replay, and analysis math. Pi system time is used only for human-readable session naming and as observational metadata for transport-layer diagnostics — never for any math involving event ordering, intervals, or correlation.

Timer width: a 64-bit virtual counter (32-bit hardware timer plus software-tracked overflow) avoids wraparound entirely; standard pattern.

Session wall-clock anchor: at session start, the Pi records its own clock alongside the current MCU time ("this session started at 2026-MM-DD HH:MM:SS Pi time; MCU time at session start was T"). This mapping is stored once and used only for human display — analysis math always operates in MCU time.

### 2.8 Interface summary

| Interface | Protocol | Direction | Purpose |
|-----------|----------|-----------|---------|
| MCU ↔ IMU-1 | SPI (CS1) | Bidirectional | Inertial data, isolated bus |
| MCU ↔ IMU-2 | SPI (CS2) | Bidirectional | Inertial data, isolated bus |
| MCU ↔ Baro | I2C | Bidirectional | Pressure/altitude data |
| MCU → Servos | PWM × 4 | MCU to servos | Control surface actuation |
| MCU ↔ Radio (MCU-side) | UART + DMA | Bidirectional | Telemetry TX, command RX |
| Radio ↔ Radio | RF | Bidirectional | Wireless telemetry + C2 link |
| Pi ↔ Radio (Pi-side) | UART/USB | Bidirectional | Telemetry RX, command TX |
| MCU ↔ Pi (wired) | USB-Serial | Bidirectional | Dev/debug, firmware flash, backup |
| Pi → Display | HDMI/DSI | Pi to display | Dashboard rendering |
| Dev PC ↔ Pi | SSH over network | Bidirectional | Remote development and analysis |
| Dev PC ↔ MCU | USB-Serial | Bidirectional | Firmware flash, debug |

### 2.9 Key architectural boundaries

1. **Real-time boundary.** Everything inside the MCU is deterministic and JPL-Power-of-10-compliant. Everything outside (Pi, dev PC) is best-effort and non-safety-critical. No exceptions: no code running under Linux is ever in the control loop.
2. **Fault-isolation boundary.** Each IMU on its own SPI bus. A fault on one cannot propagate to the other via shared bus. Barometer alone on its own I2C bus — dissimilar sensor, dissimilar bus. Single-fault sensor isolation is hardware-enforced.
3. **Power-isolation boundary.** MCU/sensor rail is clean. Servo rail is separate. A servo stall cannot brown out the MCU or inject noise into sensor ADCs.
4. **Time-authority boundary.** MCU hardware timer is the sole source of truth for timestamps. Pi inherits MCU time. Pi time is used only for human-readable artifacts and observational metadata.
5. **Command-authority boundary.** Pi can command mode transitions (demo/flight per D020). MCU validates and acknowledges. MCU can reject invalid commands. Loss of Pi heartbeat is a detectable condition; the MCU continues operating in the absence of the Pi.

---

## 3. Airframe Structural Architecture

**Status:** Skeleton — populated as CAD work progresses (per D031).

The airframe is a custom-designed, 3D-printed semi-monocoque structure, treated as a parallel structural engineering deliverable alongside the avionics chapter. It is not packaging; it is engineering on its own merit, with CAD design, structural analysis, materials selection, fabrication, and test as deliverables.

**Explicit scope boundary (per D031):** the structural reinforcement strategy is FDM-print-only — filament selection, wall thickness, infill density and pattern, print orientation, and printed-in ribs/stringers. No fibrous post-print reinforcement, no resin layup, no epoxy work.

### 3.1 Design philosophy

Semi-monocoque construction adapted to FDM-printed fabrication. The printed outer skin carries primary loads, supported by printed-in internal frames and longitudinal stringers integrated into the same print model. Load envelope is bench-only — self-weight, perturbation forces during demos (tens of newtons peak), servo reaction forces on fin pivots, and handling loads from drops off the bench. No transonic flight loads, no aerodynamic stability requirements.

### 3.2 Skin / frame / stringer layout

*To be populated during airframe CAD work.*

### 3.3 Segmentation strategy

*To be populated during airframe CAD work. The airframe will be printed in multiple segments, joined mechanically; segmentation choice trades off print volume against joint count and joint structural integrity.*

### 3.4 Avionics bay integration

*To be populated during airframe CAD work. Avionics bay is purpose-designed for the avionics stack — MCU mount, sensor positioning, radio antenna placement, cable routing, access for in-place flashing and debugging via the wired link.*

### 3.5 Fin pivot integration

*To be populated during airframe CAD work. Four fin pivots integrated into the airframe structure; servo mounting brackets printed into the body adjacent to the fin hinge points; servo linkages designed in.*

### 3.6 Material selection and print strategy

*To be populated. Trade space: filament family (PETG / ABS / ASA / CF-filled PLA or PETG); wall thickness and perimeter count; infill density and pattern; print orientation relative to expected load axes; printed-in reinforcement features (ribs, fillets, gussets). Material choice driven by coupon testing per §3.9.* **No fibrous layup or epoxy work.**

### 3.7 Fabrication process

*To be populated. Workflow from CAD model → slicer settings → print → post-processing (support removal, dimensional verification) → assembly. Iteration discipline (design freeze after one or two revisions) is a guardrail flagged in the risk register.*

### 3.8 Structural analysis and validation

*To be populated. Hand calculations on the dominant load paths at minimum; FEA optional (Fusion 360's built-in FEA module is a candidate tool). Validation via coupon testing in candidate materials/orientations/infill patterns before committing to a full airframe print; full-airframe load testing post-fabrication.*

---

## 4. Software Architecture

**Status:** Locked — D043. Scaffold produced 2026-05-20.

### 4.1 Software module decomposition

The avionics firmware is decomposed into 12 C modules plus one shared type header. Module boundaries are enforced by the header interface — internal implementation is file-private (static). No module reaches into another module's internal state. Directory structure mirrors the layer hierarchy: a file's folder determines its layer, not its name.

Layers (bottom to top):

| Module | Files | Layer | Responsibility |
|---|---|---|---|
| Shared types | `avionics_types.h` | — | All shared structs and enums. Depends only on stdint.h / stdbool.h |
| Platform | `hardware/platform.c/h` | Hardware | MCU clock config, peripheral init, IWDG watchdog, TIM2 |
| IMU-1 driver | `hardware/bmi160.c/h` | Hardware | BMI160 SPI read, raw→SI scaling, TIM2 timestamp |
| IMU-2 driver | `hardware/icm42688p.c/h` | Hardware | ICM-42688-P SPI read, raw→SI scaling, TIM2 timestamp |
| Barometer | `hardware/ms5611.c/h` | Hardware | MS5611 I2C read, pressure compensation, altitude derivation |
| Radio driver | `hardware/sik_radio.c/h` | Hardware | UART+DMA ring buffer, raw byte RX/TX — no protocol logic |
| ISR handlers | `hardware/stm32f4xx_it.c` | Hardware | HAL ISR callbacks: SPI DMA TC, I2C DMA TC, UART DMA RX, TIM2 |
| MSP init | `hardware/stm32f4xx_hal_msp.c` | Hardware | HAL peripheral clock, GPIO, DMA configuration callbacks |
| FDIR | `algorithm/fdir.c/h` | Algorithm | Two-pass: admission (staleness, bounds, gyro-vs-gyro) + innovation gate; sole writer of health flags |
| Estimator | `algorithm/estimator.c/h` | Algorithm | Two-phase EKF: predict (exports `predicted_readings_t`) + correct; degraded mode management; covariance |
| Control law | `algorithm/control_law.c/h` | Algorithm | Attitude error → deflection commands, reconfigurable mixing matrix |
| Actuators | `output/actuators.c/h` | Output | Deflection → PWM registers, hard clamp, safe-state command |
| Telemetry | `output/telemetry.c/h` | Output (TX) | Downlink only: frame packing, CRC-16, dispatch to radio driver |
| C2 | `output/c2.c/h` | Output (RX) | Uplink only: drain RX ring buffer, CRC validate, parse command frames |
| Mode FSM | `orchestration/mode_fsm.c/h` | Orchestration | System mode (flight/demo/safe), C2 command dispatch, mode ack |

### 4.2 Scheduling model

Bare-metal superloop (D043). TIM2 hardware timer fires a tick flag at the control loop rate (rate TBD pending LR-1; nominal 500–1000 Hz). The main loop waits on the tick flag, then executes the full pipeline in deterministic sequence every tick.

**Main loop tick sequence:**

```
[1]  bmi160_read()              — consume IMU-1 DMA buffer, attach TIM2 timestamp
[2]  icm42688p_read()           — consume IMU-2 DMA buffer, attach TIM2 timestamp
[3]  ms5611_service()           — barometer: poll every N ticks (~50 Hz, decimated internally)
[4]  fdir_admit()              — absolute checks (staleness, bounds, gyro-vs-gyro) → preliminary health_flags
[5]  estimator_predict()       — propagate on admitted gyro → predicted_readings_t (NULL for an admitted-out gyro)
[6]  fdir_gate()               — innovation gate vs predicted measurements → final health_flags (restrict-only)
[7]  estimator_update()        — EKF correction on healthy channels; NULL pointers for isolated sensors
[8]  control_law_update()       — attitude error → fin deflection commands
[9]  actuators_write()          — deflection → PWM registers (hard-clamped to mode limits)
[10] telemetry_pack_and_send()  — pack downlink frame, CRC-16, queue to SiK DMA TX
[11] c2_receive()               — drain RX ring buffer, CRC validate, parse → command_frame_t
[12] mode_fsm_update()          — consume parsed C2 command, update system mode, queue ack
[13] platform_watchdog_kick()   — MCU IWDG hardware watchdog
[14] tick_wait()                — spin until TIM2 sets tick flag
```

**ISR responsibilities — flag-set only, never more than ~10 lines:**

| ISR | Action | Forbidden |
|---|---|---|
| SPI1 DMA TC (IMU-1) | Set `imu1_data_ready`, swap double-buffer pointer | No math, no application calls |
| SPI2 DMA TC (IMU-2) | Set `imu2_data_ready`, swap double-buffer pointer | No math, no application calls |
| I2C DMA TC (baro) | Set `baro_data_ready` | No math, no application calls |
| UART1 DMA RX (radio) | Advance RX ring buffer write pointer | No math, no application calls |
| TIM2 update | Set `tick_flag` | No math, no application calls |

**`volatile` discipline:** every variable shared between an ISR and the main loop is declared `volatile_flag_t` (see §4.3). Without `volatile`, the compiler may cache the value in a register and the main loop will never observe the ISR's write. All ISR-shared variables are enumerated in `isr_flags.h` — that file is the authoritative manifest of every ISR/main-loop boundary crossing.

**Overrun detection:** TIM2 is sampled at loop entry. If elapsed since the previous tick exceeds the loop period, an assert fires and the system halts. Overrun is never silent.

**Watchdog during init:** IWDG is started inside `platform_init()` before any peripheral init begins, with a timeout of ~2–4 s covering the full init sequence. Each module's `_init()` calls `platform_watchdog_kick()` on completion — a hung peripheral init resets the MCU rather than hanging forever. In steady-state the main loop kicks the watchdog every ~1 ms, well within the timeout.

### 4.3 Canonical data types

All types shared between modules are defined in `avionics_types.h`. No module defines public types outside this file. Depends only on `<stdint.h>` and `<stdbool.h>`.

**ISR-shared flag type:**
- `volatile_flag_t` — `typedef volatile uint8_t volatile_flag_t`. Any variable of this type crosses the ISR/main-loop boundary. Self-documenting by type name.

**Protocol version:**
- `AVIONICS_PROTOCOL_VERSION` — `uint8_t` constant, increment on any frame layout change. First field in both `telemetry_frame_t` and `command_frame_t` so the GCS detects mismatches at runtime before bad data propagates.

**Status enums:**
- `imu_status_t`: `IMU_OK`, `IMU_BUS_ERROR`, `IMU_TIMEOUT`, `IMU_STALE_DATA`, `IMU_OUT_OF_RANGE`, `IMU_ISOLATED`
- `baro_status_t`: `BARO_OK`, `BARO_BUS_ERROR`, `BARO_TIMEOUT`, `BARO_STALE_DATA`, `BARO_OUT_OF_RANGE`, `BARO_ISOLATED`
- `estimator_mode_t`: `EST_MODE_DUAL_IMU`, `EST_MODE_IMU1_ONLY`, `EST_MODE_IMU2_ONLY`, `EST_MODE_DEAD_RECKONING`, `EST_MODE_FAULT`
- `control_mode_t`: `CTL_MODE_FULL_AUTHORITY`, `CTL_MODE_3FIN_REDUCED`, `CTL_MODE_2FIN_REDUCED`, `CTL_MODE_SAFE_HOLD`
- `system_mode_t`: `SYS_MODE_FLIGHT`, `SYS_MODE_DEMO`, `SYS_MODE_SAFE_HOLD`
- `command_id_t`: `CMD_SET_MODE_FLIGHT`, `CMD_SET_MODE_DEMO`, `CMD_SET_MODE_SAFE_HOLD`, `CMD_ACK_REQUEST`, `CMD_RESET_ESTIMATOR`

**Data structs:**
- `imu_reading_t`: `accel_mss[3]`, `gyro_rads[3]`, `timestamp_us`, `status`
- `baro_reading_t`: `pressure_pa`, `temperature_c`, `altitude_m`, `timestamp_us`, `status`
- `fdir_gate_result_t`: `chi2_imu1`, `chi2_imu2`, `imu1_gate_open`, `imu2_gate_open`, `imu1_stale_us`, `imu2_stale_us`
- `health_flags_t`: `imu1_healthy`, `imu2_healthy`, `baro_healthy`, `actuator_healthy[4]`, `radio_healthy`, `wired_healthy`
- `attitude_estimate_t`: `roll_rad`, `pitch_rad`, `yaw_rad`, `roll_rate_rads`, `pitch_rate_rads`, `yaw_rate_rads`, `covariance[6]`, `mode`, `timestamp_us`
- `actuator_cmd_t`: `deflection_rad[4]`, `timestamp_us`, `mode`
- `telemetry_frame_t`: `protocol_version`, `frame_id`, `timestamp_us`, `imu1`, `imu2`, `baro`, `estimate`, `actuators`, `health`, `sys_mode`, `crc16`
- `command_frame_t`: `protocol_version`, `cmd_seq`, `command`, `timestamp_us`, `crc16`

**TBD constants** (pending literature reviews):
- `AVIONICS_LOOP_RATE_HZ` — TBD, LR-1
- `IMU_STALENESS_THRESHOLD_US` — TBD, LR-1 (nominal: 5 × sample interval)
- `CHI2_THRESHOLD_2DOF` — TBD, LR-3
- `AVIONICS_BARO_RATE_HZ` — fixed at 50

### 4.4 Fault propagation chain

**Guarantee: no fault is swallowed, no fault produces undefined behavior.**

```
Sensor driver
  └─ imu_reading_t.status       ← bus errors, timeouts, range violations
  └─ imu_reading_t.timestamp_us ← staleness source for FDIR watchdog
        ↓
FDIR module
  └─ health_flags_t             ← authoritative per-channel health
  └─ fdir_gate_result_t         ← chi² values, gate open/closed, stale durations
        ↓
Estimator
  └─ attitude_estimate_t.mode   ← DUAL_IMU → IMU1_ONLY → IMU2_ONLY → DEAD_RECKONING
  └─ attitude_estimate_t.covariance ← grows as sensors are lost; confidence made numeric
        ↓
Control law
  └─ actuator_cmd_t.mode        ← FULL_AUTHORITY → 3FIN_REDUCED → 2FIN_REDUCED → SAFE_HOLD
        ↓
Telemetry (every downlink frame)
  └─ telemetry_frame_t carries health_flags, estimate.mode, actuators.mode, all sensor readings
        ↓
Ground station dashboard
  └─ sensor health panel, estimator mode indicator, covariance display, fin deflection view
```

**Invariants:**
1. FDIR sets health flags. Estimator reads them. Estimator never sets health flags. No module crosses this boundary.
2. An isolated sensor is passed as NULL to the estimator — not as zeroed data. NULL is unambiguous; zeroed data is not.
3. `attitude_estimate_t.covariance` is mandatory in every output — a consumer that ignores it ignores the system's self-reported confidence. Absence of covariance propagation is a reviewer finding.
4. `telemetry_frame_t` carries `health_flags_t` unconditionally — health state is never omitted from a downlink frame.
5. FDIR writes `health_flags_t` in two passes: `fdir_admit` sets the preliminary verdict (pre-predict); `fdir_gate` may only restrict it (post-predict) and never resurrects a channel admission isolated. Both passes live in `fdir.c`; no other module writes health. (D050)

### 4.5 Module interface contracts

**platform:** `platform_init()` — starts IWDG first, then configures clocks and peripherals. `platform_timer_us()` — reads TIM2, the authoritative time source (D034). `platform_watchdog_kick()` — callable from module init functions and main loop tick [13].

**bmi160 / icm42688p:** `_init()`, `_read(imu_reading_t *out) → imu_status_t`. Caller checks status before using data.

**ms5611:** `_init()`, `_service(baro_reading_t *out) → baro_status_t`. Decimated; driver manages I2C state machine across calls.

**sik_radio:** `_init()`, `_rx_pending() → bool`, `_rx_read(command_frame_t *out) → bool`, `_tx_send(const telemetry_frame_t *frame) → bool`. Raw byte transport only — no protocol logic.

**fdir:** `_init()`. `_admit(imu1, imu2, baro, health_out, gate_out)` — absolute checks (staleness, bounds, gyro-vs-gyro), writes the preliminary health verdict; runs before `estimator_predict()`. `_gate(imu1, imu2, baro, predictions, health_inout, gate_out)` — innovation gate against the estimator's predicted measurements; restrict-only on health; runs after `estimator_predict()`. FDIR is the sole writer of `health_flags_t` and never imports the estimator (D050).

**estimator:** `_init()`, `_reset()`. `_predict(imu1, imu2, predictions_out)` — propagates the a-priori state on the admitted gyro and exports `predicted_readings_t` for the gate; a gyro isolated by `fdir_admit` arrives as NULL. `_update(imu1, imu2, baro, health, out) → estimator_mode_t` — EKF correction on healthy channels only; NULL = isolated; covariance grows for any skipped channel (D050).

**control_law:** `_init()`, `_update(est, health, sys_mode, out) → control_mode_t`. Reconfigures mixing matrix based on `health.actuator_healthy[4]`.

**actuators:** `_init()`, `_write(const actuator_cmd_t *cmd)` — hard-clamps to mode limits before writing PWM. `_safe()` drives all fins to zero deflection.

**mode_fsm:** `_init()`, `_update(const command_frame_t *cmd, const health_flags_t *health) → system_mode_t`. `cmd` may be NULL if no frame received this tick.

**telemetry:** `_init()`, `_pack_and_send(imu1, imu2, baro, est, act, health, mode)`. Downlink only. Packs `telemetry_frame_t`, computes CRC-16/CCITT, calls `sik_radio_tx_send()`.

**c2:** `_init()`, `_receive(command_frame_t *out) → bool`. Uplink only. Drains SiK RX ring buffer, validates CRC, deserialises one `command_frame_t`. Returns false if no complete valid frame this tick. Never touches TX.

### 4.6 Test harness strategy

Algorithm modules (estimator, FDIR, control_law) are exercised on the dev PC without hardware. The `avionics/test/` directory contains dev-PC entry points. `test_main.c` replaces `main.c` for test builds, feeding synthetic or recorded `imu_reading_t` / `baro_reading_t` structs directly into the algorithm pipeline.

This is possible because algorithm module inputs are data structs, not hardware registers. The hardware abstraction is complete — no algorithm module calls a sensor driver function directly.

---

## 5. Ground Station Architecture

**Status:** Skeleton — populated alongside requirements.

Pi 5 role bounded by D032 to: telemetry reception, logging, dashboard UI, C2 commands. No estimation, FDIR, or control-path logic. Symmetric transport abstraction to the MCU side (radio or wired). Dashboard rendering covers the multi-pane UI specified in Constraints §7 (live telemetry, sensor health, 3D attitude, fin deflection).

---

## 6. Integration Points

**Status:** Skeleton — populated alongside requirements.

Each interface in §2.8 generates integration test requirements. Each architectural boundary in §2.9 generates boundary test requirements. The avionics ↔ airframe integration boundary (sensor mount alignment, servo bracket fit, cable routing) generates mechanical integration test requirements.
