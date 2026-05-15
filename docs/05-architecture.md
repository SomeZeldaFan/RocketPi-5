# Architecture

**Status:** Hardware and structural architecture established (per D024–D034). Software, ground station, and simulation sections pending requirements.
**Created:** 2026-05-11
**Last updated:** 2026-05-15

This document captures the "how" of the system. Component breakdown, data flow, hardware block diagram, software module structure, interface definitions between subsystems.

The architecture is designed to support fault tolerance from day one (see Constraints §10.2). Interfaces between estimator, controller, sensor abstraction, and health monitor are defined such that fault-tolerance logic can be added incrementally without restructuring.

---

## 1. System Block Diagram

**Deferred.** Formal system block diagram to be produced using proper diagramming methodology appropriate for the project's documentation standard. The nodes, interfaces, and architectural boundaries described in §2 are the authoritative system concept; the diagram visualizes them, it does not define them.

---

## 2. Hardware Architecture

### 2.1 System concept

Three operational nodes and one offline node:

**Test stand (bench).** The custom semi-monocoque airframe (per §3) mounted on a gimbal/pivot, housing the avionics bay. The airframe is a deliberate structural engineering deliverable — not packaging.

**Avionics bay (inside airframe).** MCU, dual heterogeneous IMUs, barometer, radio module, four servos driving four independently-actuated fins. The MCU owns the entire real-time path: sensor read, fusion, FDIR, control law, actuator command, mode FSM, and time authority. Nothing running on Linux is in the control loop (per D025).

**Ground station.** Pi 5 (DietPi) with display and radio module. Bounded role (per D032): telemetry reception, logging, dashboard UI, C2 command transmission. Not in the real-time loop. Loss of the Pi does not affect MCU operation.

**Dev PC (offline).** Ryzen 7 9700X, RTX 4080 Super, 64 GB DDR5. HIL simulation runtime host, post-test analysis host, firmware development environment. Not part of the deployed system. Connected to the Pi via SSH; connected to the MCU via wired USB-serial.

### 2.2 Sensor stack

**Dual heterogeneous IMU on isolated SPI buses.** Two IMUs from different chip families (different manufacturer / different sensor family), each on its own SPI chip-select line on a shared SPI bus. SPI's per-device chip-select gives hardware-enforced fault isolation: one IMU pulling its pins into bad states cannot reach the other IMU. Heterogeneous chips prevent common-mode silicon failures from defeating the cross-check.

- IMU-1 (chip A): SPI, CS1.
- IMU-2 (chip B, different family from A): SPI, CS2.

Specific IMU part selection is deferred to the component-selection phase, bounded by Constraints §11.2.

**Barometer on dedicated I2C.** A single barometric pressure sensor alone on its own I2C peripheral. Pressure is a dissimilar measurement axis (pressure vs. inertial; membrane deflection vs. mass-on-spring), giving FDIR a third sensor type immune to the failure modes that fool inertial sensors. Required for Demo 3's altitude channel; enables the bench cross-modal demo (tap the stand → IMUs see acceleration, baro sees no altitude change).

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

**Dev PC.** Offline node. HIL simulation runtime host (per D032); analysis tooling; firmware development and flashing.

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

**Wired link (dev/debug/backup channel).** USB-serial between MCU and Pi (per D029). Used for firmware flashing, high-bandwidth log dumps, pre-demo subsystem checks, HIL data injection from the dev PC, and as a fallback if the radio fails during a demo.

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
| MCU ↔ Pi (wired) | USB-Serial | Bidirectional | Dev/debug, firmware flash, HIL injection, backup |
| Pi → Display | HDMI/DSI | Pi to display | Dashboard rendering |
| Dev PC ↔ Pi | SSH over network | Bidirectional | Remote development and analysis |
| Dev PC ↔ MCU | USB-Serial | Bidirectional | Firmware flash, HIL simulation injection |

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

Semi-monocoque construction adapted to FDM-printed fabrication. The printed outer skin carries primary loads, supported by printed-in internal frames and longitudinal stringers integrated into the same print model. Load envelope is bench-only — self-weight under gimbal mounting, perturbation forces during demos (tens of newtons peak), servo reaction forces on fin pivots, and handling loads from drops off the bench. No transonic flight loads, no aerodynamic stability requirements.

### 3.2 Skin / frame / stringer layout

*To be populated during airframe CAD work.*

### 3.3 Segmentation strategy

