# Decisions Log

Append-only. Do not edit past entries. If a decision is later overturned, add a new entry referencing the prior decision and explaining why it's being changed. The point of this log is not to revisit decisions; it's to remember *why* they were made when something later seems to contradict them.

Format: **Date — Decision — Rationale — Alternatives considered.**

---

## 2026-05-11

### D001 — Adopt phased requirements-elicitation approach

**Decision:** Project follows Phase 0 (requirements scoping) → Phase 1 (detailed requirements + architecture) → Phase 2+ (build, test, integrate). No concept-locking until requirements and constraints are documented.
**Rationale:** Real engineering does not jump from mission need to solution. Premature concept lock is the dominant failure mode for ambitious student projects.
**Alternatives:** "Just pick a project and start" — rejected as the failure mode we're trying to avoid.

### D002 — Time budget set at ~400 hours (350–450 range) *(superseded by D021)*

**Decision:** Working assumption is ~20 hrs/week baseline + ~45 hrs/week × ~6 vacation weeks, with conservative haircuts for life and motivation troughs. Total budget ~400 hours across 16 weeks.
**Rationale:** Initial estimate of 128 hours was based on misread "3 hrs/week" — corrected after clarification that 20 hrs/week is sustainable.
**Alternatives:** Treating time as unbounded — rejected; unbounded budgets distort trade studies.

### D003 — Budget soft ceiling: $2,000 USD

**Decision:** Soft ceiling of $2,000 across project lifetime, excluding tools (soldering iron, multimeter, 3D printer access).
**Rationale:** Bounded budget forces honest trade-offs. Stipend supports this level comfortably.
**Alternatives:** Unbounded budget — rejected.

### D004 — Slog estimated explicitly during scoping

**Decision:** Every concept and major task is evaluated for the ratio of learning-dense work to mechanical-tedium work. Slog phases are budgeted, not encountered.
**Rationale:** Real engineering organizations plan integration and test time explicitly because the alternative is schedule lies. The failure mode "discover the project is 60% slog at week 10" is well-documented.
**Alternatives:** "Plan for the fun parts, react to the rest" — explicitly considered and rejected after discussion.

### D005 — Domain locked: model rocketry

**Decision:** Aerospace subdomain for this project is model rocketry, with framing TBD.
**Rationale:** Builder has demonstrated sustained pull (multiple full passes through BPS.space content prior to project initiation). Sustained motivation is the strongest predictor of completion across 400 hours.
**Alternatives:** Drones (pivot considered when procurement issues surfaced; rejected when bench-only rocketry path opened).

### D006 — Framing: avionics- and systems-centric, not airframe-centric

**Decision:** Project's focus is the avionics suite and control system. Airframe is a vehicle for the engineering, not the engineering itself.
**Rationale:** Plays to builder's software/systems strengths, pulls toward genuine aerospace (GNC), differentiates from typical hobbyist builds.
**Alternatives:** Airframe-centric build (more typical hobbyist framing) — rejected as less differentiated.

### D007 — Propulsion work excluded

**Decision:** No motor manufacture, no propellant work, no static fire tests, no propulsion engineering content. Any motors used (if any) are commercial.
**Rationale:** Propulsion safety ramp is multi-year. Inappropriate for project 1.
**Alternatives:** Commercial motor static fire — rejected for safety and complexity.

### D008 — TVC excluded from any flight-intended article

**Decision:** Thrust vectoring control is not part of the flight configuration. (Moot after no-flight decision.)
**Rationale:** Mechanical and control complexity beyond project 1 scope. BPS.space spent years refining TVC; we do not have that runway.
**Alternatives:** Aero surfaces (selected).

### D009 — Three-demo target defined

**Decision:** Initial demo set was (1) live bench control demo, (2) flight + telemetry video, (3) sim-vs-actual flight data comparison.
**Rationale:** Captures bench depth, flight validation, and engineering rigor.
**Subsequent revision:** See D012.

### D010 — Executive decision: no flight

**Decision:** This project does not fly anything.
**Rationale:** Procurement of mid-class motors restricted in UAE consumer channels; UAE wartime aviation posture adds regulatory friction; recovery and logistics in summer desert conditions impose further cost; institutional cooperation for launch logistics not available within timeline. Builder explicitly chose bench-only as the higher-confidence path.
**Alternatives:** Pursue institutional procurement channel and GCAA authorization for small motors — considered, rejected on time grounds.

### D011 — Custom airframe build excluded

**Decision:** No custom airframe design and fabrication. If an airframe is used, it is commercial.
**Rationale:** Custom airframe = aerodynamic stability analysis + structural design + fabrication. Three hard problems on top of an already-deep avionics project. Standard hobbyist practice is to modify commercial kits; this is the right path for project 1.
**Alternatives:** Build from scratch (3D printed or composite) — rejected on scope grounds.

### D012 — Demo 3 redefined

