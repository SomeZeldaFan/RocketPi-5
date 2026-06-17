# Airframe Design — Comprehensive Reference (Head to Tail)

**Status:** Primary airframe design document. This is the single authoritative, words-only description of the RocketPi-5 airframe — geometry, joints, datums, avionics housing, control surfaces, power/cabling, and fabrication. If the CAD and this document disagree, the discrepancy is a defect to be resolved, not a silent CAD win.
**Last updated:** 2026-06-17.
**Units:** millimetres unless stated.
**Governing decisions:** D031 (custom semi-monocoque airframe), D044 (metal reinforcements in scope), **D055** (segmentation + section-joint standard), **D056** (control surfaces: static aft fins + actuated canards), **D057** (avionics capsule + dedicated servo rail).
**Companion note:** `docs/cad/aft-most-section.md` carries the parametric insert/clamp formulas and the aft-most-section STEP read-out; this document is the airframe-level reference and does not duplicate those formulas — it points to them.

---

## 1. Purpose and Scope

This document describes the entire airframe well enough to lock the design in words before committing print time. It is comprehensive by intent: section geometry, the section-to-section joint, the alignment/datum scheme, the avionics bay capsule, control-surface architecture, power and cabling, and the fabrication plan. Anything not yet decided lives in §12 (Open / To-Be-Locked) rather than being left implicit.

The airframe is bench-only (Constraints §4, §8). Load envelope: self-weight, handling, and tens-of-newtons perturbation forces during demos. No flight loads, no aerodynamic stability requirement in service.

---

## 2. Top-Level Configuration

The airframe is a stack of identical-cross-section printed cylindrical **sections**, joined by an internal twin-flange bolted joint and bridged by inserted aluminium stringers, capped by a nose cone forward and a motor-mount-like interface aft. The outer skin is a **continuous, fastener-free surface** — every fastener is internal.

```text
            ┌───────────┐
            │ nose cone  │   retention mechanism: TBD (§12.4)
            ├───────────┤
            │ forward    │   AVIONICS SECTION — houses the detachable
            │ (avionics) │   avionics capsule; service panels; canard servos
            │ section    │   (canard body station TBD §12.1)
            ├───────────┤ ← section joint (twin flange + 8 bolts + 4 stringers)
            │ section    │
            ├───────────┤ ← section joint
            │   ...      │   (~6 sections total for a ~1.8 m article — TBD §12.9)
            ├───────────┤ ← section joint
            │ aft-most   │   4 static aft fins; Ø120 aft opening (motor-mount
            │ section    │   interface — role TBD §12.8)
            └───────────┘
```

Total length and section count are a target, not yet locked: ~1.8 m ⇒ ~6 sections + nose + aft interface ⇒ ~6 joints. See §12.9.

---

## 3. Master Parameter and Datum Table

Locked values are stated; provisional values are marked **(prov.)** and resolved in §12. "Datum role" states what each feature is responsible for locating.

