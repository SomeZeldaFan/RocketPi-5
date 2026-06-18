# GCS Dashboard — Design Brief

*A self-contained hand-off for UI generation. Paste into a design tool (Claude Design /
artifacts / v0-style) to produce the dashboard look, then iterate. The companion
`functional-spec.md` defines behaviour; this defines the experience and visual language.
RocketPi5 is a **bench-only static** GNC demonstrator — see `docs/01-constraints.md`.*

---

## 0. How to use this brief
You are designing the **ground control station (GCS) dashboard** for RocketPi5. Produce a
**modern React app** (TypeScript) styled like the polished UIs from Apple / Linear / Vercel /
Claude. Build it against **mock data** driven by a single live state object so it looks alive
immediately. Iterate in this order: **(1) design system → (2) Command Console → (3) demo pages.**

## 1. What this is — and is NOT
**Is:** RocketPi5 is a **bench-only, STATIC** rocket-avionics demonstrator (never flown). A custom
flight computer (dual IMU + barometer + magnetometer → an EKF attitude **estimator** → **FDIR**
fault detection/isolation → a **canard** control law) sits on a **static test stand**. This
dashboard is the **operator's console + the demo showpiece**, running on a **Raspberry Pi 5 with a
touchscreen**. It shows live telemetry over a radio link and sends commands back.

**The story the demo tells (design for this):**
- The stand is **perturbed by hand** → the estimator senses the attitude change → the **canards
  throw in the correct restoring direction** (a *commanded* response; demo mode exaggerates the
  throw for visibility).
- A **sensor is unplugged live** → the system **detects, isolates, and degrades gracefully**: the
  channel faults, its statistical gate slams shut, the estimator's uncertainty (covariance)
  **visibly grows**, and the estimator **steps down** through fallback modes. Fault tolerance,
  made visible.

**Audience:** aerospace faculty, professors, UAE aerospace/defense industry. Tone: **serious,
precise, aerospace-grade — but modern and sleek**, not cluttered retro-NASA.

**Is NOT (avoid these clichés):** no flight, no **launch countdown**, no **altitude / apogee /
trajectory map**, no GPS track, no thrust/propulsion gauges, no "T-minus." It is a **static attitude
+ health + fault-tolerance instrument.** Don't add generic rocket-launch dashboard tropes.

## 2. Tech & component direction
- **React + TypeScript.**
- **Tailwind CSS** for styling.
- **shadcn/ui (Radix primitives)** for accessible, polished base components — the toolkit behind a
  lot of Linear/Vercel-grade polish.
- **Framer Motion** for purposeful micro-interactions and page/element transitions.
- **lucide-react** for icons.
- **react-three-fiber + drei** (three.js) for the 3D rocket attitude.
- **Charts:** crisp, minimal line charts (Recharts or visx is fine for the design pass; a
  performance lib can come later).