**Decision:** Without real flight data, "sim vs. actual comparison" is not deliverable. Demo 3 is redefined as: high-fidelity simulation of the vehicle in hypothetical flight, with the actual avionics software running against simulated sensor inputs (hardware-in-the-loop), and stress-test results.
**Rationale:** Honest about what is deliverable without flight. Simulation as a standalone deliverable is still rigorous and aerospace-authentic.
**Alternatives:** Drop Demo 3 entirely — rejected; simulation work is genuine engineering content.

### D013 — Depth axis locked: Direction 1 (fault-tolerant graceful degradation)

**Decision:** Project's distinctive depth axis is graceful degradation under sensor compromise. Sensor health monitoring, fault detection, isolation, and continued operation under degraded sensors are core scope.
**Rationale:** Honest to bench setup (sensor robustness is not motion-dependent). Maps to real aerospace FDIR subdiscipline. Genuinely defense-relevant. Doesn't force project-identity reframe.
**Alternatives:**

- Direction 2 (visual odometry on Pi) — interesting but adds ~100–160 hours and a whole second estimation modality.
- Direction 3 (reframe as GNC tech demonstrator) — abandons rocketry identity.
- Direction 4 (no depth axis added) — defensible but lower ceiling.

### D014 — Architect for robustness from day one (principle)

**Decision:** Fault tolerance is not Phase 2 work. The architecture is designed for fault tolerance from the first commit, even if implementation is incremental.
**Rationale:** Retrofitting robustness costs 2–3× designing for it. "Stretch goal" features in solo projects reliably get cut. If we deferred fault tolerance, we would not ship it.
**Alternatives:** "Make it work, then make it robust" at project scope level — discussed and rejected. The principle applies at component scope, not project scope.

### D015 — GCAA permission-seeking out of scope

**Decision:** Email to GCAA will not be sent. The regulatory question is closed for this project because flight is closed.
**Rationale:** Builder's executive decision. Relationship-building rationale acknowledged but deprioritized.
**Alternatives:** Send a low-stakes relationship-building email — considered, declined.

### D016 — Defense framing permitted; weaponization excluded

**Decision:** Project may be framed for defense-aerospace relevance (sensor fusion, fault-tolerant GNC, GPS-denied operation analogs) without any kinetic, projectile, or payload-delivery elements.
**Rationale:** Defense framing helps with the HALCON/EDGE audience. Weaponization framing would be counterproductive (UAE wartime context, age, and student status make any weapon-adjacent work a serious mistake).
**Alternatives:** No defense framing — rejected as leaving real value on the table.

### D017 — Document architecture defined

**Decision:** Project maintains 9 documents: Constraints, Current State, Decisions Log (Tier 1, actively maintained); Requirements, Architecture, Risk Register (Tier 2, created in Phase 1); Test Plan, Lab Notebook, Bibliography (Tier 3, append-only as project progresses). All in plain Markdown in a git repo.
**Rationale:** Minimum viable document set that survives chat boundaries and provides project memory. Heavy enough to be useful, light enough to maintain.
**Alternatives:** Notion or Google Docs — rejected for git incompatibility. Lighter doc set — rejected for risk of context loss.

### D018 — Pi 5 OS: DietPi

**Decision:** Raspberry Pi 5 runs DietPi for bring-up and the project lifetime. Pi OS Lite retained as documented fallback if DietPi-specific issues surface (15-minute reflash).
**Rationale:** Lowest combined disk + cognitive overhead among Pi-5-supported distros. Debian-based, so the `apt` ecosystem and the Pi 5 GPIO stack (`lgpio`, `gpiozero`) are first-class — no compatibility cost for later Python scientific dependencies (NumPy, SciPy, RocketPy). First-boot `dietpi-software` TUI implements the "install only what I need" principle structurally rather than by post-hoc cleanup. Pi 5 officially supported since 2024.
**Alternatives:**

- Raspberry Pi OS Lite — rejected: larger footprint (~1 GB vs ~400 MB), runs default services we'd disable anyway. Retained as fallback.
- Raspberry Pi OS Full (Desktop) — rejected: desktop/browser/office overhead unjustified for a headless bench instrument.
- Ubuntu Server 24.04 LTS — rejected: Pi-5-specific peripheral support is second-tested vs Pi-OS-derived distros; gains nothing at this project's scale.
- Arch Linux ARM — rejected: rolling-release maintenance load across 16 weeks; Pi 5 kernel-patch support sometimes lags.
- Alpine Linux — rejected: musl libc breaks Python scientific wheels needed for simulation work later.
- Buildroot/Yocto — rejected: hours-to-days image build effort; not viable on this timeline.

---

## 2026-05-12

### D019 — Coding standard: NASA JPL Power of 10

