# Requirements

**Status:** First pass — sections 1–8 populated. Sections 9–11 deferred (see stubs). Several performance numbers pending literature review (marked TBD).
**Created:** 2026-05-11
**Last updated:** 2026-05-18

This document captures the formal specification of what the system must do. Functional requirements (what it does), performance requirements (how well), interface requirements (how it connects), and constraint requirements (what standards it must meet). It is the thing we test against to know if the project is done.

Without this document, "done" is a vibe. With it, "done" is a checklist.

---

## Format

Each requirement has a unique ID, a statement, a category, a priority, a rationale, and a verification method.

- **Category:** Functional / Performance / Interface / Constraint
- **Priority:** Must (non-negotiable) / Should (strongly desired, implemented unless time doesn't permit) / Could (nice to have)
- **Rationale:** traces to a decision log entry (Dxxx) or named constraint section
- **Verification:** how compliance is confirmed on the bench

---

## 1. System-level requirements

> **REQ-SYS-001 — MCU owns the entire real-time path**
> No code executing under Linux shall participate in the real-time control loop. Sensor read, sensor fusion, FDIR, control law, actuator command, mode FSM, and time authority all reside exclusively on the MCU.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D025. Linux scheduling jitter and GC-style stalls are incompatible with deterministic control loop timing. JPL Power of 10 (D019) cannot be applied to code running under a general-purpose OS. A clean architectural boundary ensures that if the Pi crashes, the MCU continues operating unaffected.
> *Verification:* Code review — confirm no Pi-side code has any path into the control loop. Architecture diagram inspection confirms the real-time boundary (Architecture §2.9, boundary 1).

---

> **REQ-SYS-002 — JPL Power of 10 compliance**
> All MCU firmware on the control path, sensor driver layer, and FDIR module shall comply with the NASA JPL Power of 10 coding rules.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D019. The JPL Power of 10 is a 10-rule disciplined coding standard designed for safety-critical embedded software. It is credible to aerospace reviewers and enforceable by a solo developer without specialised tooling. Compliance is a non-negotiable quality floor, not a stretch goal.
> *Verification:* Code review of all control-path and FDIR source files against the JPL Power of 10 checklist. Each rule is verified individually as code is written.

---

> **REQ-SYS-003 — Dual heterogeneous IMUs on isolated SPI buses**
> The avionics board shall carry exactly two IMUs from different chip families (different manufacturer or different sensor family). Each IMU shall connect to the MCU on its own SPI chip-select line. A fault on one IMU's SPI bus shall not affect the other IMU's data acquisition.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D024, D026. A single IMU cannot demonstrate sensor disagreement — there is no second channel to disagree with. Heterogeneous chips prevent common-mode silicon failures from defeating the cross-check. SPI's per-device chip-select provides hardware-enforced bus isolation.
> *Verification:* Schematic review confirms two distinct chip families on separate chip-select lines. Bench test — drive one IMU's SPI lines to fault state; confirm via oscilloscope and log inspection that the other IMU continues producing valid data.

---

> **REQ-SYS-004 — Barometric pressure sensor on dedicated I2C bus**
> The avionics board shall carry one barometric pressure sensor connected to the MCU via a dedicated I2C peripheral on which no other device resides.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D026, D027. The barometer is a dissimilar sensor type (pressure vs. inertial) providing a third independent measurement axis for FDIR cross-modal checks. Placing it alone on its own I2C bus ensures a barometer I2C hang affects only the barometer, leaving IMU data acquisition intact.
> *Verification:* Schematic review confirms barometer is the sole device on its I2C bus. I2C bus scan during bring-up confirms single device enumerated.

---

> **REQ-SYS-005 — Four independently actuated aerodynamic fins**
> The system shall actuate four aerodynamic fins independently. Each fin shall be driven by a dedicated PWM channel from the MCU. Fins shall be arranged at 90° spacing in "+" configuration aligned with the principal body axes.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D030. Four independently actuated fins enable actuator fault tolerance: losing one fin leaves three, which is the minimum for three-axis moment control. A mixing-matrix reconfiguration can redistribute authority across the remaining three. This makes actuator fault tolerance demonstrable.
> *Verification:* Schematic review confirms four distinct PWM channel assignments. Bench test — command each fin independently to distinct deflection angles, confirm via physical observation and servo position feedback that all four move independently.

---

> **REQ-SYS-006 — MCU hardware monotonic timer is the sole time authority**
> The MCU's hardware monotonic timer (microsecond resolution, dedicated peripheral) shall be the authoritative source of time for the entire system. Pi system clock shall not be used for any math involving event ordering, intervals, or cross-session data correlation.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D034. The estimator's fusion math and FDIR cross-checks depend on accurate, consistent dt between measurements. Pi-side timestamps are contaminated by radio transmission variance and Linux scheduling jitter — milliseconds of randomness that corrupt dt and break replay analysis. Hardware-monotonic time is the discipline every real avionics system follows.
> *Verification:* Code review confirms all timing-sensitive computations use the MCU hardware timer peripheral. Code review confirms Pi-side code uses only MCU-provided timestamps for data operations.

---

> **REQ-SYS-007 — Every sensor reading timestamped at read completion**
> Every sensor reading shall be paired with a timestamp from the MCU hardware monotonic timer captured at the moment of read completion. This timestamp shall propagate with the reading through all downstream processing.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D034. The FDIR cross-check between IMU-1 and IMU-2 requires time-aligned pairs of readings. A timestamp captured at read completion — not at processing time — is the only way to correctly account for bus latency differences between sensors.
> *Verification:* Code review of sensor driver layer confirms timestamp capture is the first action after each SPI/I2C transaction completes. Log inspection confirms timestamp field populated for every sensor record.

---

> **REQ-SYS-008 — Two commandable operating modes**
> The system shall support two operating modes commandable from the ground station: flight mode and demo mode. The MCU shall validate and acknowledge all mode commands. Mode transitions shall be logged with MCU timestamp.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D020. A demo/flight mode toggle turns the ground station into a genuine C2 node. Flight mode applies real aerodynamic deflection limits; demo mode relaxes limits for presentation legibility. The acknowledgment requirement prevents silent mode transitions.
> *Verification:* Bench test — issue mode toggle command from ground station in both directions, confirm MCU acknowledgment received and mode indicator updated on dashboard. Log inspection confirms mode transition events timestamped.

---

> **REQ-SYS-009 — MCU operates independently of ground station**
> The MCU shall continue operating normally if the ground station goes offline. Loss of ground station heartbeat shall be a detectable condition that the MCU logs and includes in subsequent telemetry frames.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D025, D032. A clean architectural boundary requires that Pi loss does not halt MCU operation. This also makes the system more robust during demos — a Pi crash or radio fade does not stop the control loop.
> *Verification:* Bench test — kill the ground station process during active operation; confirm via wired link that the MCU continues producing telemetry and logs a heartbeat-loss event within the detection window (REQ-TLM-012).

---

> **REQ-SYS-010 — Single-fault graceful degradation**
> The system shall detect a single sensor or actuator fault, isolate the faulted channel, and continue operating with degraded but bounded accuracy. The system shall not halt, reset, or enter an undefined state on a single fault.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D013, Constraints §5. Graceful degradation under sensor compromise is the project's depth axis. The system must demonstrate that fault tolerance is not a paper property — the bench demo shows continued operation with a sensor disconnected mid-run.
> *Verification:* Bench test — disconnect IMU-1 during active operation; confirm estimator continues (with degraded uncertainty), sensor health reflects the fault, and the system does not halt. Repeat for barometer and for one fin servo.

---

> **REQ-SYS-011 — Control loop rate**
> The MCU control loop shall execute at ≥ 1000 Hz.
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* LR-1 complete (2026-05-22, D052). Derived from 10 Hz disturbance bandwidth × 5× control-bandwidth margin (50 Hz) × Franklin §11.2 Eq. (11.3) sampling rule (ωs/ωBW ≥ 20). ZOH delay verified at 5.6% of rise time (< 10% Franklin criterion). MOI sweep across full build-weight range confirms recommendation is stable. Full derivation: docs/derivations/LR-1-loop-rate.md.
> *Verification:* Bench measurement — log MCU timer at entry and exit of each control loop iteration over a 60-second run; compute mean and minimum rate; confirm both exceed the TBD floor.

---

> **REQ-SYS-012 — IMU SPI bus fault isolation**
> A fault condition on one IMU's SPI bus (including bus contention, device hold, or signal line fault) shall not propagate to the other IMU's SPI bus or affect its data acquisition.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D024, D026. SPI's per-device chip-select is the mechanism: with CS deasserted, a misbehaving device is electrically invisible to the bus. This requirement verifies that the layout and firmware implementation preserve this guarantee.
> *Verification:* Bench test — assert fault conditions on IMU-1's SPI lines while monitoring IMU-2's data output; confirm no data corruption or communication loss on IMU-2 via oscilloscope and log analysis.

---

## 2. Estimation requirements

> **REQ-EST-001 — Dual-IMU and barometer fusion**
> The state estimator shall fuse data from both IMUs and the barometer into a single unified attitude and altitude state. All three sensor channels shall be active inputs to the estimator under nominal operation.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D024, D027. The FDIR depth axis requires all three sensor channels to be present and cross-checkable. A single-IMU or baro-excluded estimator cannot demonstrate the cross-modal fault detection that is the project's depth axis.
> *Verification:* Code review of estimator input pipeline confirms three sensor channels feeding the estimator. Log inspection during a nominal run confirms all three channels actively contributing (non-zero innovation terms for all three).

---

> **REQ-EST-002 — Estimator update rate**
> The attitude estimator shall produce attitude estimates at ≥ 1000 Hz.
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* Follows REQ-SYS-011 (1000 Hz, D052). The estimator output feeds the control law; the estimator must produce new estimates at the control loop rate. LR-1 complete (2026-05-22). See docs/derivations/LR-1-loop-rate.md.
> *Verification:* Bench measurement — log estimator output timestamps over 60 seconds; confirm mean and minimum rates exceed the TBD floor.

---

> **REQ-EST-003 — Estimator output timestamped**
> Every attitude estimate produced by the estimator shall be associated with an MCU timestamp captured from the hardware monotonic timer at the moment of estimate completion.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D034. Replay analysis and FDIR cross-checks require precise timing of estimator outputs relative to sensor readings. A timestamp captured at estimate completion — not at transmission — is the correct reference.
> *Verification:* Log inspection — confirm every estimator output record in the telemetry log contains an MCU timestamp field.

---

> **REQ-EST-004 — Operation under single-IMU fault**
> The estimator shall continue producing attitude estimates when either IMU is faulted and isolated. The estimator shall widen its uncertainty bounds to reflect the reduced sensor information, and shall not produce undefined or unbounded output.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D024, Constraints §5. Graceful degradation requires that single-channel loss does not collapse the estimator. The estimator must be designed from day one to operate with one IMU channel absent — not retrofitted.
> *Verification:* Bench test — disconnect IMU-1 during active estimation; confirm estimator continues producing finite attitude estimates, uncertainty bounds are observably wider, and no NaN or overflow values appear in the log.

---

> **REQ-EST-005 — Operation under barometer fault**
> The estimator shall continue producing attitude estimates when the barometer is faulted and isolated. The altitude channel shall be flagged as unavailable; attitude estimation shall be unaffected.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D027. The barometer is a dissimilar measurement axis. Its loss is an acceptable single-channel degradation — the IMU cross-check remains intact. The requirement verifies that barometer loss is handled gracefully rather than propagating an error into the attitude estimate.
> *Verification:* Bench test — disconnect barometer during active estimation; confirm attitude estimates continue normally, altitude channel marked as faulted in health output, and no attitude estimate corruption observed.

---

> **REQ-EST-006 — Steady-state attitude accuracy**
> Under no-perturbation bench conditions, the estimator's attitude output shall maintain a steady-state error of ≤[TBD — pending LR-2]° RMS relative to the known physical orientation.
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* LR-2 pending. The achievable accuracy floor depends on the specific IMUs selected — their noise density, bias instability, and Allan deviation. A number written before IMU selection and noise characterisation is not a requirement; it is a guess. LR-2 determines what is realistically achievable and sets the target accordingly.
> *Verification:* Bench measurement — mount the airframe at a known physical orientation; run the estimator for 60 seconds with no perturbation; compute RMS error between estimator attitude output and known reference.

---

> **REQ-EST-007 — Per-channel confidence output**
> The estimator shall produce a per-sensor-channel confidence or uncertainty estimate as part of its output interface. This output shall be usable by the FDIR module to inform fault detection decisions.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* Architecture §4. Innovation gating and Bayesian fault detection require uncertainty estimates from the estimator. An FDIR module operating without estimator-provided uncertainty bounds cannot do principled innovation gating — it can only apply hard thresholds, which is less robust.
> *Verification:* Code review — confirm a defined interface between the estimator output and FDIR module input that includes uncertainty/confidence terms. Bench test — confirm the FDIR module receives and uses these terms.

---

> **REQ-EST-008 — User-implemented estimation algorithm**
> The estimation algorithm shall be implemented by the project author. Proprietary on-chip fusion firmware (including vendor-provided fusion modes such as the BNO055's NDOF mode) is excluded. The algorithm source code shall be fully inspectable and JPL Power of 10 compliant.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D035. On-chip fusion is a black box — the algorithm, its failure modes, and its internal state are not accessible. JPL Power of 10 compliance cannot be claimed for code that cannot be read. The project's educational rationale (D005) requires that the fusion algorithm is built and understood, not purchased. A user-implemented EKF or equivalent (e.g., Madgwick/Mahony complementary filter) satisfies this requirement.
> *Verification:* Code review — confirm no vendor fusion firmware is active on either IMU; confirm estimation algorithm source is present in the avionics/ codebase and passes JPL Power of 10 review.

---

## 3. Control requirements

> **REQ-CTL-001 — Closed-loop attitude control**
> The control law shall close the attitude loop using the estimator's attitude output as feedback. The control law shall compute actuator commands that drive the attitude error toward zero.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* Constraints §4, Architecture §4. Closed-loop attitude control is the primary functional deliverable of the avionics system. Open-loop actuation does not constitute a control system.
> *Verification:* Bench test — apply a manual perturbation to the test stand; confirm control surfaces respond and attitude returns toward the commanded setpoint. Log confirms attitude error decreases monotonically after perturbation peak.

---

> **REQ-CTL-002 — Mixing-matrix control allocation**
> The control allocator shall map desired pitch, yaw, and roll moments to individual fin deflection commands via a mixing matrix. The mixing matrix shall be explicit and modifiable at runtime to support fault reconfiguration.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* Architecture §4, D030. An explicit mixing matrix is required for fault reconfiguration (REQ-CTL-005) — the matrix must be swappable at runtime. Hardcoded fin-to-moment mappings cannot be reconfigured without recompilation.
> *Verification:* Code review — confirm mixing matrix is a defined data structure, not hardcoded constants. Bench test — confirm four fins receive distinct deflection commands during a multi-axis perturbation response.

---

> **REQ-CTL-003 — Flight mode deflection limits**
> In flight mode, fin deflection commands shall be clamped to ±[TBD — pending airframe CAD and dynamics analysis]°. Commands exceeding this limit shall be saturated at the limit value; no out-of-range command shall reach the servo.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D020. Flight mode applies real aerodynamic deflection limits appropriate to a flight vehicle. The specific limit angle is determined by airframe dynamics analysis and servo selection — it is the maximum deflection that does not cause aerodynamic stall or structural overload on the fin.
> *Verification:* Bench measurement — command flight mode; apply maximum perturbation; measure peak fin deflection via position feedback; confirm it does not exceed the TBD limit.

---

> **REQ-CTL-004 — Demo mode deflection limits**
> In demo mode, fin deflection limits shall be relaxed to ±[TBD — pending airframe CAD and dynamics analysis]°, exceeding the flight mode limits, to make control surface motion visually legible to an observer.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D020. On a bench demo, the fin deflections in flight mode may be too small for an observer to see clearly. Demo mode relaxes the limits to make the control system's response visually unambiguous. The specific demo limit is chosen to be within servo travel and structurally safe on the bench article.
> *Verification:* Bench measurement — command demo mode; apply perturbation; confirm peak fin deflection observably exceeds the flight mode limit and does not exceed servo physical travel.

---

> **REQ-CTL-005 — Mixing-matrix reconfiguration on fin fault**
> Upon isolation of a faulted fin by the FDIR module, the control allocator shall reconfigure to a mixing matrix that redistributes pitch, yaw, and roll moment commands across the remaining three fins. The reconfigured system shall continue producing bounded actuator commands.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D030. Four fins in "+" configuration with independent actuation enables this reconfiguration. Three fins are the minimum for three-axis moment control. This requirement verifies that the fault-tolerance story is not just hardware — the software handles the reconfiguration correctly.
> *Verification:* Bench test — disable one servo physically; confirm FDIR isolates the fin, mixing matrix reconfiguration is triggered (per REQ-FDR-009), and the remaining three fins continue producing commanded deflections. Log confirms mode transition and new matrix in use.

---

> **REQ-CTL-006 — Perturbation rejection settling time**
> Following a manual perturbation of the test stand, the system shall return the estimated attitude to within a defined band of the commanded setpoint within [TBD — pending airframe CAD and dynamics analysis] seconds of peak perturbation.
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* Pending airframe CAD. The settling time is a function of the airframe's moment of inertia, fin effectiveness, and the control law's bandwidth — none of which can be specified without airframe geometry and dynamics analysis. These inputs come from CAD and structural analysis, which run in parallel with early airframe design.
> *Verification:* Bench test — apply a repeatable manual perturbation (defined angular impulse); measure time from perturbation peak to restoration within the defined band; average over five trials.

---

> **REQ-CTL-007 — Steady-state attitude error**
> Under no-perturbation conditions, the steady-state attitude error between the commanded setpoint and the estimator output shall be ≤[TBD — follows REQ-EST-006].
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* Follows REQ-EST-006. The steady-state control error floor is set by the estimator's accuracy — the control law cannot drive attitude error below what the estimator can resolve. This requirement is derived from the estimation accuracy requirement and is set accordingly once LR-2 is complete.
> *Verification:* Bench measurement — 60-second static run at commanded setpoint with no perturbation; compute mean absolute attitude error from log; confirm below TBD floor.

---

> **REQ-CTL-008 — Control law computation deadline**
> The control law computation, from estimator input read to actuator command write, shall complete within ≤ 1 ms (one control loop period).
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* Follows REQ-SYS-011 (1000 Hz → 1 ms loop period; D052, LR-1 complete). Missing a control loop deadline means the actuator command is computed against stale sensor data, introducing a latency spike that can destabilise the control loop. JPL Power of 10 discipline (bounded loops, no dynamic allocation) makes this deadline deterministic.
> *Verification:* Bench measurement — instrument control law entry and exit with MCU timer; log execution time over 10,000 consecutive control cycles; confirm maximum execution time does not exceed one loop period.

---

## 4. Fault detection and graceful degradation requirements

> **REQ-FDR-001 — Per-channel independent health monitoring**
> The FDIR module shall maintain a dedicated health status flag for each sensor channel (IMU-1, IMU-2, barometer) and each actuator channel (fin 1–4). Health status shall be updated every control loop cycle.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* Architecture §4, D013. Independent per-channel monitoring is required to isolate faults to specific channels without affecting others. A single aggregate health flag cannot support the per-channel isolation required by the graceful degradation architecture.
> *Verification:* Code review — confirm dedicated health flag per channel in FDIR data structures. Log inspection — confirm health flags present for all channels in every telemetry frame.

---

> **REQ-FDR-002 — Detection of IMU output flatline**
> The FDIR module shall detect when an IMU channel produces a constant output value (flatline) over a defined window of consecutive samples, and shall flag that channel as faulted.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* Constraints §5. A flatline is the characteristic signature of a sensor disconnection or complete internal failure. At any non-zero control loop rate, a physically moving or stationary (but noise-producing) IMU will never produce a perfectly constant output — flatline is unambiguous.
> *Verification:* Bench test — hold IMU output fixed at a constant value (simulate by freezing the SPI driver's return value in a test harness); confirm FDIR flags the channel as faulted within the detection window.

---

> **REQ-FDR-003 — Detection of IMU output saturation**
> The FDIR module shall detect when an IMU channel produces output values at or beyond the physical sensor limits (full-scale range) for a defined window of consecutive samples, and shall flag that channel as faulted.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* Constraints §5. Saturated output indicates the sensor is being driven beyond its measurement range — either by a genuine extreme event or by internal sensor failure. In either case, the output is not usable for fusion.
> *Verification:* Bench test — inject out-of-range values at the sensor driver layer via test harness; confirm FDIR flags the channel as saturated within the detection window.

---

> **REQ-FDR-004 — Detection of IMU channel disagreement via innovation gating**
> The FDIR module shall compare IMU-1 and IMU-2 outputs via an innovation gate. When the innovation (difference between predicted and measured values) for a channel exceeds the gating threshold for a defined window of consecutive samples, the diverging channel shall be flagged as faulted.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D024. The dual heterogeneous IMU configuration exists to enable this check. Innovation gating is the standard aerospace technique for cross-checking redundant sensors. If both IMUs agree, the estimate is credible. If they disagree beyond noise bounds, one is faulted. This is the core of the project's FDIR depth axis.
> *Verification:* Bench test — apply a deliberate offset or vibration to one IMU while the other remains undisturbed; confirm FDIR detects disagreement and flags the disturbed channel. The undisturbed channel shall remain healthy.

---

> **REQ-FDR-005 — Detection of IMU communication loss**
> The FDIR module shall detect when an IMU channel produces no new data within the expected inter-sample period, and shall flag that channel as faulted.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* Constraints §5. A communication loss (SPI disconnect, device power loss) produces no data rather than bad data. A timeout-based detection mechanism is required to catch this failure mode, as the innovation gate cannot fire on an absent sample.
> *Verification:* Bench test — disconnect IMU-1 SPI lines during active operation; confirm FDIR flags the channel as communication-lost within one inter-sample timeout period.

---

> **REQ-FDR-006 — Cross-modal barometer and inertial altitude consistency check**
> The FDIR module shall compare the altitude derived from integrated inertial acceleration (IMU-based) against the barometrically measured altitude. Sustained disagreement beyond a defined threshold shall flag a cross-modal inconsistency.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D027, Architecture §2.2. The barometer is a dissimilar sensor — pressure vs. inertial. A failure mode that corrupts both IMUs (common-mode inertial failure) will not corrupt the barometer, and vice versa. This cross-check provides a fault detection path that the IMU-IMU innovation gate alone cannot provide.
> *Verification:* Bench test — apply a rapid vertical motion to the test stand (lift and lower); confirm inertial and barometric altitude channels track consistently. Separately, inject a barometer fault and confirm cross-modal disagreement is detected.

---

> **REQ-FDR-007 — Detection of actuator fault**
> The FDIR module shall detect when a servo is unresponsive or jammed by comparing commanded fin deflection to measured fin position. Persistent mismatch beyond a defined threshold shall flag the actuator channel as faulted.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* Architecture §2.4, D030. Actuator faults (jams, disconnections) must be detectable to enable mixing-matrix reconfiguration (REQ-CTL-005). Position-feedback servos are required for direct fault detection; their selection is driven by this requirement.
> *Verification:* Bench test — physically stall a servo while commanding deflection; confirm FDIR detects the position mismatch and flags the actuator channel as faulted within the detection window.

---

> **REQ-FDR-008 — Sensor fault detection and isolation latency**
> The FDIR module shall detect a sensor fault and complete channel isolation within [TBD — pending LR-3] ms of fault onset.
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* LR-3 pending. The achievable detection latency is a consequence of the innovation gating design — the window size and threshold are chosen to balance false-positive rate against detection speed. This design cannot be specified before LR-3 characterises the tradeoff at the chosen control loop rate. The latency number is derived from the gating design, not prescribed to it.
> *Verification:* Bench test — inject a sensor fault at a known MCU timestamp (via test harness); measure the time from injection to isolation flag in the telemetry log; confirm it does not exceed the TBD limit.

---

> **REQ-FDR-009 — Mixing-matrix swap within one control loop period**
> Upon isolation of a faulted fin channel by the FDIR module, the control allocator shall swap to the reconfigured mixing matrix within one control loop period of the isolation event.
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* D030. Once a fin fault is confirmed and isolated, delay in applying the new mixing matrix produces a control loop cycle where moment commands are sent to a faulted actuator. The swap itself (a pre-computed matrix load) is trivial — the requirement ensures it is not deferred.
> *Verification:* Bench test — trigger a fin fault; inspect telemetry log timestamps; confirm the mixing-matrix reconfiguration event timestamp is within one loop period of the isolation event timestamp.

---

> **REQ-FDR-010 — All detected faults logged with timestamp and type**
> Every fault event detected by the FDIR module shall be logged to the telemetry stream with an MCU timestamp and a fault type identifier indicating which channel faulted and the nature of the fault.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D034, D017. Post-test analysis requires a precise fault event record to correlate with estimator and control law behaviour. A fault log without timestamps is not usable for analysis or for demonstrating detection latency compliance.
> *Verification:* Log inspection — for each bench fault test, confirm a fault event record appears in the telemetry log with MCU timestamp and fault type code.

---

> **REQ-FDR-011 — Sensor health status in every telemetry downlink frame**
> The FDIR module shall include the health status of all sensor and actuator channels in every telemetry downlink frame.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* Architecture §4. The ground station dashboard must display live sensor health (Constraints §7). Health status that is absent from the downlink frame cannot be displayed. Including it in every frame ensures the ground station always has current health information.
> *Verification:* Log inspection — parse 100 consecutive telemetry frames from a bench run; confirm health status field present and populated in all 100 frames.

---

> **REQ-FDR-012 — No halt on single fault**
> The system shall not halt, reset, or enter an undefined state following isolation of a single sensor or actuator fault. Graceful degradation — continued operation with reduced but bounded performance — is the required response to every single fault in scope.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D013, Constraints §5. Graceful degradation is the project's depth axis. A system that halts on a fault has not demonstrated fault tolerance — it has demonstrated fault detection followed by shutdown. The requirement that operation continues is non-negotiable.
> *Verification:* Bench test — fault each sensor channel and each actuator channel in turn; confirm system continues producing attitude estimates and actuator commands in all cases. No MCU reset observed in log.

---

## 5. Telemetry requirements

> **REQ-TLM-001 — Radio module connected via UART with DMA**
> The telemetry radio module shall connect to the MCU via a dedicated UART peripheral. Both receive and transmit paths shall use DMA-driven ring buffers.
> *Category:* Interface.
> *Priority:* Must.
> *Rationale:* D028. DMA-driven buffering reduces radio UART overhead to near-zero CPU cost, allowing the radio driver to coexist with the control loop without consuming compute budget. Without DMA, UART interrupt handling can introduce latency spikes that violate the control loop deadline.
> *Verification:* Code review — confirm UART peripheral configuration with DMA on both RX and TX. Bench measurement — confirm radio driver CPU overhead does not cause control loop deadline misses.

---

> **REQ-TLM-002 — Radio driver shall not stall the control loop**
> The radio telemetry driver shall not block or stall the control loop. All radio driver operations shall complete within bounded time consistent with JPL Power of 10 requirements (bounded loops, no blocking calls).
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D028, D019. A blocking radio driver can introduce unbounded latency into the control loop, violating the computation deadline (REQ-CTL-008) and potentially destabilising the control system. JPL Power of 10 discipline (bounded loops, no dynamic allocation) ensures the driver is deterministic.
> *Verification:* Code review of radio driver against JPL Power of 10 checklist. Bench measurement — instrument control loop execution time over 10,000 cycles with radio active; confirm no deadline misses attributable to radio driver.

---

> **REQ-TLM-003 — Both radio and wired transports supported**
> The system shall support both the wireless radio link and the wired USB-serial link as active telemetry transports. Both shall be capable of carrying telemetry and command traffic.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D029. Radio-only operation blocks development and integration on radio bring-up, and leaves no fallback if the radio fails during a demo. Wired-only walks back the wireless C2 deliverable (D020). Both transports are required to support the development workflow and operational reliability.
> *Verification:* Bench test — verify nominal telemetry and C2 operation over radio; switch to wired; verify same operation. Log inspection confirms both transport paths produce valid frames.

---

> **REQ-TLM-004 — Identical protocol on both transports**
> Both the radio and wired transports shall use the same packet format, CRC algorithm, and command structure. A single protocol definition shall serve both transports.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D029. Two protocol definitions would require maintaining two parsers on both the MCU and ground station — double the implementation, double the failure surface, and synchronisation risk. One protocol, two transport drivers is the correct architecture.
> *Verification:* Code review — confirm a single packet format and CRC definition shared by both transport drivers. Bench test — capture frames from both transports and verify byte-for-byte identical structure.

---

> **REQ-TLM-005 — Transport switching transparent to application layer**
> Switching between radio and wired transports shall require no changes to the application layer on either the MCU or the ground station. The transport abstraction layer shall handle routing transparently.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D029. If transport switching requires application-layer changes, switching mid-demo or during integration testing requires code changes — unacceptable. A transport abstraction layer makes the application layer transport-agnostic.
> *Verification:* Bench test — switch transport mid-session without modifying application-layer code or ground station configuration; confirm telemetry and C2 continue without interruption.

---

> **REQ-TLM-006 — MCU timestamp in every frame header**
> Every telemetry frame transmitted on either transport shall include an MCU hardware timer timestamp in its header, captured at the time of frame assembly.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D034. The ground station uses MCU timestamps for all data operations (REQ-GCS-004). A frame without a timestamp cannot be used for replay analysis, cross-session correlation, or detection latency verification.
> *Verification:* Log inspection — parse 100 consecutive frames from a bench run; confirm MCU timestamp field present and monotonically increasing in all 100 frames.

---

> **REQ-TLM-007 — CRC on all telemetry frames**
> Every telemetry frame on either transport shall include a CRC field. The receiver shall verify the CRC and discard any frame that fails.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* Architecture §2.6. Radio links are subject to bit errors. A CRC-failed frame that is silently accepted produces corrupted sensor health or attitude data in the ground station log — a silent data quality failure. Discard on CRC failure ensures only valid frames enter the analysis pipeline.
> *Verification:* Code review of framing layer confirms CRC computation and validation. Bench test — corrupt a known byte in a frame; confirm the receiver detects the CRC mismatch, discards the frame, and logs a CRC error event.

---

> **REQ-TLM-008 — Radio downlink minimum frame rate**
> The radio telemetry downlink shall achieve a sustained frame rate of ≥5 Hz during nominal operation.
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* LoRa at SF7/BW125 supports 5–10 frames per second at the expected frame size (~50 bytes). 5 Hz is the physical floor given the radio link budget. Below 5 Hz, the ground station dashboard cannot maintain a useful live view of system state.
> *Verification:* Bench measurement — log radio frame reception timestamps at the ground station over 60 seconds; compute mean and minimum frame rate; confirm both ≥5 Hz.

---

> **REQ-TLM-009 — Wired downlink minimum frame rate**
> The wired USB-serial telemetry downlink shall achieve a sustained frame rate of ≥50 Hz during nominal operation.
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* USB-serial at 115200 baud can sustain well over 50 frames per second at the expected frame size. 50 Hz on the wired link enables high-resolution data capture during integration testing, where radio bandwidth would be a bottleneck.
> *Verification:* Bench measurement — log wired frame reception timestamps over 60 seconds; confirm mean and minimum frame rate ≥50 Hz.

---

> **REQ-TLM-010 — Minimum downlink frame content**
> Every telemetry downlink frame shall contain at minimum: MCU timestamp, attitude estimate (roll, pitch, yaw), per-channel sensor health status, active operating mode, and active transport identifier.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* Architecture §2.6, Constraints §7. The ground station dashboard requires all of these fields to render its mandatory panes (REQ-GCS-005). A frame missing any of these fields cannot support the full dashboard view.
> *Verification:* Log inspection — parse a complete bench run log; confirm all mandatory fields present in every frame.

---

> **REQ-TLM-011 — Command acknowledgment within 500 ms**
> The MCU shall acknowledge every received and validated command within 500 ms of receipt. The acknowledgment shall identify the command type and the resulting system state.
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* At a 5 Hz radio frame rate, 500 ms spans two frame intervals — sufficient to survive a single dropped frame before the ground station logs a missing acknowledgment. An acknowledgment that arrives after 500 ms is not useful for confirming that a mode switch took effect before the next operator action.
> *Verification:* Bench test — issue 20 mode toggle commands from the ground station at 1-second intervals; measure time from command transmission to acknowledgment receipt for each; confirm all ≤500 ms.

---

> **REQ-TLM-012 — Ground station heartbeat loss detection within 5 seconds**
> The MCU shall detect the absence of a ground station heartbeat and log a heartbeat-loss event within 5 seconds of the last received heartbeat.
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* Five seconds is short enough to detect a genuine ground station crash without triggering on brief radio fades or repositioning pauses during a demo. The MCU continues operating normally after heartbeat loss — it does not halt (per REQ-SYS-009).
> *Verification:* Bench test — kill the ground station process during active operation; confirm a heartbeat-loss event appears in the MCU telemetry log within 5 seconds; confirm MCU continues producing control outputs.

---

## 6. Ground station requirements

> **REQ-GCS-001 — Platform: Pi 5 running DietPi**
> The ground station shall run on the Raspberry Pi 5 under DietPi.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D018, D032. DietPi was selected for its minimal footprint and compatibility with the Pi 5 GPIO stack and Python scientific ecosystem. The Pi 5 is the available hardware.
> *Verification:* Inspection — confirm Pi 5 running DietPi at session start. Confirm ground station software executes without error on this platform.

---

> **REQ-GCS-002 — Bounded ground station role**
> The ground station software shall perform only the following functions: telemetry frame reception, frame logging to local storage, dashboard UI rendering, and C2 command transmission.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D032. An explicit role boundary prevents implementation-time scope creep that could drag estimation or FDIR logic across the Linux boundary in violation of D025 and REQ-SYS-001.
> *Verification:* Code review — confirm the ground station codebase contains no estimation, FDIR, or control-path logic.

---

> **REQ-GCS-003 — No estimation, FDIR, or control logic on the ground station**
> The ground station shall not execute any estimation algorithm, fault detection logic, or control law computation.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D025, D032. All real-time path logic must reside on the MCU (REQ-SYS-001). This requirement is stated separately from REQ-GCS-002 to emphasise its derivation from the real-time boundary architectural rule — it is not merely a scoping preference.
> *Verification:* Code review — confirm no estimation, FDIR, or control-path functions exist in the ground station codebase.

---

> **REQ-GCS-004 — MCU timestamps for all data operations**
> The ground station shall use MCU-provided timestamps for all data operations including logging, replay, dashboard rendering, and analysis. Pi system clock shall be used only for human-readable session naming and transport-layer diagnostics.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D034. Pi system clock is contaminated by radio transmission variance and Linux scheduling jitter. Using Pi timestamps for data math produces noisy dt values that corrupt any cross-session analysis or detection latency measurement.
> *Verification:* Code review — confirm all time-indexed data operations reference the MCU timestamp field from the received frame, not the Pi system clock.

---

> **REQ-GCS-005 — Multi-pane dashboard**
> The ground station dashboard shall display the following panes simultaneously during active operation: live telemetry charts (attitude and sensor outputs vs. time), sensor health status and confidence indicators, a real-time 3D attitude visualisation of the rocket body, and a view of the fin assembly showing actuated control surface deflections.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* Constraints §7. The multi-pane dashboard is the primary interface for demonstrating system state to an observer. Each pane serves a distinct observational function: telemetry charts for trend analysis, health status for FDIR visibility, 3D attitude for intuitive state reading, fin view for control system response visibility.
> *Verification:* Functional test — run a complete bench session; confirm all four panes render and update during the session. Confirm panes are simultaneously visible without switching.

---

> **REQ-GCS-006 — 3D attitude visualisation refresh rate**
> The 3D attitude visualisation shall refresh at a rate no lower than the active telemetry downlink frame rate (must). The visualisation should additionally interpolate between received frames to achieve a display refresh rate of ≥10 Hz (should).
> *Category:* Performance.
> *Priority:* Must (floor) / Should (interpolation).
> *Rationale:* The display cannot be smoother than the data arriving — the must floor is the raw frame rate (≥5 Hz over radio per REQ-TLM-008). Interpolation to ≥10 Hz improves visual smoothness for a demo audience but requires attitude prediction between data arrivals, which is additional implementation work with no required accuracy guarantee.
> *Verification:* Bench measurement — log display refresh timestamps over a 60-second radio-linked run; confirm minimum refresh rate ≥5 Hz. Interpolated refresh (should) verified by visual inspection and frame interval analysis.

---

> **REQ-GCS-007 — Active transport and mode display**
> The dashboard shall display the currently active transport (radio or wired) and the currently active operating mode (flight or demo) in real time.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D020, D029. An operator must be able to confirm which transport is active and which mode the MCU is in before issuing commands or making demo transitions. Silent state is a demo failure risk.
> *Verification:* Bench test — switch transport and mode; confirm dashboard indicators update to reflect the new state within one telemetry frame period.

---

> **REQ-GCS-008 — Command acknowledgment status displayed**
> The dashboard shall display the acknowledgment status of the most recently issued command, including the command type and whether the MCU has acknowledged it.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D020. The mode toggle C2 link is only useful if the operator can confirm the command was received and acted upon. An unacknowledged command that the operator assumes was received is a demo failure mode.
> *Verification:* Bench test — issue a mode command; confirm the dashboard acknowledgment indicator transitions from pending to acknowledged within the acknowledgment window (REQ-TLM-011).

---

> **REQ-GCS-009 — All frames logged to local storage**
> The ground station shall log every received and CRC-valid telemetry frame to local storage. Log files shall be identified by the MCU session start timestamp recorded at session initialisation.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D032. Post-test analysis requires a complete frame log. MCU session timestamp as the log identifier ensures logs are sortable and cross-referenceable without relying on Pi system clock accuracy.
> *Verification:* Bench test — run a 5-minute session; confirm log file created and named with MCU session timestamp; parse log and confirm frame count matches expected count for the session duration and frame rate.

---

> **REQ-GCS-010 — Transport abstraction mirrors MCU**
> The ground station transport abstraction shall be symmetric to the MCU's — switching between radio and wired transports shall require no changes to the application layer or UI configuration.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D029. Symmetric transport abstraction on both ends ensures the same transport-agnostic protocol definition serves the whole system. It also means the ground station can be switched between radio and wired during development without touching application-layer code.
> *Verification:* Bench test — switch transport mid-session via a single configuration change at the transport layer; confirm application layer and UI continue without reconfiguration.

---

## 7. Power requirements

> **REQ-PWR-001 — Split power rails with common ground**
> The avionics power architecture shall use two electrically separate positive supply rails sharing a single common ground plane.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D033. Servo current spikes (up to 8 A peak with four servos in simultaneous slew) cause voltage sag and high-frequency noise on a shared rail. These effects corrupt IMU ADC references and produce FDIR false positives correlated with servo activity. Topology-level rail separation eliminates this failure class structurally.
> *Verification:* Schematic review confirms two separate positive supply networks. Bench measurement — with both rails active, confirm non-zero voltage between Rail A positive and Rail B positive (confirming electrical separation on the positive supply).

---

> **REQ-PWR-002 — Rail A load assignment**
> Rail A (clean rail) shall supply power exclusively to the MCU, IMU-1, IMU-2, barometer, and radio module. No servo or other high-current load shall draw from Rail A.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D033. The clean rail's noise isolation is only guaranteed if high-current loads are absent from it. A single servo drawing from Rail A reintroduces the current-spike noise that Rail A is designed to prevent.
> *Verification:* Schematic review — confirm no servo power connection to Rail A. Bench measurement — confirm Rail A current draw during maximum servo activity is consistent with MCU and sensor loads only (no step changes correlated with servo commands).

---

> **REQ-PWR-003 — Rail A voltage regulation**
> Rail A shall be regulated to 3.3 V ±2% or 5 V ±2% as required by the selected components. Rail A ripple specification is [TBD — pending IMU datasheet review at component selection].
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* ±2% voltage regulation is a standard requirement for digital microcontrollers and MEMS sensors. The ripple specification depends on the PSRR (Power Supply Rejection Ratio) characteristics of the specific IMUs selected — this cannot be finalised before component selection.
> *Verification:* Bench measurement — measure Rail A voltage at idle and at full MCU and sensor load; confirm within ±2% of nominal. Ripple measurement deferred to component selection phase.

---

> **REQ-PWR-004 — Rail B load assignment**
> Rail B (servo rail) shall supply power exclusively to the four fin servos. No MCU or sensor load shall draw from Rail B.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D033. The servo rail's voltage is permitted to sag under high current draw. Any MCU or sensor drawing from Rail B would experience this sag, potentially causing brownout or noise corruption — the exact failure mode Rail B isolation is designed to prevent.
> *Verification:* Schematic review — confirm no MCU or sensor connection to Rail B.

---

> **REQ-PWR-005 — Rail B current capacity**
> Rail B shall sustain a continuous current of ≥4 A and a peak current of ≥8 A without voltage collapse.
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* Architecture §2.5. Four metal-gear hobby servos in simultaneous hard slew draw 1–2 A each. Peak load of 8 A represents all four servos at maximum simultaneous demand. The regulator and wiring must be sized for this worst-case condition, which occurs during maximum perturbation response.
> *Verification:* Bench measurement — command all four servos to simultaneous maximum slew; measure Rail B voltage and current; confirm voltage remains within servo operating range and current measurement reaches the expected peak without tripping the regulator.

---

> **REQ-PWR-006 — Power-on sequencing**
> Rail A shall reach its regulated voltage before Rail B is enabled. Rail B shall not be enabled until the MCU has initialised and asserted commanded servo positions.
> *Category:* Functional.
> *Priority:* Must.
> *Rationale:* D033. If Rail B energises before the MCU has commanded servo positions, the servos will move to random positions driven by floating PWM lines. This risks mechanical damage and is visually alarming during a demo. Rail A first, MCU init, then Rail B is the safe sequence.
> *Verification:* Bench measurement — oscilloscope trace on both rail voltages at power-on; confirm Rail A reaches regulation before Rail B begins to rise.

---

> **REQ-PWR-007 — Rail voltage monitoring via MCU ADC (Should)**
> The MCU should monitor both Rail A and Rail B voltages via dedicated ADC channels. Sampled voltages should be included in the telemetry downlink frame and accessible to the FDIR module.
> *Category:* Functional.
> *Priority:* Should.
> *Rationale:* Architecture §2.5. ADC-based power monitoring is cheap to implement (resistor divider per rail, two ADC channels). It gives the FDIR module visibility into brownout conditions and correlates rail sag events with servo activity in the telemetry log. Not demo-critical if the power architecture is correct, but provides meaningful diagnostic depth.
> *Verification:* Code review — confirm ADC channel reads for both rails in firmware. Log inspection — confirm rail voltage fields present in telemetry frames during a bench run.

---

## 8. Documentation requirements

> **REQ-DOC-001 — All decisions logged**
> Every architectural and scope decision shall be recorded in the decisions log (docs/03-decisions-log.md) with date, decision statement, rationale, and alternatives considered.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D017, Constraints §10.3. The decisions log is the mechanism by which future design choices are made without relitigating past ones. An undocumented decision is effectively an unknown — it cannot be referenced, defended, or revised with confidence.
> *Verification:* Inspection of decisions log — confirm all architectural and scope decisions made during the project have corresponding log entries.

---

> **REQ-DOC-002 — All requirements traceable**
> Every requirement in this document shall include a rationale that traces to a specific decision log entry (Dxxx) or a named section of the constraints document.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* Constraints §9 item 4. Traceability allows a reviewer to follow the chain from system requirement back to the engineering decision that motivated it. Requirements without rationale are unjustified constraints — they can be challenged and cannot be defended.
> *Verification:* Inspection of this document — confirm every requirement rationale field references at least one decision log entry or constraints section.

---

> **REQ-DOC-003 — Test results recorded at time of testing**
> Every bench test result shall be recorded in the lab notebook (docs/08-lab-notebook.md) at the time of testing, including date, test conditions, observed results, and any anomalies.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D017, Constraints §9 item 4. Test results recorded after the fact are reconstructions, not records. A contemporaneous lab notebook entry is the only form of test evidence that an external reviewer can trust.
> *Verification:* Inspection of lab notebook — confirm entries exist for all completed bench tests, dated at the time of testing.

---

> **REQ-DOC-004 — Requirements document kept current**
> When a design decision materially changes the system's required behaviour or performance, the affected requirements shall be revised in this document before or alongside the design change.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* A requirements document that does not reflect the current design is not a requirements document — it is a historical artefact. Requirements drift is the failure mode that makes verification impossible at project close.
> *Verification:* Process check — at each design review, confirm no known discrepancy exists between the current design and the requirements as written.

---

> **REQ-DOC-005 — Architecture document updated as subsystems are designed**
> The architecture document (docs/05-architecture.md) shall be updated to reflect each subsystem's design as that subsystem is defined. Skeleton sections shall not remain skeleton at the end of the phase in which they are designed.
> *Category:* Constraint.
> *Priority:* Must.
> *Rationale:* D017. The architecture document is the definitive "how" of the system. If it lags the design, the team (or future reviewer) cannot determine what the system actually does from the documentation alone. Keeping it current as design progresses is lower cost than a post-hoc reconstruction.
> *Verification:* Inspection of architecture document at each phase close — confirm no skeleton sections remain for subsystems that have been designed.

---

## 9. Airframe requirements

*Deferred — to be populated once CAD work is underway and structural analysis has bounded the load envelope and material selection.*

---

## 10. Demonstration requirements

*Deferred — demo deliverables described in Constraints §6 are held open. The specific demo format will be defined once the core system is functional and its actual capabilities are known. The §6 descriptions are illustrative, not prescriptive.*
