# LR-1: Control Loop Rate Derivation

**Project:** RasPiProject — bench-only rocketry GNC suite
**Date:** 2026-05-22
**Status:** FINAL
**Decision reference:** D052
**Unblocks:** LR-3 · REQ-SYS-011 · REQ-EST-002 · REQ-CTL-008

---

## Abstract

`AVIONICS_LOOP_RATE_HZ` is the rate-determining constant for the entire MCU timing architecture and cannot be assumed or borrowed from a comparable vehicle — it must be derived. A first-principles mass budget produces transverse-axis moment-of-inertia bounds for the airframe across its full build-weight range. Franklin, Powell & Workman's Chapter 11 §11.2–11.3 provides the operative sampling margin rule for digital control of continuous-time systems. A log-spaced MOI sweep confirms that the recommendation is stable across the build-weight range, because aerodynamic fin effectiveness is excluded (bench demo, no airflow) and MOI therefore does not enter the bandwidth requirement. The result: **1000 Hz**, loop period 1000 μs, `IMU_STALENESS_THRESHOLD_US` = 5000 μs.

---

## 1. Introduction

### 1.1 Why this number matters

`AVIONICS_LOOP_RATE_HZ` sets `dt` for the EKF prediction step, determines the FDIR innovation-gate window size (LR-3 input), governs the D047 overrun-counter halt threshold, and sizes every timing constant in the system. It cannot be chosen arbitrarily or borrowed from a comparable vehicle.

### 1.2 Methodology (locked)

A prior session fixed three methodology decisions — they are not re-derived here:

1. **Disturbance bandwidth ceiling: 10 Hz.** Bench manual perturbation upper bound. Source of truth for this derivation.
2. **MOI bounds: first-principles analytical mass budget.** Not comparable-vehicle lookup.
3. **Sampling margin rule: Franklin, Powell & Workman Chapter 11.** Parsed directly from the 3rd-edition text.

### 1.3 Scope exclusions

Fin aerodynamic effectiveness is irrelevant — bench demo, no airflow. MOI therefore does not enter the bandwidth calculation; its role is confined to the sweep that confirms the recommendation's stability across the build-weight range.

---

## 2. Source Material

> [1] G. F. Franklin, J. D. Powell, and M. L. Workman, *Digital Control of Dynamic Systems*, 3rd ed. Menlo Park, CA: Addison-Wesley Longman, Inc., 1998, ISBN 0-201-33153-5.
>
> [2] F. P. Beer and E. R. Johnston Jr., *Vector Mechanics for Engineers: Dynamics*. (Standard rigid-body MOI formulae, hollow cylinder transverse and axial axes.)

Specific Franklin passages used (with page numbers):

| Equation | Location | Content |
|---|---|---|
| (11.1) | §11.1, p. 450 | Nyquist absolute bound: ωs ≥ 2ωBW |
| (11.2) | §11.2, p. 451 | Practical smooth-response range: 20 < ωs/ωBW < 40 |
| (11.3) | §11.2, p. 453 | Delay criterion (≤ 10% rise time): ωs/ωBW ≥ 20 |

Verbatim text from §11.2 supporting Eq. (11.2):

> "For a system with a rise time on the order of 1 sec (which translates to a closed-loop bandwidth on the order of 0.5 Hz), it would be typical to choose a sample rate of 10 to 20 Hz in order to provide some smoothness in the response and to limit the magnitude of the control steps. This means that the desired sampling *multiple* (= ωs/ωBW) for a reasonably smooth time response is: 20 < ωs/ωBW < 40."

Note on units: the ratio ωs/ωBW is dimensionless. Expressing both frequencies in Hz (fs, fBW) gives the identical numerical ratio because the 2π factors cancel. All calculations below use Hz.

---

## 3. Phase A: Moment of Inertia Bounds

### 3.1 Rigid-Body Formulae [2]