**Decision:** All flight-critical and control-path code written for this project abides by the NASA JPL Power of 10 rules.
**Rationale:** Short (10 rules), principled, credible to aerospace reviewers, and actually enforceable by a solo developer without specialized tooling. Strikes the right balance between rigor and practicality for a student-level project that aims to read as professional aerospace engineering.
**Alternatives considered:**
- MISRA-C — more comprehensive (~140 rules) but requires static analysis tooling to enforce properly; overhead not justified at this scale.
- DO-178C — airborne software certification standard; overkill for a bench demo, not applicable without a certification authority.
- Custom disciplined subset — valid but less credible without a named standard to reference in documentation and writeup.

### D020 — Telemetry link made bidirectional: demo/flight mode toggle

**Decision:** The telemetry link between the MCU and Pi ground station is bidirectional. The Pi can command the MCU to switch between two operating modes:
- **Flight mode** — real deflection limits applied (±3° or equivalent per control law)
- **Demo mode** — relaxed deflection limits for presentation legibility

The MCU acknowledges mode commands. The ground station UI reflects the active mode and acknowledgment status.
**Rationale:** Turns the ground station into a genuine command and control node, not just a display. Strengthens defense-relevance framing. No new hardware required — LoRa modules are inherently bidirectional. The mode toggle is immediately legible to any audience. Scoped conservatively: basic acknowledgment only, not full C2 protocol.
**Alternatives considered:**
- Visual exaggeration only (ground station exaggerates fin movement on screen, hardware stays at real limits) — rejected as less honest and less interesting engineering.
- Full C2 implementation (authentication, command sequencing, link-loss fail-safe) — deferred to stretch goal; cost is 40–60 additional hours and adds real-time complexity to the MCU control loop.
- No C2 at all — rejected; bidirectional link is a meaningful scope addition with no hardware cost.

### D021 — Time budget tracking removed from documentation (supersedes D002)

**Decision:** Explicit time budgets, hour estimates, and week counts are no longer tracked in the project documentation. D002 is retired.
**Rationale:** Time constraints are an internal planning concern, not a property of the project or its engineering. Public-facing documentation should reflect what is being built and to what standard — not how long it takes.
**Alternatives considered:**
- Keep D002 and time tracking in constraints doc — rejected; it adds no value to external readers and frames the project as a student assignment rather than an engineering artifact.

### D022 — Repo structure: domain-first, aerospace-adjacent naming

**Decision:** Top-level repo structure is domain-first with aerospace-conventional naming. Six top-level directories: `avionics/` (MCU code: control loop, estimator, FDIR, telemetry), `gcs/` (Pi 5 ground control station: dashboard, C2, telemetry receiver), `sim/` (RocketPy + hardware-in-the-loop simulation), `analysis/` (post-test Python tooling), `hardware/` (KiCad schematics, STL files, wiring diagrams), `docs/` (engineering documents, formerly `Core/`). Each folder is an independent system with its own toolchain and build system.
**Rationale:** The folder layout reflects the system architecture directly — a reader opening the repo sees the system decomposition before reading a single line of code. Terms `avionics` and `gcs` locate the project in aerospace, not just embedded systems. Domain-first structure avoids the false unity implied by `src/` when the codebase spans C firmware, Python application, and Python simulation with no shared build system.
**Alternatives considered:**
- Organic/incremental (keep `Core/`, add folders as needed) — rejected; defers a decision cheaper to make now than after code exists.
- Traditional `src/` monorepo — rejected; `src/` flattens heterogeneous systems that don't share a build system or deployment target; reads as a software project, not an avionics project.
- Domain-first with generic naming (`firmware/`, `groundstation/`) — rejected in favor of aerospace-conventional terms.
- Keep `Core/` folder name — rejected in favor of `docs/`; standard name communicates faster to outside readers; the numbered file convention already signals deliberate structure without the folder name needing to carry that weight.

### D023 — Phase 1 opens with top-level system architecture review preceding requirements

**Decision:** Phase 1 begins with a structured options review of the top-level system architecture before requirements are written. The review identifies the major system nodes, interface choices, and structural decisions with genuine options and downstream consequences. The output is an explicit system concept that requirements and subsequent Phase 1 decisions sequence from.
**Rationale:** Requirements written without an explicit system concept are anchored to an implicit architecture scattered across the constraints doc and the engineer's working memory — not in a single authoritative artifact. Making the system concept explicit first ensures requirements are written against something concrete, and surfaces structural choices (IMU count, real-time path ownership, sensor bus topology) that directly constrain what requirements are achievable.
**Alternatives considered:**
- Write requirements first, derive system concept from them — rejected; requirements without a concrete system reference are either too abstract (no implementation path) or lock structural choices without examining them.
- Derive system concept informally from constraints doc prose — rejected; the constraints doc describes the system across multiple sections in prose; that is not an architecture artifact, it is a description.

---

## 2026-05-15

### D024 — IMU configuration: dual heterogeneous, separate buses

