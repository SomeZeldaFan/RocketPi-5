# Airframe Design — Comprehensive Reference (Head to Tail)

**Status:** Primary airframe design document. This is the single authoritative, words-only description of the RocketPi-5 airframe — geometry, joints, datums, avionics housing, control surfaces, power/cabling, and fabrication. If the CAD and this document disagree, the discrepancy is a defect to be resolved, not a silent CAD win.
**Last updated:** 2026-06-18.
**Units:** millimetres unless stated.
**Governing decisions:** D031 (custom semi-monocoque airframe), D044 (metal reinforcements in scope), **D055** (segmentation + section-joint standard), **D056** (control surfaces: static aft fins + actuated canards), **D057** (avionics capsule + dedicated servo rail), **D058** (stringers removed, wall thinned to 6 mm), **D059** (control surfaces are visual/commanded-response only on the static bench).
**Companion note:** `docs/cad/aft-most-section.md` carries the parametric insert/clamp formulas and the original STEP read-out; this document is the airframe-level reference and points there rather than duplicating the formulas. (That note predates D058 — its OD/wall and stringer figures are superseded here.)

---

## 1. Purpose and Scope

This document describes the entire airframe well enough to lock the design in words before committing print time. It is comprehensive by intent: section geometry, the section-to-section joint, the alignment/datum scheme, the avionics bay capsule, control-surface architecture, power and cabling, and the fabrication plan. Anything not yet decided lives in §12 (Open / To-Be-Locked) rather than being left implicit.

The airframe is bench-only (Constraints §4, §8). Load envelope: self-weight, handling, and tens-of-newtons perturbation forces during demos. No flight loads, no aerodynamic stability requirement in service — and (D059) no aerodynamic control authority either.

---

## 2. Top-Level Configuration

The airframe is a stack of identical-cross-section printed cylindrical **sections**, joined by an internal twin-flange bolted joint, capped by a nose cone forward and a (cosmetic) motor-mount interface aft. The outer skin is a **continuous, fastener-free surface** — every fastener is internal. Primer, paint, and a decal go on afterward, so minor seam steps at the joints are acceptable and will be filled/finished.

**First article = a minimal stack (D058)** — build the fewest sections that prove every hard interface, then extend for length/pizzazz later:

```text
            ┌───────────┐
            │ nose cone  │   retention mechanism: TBD (§12.5)
            ├───────────┤
            │ forward    │   AVIONICS SECTION — houses the detachable
            │ (avionics) │   avionics capsule; service panels; canard servos
            │ section    │   (canard body station TBD §12.1)
            ├───────────┤ ← section joint (twin flange + 8 bolts)
            │ one empty  │   structural / length only
            │ section    │
            ├───────────┤ ← section joint
            │ aft-most   │   4 static aft fins
            │ section    │
            ├───────────┤
            │ cosmetic   │   purely visual motor-mount (no propulsion;
            │ motor mount│   commercial motors only as static prop, never fired)
            └───────────┘
```

Once the joint, capsule fit, canard mount, and nose mating are proven on this stack, additional **empty** sections can be inserted for length. Total length is therefore deliberately open (§12.10).

---

## 3. Master Parameter and Datum Table

Locked values are stated; provisional values are marked **(prov.)** and resolved in §12. "Datum role" states what each feature is responsible for locating.