*To be populated during airframe CAD work. The airframe will be printed in multiple segments, joined mechanically; segmentation choice trades off print volume against joint count and joint structural integrity.*

### 3.4 Avionics bay integration

*To be populated during airframe CAD work. Avionics bay is purpose-designed for the avionics stack — MCU mount, sensor positioning, radio antenna placement, cable routing, access for in-place flashing and debugging via the wired link.*

### 3.5 Fin pivot integration

*To be populated during airframe CAD work. Four fin pivots integrated into the airframe structure; servo mounting brackets printed into the body adjacent to the fin hinge points; servo linkages designed in.*

### 3.6 Gimbal mount integration

*To be populated during airframe CAD work. Gimbal mount printed into the airframe near the center of mass; gimbal choice (2-axis pitch+yaw vs. 3-axis full) deferred.*

### 3.7 Material selection and print strategy

*To be populated. Trade space: filament family (PETG / ABS / ASA / CF-filled PLA or PETG); wall thickness and perimeter count; infill density and pattern; print orientation relative to expected load axes; printed-in reinforcement features (ribs, fillets, gussets). Material choice driven by coupon testing per §3.9.* **No fibrous layup or epoxy work.**

### 3.8 Fabrication process

*To be populated. Workflow from CAD model → slicer settings → print → post-processing (support removal, dimensional verification) → assembly. Iteration discipline (design freeze after one or two revisions) is a guardrail flagged in the risk register.*

### 3.9 Structural analysis and validation

*To be populated. Hand calculations on the dominant load paths at minimum; FEA optional (Fusion 360's built-in FEA module is a candidate tool). Validation via coupon testing in candidate materials/orientations/infill patterns before committing to a full airframe print; full-airframe load testing post-fabrication.*

---

## 4. Software Architecture

**Status:** Skeleton — populated alongside requirements.

Module structure is constrained by the hardware decisions (D024–D034):

- Sensor driver layer: one SPI driver shared between IMU-1 and IMU-2 (different chip-select lines, possibly different register maps if heterogeneous); one I2C driver for the barometer; each driver pairs every reading with an MCU timestamp.
- State estimator: consumes dual-IMU + baro; designed from day one to operate with widened uncertainty bounds on any single channel.
- FDIR module: per-sensor health channels, innovation gating between IMU-1 and IMU-2, cross-modal check between integrated inertial altitude and barometric altitude, actuator health channels with mixing-matrix reconfiguration on detected actuator fault.
- Control law: produces desired pitch/yaw/roll moments; control allocator maps moments to fin deflections through the current (possibly reconfigured) mixing matrix.
- Mode FSM: explicit states for demo/flight mode (per D020) and nominal/degraded/safe operation; transitions logged and telemetered.
- Transport layer: abstraction over radio UART and wired UART/USB; same protocol on both.
- Telemetry framing layer: packet format, CRC, MCU-timestamp header.
- Command dispatch layer: receives validated commands; routes to mode FSM.

### 4.1 Module breakdown
### 4.2 Interfaces between modules
### 4.3 Data flow
### 4.4 Threading / scheduling model
### 4.5 Error handling and fault propagation paths

---

## 5. Ground Station Architecture

**Status:** Skeleton — populated alongside requirements.

Pi 5 role bounded by D032 to: telemetry reception, logging, dashboard UI, C2 commands. No estimation, FDIR, or control-path logic. Symmetric transport abstraction to the MCU side (radio or wired). Dashboard rendering covers the multi-pane UI specified in Constraints §7 (live telemetry, sensor health, 3D attitude, fin deflection).

---

## 6. Simulation Architecture

**Status:** Skeleton — populated when HIL simulation scope is defined.

HIL simulation runs on the dev PC (per D032). Dev PC injects simulated sensor data into the MCU over the wired link (per D029); MCU runs its real fusion / FDIR / control firmware against the injected data; resulting telemetry flows back through the Pi as ground station. From the Pi's perspective, HIL telemetry is indistinguishable from real-bench telemetry — same packet format, same MCU timestamps, same dashboard rendering.

Mass properties for the simulated rocket are derived directly from the airframe CAD model (per §3), giving the HIL configuration a tighter validation story than a measured commercial kit could provide.

---

## 7. Integration Points

**Status:** Skeleton — populated alongside requirements.

Each interface in §2.8 generates integration test requirements. Each architectural boundary in §2.9 generates boundary test requirements. The avionics ↔ airframe integration boundary (sensor mount alignment, servo bracket fit, cable routing, gimbal alignment) generates mechanical integration test requirements.