**Decision:** The avionics board carries two IMUs from different chip families (different manufacturers / different sensor families), each connected on its own physically isolated bus.
**Rationale:** Constraints §5 lists sensor disagreement (innovation gating) as an in-scope failure mode. A single IMU cannot demonstrate sensor disagreement at the hardware level — there is no second sensor to disagree with. Two IMUs are the minimum configuration that makes the depth axis demonstrable. Heterogeneous chips (different silicon, different firmware paths, different temperature behaviors) prevent common-mode failures from defeating the cross-check.
**Alternatives considered:**
- Single IMU — rejected; sensor disagreement detection becomes undemonstrable; only self-consistency checks (NaN, saturation, flatline) remain. The depth axis is partially gutted at the hardware level.
- Triple IMU with majority voting — rejected; 3× wiring, calibration, and bus consumption for marginal demo gain; voting is less aerospace-authentic at this project scale than innovation gating, which dual-IMU enables.

### D025 — Real-time path ownership: MCU-only

**Decision:** The MCU owns the entire real-time path: sensor read, sensor fusion, FDIR, control law, actuator command, mode FSM, and time authority. No code running under Linux (i.e., on the Pi 5) is ever in the control loop. This is stated as an explicit architectural rule, not merely an implementation preference.
**Rationale:** JPL Power of 10 (D019) cannot apply to Python on a general-purpose OS — half the rules are violated by the Python interpreter itself before user code runs. Any Pi participation in the real-time path would require either weakening the JPL claim or carving out an unprincipled exception. Linux scheduling jitter (kernel preemption, GC-style stalls, NTP adjustments) is incompatible with deterministic dt for the estimator. Clean architectural boundary: if the Pi crashes, the MCU continues operating and can detect loss of ground station heartbeat.
**Alternatives considered:**
- Hybrid with Pi running the estimator — rejected; round-trip through Linux into the control loop violates constraints §4 and makes JPL safety claim incoherent.
- Hybrid with Pi running slow offline FDIR diagnostics — deferred, not adopted; harmless but adds no Phase 1 demo value; can be layered on later without restructuring if a use case earns its keep.

### D026 — Sensor bus topology: SPI for IMUs, I2C for barometer alone

**Decision:** Each IMU connects to the MCU on its own SPI chip-select line on a shared SPI bus (point-to-point per device). The barometer connects on a dedicated I2C bus, alone on that bus.
**Rationale:** The dual-IMU decision (D024) only earns its keep if the bus topology actually isolates the two IMUs at the hardware level. I2C has a well-known hazard where one device can hang the bus (holding SDA or SCL low) and take all other devices on that bus with it — a single-fault common-mode failure that defeats fault isolation entirely. SPI's per-device chip-select gives hardware-enforced isolation: a misbehaving device with CS deasserted is electrically invisible to the bus, so one IMU's failure cannot reach the other IMU. The barometer is a slow, dissimilar sensor not in the IMU cross-check critical path; placing it alone on its own I2C bus means an I2C hang only loses the barometer, which is an acceptable single-channel loss. SPI also offers higher data rates that better suit the IMUs' desired sample rate.
**Alternatives considered:**
- Shared I2C bus for all sensors — rejected; one hung device takes all sensors down; defeats fault isolation; architecturally self-defeating for an FDIR project.
- Split I2C with IMU-1 on bus 1 and IMU-2 + baro on bus 2 — rejected; partial isolation only; IMU-2 and baro remain coupled.
- SPI for all sensors including baro — rejected; extra pin and driver for no isolation gain (baro is already alone on its bus in the chosen topology).
- I2C bus multiplexer — rejected; adds a part that itself can hang; complexity unjustified at this scale.

### D027 — Barometer included on MCU via I2C

**Decision:** A barometric pressure sensor is included on the avionics board, connected to the MCU via the dedicated I2C bus established in D026.
**Rationale:** Demo 3 (HIL simulation of hypothetical flight) requires an altitude channel; a flight simulation against avionics with no barometer driver is incomplete. The barometer is a dissimilar sensor type (pressure vs. inertial; membrane deflection vs. mass-on-spring) that supplies sensor diversity for FDIR — a common-mode failure that affects both IMUs cannot fool the barometer, strengthening the cross-check beyond what same-type redundancy alone supplies. Enables a clean bench demo of cross-modal sensor reasoning: a tap on the test stand makes both IMUs see acceleration, but the barometer sees no altitude change; the fused estimate correctly attributes this to horizontal perturbation rather than vertical motion.
**Alternatives considered:**
- Exclude entirely — rejected; Demo 3 becomes thin (no altitude channel in HIL); FDIR loses dissimilar-sensor diversity; bench demo loses the cross-modal narrative.
- Software-only barometer (simulated in HIL only, not on physical hardware) — rejected; breaks the "same firmware on real and simulated hardware" property that makes HIL credible.

### D028 — Telemetry radio direct to MCU via UART + DMA