The relevant axis for pitch/yaw attitude control is the **transverse axis** (perpendicular to the rocket's longitudinal axis, through its centroid). The airframe is modelled as a hollow cylindrical shell; avionics components are modelled as point masses offset axially from the centroid.

**Hollow cylinder — transverse (pitch/yaw) MOI:**

$$I_\perp = m\!\left[\frac{r_o^2 + r_i^2}{4} + \frac{L^2}{12}\right]$$

**Hollow cylinder — axial (roll) MOI** (computed for completeness; not used in loop-rate derivation):

$$I_\parallel = \frac{m\!\left(r_o^2 + r_i^2\right)}{2}$$

**Point mass at axial offset z from centroid** (parallel-axis theorem):

$$\Delta I_\perp = m\,z^2$$

where $r_o$ = outer radius, $r_i$ = inner radius, $L$ = cylinder length, $m$ = mass, $z$ = axial distance of component centroid from vehicle centroid.

### 3.2 Shell Geometry

Geometry chosen to be generous — covers all realistic avionics stack footprints and build weights.

| Parameter | Symbol | Value | Rationale |
|---|---|---|---|
| Inner diameter | 2rᵢ | 100 mm | Clears all identified avionics components with margin |
| Wall thickness | t | 3 mm | Robust 3D-print spec (≥ 3 perimeters at 0.4 mm nozzle) |
| Outer diameter | 2r_o | 106 mm | 2rᵢ + 2t |
| Length | L | 800 mm | First-pass rocket proportion (~8× diameter) |
| Material | — | PLA | ρ = 1.25 g/cm³ (conservative mid-range for PLA) |

**Shell volume:**

$$V = \pi\!\left(r_o^2 - r_i^2\right)L = \pi\!\left(0.053^2 - 0.050^2\right)(0.800) = 776.8\ \text{cm}^3$$

**Shell mass:**

$$m_\text{shell} = \rho V = 1.25\ \text{g/cm}^3 \times 776.8\ \text{cm}^3 = 971\ \text{g}$$

**Shell transverse MOI:**

$$I_{\perp,\,\text{shell}} = 0.971\!\left[\frac{0.053^2 + 0.050^2}{4} + \frac{0.800^2}{12}\right] = 0.971 \times 0.05466 = 0.0531\ \text{kg·m}^2$$

### 3.3 Component Mass Inventory

Two build scenarios bracket the realistic mass range. Servo mass is the dominant variable (4× lever arm at 300 mm from CG).

| Component | Lower bound (g) | Upper bound (g) | Source |
|---|---|---|---|
| STM32F407ZGT6 core board | 22 | 28 | Module vendor range |
| GY-BMI160 (Bosch BMI160 breakout) | 1.5 | 2.0 | Chip datasheet + PCB estimate |
| ICM-42688-P breakout (TDK InvenSense) | 1.5 | 2.0 | Chip datasheet + PCB estimate |
| GY-63 (MS5611 barometer breakout) | 2.0 | 2.0 | Module vendor |
| HolyBro SiK V3 433 MHz air module | 11.0 | 13.0 | HolyBro official spec (11.2 g) |
| 4× servos (SG90-class / MG996R) | 36 (4 × 9) | 220 (4 × 55) | Tower Pro datasheets |
| Battery + BEC (2S 1000 mAh / 3S 2200 mAh) | 60 | 160 | Manufacturer specs |
| Wiring, connectors, mounting hardware | 20 | 50 | Estimate |
| Shell (fixed — geometry above) | 971 | 971 | Calculated §3.2 |
| **Total vehicle mass** | **1125 g** | **1449 g** | |

### 3.4 Component Placement Model

Electronics are located in the avionics bay, modelled as centred at the vehicle CG ($z = 0$) — their axial contribution to transverse MOI is therefore zero. Servos and battery are located in the tail section.

| Component group | Placement (z from CG) | Contribution |
|---|---|---|
| Electronics cluster (MCU, sensors, radio, wiring) | z = 0 | ΔI = 0 |
| 4× servos | z = +300 mm | ΔI = 4·m_servo·z² |
| Battery + BEC | z = +200 mm | ΔI = m_batt·z² |

**Lower-bound component contributions:**

$$\Delta I_{\text{servos,lo}} = 4 \times 0.009 \times 0.300^2 = 0.00324\ \text{kg·m}^2$$

$$\Delta I_{\text{batt,lo}} = 0.060 \times 0.200^2 = 0.00240\ \text{kg·m}^2$$

**Upper-bound component contributions:**

$$\Delta I_{\text{servos,hi}} = 4 \times 0.055 \times 0.300^2 = 0.01980\ \text{kg·m}^2$$

$$\Delta I_{\text{batt,hi}} = 0.160 \times 0.200^2 = 0.00640\ \text{kg·m}^2$$

### 3.5 MOI Bounds Summary

$$I_{\perp,\,\text{lower}} = 0.0531 + 0.00324 + 0.00240 = 0.0587\ \text{kg·m}^2$$

$$I_{\perp,\,\text{upper}} = 0.0531 + 0.01980 + 0.00640 = 0.0793\ \text{kg·m}^2$$

The shell dominates in both cases (90.4% of lower bound, 66.9% of upper bound), confirming the geometry assumption drives the result more than component selection.

---

## 4. Phase B: Loop Rate Derivation

### 4.1 Disturbance Bandwidth (Fixed Input)

$f_{BW,\text{dist}} = 10\ \text{Hz}$ — fixed in the prior session and treated as the source of truth for this derivation. Physical interpretation: the highest-frequency manual perturbation applied to the bench test stand that the attitude controller is required to reject. Literature support to be added in a later session.

### 4.2 Required Control Bandwidth

For a closed-loop system to attenuate a disturbance at frequency $f_d$, the closed-loop bandwidth must satisfy $f_{BW} > f_d$. The margin factor determines how much attenuation is achieved at $f_d$. For a first-order closed-loop rolloff at $n \times f_d$:

$$\text{Attenuation at } f_d = 20\log_{10}(n)\ \text{dB}$$

A 5× margin ($n = 5$) gives:

$$20\log_{10}(5) = 14.0\ \text{dB attenuation at}\ f_d = 10\ \text{Hz}$$

$$f_{BW,\text{ctrl}} = 5 \times f_{BW,\text{dist}} = 5 \times 10\ \text{Hz} = 50\ \text{Hz}$$

Additional justification for 5×:

- Settling time at $f_{BW} = 50\ \text{Hz}$: $t_s \approx 4/(2\pi \times 50) \approx 13\ \text{ms}$ — well within the response time a human observer can perturb.
- The STM32F407ZGT6 at 168 MHz with FPU executes the full tick (EKF + FDIR + PID + telemetry) in < 500 μs at 1 kHz [D043], so the compute envelope does not tighten this bound.
- MOI does not enter this calculation: with aerodynamic fin effectiveness excluded (bench demo, no airflow), the required bandwidth is set entirely by the external disturbance specification.

### 4.3 Franklin Sampling Rule

Operative rule: Franklin §11.2, Eq. (11.3) [1]:

$$\frac{f_s}{f_{BW}} \geq 20$$

Applied to the required control bandwidth:

$$f_s \geq 20 \times 50\ \text{Hz} = \mathbf{1000\ \text{Hz}}$$

The upper end of Franklin's practical range [Eq. (11.2): $20 < f_s/f_{BW} < 40$] yields $f_s = 2000\ \text{Hz}$. This adds no further control-quality benefit for a bench system with manual perturbations below 10 Hz and is not committed.

### 4.4 Zero-Order Hold Delay Verification

The ZOH introduces a worst-case input-to-output delay of one full sample period $T$; the effective mean delay is $T/2$:

$$t_\text{ZOH} = \frac{T}{2} = \frac{1}{2 \times 1000\ \text{Hz}} = 0.5\ \text{ms}$$

Rise time at $f_{BW} = 50\ \text{Hz}$ (first-order approximation):

$$t_r \approx \frac{0.45}{f_{BW}} = \frac{0.45}{50} = 9\ \text{ms}$$

Delay fraction:

$$\frac{t_\text{ZOH}}{t_r} = \frac{0.5\ \text{ms}}{9\ \text{ms}} = 5.6\% < 10\%\ \checkmark$$

This satisfies the Franklin §11.2 criterion (ZOH delay ≤ 10% of rise time). The chosen rate is not under-sampled.

### 4.5 MOI Sweep

Five log-spaced points spanning the full MOI range confirm whether the recommendation shifts with build weight. Five points are sufficient: the range spans a ratio of only 1.35× (< one octave), and the sweep is confirmatory rather than exploratory.

Step factor: $(0.0793 / 0.0587)^{1/4} = 1.078$

| Point | $I_\perp$ (kg·m²) | $f_{BW,\text{ctrl}}$ (Hz) | $f_s = 20\,f_{BW}$ (Hz) |
|:---:|---:|---:|---:|
| 1 (lower bound) | 0.0587 | 50 | **1000** |
| 2 | 0.0633 | 50 | **1000** |
| 3 | 0.0681 | 50 | **1000** |
| 4 | 0.0735 | 50 | **1000** |
| 5 (upper bound) | 0.0793 | 50 | **1000** |

**Recommendation: stable.** The required control bandwidth is a function of the fixed disturbance ceiling and the bandwidth margin factor alone — MOI enters no formula in the absence of aerodynamic fin effectiveness. The loop rate recommendation does not shift across the build-weight range. Per methodology, the upper-bound result is committed.

---

## 5. Result

$$\boxed{f_s = 1000\ \text{Hz}}$$

| Constant | Value | Units | Derivation path |
|---|---|---|---|
| `AVIONICS_LOOP_RATE_HZ` | **1000** | Hz | Franklin Eq. (11.3), 5× BW margin |
| Loop period T | **1000** | μs | 1 / 1000 Hz |
| `IMU_STALENESS_THRESHOLD_US` | **5000** | μs | 5 × T |

Requirements resolved by this derivation:

| Requirement | Resolution |
|---|---|
| REQ-SYS-011 — Control loop rate | ≥ **1000 Hz** |
| REQ-EST-002 — Estimator update rate | ≥ **1000 Hz** |
| REQ-CTL-008 — Control law computation deadline | ≤ **1 ms** (one loop period) |

---

## 6. References

[1] G. F. Franklin, J. D. Powell, and M. L. Workman, *Digital Control of Dynamic Systems*, 3rd ed. Menlo Park, CA: Addison-Wesley Longman, 1998. ISBN 0-201-33153-5. §11.1 p. 450, §11.2 pp. 451–453.

[2] F. P. Beer and E. R. Johnston Jr., *Vector Mechanics for Engineers: Dynamics*. Hollow cylinder transverse MOI formula: $I_\perp = m[(r_o^2+r_i^2)/4 + L^2/12]$.

[3] Decision D043 (2026-05-20) — Scheduling model: bare-metal superloop with DMA + ISRs. Confirms STM32F407ZGT6 compute headroom: full tick < 500 μs at 1 kHz.

[4] Decision D047 — Loop overrun policy. References 1 kHz loop rate as the timing basis for the 3-consecutive-overrun halt threshold (~3 ms of sustained real-time failure).