| # | Parameter | Value | Datum role / notes |
|---|---|---:|---|
| G1 | Section length | 280 mm | Capped by ~300 mm print build volume |
| G2 | Outer diameter (OD) | **162 mm** | Was 174; reduced by the wall thinning (D058) |
| G3 | Outer radius | 81 mm | — |
| G4 | Inner bore diameter | **150 mm** | **Hard requirement** — preserved working bore / reach-in access |
| G5 | Inner bore radius | 75 mm | — |
| G6 | Skin wall thickness | **6 mm** | Thinned from 12 mm (D058); flange and mounting bosses stay locally thick |
| G7 | Internal flange radial width | 15 mm | Projects inward from bore (R75) to opening (R60) |
| G8 | Flange opening (ID) | 120 mm | Hand/tool reach for internal assembly & service |
| G9 | Aft-most end opening | Ø120 | Cosmetic motor-mount interface — role TBD (§12.9) |
| J1 | Bolt ring count | 8 | Provides axial clamp **and** reacts joint bending (stringers removed) |
| J2 | Bolt pitch radius | 67.5 mm | Centred in the 15 mm flange (R60→R75) |
| J3 | Bolt size | M3 **(prov.)** | M3 vs M4 open (§12.11) |
| J4 | Thin-flange hole (upper) | screw clearance, Ø≈3.3 for M3 **(prov.)** | Pass-through; **current CAD carries Ø5.7 placeholder** |
| J5 | Thick-flange hole (lower) | stepped insert pilot (see §5.2) | Heat-set insert seat + screw-tip relief |
| J6 | Thin-flange depth (upper) | ~3 mm **(prov.)** | Pass-through plate |
| J7 | Thick-flange boss depth (lower) | 15 mm | For boss strength; **not** the insert length |
| D-AX | **Axial datum** | flange face | Thin-flange face seats on thick-flange face |
| D-CLAMP | **Clamp + bending** | the 8 bolts | Clamp the joint and react the (small) bench bending moment |
| D-RAD | **Radial / clocking** | flange face + bolts (loose) today; flange-tab datum is the agreed direction | No precise datum locked yet. Agreed direction: flange-integrated asymmetric locating tabs between the bolts (one tight tab centres + clocks, extras foolproof), bore kept clean — §12.2. Round spigot kept as the higher-precision alternative. Seam steps accepted, finished under paint. |

---

## 4. Section Module

Each section is an analytic cylinder: OD 162, bore 150, **6 mm wall**, 280 mm long, with the 15 mm internal flange at one or both ends per its role in the stack. The bolt ring (R67.5) sits in the flange. The skin is thin; the **flange ring, insert bosses, servo brackets, and fin roots are locally thickened** as needed — thin skin, thick features.

The 12 mm wall of the original design was driven partly by embedding aluminium stringer bores; with the stringers removed (D058), the wall is set only by bending/denting margin and print robustness, and 6 mm at this diameter is comfortably stiff (§5.3). Sections are nominally identical so the joint standard is uniform up the stack (subject to §12.12).

---

## 5. Section-to-Section Joint Standard

The joint uses two features: the **flange faces** set axial position, and the **8 bolts** clamp the joint and react its (small) bending moment. There are no stringers (D058). Radial centring + clocking is loose today (flange face + bolt clearance); the agreed direction is **flange-integrated asymmetric locating tabs between the bolts** (one tight tab centres + clocks, extras foolproof; geometry not yet locked — §12.2), with a round centring spigot kept as the higher-precision alternative. Seam steps are accepted because the skin is painted.

Orientation convention: viewing the rocket nose-up, the **lower** section (section A) presents a **thick flange** at its top; the **upper** section (section B) presents a **thin flange** at its bottom. Screws are driven downward from inside section B, through the thin-flange clearance holes, into heat-set inserts seated in section A's thick flange.

### 5.1 Twin flange + bolt ring

Eight M3 screws on a 67.5 mm pitch-radius ring clamp the joint. The thin flange is the pass-through side (clearance holes); the thick flange (15 mm boss) is the threaded reaction side (heat-set inserts). Clamp force is generated by the **screw head bearing on the thin flange**, with the threads pulling against the brass insert — never by the screw bottoming out. The 15 mm boss depth is chosen for boss strength and is independent of the insert length.

### 5.2 Stepped heat-set insert pilot (the depth solution)

The thick-flange hole is a **stepped (counterbored) pilot**, which simultaneously (a) gives the screw tip clearance below the insert and (b) makes a diameter-step shoulder that is the insert's repeatable bottom datum, so every insert seats at the same depth regardless of installer feel:

```text
            screw from above
                  |
   thin flange    v   ~3 mm, Ø = screw clearance (≈Ø3.3 for M3)
  ┌───────────────────────────┐  ← flange faces mate (AXIAL DATUM)
  │  Stage 1: insert pilot     │  Ø = insert pilot (≈Ø4.0 M3), depth = L_insert
  │  ███ brass insert ███      │
  ├───────────────────────────┤  ← SHOULDER (diameter step) = insert bottom stop
  │  Stage 2: screw-tip relief │  Ø = screw clearance (≈Ø3.3), < insert body OD
  │  (clearance; screw floats) │
  └───────────────────────────┘  thick-flange / 15 mm boss
```

Sizing rules:

- **Stage-1 diameter** = the chosen insert's datasheet pilot diameter (insert-specific; *not* derived from nominal screw size). See `aft-most-section.md` §6.2.
- **Stage-1 depth** = `L_insert` (the insert's own length), so "insert bottomed on the shoulder" coincides with "insert top flush." Deeper ⇒ insert sits below flush; shallower ⇒ insert stands proud.
- **Stage-2 diameter** = screw clearance (e.g. ≈Ø3.3 for M3), strictly **smaller than the insert body OD** so the insert physically cannot drop into Stage 2 — its bottom face lands on the shoulder.
- **Stage-2 depth** = deep enough that the screw tip never bottoms (Stage 2 is clearance, not a stop), with a small margin. Also serves as relief for any displaced plastic during heat-set installation.
- **Screw length** = thin-flange thickness + full insert engagement (≤ `L_insert`) + a tip-clearance gap that stays short of the Stage-2 floor. See `aft-most-section.md` §6.4.

The 15 mm boss comfortably contains Stage 1 (≈`L_insert`) + Stage 2 (a few mm) with material to spare.

### 5.3 Joint bending and torsion (no stringers)

The aluminium stringers were removed (D058) because bench loads do not justify them:

- **Bending strength.** The worst handling/perturbation moment is ~tens of N·m (≈50 N at the top of a ~0.9 m article ≈ 45 N·m). The bolted flange reacts this through the tension-side inserts on the 135 mm bolt circle — a couple hundred N·m before insert pull-out — i.e. several times the worst bench moment, with no stringer contribution.
- **Bending stiffness.** A 162 mm tube with a 6 mm wall has a section moment of inertia ≈ 9×10⁶ mm⁴; the same 0.9 m cantilever under 50 N deflects ≈ 0.4 mm. The tube is hugely over-stiff for bench loads — 12 mm of PLA at this diameter does not meaningfully bend.
- **Torsion.** Eight M3 bolts in a ring carry hand-handling torque in shear with ample margin.

The new limiting failure mode is **local denting/buckling** of the thinner skin under a point load, not global bending. 6 mm gives good denting margin; keep a solid perimeter count, and add an occasional printed internal ring stiffener only if a coupon dents too easily (§12.14).

### 5.4 Datum hierarchy (summary)

| Job | Feature | Notes |
|---|---|---|
| Axial position | flange faces | flat faces meet; bolts pull them together |
| Clamp + bending | 8 bolts | clamp the joint and react bench bending |
| Radial centring / clocking | flange face + bolts (loose) today | a dedicated datum is the agreed direction — flange-integrated asymmetric locating tabs between the bolts (one tight = centres + clocks; extras = foolproofing), bore kept clean; geometry not yet locked (§12.2) |

---

## 6. Avionics Section and Capsule (D057)

The forward-most section (the one before the nose) is the **avionics section**. The avionics itself is packaged as a **self-contained, detachable capsule** — its own subassembly carrying the MCU, both IMUs, the magnetometer, the barometer, the radio, batteries, power conditioning, and every mounting point, each component bolted/screwed to the capsule.

Workflow intent:

1. The capsule is built and bench-tested as a standalone unit, off the airframe.
2. It drops into the avionics section and is secured by an internal fastening mechanism (**TBD §12.6**).
3. Power and PWM lines to the canard servos and the servo power rail are connected **after** the capsule is seated (mating method **TBD §12.7**).
4. The section carries **service panels** (count/location/latch **TBD §12.5**) so the capsule can be serviced in place without removing the nose for routine work.

PLA is RF-transparent, so the 433 MHz radio antenna can live inside the capsule/section — no external antenna penetration is needed, which suits the fastener-free skin. In-place flashing/debugging (the wired link) is a capsule-level concern routed out through a service panel.

### 6.1 Component mounting — direction (not yet locked)

Direction agreed (depth deferred to a dedicated avionics pass). Components are **not** screwed directly into the PCB or into raw airframe plastic. Instead each component gets a **purpose-printed clamshell caddy**: caliper-measure the part, model a cavity slightly oversized, split it into two halves screwed shut (heat-set inserts), with **purpose-routed cable holes / strain relief**. Each caddy presents a **standardised external mounting interface** to the capsule (a common footprint or rail/grid), so any caddy bolts anywhere — this is the modular realisation of D057's "slots for avionics to bolt onto."

Per-type rules (the caddy is not one-size-fits-all):

- **Battery (LiPo) — special case.** No rigid compression; a swelling pouch in a hard box is a puncture/fire path. Looser cradle with **swelling clearance, venting, no sharp/screw features against the pouch, easy removal/disconnect.**
- **Thermal.** PLA softens ~50–60 °C. Vent the warm parts — **power regulators, radio on TX, battery.** The MCU itself is low-power.
- **IMUs — rigid + precisely aligned.** The estimator assumes a fixed, known sensor-to-body transform, so IMU caddies are tight, rigid, axes deliberately aligned, with the two IMUs in a known relative orientation. Compliant capture is fine for the battery, not for these.
- **Magnetometer — magnetically clean (placement is CAD-relevant).** Put the BMM150 at the **far / nose end** of the bay, using the cylinder's axial length to separate it from the battery and the servo-power cabling (its own I2C bus is just two signal wires, so routing it far is cheap). Use **brass/nylon (non-magnetic) fasteners** near it, and route power away / as twisted pairs. (Reframe for later: *fixed* metal can be calibrated out — the real disturbance is *time-varying* servo current; full calibrate-and-characterise strategy deferred.) Ties to RISK-001 and to the §5 magnetic-disturbance failure mode (clean nominally; disturbed deliberately for the demo).

Also: preserve **connector access** for the parts you actually plug into (MCU USB/SWD for flashing, servo headers, battery plug) so the clamshell doesn't bury them; add a **crush rib or TPU/foam pad** so the fit is snug-not-stressed and vibration-damped; and model **reliefs for tall parts / pin headers**, not just an L×W×H box.

**Bay occupants & power distribution.** Besides the component caddies, the bay carries a permanent **power-distribution / regulator board** (Rail A 3.3 V + Rail B 5 V, per D033) as its own mounted board/caddy — this is the permanent home of the shared rails (the source feeds a bus; every device taps it). The **breadboard is a bench bring-up tool only and is never installed in the airframe**: Dupont-on-breadboard is for desk validation of each sensor, after which the proven wiring is rebuilt permanently (soldered / crimped-connectorised) in the bay. Allocate caddy space + a mount for the power board in the bay layout.

To think about in the avionics pass: the standard caddy→capsule interface (footprint vs. rail/dovetail vs. insert grid), capsule volume budget, mag/cable routing layout, and which caddies need frequent-access (battery, MCU) vs. set-and-forget.

---

## 7. Control Surfaces (D056, D059)

Eight aerodynamic surfaces total, in two functionally distinct sets:

- **Four static aft fins.** Fixed, non-actuated, 90° spacing in "+", at the aft-most section. Passive/representative stabilisers; they do not move and carry no PWM channel. Attachment method **TBD §12.4**.
- **Four actuated canards.** Forward, 90° spacing, each driven by its own servo on its own dedicated PWM channel from the MCU. The canards are the system's control-authority surfaces in the *software* sense — the closed-loop control law, the mixing matrix, deflection limits, and the 3-of-4 actuator-fault reconfiguration all act on the canards.

**No physical control authority on the bench (D059).** The article is static — there is no airflow — so deflecting a canard exerts essentially no moment on the airframe. The demo is therefore **commanded-response, not closed physical control**: perturb the stand by hand, the estimator senses the attitude error, and the canards throw in the correct restoring sense. **Demo mode deliberately exaggerates the deflection** (REQ-CTL-004) so the reaction reads as a strong, obvious "violent" response for the audience. This is the honest framing — the value on a static bench is the visible, correct *commanded* reaction plus the live estimator/health telemetry, not vehicle motion. Requirements `REQ-CTL-001` and `REQ-CTL-006` are reworded accordingly (the verifiable behaviour is the commanded canard response, not physical attitude restoration).

This supersedes the earlier "four actuated aft fins" intent (D030); the fault-tolerance rationale of D030 (four independent actuated surfaces ⇒ survive one fault on three) is preserved and now applies to the canards.

Canard body station, pivot/hinge design, and servo bracket/linkage are **TBD §12.1, §12.3**.

---

## 8. Power and Cabling

The canard servos are powered from a **dedicated 5 V rail (Rail B), separate from the MCU's 3.3 V rail (Rail A)** — D057, per the existing power topology in D033 / REQ-PWR-004 and architecture §2.5. Four servos in simultaneous hard slew draw several amps in transients; isolating their rail prevents brown-out and ADC-reference noise on the MCU that would otherwise present to FDIR as false sensor faults.

Cable routing from the capsule to the canard servos and to the servo rail crosses from the avionics section to the canard station; the connector/mating approach (hand-connect through a service panel vs. blind-mate on capsule insertion) is **TBD §12.7**.

---

## 9. Print and Fabrication Strategy

- **Material:** PLA+ for this article (see §10).
- **Print orientation matters at the joint.** If a section prints axis-vertical, the flange bolt holes load the inter-layer bond in pull-out/tension — the weak FDM direction. Orientation is a first-class joint-strength decision, to be locked alongside the joint, not after.
- **Coupon-before-commit (strongly recommended, and consistent with architecture §3.8 and Constraints §10.7).** Before committing a 280 mm section, print a **single-joint test coupon**: two short flange rings carrying the full 8-bolt + stepped-insert geometry, at the 6 mm wall. ~1 h of printing validates insert seat depth, clamp behaviour, and skin denting. This protects the end-of-month deadline far more than it costs.
- **Cosmetic finishing is in scope.** Primer + paint + decal (e.g. an armed-forces logo) are cosmetic only; they are **not** the structural "chemical coating" excluded by Constraints §4 (that exclusion targets resin/fibreglass structural reinforcement). Seam steps at joints are filled/sanded under primer.
- **Filament/print-time reality (improved).** Thinning the wall 12 → 6 mm and reducing OD 174 → 162 cuts wall volume by well over half, and the minimal first stack is only a few sections — a large improvement on the original mass/print/cost picture (risk R-AF-03).

### 9.1 CAD modelling methodology

How the model is built (tool: Onshape, parametric — the existing `aft-most.step` was an Onshape export):

- **Parametric, master-parameter-driven.** The §3 table values become **named variables** (section length, OD, bore, wall, flange width, bolt PR/count, fit clearances) so a change propagates instead of being re-drawn. FDM fit allowances (insert pilot, tab/pocket clearance, bolt clearance, slip fits) are variables, *tuned on the coupon then locked*.
- **One canonical section as a base part.** Model a single section (shell + flange + bolt ring + joint mating features) once; the avionics / empty / aft-most sections are **derived variants/configurations** of it (add service panels, fin roots, canard/servo mounts), not re-modelled from scratch.
- **The joint modelled once, on the section's mating faces** (top = male flange + locating tabs; bottom = female flange + pockets) so **every section mates with every other** — this *is* the uniform joint standard (D055). Single source of truth; never re-draw the joint per section.
- **Coupon-first.** Model + print the single-joint coupon, measure with the caliper, **tune the fit-clearance variables**, *then* propagate the validated numbers into the full sections. Caliper-in-the-loop, not one-shot.
- **Components → caddies, measurement-driven.** Caliper each board into a parametric **envelope solid** (measured L×W×H + reliefs for tall parts / pin headers / connectors); generate the caddy around the envelope (oversized cavity + clamshell split + the standard mounting footprint + cable holes). Templated so the ~7 components reuse one caddy generator.
- **Top-down skeleton/layout.** A master layout (rocket centreline, section stack positions, canard/fin body stations, avionics-bay envelope, CG target) that the sections and caddies reference, so body stations and mounts stay coordinated.
- **Design-for-FDM baked in.** Thin shell + local bosses; fillet stress risers (tab/pocket corners); chamfer/lead-in on every mating feature; print-orientation-aware feature placement (bolt holes vs. layer direction); split at the ~300 mm build-volume (the 280 mm section *is* that split).
- **Track mass + CG from CAD mass properties** as the avionics load is added (top-heavy watch — informs the test-stand pivot).
- **Design-freeze discipline** (risk register: CAD iteration creep) — freeze the joint standard after the coupon validates it, before committing full-section prints.

---

## 10. Accepted Limitations / Scope Notes

- **PLA creep — accepted, out of scope for this article.** PLA+ creeps under sustained clamp load, so bolt tension will relax over time. This is deliberately accepted: the article is bench-only, low-load, and intended for sub-month engagement. If the airframe is to live beyond ~a month, it will be redesigned in a better filament/material rather than mitigated here. Recorded so the reasoning is not lost; not tracked as a live risk.
- **No aerodynamic control authority (D059).** See §7 — the surfaces are for visible commanded response, not vehicle control.

---

## 11. Cross-References

- `docs/cad/aft-most-section.md` — parametric insert/clamp formulas (OD/wall/stringer figures there predate D058).
- `hardware/cad/aft-most.step` — original section geometry (predates D058).
- `docs/05-architecture.md` §3 — airframe structural architecture (summarised; points here).
- `docs/03-decisions-log.md` — D055, D056, D057, D058, D059 (and D030, D031, D033, D044).
- `docs/04-requirements.md` — REQ-SYS-005/013, REQ-CTL-001/002/003/004/005/006, REQ-FDR-001/007/009, REQ-PWR-004, REQ-GCS-005.

---

## 12. Open / To-Be-Locked Register

These do not block printing the cylindrical sections (the section + joint geometry is fully specified above), but a head-to-tail lock requires them. Each should be resolved and folded into the relevant section above.

1. **Canard body station** — axial location of the canards (in/near the avionics section, or a dedicated section); fixes where the servos mount.
2. **Section clocking + centring — DIRECTION AGREED, geometry not yet locked (dedicated pass next).** The joint needs a way to (a) centre the two skins and (b) clock the sections so every section's 12 o'clock lines up (canards over canards, fins, panels, wiring) and the bolt holes align. **Direction:** keep the central bore/opening clean (no inner-edge ring) by putting the locating features **out on the 15 mm flange face, in the arcs between the 8 bolts** — male rectangular tabs into matching female pockets, arranged at **unequal (asymmetric) spacing** so exactly one rotation assembles (unique clock + poka-yoke; the rule is simply "no rotational symmetry in the tab pattern").
   - **Constraint principle (avoid the stringers' over-constraint):** exactly **one** tab is a precise fit — a rectangular tab trapped in a pocket fixes x, y *and* θ, i.e. it centres *and* clocks from one feature. Any **extra** tabs are **clearance-fit foolproofing** only (they still force the single orientation, but don't fight the locator). Bolts stay loose clearance (clamp only); flange faces set axial + tilt. Result is an exactly-constrained joint.
   - **Alternative kept on the table:** a round **centring spigot** at the bore edge gives the best all-around skin concentricity (flushest seam) but adds an inner ring and touches the opening. The flange-tab route trades a little centring precision for a clean bore and no extra part — acceptable since seams are primed/filled.
   - **To resolve in the geometry pass:** number of tabs (one tight + N loose), exact asymmetric pattern (verify no rotational symmetry), tab/pocket fit clearance (coupon-tuned), **fillet pocket corners** (FDM crack-starter), lead-in chamfers for blind assembly, keep tabs clear of the bolt-driver path (~50 mm of arc between bolts), which flange carries the male tabs, and whether to also keep a round spigot for skin flushness. Slip fit so sections pull straight apart for service.
3. **Canard pivot/hinge** — printed pivot vs. bearing; canard shaft material; bearing seat if any.
4. **Static aft-fin attachment** — integral print vs. bolted root; root geometry; interaction with the Ø120 aft opening.
5. **Nose cone retention/removal** — mechanism with no external fasteners.
6. **Service panels** — count, location, and fastener-free retention/latch type.
7. **Avionics capsule fastening** — how the capsule is secured inside the avionics section (internal collar/clamp/threaded ring).
8. **Cable mating** — capsule ↔ canard servos ↔ servo power: hand-connect through a panel vs. blind-mate on insertion.
9. **Aft-most Ø120 end opening role** — confirm the cosmetic motor-mount interface; model it after a recognisable motor (e.g. a KSP-style engine) for visual appeal.
10. **Total length / additional empty sections** — extend the minimal first stack with empty sections for length once the hard interfaces are proven.
11. **Screw size** — M3 vs M4 for the bolt ring; drives J3/J4/J5 and the §5.2 numbers. Replace the current Ø5.7 CAD placeholder once chosen.
12. **Joint-standard uniformity** — whether the avionics and aft-most sections use the identical joint standard or deviate.
13. **Heat-set insert model** — exact insert + vendor drawing (sets the §5.2 Stage-1 diameter and `L_insert`).
14. **Wall-denting coupon at 6 mm** — confirm the 6 mm skin resists handling point-loads; add a ring stiffener only if needed.

---

## 13. Change Log

- **2026-06-17 (a)** — Document created. Section/joint geometry, stepped-insert depth solution, stringer datum scheme, avionics capsule, static-fin + canard control architecture, power/cabling, and fabrication plan captured. Decisions D055–D057 logged. Canard requirements re-pointed.
- **2026-06-17 (b)** — D058: aluminium stringers removed (8-bolt flange carries bench loads with ample margin); skin wall thinned 12 → 6 mm with bore 150 preserved (OD 174 → 162); centring spigot demoted to an exploratory TODO (§12.2). D059: control surfaces are visual/commanded-response only on the static bench; REQ-CTL-001/006 reworded. First article set to a minimal stack (avionics / one empty section / aft-most / cosmetic motor mount), extend later.
- **2026-06-17 (c)** — Directions captured (not locked, depth deferred): §12.2 section clocking + centring via flange-integrated asymmetric locating tabs between the bolts (one tight tab centres + clocks, extras foolproof, bore kept clean; round spigot kept as the higher-precision alternative); §6.1 per-component clamshell caddies with a standardised caddy→capsule interface and per-type rules (battery swelling/venting, IMU rigid+aligned, magnetometer clean, thermal venting, connector access).
- **2026-06-18 (d)** — Audit + gap-fill: added §9.1 CAD modelling methodology (parametric master model, one canonical section + derived variants, joint modelled once on the mating faces, coupon-first fit-tuning, caliper-driven caddy generation, top-down skeleton, design-for-FDM, mass/CG tracking, design-freeze). §6.1: magnetometer **axial** placement (far/nose end, twisted pairs, non-magnetic fasteners) and the permanent power-distribution/regulator board as a bay occupant (breadboard is bench-only, never installed). §3/§5 clocking rows reconciled to the flange-tab direction. No design changes — documentation only.