**Decision:** The telemetry radio module connects directly to the MCU via a UART peripheral with DMA-driven ring buffer for both receive and transmit. The radio protocol stack (packetization, CRC, framing, command dispatch) lives in MCU firmware, JPL Power of 10 compliant. No dedicated communications coprocessor.
**Rationale:** At LoRa-class data rates (a few kilobits per second, a few hundred bytes per frame, ≤10 Hz cadence), the radio is not a CPU load problem; DMA reduces it to near-zero CPU overhead. JPL Power of 10 (D019) is specifically designed to make code like a radio driver coexist safely with flight-critical code on the same processor — bounded buffers, bounded loops, no dynamic memory, bounded ISRs. Adding a coprocessor admits we don't trust the standard to do its job; introduces a new node, new firmware, new interface, and a new failure mode (the coprocessor itself can fail, severing telemetry with the radio and MCU otherwise healthy).
**Alternatives considered:**
- Dedicated comms coprocessor — rejected; solves a problem that doesn't exist at our data rates; introduces a new failure mode; contradicts the JPL quality narrative.
- Terminate radio at Pi rather than MCU — rejected; puts Linux in the wireless C2 path, reintroducing scheduling jitter and breaking the determinism rationale.

### D029 — MCU↔Pi physical link: both radio and wired, same protocol

**Decision:** Both transports are present. The radio link is the primary operational link — used for live demos, satisfying the wireless C2 deliverable in D020. A wired USB-serial link is the dev/debug/backup channel — used for firmware flashing, log dumps, HIL data injection from the dev PC, and as a fallback if radio fails during a demo. Both transports carry the same protocol (same packet format, CRC, command structure); a transport abstraction layer in firmware and ground station software allows runtime switching without application-layer changes.
**Rationale:** Real avionics programs universally have a wired umbilical for development and a radio for operational use — you don't debug a flight computer over its telemetry radio. Radio-only would force all integration testing to wait on radio bring-up (a multi-front debugging trap) and leaves no demo fallback. Wired-only walks back D020's wireless C2 commitment. Same-protocol-on-both eliminates the two-codebases-to-maintain problem; the application layer is transport-agnostic.
**Alternatives considered:**
- Radio only — rejected; blocks all dev on radio bring-up; no HIL injection path; no demo fallback; not what real avionics programs do.
- Wired only — rejected; walks back D020 wireless C2 deliverable; loses defense-relevance framing.
- Both transports with different protocols — rejected; two command parsers, two ground-station modes, sync problems for no benefit.

### D030 — Control surfaces: four fins, independently actuated, "+" configuration

**Decision:** Four aerodynamic fins at 90° spacing in "+" configuration (fins aligned with principal axes), each independently actuated by its own servo on its own PWM channel.
**Rationale:** Symmetric to D024 on the actuator side. Three fins is the minimum count for 3-axis control; a single fin failure leaves only two fins, which cannot control three axes — single-fault unrecoverable. Four fins leave three after a single failure, which is the minimum for 3-axis control; a reconfigurable mixing matrix can redistribute control authority across the remaining three. This makes actuator fault tolerance demonstrable, and unlocks the compound-failure demo (simultaneous sensor and actuator faults, both isolated, system continues stabilizing) — the single most aerospace-authentic moment the project can produce. "+" configuration over "×" because pitch/yaw/roll mixing is simpler and more intuitive; the exact orientation is a downstream detail.
**Alternatives considered:**
- Three fins at 120° spacing — rejected; actuator fault tolerance is structurally undemonstrable; half the FDIR story missing.
- Four fins ganged in pairs (pitch pair, yaw pair) — rejected; pays the four-servo cost without earning fault-tolerance redistribution capability; loses on roll authority.
- "×" configuration — deferred; viable alternative orientation but "+" is the simpler default for a student-scale project.

### D031 — Custom semi-monocoque airframe as parallel structural engineering chapter; retires D011

