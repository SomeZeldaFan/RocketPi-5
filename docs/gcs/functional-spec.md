# GCS Dashboard — Functional Specification

*Defines **what** the ground-station dashboard shows and what the operator can do —
render-agnostic (framework and visual language belong to the design spec). Grounded in
`docs/04-requirements.md §6` (REQ-GCS-001…010), REQ-SYS-008, and decisions D020 / D032 /
D053 / D060. Bench-only static GNC suite — see `docs/01-constraints.md`.*

*Build approach: dashboard-first. The UI reads a stable **state model** derived from the
locked `avionics_types.h`, fed by a fake-MCU emitter now and the real radio later — so the
wire format (G2) is deferred behind a seam, not skipped.*

---

## 1. Purpose & role
The dashboard is the ground-station operator's window into the bench rocket and the
audience-facing showpiece for the demo. Per **REQ-GCS-002 / REQ-GCS-003 / D032**, its role is
strictly bounded: **telemetry display, frame logging, and C2 command transmission only** — no
estimation, FDIR, or control logic ever runs on the Pi. It consumes decoded telemetry values;
it never computes them.

## 2. Operating model (self-contained, plug-and-play)
- **Runs entirely on the Pi 5 (DietPi), operated by touch** on the attached display. The Mac is
  **not required** to run or operate it — Mac is dev/backstage (SSH + rsync) and an *optional*
  remote viewer only.
- **Auto-launch on boot** into a fullscreen dashboard, no login or manual start; survives reboot.
- **Display-presence failsafe:** if **no connected display is detected within 60 s of boot**, the
  dashboard does **not** launch — the Pi stays a normal headless box for dev/SSH. Auto-launch
  fires only when a display is actually present.
- **Display-agnostic:** layout adapts responsively to whatever resolution is plugged in (no
  hardcoded screen size).
- **Touch-first:** every control and the page navigation are touch targets. **Tab navigation is
  plain click/tap only — no swipe, no gestures** (deliberate: minimize complexity).
- **Data-source seam:** the UI reads a **state model** (decoded telemetry values mirroring
  `attitude_estimate_t`, `health_flags_t`, `actuator_cmd_t`, `fdir_gate_result_t`, modes, link
  stats). Fed by the **fake-MCU emitter** now; the real radio swaps in later with **no UI change**.
- **Aesthetic:** sleek and minimalist throughout. The **Command console favours density and
  legibility over beauty (no pizzazz)**; the demo pages get the polished treatment.

## 3. Page structure
A top **tab bar** (click/tap) switches between full-screen pages. Page 1 is the operator console;
the rest are demo pages.

### Page 1 — Command console (operator-grade, functional, no pizzazz)
The single screen a genuine operator monitors and commands from. Dense, legible, touch-operable:
- **Attitude readout** — roll/pitch/yaw + body rates, as live numbers.
- **Health matrix** — plain per-channel status cells: IMU1, IMU2, baro, mag, actuators 0–3, radio,
  wired. *(source: `health_flags_t`)*
- **Estimator / FDIR state** — current estimator mode (DUAL_IMU → IMU1/2_ONLY → DEAD_RECKONING /
  FAULT), `covariance[6]`, and per-channel chi2 + gate-open flags, numeric. *(source:
  `attitude_estimate_t`, `fdir_gate_result_t`)*
- **Status line** — active transport (radio/wired), active mode (flight/demo/safe-hold), link rate,
  dropped-frame count (from `frame_id` gaps), last `reset_cause`, MCU session timestamp + current
  frame age. *(REQ-GCS-007, D053)*
- **Command controls** — `[Flight]` `[Demo]` `[Safe-Hold]`, `[Reset Estimator]`, `[Ping]` (the
  `command_id_t` set: CMD_SET_MODE_FLIGHT/DEMO/SAFE_HOLD, CMD_RESET_ESTIMATOR, CMD_ACK_REQUEST).
  Each shows **pending → acknowledged** with the command type. *(REQ-GCS-008)*

### Demo pages (audience-facing, pretty, minimalist)
- **Rocket view** — live 3D attitude of the rocket body. Refresh ≥ frame rate (≥5 Hz floor),
  smoothed toward ~10 Hz via interpolation. *(REQ-GCS-006)*
- **FDIR view** — the depth-axis showpiece: health channels flipping on isolation, chi2 gates
  opening/closing, `covariance` visibly **growing** as a sensor drops, and estimator mode degrading
  gracefully — the live sensor-disconnect story made visual.
- **Canard view** — the 4 **commanded** control-surface (canard) deflections from
  `actuator_cmd_t.deflection_rad[4]`, animated. Per **D060** these are *commanded-response* on the
  static bench (no airflow / no closed loop); **demo mode deliberately exaggerates the throw**
  (REQ-CTL-004) — surface that exaggeration as the legibility feature it is.
- **Telemetry charts** — scrolling roll/pitch/yaw and key sensor outputs vs. time.

## 4. Cross-cutting behaviour
- **MCU timestamps** drive all data operations (charts, logging, frame age); the Pi clock is used
  only for human-readable session naming. *(REQ-GCS-004)*
- **Logging:** every CRC-valid frame is logged to local storage, file named by MCU session
  timestamp. Runs whenever telemetry is flowing, independent of which page is shown. *(REQ-GCS-009)*
- **Transport abstraction:** radio ↔ wired switch requires no UI change. *(REQ-GCS-010)*
- **Command ack:** the most-recent command's type + pending/acknowledged status is always visible
  on the console. *(REQ-GCS-008)*

## 5. Requirements mapping
| Feature | Requirement |
|---|---|
| Bounded role (display/log/C2 only) | REQ-GCS-002, REQ-GCS-003 |
| MCU timestamps for all data ops | REQ-GCS-004 |
| Rocket 3D attitude ≥5 Hz / ~10 Hz | REQ-GCS-006 |
| Active transport + mode display | REQ-GCS-007 |
| Command ack status | REQ-GCS-008 |
| Log every CRC-valid frame | REQ-GCS-009 |
| Transport abstraction | REQ-GCS-010 |
| Mode commands (flight/demo) + ack | REQ-SYS-008, D020 |
| Canard commanded-response + demo exaggeration | D060, REQ-CTL-004 |
| reset_cause / health / chi2 surfaced | D053 |

## 6. Requirement changes pending logging
*Flagged here; to be logged as a decision in `docs/03-decisions-log.md` + edited into
`docs/04-requirements.md` when this spec is locked (approve-then-log cadence).*
1. **Amend REQ-GCS-005** — relax "panes simultaneously visible without switching." The four
   observational functions (telemetry charts, health/confidence, 3D attitude, canard) are
   **preserved but redistributed** across the Command console + demo pages, navigated by tabs.
   Content unchanged; only simultaneity relaxed.
2. **Add new requirements** — (a) auto-launch fullscreen on boot when a display is present;
   (b) **display-presence failsafe** (no display within 60 s → don't launch); (c) display-agnostic
   responsive layout; (d) touch-first, click-only tab navigation.

## 7. Out of scope (this document)
Wire format / codec (G2), real radio transport internals, any estimation/FDIR/control logic,
framework/render-stack choice, and the visual design language (→ design spec + the "Claude Design"
brief).