| # | Parameter | Value | Datum role / notes |
|---|---|---:|---|
| G1 | Section length | 280 mm | Capped by ~300 mm print build volume |
| G2 | Outer diameter (OD) | 174 mm | Continuous skin; no external fasteners |
| G3 | Outer radius | 87 mm | — |
| G4 | Inner bore diameter | 150 mm | Preserved working bore for interior access |
| G5 | Inner bore radius | 75 mm | — |
| G6 | Wall thickness | 12 mm | Set partly by the embedded Ø5 stringer bore |
| G7 | Internal flange radial width | 15 mm | Projects inward from bore (R75) to opening (R60) |
| G8 | Flange opening (ID) | 120 mm | Hand/tool reach for internal assembly & service |
| G9 | Aft-most end opening | Ø120 | Motor-mount-like interface — role TBD (§12.8) |
| J1 | Bolt ring count | 8 | Provides **all** axial clamp load |
| J2 | Bolt pitch radius | 67.5 mm | Centred in the 15 mm flange (R60→R75) |
| J3 | Bolt size | M3 **(prov.)** | M3 vs M4 open (§12.10) |
| J4 | Thin-flange hole (upper) | screw clearance, Ø≈3.3 for M3 **(prov.)** | Pass-through; **current CAD carries Ø5.7 placeholder** |
| J5 | Thick-flange hole (lower) | stepped insert pilot (see §5.2) | Heat-set insert seat + screw-tip relief |
| J6 | Thin-flange depth (upper) | ~3 mm **(prov.)** | Pass-through plate |
| J7 | Thick-flange boss depth (lower) | 15 mm | For boss strength; **not** the insert length |
| S1 | Stringer count | 4 | At 12/3/6/9 o'clock |
| S2 | Stringer bore diameter | Ø5.0 | Slip fit to rod; ream/drill to size post-print (§5.3) |
| S3 | Stringer pitch radius | 81 mm | Mid-wall (R75→R87) |
| S4 | Stringer material | CNC solid aluminium | Per D044 (metal reinforcement in scope) |
| S5 | Stringer length | 200 mm | Spans the joint, not the section |
| S6 | Stringer engagement, lower section | 100 mm | Bottoms in lower bore |
| S7 | Stringer engagement, upper section | 100 mm | Into upper socket |
| S8 | Upper stringer socket depth | 101 mm | 1 mm axial relief → stringer carries **no** clamp |
| D-AX | **Axial datum** | flange face | Thin-flange face seats on thick-flange face |
| D-RAD | **Radial + clocking datum** | the 4 stringers | Stringers locate the joint; bolts have clearance |
| D-CLAMP | **Clamp** | the 8 bolts | Bolts clamp only; do not locate |

---

## 4. Section Module