**Decision:** The bench article is a custom-designed, 3D-printed semi-monocoque airframe — designed, structurally analyzed, fabricated, and tested as a parallel engineering deliverable to the avionics work. The airframe becomes a real chapter of the project, not a piece of packaging. This decision retires D011 (commercial-kit-only) and resolves the airframe-presence open question previously held at constraints §11.4. **Explicit scope boundary:** the structural reinforcement strategy is FDM-print-only — filament selection (among PETG / ABS / ASA / CF-filled variants), wall thickness, infill density and pattern, print orientation, and printed-in ribs/stringers. No fibrous post-print reinforcement, no resin layup, no epoxy work.
**Rationale:** D011's original rationale had three legs: (1) aerodynamic stability analysis, (2) structural design, (3) fabrication. Leg (1) is invalidated by D010 (no flight) — a bench-only article has no aerodynamic stability requirement. Leg (2) becomes tractable in a bench-only load envelope (perturbation forces in the tens of newtons, well within FDM-printed material capability). Leg (3) is accessible via the 3D printer available per constraints §3.2. Two of three legs are gone or substantially weakened; the third (engineering effort) becomes a deliberate scope choice that the lead engineer accepts. The custom airframe supplies aerospace authenticity at a structural layer that no modified hobbyist kit can match (commercial kit body tubes are not semi-monocoque structures), and gives Demo 3 mass properties derived directly from CAD rather than measured from an opaque kit. The builder's motivation for the structural work is a real engineering input — D005's reasoning applies (sustained pull is the strongest completion predictor).
**Alternatives considered:**
- Reaffirm D011 (modified commercial airframe) — rejected; the locked-decision rationale is materially weaker post-D010, and the structural-engineering opportunity is one the lead engineer wants to take.
- Purpose-built bare avionics test rig (no airframe) — rejected; constraints §9 calls domain authenticity non-negotiable; a bare rig reads as embedded systems with aerospace vocabulary, not as rocketry.
- Hybrid (rig for development, airframe for final demo) — rejected; two integrations create integration debt that always surfaces close to the demo deadline; the right pattern is to integrate in the target geometry from the start.
- 3D-printed packaging shell without genuine structural design — rejected; pays most of the cost of going custom without the rigor that justifies it.

### D032 — Pi 5 role: bounded to display, C2, logging

**Decision:** The Pi 5's role is explicitly bounded to: telemetry reception (over radio or wired transport), logging of all received frames to local storage, dashboard UI rendering, and C2 command transmission (operator inputs to the MCU). HIL simulation runtime hosts on the dev PC, not on the Pi. Post-test analysis hosts on the dev PC, not on the Pi.
**Rationale:** An explicit role boundary is needed to prevent implementation-time scope creep that drags estimation or FDIR-adjacent logic across the Linux boundary in violation of D025. Resource-honest sizing: the dev PC (Ryzen 7, 64 GB DDR5, RTX 4080 Super) is dramatically more capable than the Pi 5 for compute-intensive HIL simulation and analysis; cramming those workloads onto an SBC wastes available capacity for no benefit. Pi-side logging is consistent with constraints v0.4's earlier scope lock (MCU-side onboard logging removed).
**Alternatives considered:**
- Display + C2 only with no logging — rejected; contradicts constraints v0.4.
- Pi as HIL simulation host — rejected; wastes the dev PC; crams compute-heavy simulation onto resource-constrained SBC; ties simulation development to Pi's environment.
- Pi as offline analysis host — rejected; strictly worse than dev PC for analysis work, which is iteration-heavy.

### D033 — Power architecture: split rails, common ground

