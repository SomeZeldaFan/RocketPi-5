# Decisions Log

Append-only. Do not edit past entries. If a decision is later overturned, add a new entry referencing the prior decision and explaining why it's being changed. The point of this log is not to revisit decisions; it's to remember *why* they were made when something later seems to contradict them.

Format: **Date — Decision — Rationale — Alternatives considered.**

---

## 2026-05-11

### D001 — Adopt phased requirements-elicitation approach

**Decision:** Project follows Phase 0 (requirements scoping) → Phase 1 (detailed requirements + architecture) → Phase 2+ (build, test, integrate). No concept-locking until requirements and constraints are documented.
**Rationale:** Real engineering does not jump from mission need to solution. Premature concept lock is the dominant failure mode for ambitious student projects.
**Alternatives:** "Just pick a project and start" — rejected as the failure mode we're trying to avoid.

### D002 — Time budget set at ~400 hours (350–450 range)

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
