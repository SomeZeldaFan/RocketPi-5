# Airframe Design — Comprehensive Reference (Head to Tail)

**Status:** Primary airframe design document. This is the single authoritative, words-only description of the RocketPi-5 airframe — geometry, joints, datums, avionics housing, control surfaces, power/cabling, and fabrication. If the CAD and this document disagree, the discrepancy is a defect to be resolved, not a silent CAD win.
**Last updated:** 2026-06-17.
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
| D-RAD | **Radial / clocking** | flange face + bolts (loose) | No precise radial datum at present; a centring spigot is an exploratory option (§12.2). Seam steps accepted and finished under paint. |

---

## 4. Section Module

Each section is an analytic cylinder: OD 162, bore 150, **6 mm wall**, 280 mm long, with the 15 mm internal flange at one or both ends per its role in the stack. The bolt ring (R67.5) sits in the flange. The skin is thin; the **flange ring, insert bosses, servo brackets, and fin roots are locally thickened** as needed — thin skin, thick features.

The 12 mm wall of the original design was driven partly by embedding aluminium stringer bores; with the stringers removed (D058), the wall is set only by bending/denting margin and print robustness, and 6 mm at this diameter is comfortably stiff (§5.3). Sections are nominally identical so the joint standard is uniform up the stack (subject to §12.12).

---

## 5. Section-to-Section Joint Standard

The joint uses two features: the **flange faces** set axial position, and the **8 bolts** clamp the joint and react its (small) bending moment. There are no stringers (D058). Radial centring is currently loose (flange face + bolt clearance); a centring spigot is an exploratory option (§12.2), and seam steps are accepted because the skin is painted.

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
| Radial centring / clocking | flange face + bolts (loose) | no precise datum yet; spigot is an exploratory option (§12.2); seam steps finished under paint |

---

## 6. Avionics Section and Capsule (D057)

The forward-most section (the one before the nose) is the **avionics section**. The avionics itself is packaged as a **self-contained, detachable capsule** — its own subassembly carrying the MCU, both IMUs, the magnetometer, the barometer, the radio, batteries, power conditioning, and every mounting point, each component bolted/screwed to the capsule.

Workflow intent:

1. The capsule is built and bench-tested as a standalone unit, off the airframe.
2. It drops into the avionics section and is secured by an internal fastening mechanism (**TBD §12.6**).
3. Power and PWM lines to the canard servos and the servo power rail are connected **after** the capsule is seated (mating method **TBD §12.7**).
4. The section carries **service panels** (count/location/latch **TBD §12.5**) so the capsule can be serviced in place without removing the nose for routine work.

PLA is RF-transparent, so the 433 MHz radio antenna can live inside the capsule/section — no external antenna penetration is needed, which suits the fastener-free skin. In-place flashing/debugging (the wired link) is a capsule-level concern routed out through a service panel.

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
2. **Centring spigot/register — EXPLORATORY TODO (builder to explore).** With the stringers gone, the joint has no precise radial datum. A short stepped lip on one flange nesting into a counterbore on the other would self-centre each joint, keep the skin flush, and make assembly drop-together — and, with no stringers, it would not be over-constrained. Before adopting: explore the geometry (lip height/clearance, which flange carries the male feature), what assembly looks like (insertion clearance, paint-thickness allowance), and whether it's worth it given the skin is painted. Decide adopt/skip.
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