Each section is an analytic cylinder: OD 174, bore 150, 12 mm wall, 280 mm long, with the 15 mm internal flange at one or both ends per its role in the stack. Embedded in the wall at R81 are the four Ø5 stringer bores (12/3/6/9 o'clock). The cross-section is internally self-consistent: the bolt ring (R67.5) sits in the flange; the stringer bores (R81) sit in the wall; they do not interfere.

Sections are nominally identical so the joint standard is uniform up the stack (subject to §12.11 — whether the avionics and aft sections deviate). The current STEP export of the aft-most section (`hardware/cad/aft-most.step`) is the reference geometry; see `docs/cad/aft-most-section.md` §2 for its as-read dimensions.

---

## 5. Section-to-Section Joint Standard

The joint splits three jobs onto three features (see the datum rows of §3): the **flange faces** set axial position, the **stringers** set radial/clocking alignment, and the **bolts** clamp. No central spigot or register is required — the stringers are the alignment datum.

Orientation convention: viewing the rocket nose-up, the **lower** section (section A) presents a **thick flange** at its top; the **upper** section (section B) presents a **thin flange** at its bottom. Screws are driven downward from inside section B, through the thin-flange clearance holes, into heat-set inserts seated in section A's thick flange.

### 5.1 Twin flange + bolt ring (clamp)

Eight M3 screws on a 67.5 mm pitch-radius ring carry all axial clamp load. The thin flange is the pass-through side (clearance holes); the thick flange (15 mm boss) is the threaded reaction side (heat-set inserts). Clamp force is generated by the **screw head bearing on the thin flange**, with the threads pulling against the brass insert — never by the screw bottoming out. The 15 mm boss depth is chosen for boss strength and is independent of the insert length.

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

### 5.3 Stringers (bending bridges + alignment datum)

Four solid-aluminium stringers (Ø5 × 200 mm) bridge each joint, seated 100 mm into the lower section (bottoming) and 100 mm into the upper section, with the upper socket bored to **101 mm** so a 1 mm axial gap guarantees the stringer carries **no clamp load** — clamp is the bolts' job alone. The stringers therefore do two things and only two things: provide local bending stiffness across the joint and serve as the joint's **radial + clocking alignment datum**.

The stringers are deliberately **local to the joints** — they do not run the full 280 mm of a section. Mid-span bending is carried by the printed skin alone. This is an intentional mass/machining trade (no full-length aluminium spars). It is a stiffness discontinuity by design; do not "fix" it with full-length stringers without revisiting D055.

Assembly and fit notes (these are manufacturing matters, not design changes):

- The bores must be a **slip fit** to the rod, not a press fit — a press fit risks splitting a printed boss and fighting the clamp.
- FDM holes print undersize and out-of-round; plan to **ream or drill the Ø5 bores to final size** after printing.
- Add **lead-in chamfers** at the bore mouths so four rigid 200 mm rods can enter the mating section simultaneously.
- **Assembly sequence:** seat the stringers (they self-align the sections radially and in clocking), then fit and torque the 8 bolts (their clearance lets them clamp without fighting the stringers).

### 5.4 Datum hierarchy (summary)

| Job | Feature | Why it works |
|---|---|---|
| Axial position | flange faces | flat faces meet; bolts pull them together |
| Radial centring + clocking | 4 stringers | rigid rods in slip-fit bores at R81 |
| Clamp | 8 bolts | generous clearance ⇒ clamp without locating |

No additional centring spigot is required; the over-constraint concern is resolved by giving each job to exactly one feature.

---

## 6. Avionics Section and Capsule (D057)

The forward-most section (the one before the nose) is the **avionics section**. The avionics itself is packaged as a **self-contained, detachable capsule** — its own subassembly carrying the MCU, both IMUs, the magnetometer, the barometer, the radio, batteries, power conditioning, and every mounting point, each component bolted/screwed to the capsule.

Workflow intent:

1. The capsule is built and bench-tested as a standalone unit, off the airframe.
2. It drops into the avionics section and is secured by an internal fastening mechanism (**TBD §12.6**).
3. Power and PWM lines to the canard servos and the servo power rail are connected **after** the capsule is seated (mating method **TBD §12.7**).
4. The section carries **service panels** (count/location/latch **TBD §12.5**) so the capsule can be serviced in place without removing the nose for routine work.

Because the capsule is the standalone home of all avionics, in-place flashing/debugging access (the wired link) is a capsule-level concern routed out through a service panel.

---

## 7. Control Surfaces (D056)

Eight aerodynamic surfaces total, in two functionally distinct sets:

- **Four static aft fins.** Fixed, non-actuated, 90° spacing in "+", at the aft-most section. They are passive/representative stabilisers; they do not move and carry no PWM channel. Attachment method **TBD §12.3**.
- **Four actuated canards.** Forward of the aft fins, 90° spacing, each driven by its own servo on its own dedicated PWM channel from the MCU. **The canards are the entire control authority** — closed-loop attitude control, the mixing matrix, deflection limits, and the 3-of-4 actuator-fault reconfiguration all act on the canards.

This supersedes the earlier "four actuated aft fins" intent (D030); the fault-tolerance rationale of D030 (four independent actuated surfaces ⇒ survive one fault on three) is preserved unchanged — it simply now applies to the canards. Requirements have been re-pointed accordingly (REQ-SYS-005, REQ-SYS-013, REQ-CTL-002/003/004/005/006, REQ-FDR-001/007/009).

Canard body station, pivot/hinge design, and servo bracket/linkage are **TBD §12.1, §12.2**.

---

## 8. Power and Cabling

The canard servos are powered from a **dedicated 5 V rail (Rail B), separate from the MCU's 3.3 V rail (Rail A)** — D057, per the existing power topology in D033 / REQ-PWR-004 and architecture §2.5. Four servos in simultaneous hard slew draw several amps in transients; isolating their rail prevents brown-out and ADC-reference noise on the MCU that would otherwise present to FDIR as false sensor faults.

Cable routing from the capsule to the canard servos and to the servo rail crosses from the avionics section to the canard station; the connector/mating approach (hand-connect through a service panel vs. blind-mate on capsule insertion) is **TBD §12.7**.

---

## 9. Print and Fabrication Strategy

- **Material:** PLA+ for this article (see §10).
- **Print orientation matters at the joint.** If a section prints axis-vertical, the flange bolt holes load the inter-layer bond in pull-out/tension — the weak FDM direction. Orientation is a first-class joint-strength decision, to be locked alongside the joint, not after.
- **Coupon-before-commit (strongly recommended, and consistent with architecture §3.8 and Constraints §10.7).** Before committing a 280 mm section (tens of hours of print), print a **single-joint test coupon**: two short flange rings carrying the full 8-bolt + 4-stringer + stepped-insert geometry. ~1 h of printing validates insert seat depth, clamp behaviour, and stringer slip-fit. This protects the end-of-month deadline far more than it costs.
- **Filament/print-time reality.** A 12 mm-wall, 280 mm section is a large print (the wall alone is ~1.7 L of volume); plan on roughly a full spool and many hours per section, ×~6 sections. This is within budget but is real slog and cost — budget it explicitly (Constraints §10.1). "An airframe by tomorrow" is realistically "one section and a frozen joint standard by tomorrow."

---

## 10. Accepted Limitations / Scope Notes

- **PLA creep — accepted, out of scope for this article.** PLA+ creeps under sustained clamp load, so bolt tension will relax over time. This is deliberately accepted: the article is bench-only, low-load, and intended for sub-month engagement. If the airframe is to live beyond ~a month, it will be redesigned in a better filament/material rather than mitigated here. Recorded so the reasoning is not lost; not tracked as a live risk.

---

## 11. Cross-References

- `docs/cad/aft-most-section.md` — parametric insert/clamp formulas, STEP read-out (the joint math lives there).
- `hardware/cad/aft-most.step` — reference section geometry.
- `docs/05-architecture.md` §3 — airframe structural architecture (summarised; points here).
- `docs/03-decisions-log.md` — D055, D056, D057 (and D031, D044).
- `docs/04-requirements.md` — REQ-SYS-005/013, REQ-CTL-002/003/004/005/006, REQ-FDR-001/007/009.

---

## 12. Open / To-Be-Locked Register

These do not block printing the cylindrical sections (the section + joint geometry is fully specified above), but a head-to-tail lock requires them. Each should be resolved and folded into the relevant section above.

1. **Canard body station** — axial location of the canards (in/near the avionics section, or a dedicated section); fixes where the servos mount.
2. **Canard pivot/hinge** — printed pivot vs. bearing; canard shaft material; bearing seat if any.
3. **Static aft-fin attachment** — integral print vs. bolted root; root geometry; interaction with the Ø120 aft opening.
4. **Nose cone retention/removal** — mechanism with no external fasteners.
5. **Service panels** — count, location, and fastener-free retention/latch type.
6. **Avionics capsule fastening** — how the capsule is secured inside the avionics section (internal collar/clamp/threaded ring).
7. **Cable mating** — capsule ↔ canard servos ↔ servo power: hand-connect through a panel vs. blind-mate on insertion.
8. **Aft-most Ø120 end opening role** — confirm it is the motor-mount-like interface vs. a flange opening; define what mates to it.
9. **Total length / section count** — target ~1.8 m ⇒ ~6 sections; lock the number to fix joint count.
10. **Screw size** — M3 vs M4 for the bolt ring; drives J3/J4/J5 and the §5.2 numbers. Replace the current Ø5.7 CAD placeholder once chosen.
11. **Joint-standard uniformity** — whether the avionics and aft-most sections use the identical joint standard or deviate.
12. **Heat-set insert model** — exact insert + vendor drawing (sets the §5.2 Stage-1 diameter and `L_insert`).

---

## 13. Change Log

- **2026-06-17** — Document created. Section/joint geometry, stepped-insert depth solution, stringer datum scheme, avionics capsule, static-fin + canard control architecture, power/cabling, and fabrication plan captured. Decisions D055–D057 logged. Canard requirements re-pointed.