**Decision:** The avionics use two separate power rails sharing a common ground. Rail A (clean): regulated low-noise 3.3V / 5V supplying MCU, both IMUs, barometer, and radio module. Rail B (servo): 5–6V high-current supplying the four servos. Power-on sequencing brings Rail A up first, then Rail B (to avoid servo twitching before the MCU initializes commanded positions).
**Rationale:** Four servos in simultaneous hard slew (the worst case during perturbation response, made possible by D030's four-fin layout) draw 4–8 amps peak. On a shared rail, those current spikes cause voltage sag that can brown out the MCU and inject high-frequency noise into sensor ADC references. Both effects produce artifacts that look like sensor faults to FDIR — false positives correlated with servo activity, which is poisonous for a project whose depth axis is fault detection. Topology-level isolation eliminates the failure class structurally rather than relying on heroic decoupling that doesn't fully solve sustained-current sag.
**Alternatives considered:**
- Single shared rail — rejected; brownouts and noise contaminate FDIR.
- Two fully separate batteries — rejected; overkill at bench scale; doubles power infrastructure for marginal additional isolation over the split-rail topology.
- Single rail with aggressive decoupling — rejected; capacitors handle high-frequency noise but cannot supply sustained current at servo loads; partial fix that doesn't solve the actual problem.

### D034 — Authoritative time source: MCU hardware timer

**Decision:** The MCU's hardware monotonic timer (microsecond resolution, dedicated peripheral) is the authoritative source of time for the entire system. Every sensor reading is paired with an MCU timestamp at the moment of measurement; every telemetry frame carries an MCU timestamp in its header. The Pi inherits MCU time for all logging, replay, and analysis math. The Pi's own system clock is used only for human-readable session naming and as observational metadata for transport-layer diagnostics (e.g., diagnosing radio link latency). Pi time is never used for any math involving event ordering, intervals, or correlation.
**Rationale:** The estimator's fusion math depends on consistent monotonic dt between measurements; the FDIR cross-check between IMU-1 and IMU-2 depends on accurate per-reading timestamps to find time-aligned pairs. Pi-side timestamps would be contaminated by radio transmission variance and Linux scheduling jitter — milliseconds of randomness that corrupt dt and break cross-session comparison. Hardware-monotonic time is the discipline every real avionics system follows for these reasons. The MCU's view of time is continuous and immune to OS-level interruption; the Pi's view of MCU time is downstream and can be interrupted without corrupting the record.
**Alternatives considered:**
- Pi timestamps on receipt — rejected; radio + Linux variance contaminate timestamps; replay analysis becomes noisy; cross-session comparison breaks.
- Both clocks with Pi as authoritative — rejected; same problem as Pi-only.

---

## 2026-05-18

### D035 — User-implemented estimation algorithm required; proprietary on-chip fusion excluded

**Decision:** The attitude estimation algorithm shall be implemented in full by the project author. Proprietary vendor-provided on-chip fusion firmware — including fusion modes such as the BNO055's NDOF mode — is excluded. The algorithm source code must be fully inspectable and compliant with the NASA JPL Power of 10 standard. An EKF or equivalent user-implemented algorithm (e.g., Madgwick/Mahony complementary filter) satisfies this requirement.
**Rationale:** Three independent reasons, each sufficient alone. First, JPL Power of 10 (D019) cannot be applied to firmware that cannot be read — a vendor fusion blob is a black box, and claiming compliance for it is indefensible to an aerospace reviewer. Second, the project's educational rationale (D005) requires that the fusion algorithm is built and understood; delegating it to a chip vendor defeats the purpose. Third, the FDIR cross-check (D024) requires raw sensor residuals — on-chip fusion may silently correct a fault before the FDIR module sees it, hiding the fault rather than exposing it.
**Alternatives considered:**
- BNO055 in NDOF fusion mode — rejected; all three rationale legs apply.
- BNO055 in raw output mode (fusion disabled) — rejected; raw mode is not the BNO055's primary design configuration; the M0 co-processor still runs in the background; better to select chips designed as raw sensors.
- One fusion IMU + one raw IMU — rejected; asymmetric outputs make the innovation gate incoherent (different noise models, different uncertainty characteristics).
- Complementary filter (Madgwick/Mahony) instead of EKF — not rejected; this is covered by "or equivalent" and satisfies all three rationale legs. Decision on specific algorithm deferred to implementation.

### D037 — Hardware selection proceeds ahead of LR-2 and LR-3 completion

**Decision:** MCU, IMU, and barometer selection proceeds now, before LR-2 and LR-3 are complete. LR-2 (MEMS IMU estimation accuracy) is downstream of IMU selection — it requires real datasheet noise specs to produce a meaningful accuracy target, not a class-level estimate. LR-3 (innovation gating design) is downstream of both LR-1 and IMU selection, as IMU noise specs inform the gating threshold. The functional requirements already written (REQ-SYS-003, REQ-EST-008, D024, D026, constraints §11.1–11.2) are sufficient to bound the hardware trade space without LR-2 or LR-3 complete. LR-1 (control loop rate) is independent of hardware selection and runs in parallel.
**Rationale:** The direction of dependency between LR-2 and IMU selection runs from selection to LR, not the reverse. Running LR-2 before IMU selection would mean analysing against generic class-level sensor specs — producing an estimate rather than a committable requirement. Selecting IMUs on functional grounds first (SPI-capable, heterogeneous families, raw output per D035, UAE-procurable per constraints §10.4) and then running LR-2 against real datasheet specs produces a defensible, traceable requirement. Additionally, hardware procurement has lead times that compound with delay; and component dimensions (MCU, IMU packages) are needed to dimension the avionics bay in CAD.
**Alternatives considered:**
- Complete LR-2 before IMU selection — rejected; LR-2 against class-level specs produces an estimate, not a committable requirement. Direction of dependency is inverted.
- Complete LR-1 before MCU selection — not required; constraints §11.1 already bounds the MCU trade space to STM32F4-class and above; LR-1 is unlikely to produce a loop rate requirement that an F4-class MCU cannot meet; "and above" in the constraint is the explicit safety margin for this uncertainty.

### D036 — Demonstration requirements held open

**Decision:** The demonstration deliverables described in Constraints §6 (Demo 1, Demo 3) are held open and are not treated as locked prescriptive requirements. The specific demo format, scope, and acceptance criteria will be defined once the core system is functional and its actual capabilities are known. The §6 descriptions are illustrative of intent, not binding deliverables.
**Rationale:** Locking demo requirements before the system works risks designing the demo around a script rather than around the system's genuine capabilities. A demo defined after the system is working can be shaped to show what the system actually does well — which is more authentic and more compelling than one defined speculatively upfront. The core deliverable (a working fault-tolerant GNC system) is unchanged; the presentation of that deliverable is deferred.
**Alternatives considered:**
- Lock Demo 1 as a prescriptive requirement now — rejected; Demo 1 as described in §6 may not reflect the most effective way to present the system once it is working. The constraint on the demo should be the system's real capabilities, not an upfront script.
- Drop demos entirely — rejected; a live demonstration remains a project deliverable; only the prescriptive specification of its content is deferred.

---

## 2026-05-19

### D038 — UAE frequency regulatory check: 433 MHz confirmed, radio band selected

**Decision:** TDRA frequency allocation confirms 433 MHz is available for use in the UAE. 433 MHz is the selected band. HolyBro SiK V3 433 MHz module ordered accordingly.
**Rationale:** Open question in constraints §5; blocker on radio selection per D028. With the regulatory finding confirmed against the TDRA frequency sheet, radio selection is unblocked and 433 MHz is the committed band.

### D039 — MCU selected: STM32F407ZGT6 core board

**Decision:** MCU is the STM32F407ZGT6 on a pre-assembled core board with 2.54 mm through-hole headers. H7 series evaluated and rejected in favour of F4.
**Rationale:** F407 meets all peripheral requirements: 3× SPI with DMA, 3× I2C, 6× UART with DMA, multiple PWM timer channels, 32-bit hardware monotonic timer (TIM2), FPU, 3.3 V I/O, 1 MB flash, 192 KB RAM. H7 series was rejected on one structural ground: the Cortex-M7 D-cache requires explicit cache clean/invalidate around every SPI DMA transaction — the failure mode lands directly on the highest-frequency code path (IMU reads). The F407 has no D-cache; DMA is deterministic. F407 is the proven platform for flight-controller-class applications (original Pixhawk, Betaflight F4); 168 MHz with FPU is sufficient for EKF, FDIR, control loop, and telemetry at required loop rates.
**Alternatives considered:**
- STM32H743ZI Nucleo — rejected; D-cache coherency on SPI DMA adds non-trivial complexity on the critical IMU read path; overkill for this project's math load.
- STM32H743ZI2 Nucleo — same rejection as H743ZI; additionally ~1,100 AED more expensive in UAE market with no substantive benefit.
- STM32H745ZI-Q / H755ZI-Q (dual-core H7) — rejected; inter-core complexity with no benefit for a single-threaded real-time application.

### D040 — IMU selection: GY-BMI160 (Bosch) + ICM-42688-P (TDK InvenSense)

**Decision:** Two heterogeneous IMU modules: (1) GY-BMI160 breakout (Bosch BMI160) and (2) ICM-42688-P breakout (TDK InvenSense ICM-42688-P). Each on its own dedicated SPI bus. Both modules have CS, MOSI, MISO, SCLK, and interrupt pins independently accessible.
**Rationale:** Satisfies D024 (dual heterogeneous IMUs), D026 (separate SPI buses), and D035 (raw output, no on-chip fusion). Bosch and TDK InvenSense are different silicon families — systematic failure modes are not shared, satisfying the fault-diversity requirement for FDIR. BMI160: 3200 Hz gyro ODR, 1600 Hz accel ODR, data-ready interrupt. ICM-42688-P: up to 32 kHz ODR, 0.0028 °/s/√Hz gyro noise density, data-ready interrupt. Both verified as breakout modules with all required pins accessible before selection.
**Alternatives considered:**
- BMI088 module (Bosch) — selected initially; became unavailable in UAE; replaced by BMI160 which meets all functional requirements.
- ICM-42688-P bare chip (LGA-14 package) — rejected; cannot be hand-soldered; breakout module required.
- Single IMU with redundant axis measurement — rejected; D024 locks dual heterogeneous; single silicon cannot satisfy fault-diversity requirement.

### D041 — Barometer selected: MS5611 via GY-63 module

**Decision:** Barometer is the MS5611-01BA03 on the GY-63 breakout module, I2C interface.
**Rationale:** Satisfies D027 (I2C barometer). MS5611 outputs absolute pressure and temperature via 24-bit ADC. Proven in rocketry and UAV applications. GY-63 exposes all required pins; PS pin pulled high by default (I2C mode). UAE-procurable.
**Alternatives considered:**
- BMP388 — capable; MS5611 selected on availability and established rocketry use.
- BMP280 — lower resolution; rejected in favour of MS5611.

### D042 — Radio selected: HolyBro SiK Telemetry Radio V3, 433 MHz

**Decision:** HolyBro SiK V3 pair, 433 MHz, 100 mW. Air module → MCU UART. Ground module → ground station Pi via USB. Operates as transparent serial bridge.
**Rationale:** Satisfies D028 (radio to MCU via UART+DMA), D029 (dual MCU↔Pi links), D020 (bidirectional telemetry). 433 MHz UAE-legal per D038. SiK transparent serial imposes no proprietary framing on the application layer. UART is DMA-compatible. Matched pair eliminates RF tuning and antenna matching work. 100 mW at 433 MHz is more than sufficient for bench demo range.
**Alternatives considered:**
- Bare LoRa module (SX1276) — more configurable but requires manual RF bring-up and antenna matching; SiK V3 is a proven matched pair with open firmware.
