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