- Runtime reality (don't fight it, just know it): this ships **fullscreen in kiosk Chromium on a Pi
  5**, fed by a **single reactive state object pushed over a WebSocket at ~10–30 Hz**. So: **design
  around one continuously-updating state object**; tabular/monospaced numerals so live values don't
  jitter; keep animations GPU-cheap.

## 3. Design language
- **Theme:** dark, refined mission-control. Near-black canvas (≈ `#0A0B0D`), **elevated surfaces**
  (`#121419`–`#1A1D23`) with **1px hairline borders**, soft shadows, occasional subtle glass/blur on
  overlays. No heavy skeuomorphism.
- **Status color system (use everywhere — health, link, modes, gates):**
  - **Nominal** — a calm signal color (teal/green, e.g. `#3DDC97`),
  - **Caution** — amber (`#F5A623`),
  - **Fault** — red (`#FF4D4F`),
  - **Offline/stale** — desaturated grey.
  One restrained **accent** (e.g. cyan `#4DA3FF`) for interactive/primary affordances.
- **Typography:** clean grotesque sans for UI (**Inter** or SF-Pro-like). **Tabular / monospaced
  numerals** for all telemetry values (**JetBrains Mono / SF Mono**) so numbers don't shift width as
  they update.
- **Density:** the **Command Console is dense but breathable** (operator-grade); **demo pages are
  cinematic** with generous negative space and a single hero element each.
- **Touch:** all targets **≥ 44px**, comfortable spacing, clear pressed states. Landscape-primary but
  **responsive to any resolution**.
- **Motion:** fast and eased (150–250ms), purposeful. Restrained on the console; demo pages may
  breathe more. Honor `prefers-reduced-motion`.

## 4. Pages & navigation (5 pages)
Persistent **top tab bar** — **click/tap only, no swipe/gestures.** Tabs:
**Command · Rocket · FDIR · Canards · Charts.**
A slim **global header strip** is always visible across all pages: **link status**, **active
transport** (radio/wired), **active mode** (Flight/Demo/Safe-Hold), and a **live frame-age / rate**
indicator. This is the always-honest "are we connected and what mode are we in" bar.

### Page 1 — Command Console (operator-grade, functional, *no pizzazz*)
The one screen a real operator monitors and commands from. Dense, legible, instantly scannable.
Suggested grid (adapt responsively):
- **Attitude block** — roll / pitch / yaw + body rates, **large tabular numerals** with units; tiny
  inline trend sparkline optional.
- **Health matrix** — a compact grid of **status cells**: IMU1, IMU2, Baro, Mag, Canard 0–3, Radio,
  Wired. Each cell = label + status dot (nominal/caution/fault/offline). Reads at a glance.
- **Estimator / FDIR block** — current **estimator mode** (DUAL_IMU → IMU1_ONLY / IMU2_ONLY →
  DEAD_RECKONING → FAULT), the **6 covariance values**, and **per-channel chi² + gate-open flags** —
  all numeric, in a tight table.
- **Status line** — link rate, dropped-frame count, last reset cause, MCU session time + frame age.
- **Command bar** — the controls (see §5). This is where the operator acts.

Visual restraint: muted surfaces, hairline separators, no decorative gradients. Function first.

### Page 2 — Rocket View (hero 3D attitude)
Full-bleed **3D rocket body** (react-three-fiber) rotating live with roll/pitch/yaw, smoothly
interpolated (~10 Hz feel even at 5 Hz data). Dark scene, a subtle reference grid / horizon ring,
soft key light. Minimal glass HUD overlay: attitude numerals + current mode. Cinematic and calm.

### Page 3 — FDIR View (the fault-tolerance showpiece)
The demo's money shot — choreograph the **graceful-degradation** moment:
- **Channel rail** — each sensor channel as a tile; on isolation it transitions **nominal → fault**
  with a deliberate, legible animation.
- **Innovation gates** — per-channel **chi² vs. threshold** as gauges/bars; when a fault hits, the
  gate **slams shut** (animate it).
- **Confidence** — the **covariance bars visibly grow** as a channel drops (this is the point — make
  the uncertainty *expand* on screen).
- **Mode ladder** — DUAL_IMU → IMU1/2_ONLY → DEAD_RECKONING shown as rungs; the **current rung lit**,
  and it **steps down** with a satisfying transition when redundancy is lost.

### Page 4 — Canard View
The 4 canards in a **"+" configuration** around the body, **animated to their commanded deflection**
(sign + magnitude), with per-canard angle readouts. Indicate when **demo mode is exaggerating** the
throw (a subtle badge). Clean, diagrammatic, satisfying to watch react to perturbation.

### Page 5 — Telemetry Charts
**Scrolling time-series** of roll/pitch/yaw + key sensor outputs. Sleek dark line charts: thin lines,
subtle gridlines, tabular axis labels, live auto-scroll. Optional pause + scrub.

## 5. Commands & actions — make these *exceptional* (priority)
This is where to spend disproportionate craft. The command surface must feel **premium, tactile, and
safe** — Apple/Linear-grade.
- **Mode selector** = a **segmented control** `Flight | Demo | Safe-Hold` with a **smooth sliding
  active indicator**. Critically: it reflects the **MCU-confirmed** mode — on tap it shows *pending*,
  and only snaps to the new segment once the **acknowledgment** returns (never lie about state).
- **Command lifecycle animation (every command):** tap → **pending** state (subtle pulse /
  indeterminate ring) → resolves to **acknowledged** (a crisp checkmark sweep + brief accent glow) or
  **timeout** (amber shake). Tie to the command's sequence id.
- **Critical/destructive commands** (`Safe-Hold`, `Reset Estimator`) require a **deliberate confirm**
  — **slide-to-confirm** or **press-and-hold** (a ring fills over ~600ms). Prevents accidental touches
  *and* feels high-end.
- **Acknowledgment feed** — a small, elegant log of recent commands with type, time, and ack status.
- **Micro-interactions** — pressed state (scale ~0.98 + shadow shift), crisp easing, clear primary vs.
  destructive hierarchy (accent for safe actions; destructive stays restrained until confirmed).
  Optional haptic/sound hooks, but **visual feedback is primary** (kiosk).
- **Buttons** — generous touch targets, confident typography, no clutter; the kind of buttons that
  feel good to press on a touchscreen in front of an audience.

## 6. States & resilience (never fool the operator)
- **Link states** (global header + per-value): **LIVE** (accent), **STALE** (frame age past threshold
  → values **dim** + amber "STALE"), **LOST** (red **"LINK LOST"** banner; last values **frozen and
  greyed**). Frozen data must look visibly different from live data.
- Per-page **loading / empty / error** states.

## 7. The live data shape (mock against this)
Design and mock against a single state object updated continuously. Representative snapshot (values
illustrative; angles shown in degrees for display, sourced from radians):
```json
{
  "frame_id": 10432,
  "session": "2026-06-18T10:21:04Z",
  "mcu_time_us": 41523900,
  "frame_age_ms": 47,
  "link": { "transport": "radio", "rate_hz": 18.2, "dropped": 3, "state": "live" },
  "modes": { "system": "DEMO", "control": "FULL_AUTHORITY", "estimator": "DUAL_IMU" },
  "attitude": { "roll_deg": 2.4, "pitch_deg": -1.1, "yaw_deg": 87.6,
                "roll_rate_dps": 0.8, "pitch_rate_dps": -0.3, "yaw_rate_dps": 0.0 },
  "covariance": [0.012, 0.011, 0.040, 0.0008, 0.0008, 0.003],
  "health": { "imu1": true, "imu2": true, "baro": true, "mag": true,
              "canard": [true, true, true, true], "radio": true, "wired": false },
  "fdir": { "chi2_imu1": 1.8, "chi2_imu2": 2.1, "chi2_mag": 0.9,
            "gate_imu1": true, "gate_imu2": true, "gate_mag": true,
            "stale_us_imu1": 110, "stale_us_imu2": 130 },
  "canards_deg": [3.2, -3.2, 1.0, -1.0],
  "reset_cause": "POWER_ON",
  "last_command": { "type": "SET_MODE_DEMO", "seq": 7, "status": "acknowledged" }
}
```
Field notes / ranges: angles ±180°, rates small on a static bench; covariance grows when a sensor
drops (the FDIR demo); health booleans flip to drive the fault story; `last_command.status` ∈
{pending, acknowledged, timeout}; modes are enums from the avionics contract.

## 8. Iteration plan (deliverable order)
1. **Design system** — palette, type scale, the status color system, and base components: `Button`,
   `SegmentedControl`, `StatusCell`, `ValueReadout`, `Card/Panel`, `Tab`, `Banner`.
2. **Command Console** (Page 1) — the backbone; get the command interactions feeling great first.
3. **Demo pages** — Rocket, FDIR, Canards, Charts.
4. Wire everything to a **single mock state object** updated on a timer (e.g. gentle sine-wave
   attitude + a scripted "sensor drop" you can trigger) so it demos live without a backend.
