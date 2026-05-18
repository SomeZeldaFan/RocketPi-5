# Current State

**Last updated:** 2026-05-18

Live project status. Updated at the end of every work session.

---

## Where we are

**Phase:** Phase 1 — top-level system architecture review **complete**. Phase 1 now opens two parallel tracks (avionics requirements + airframe structural design).

**Constraints doc:** v0.8 locked.
**Domain:** Locked — bench-only model rocketry GNC suite.
**Depth axis:** Locked — fault-tolerant graceful degradation under sensor compromise.
**Flight:** Excluded.
**System architecture:** Locked (D024–D034). Custom semi-monocoque airframe path adopted (D031); D011 retired.

Pi 5 bring-up complete (DietPi v10.3.3, kernel 6.18.29, SSH access confirmed, GPIO stack verified). VS Code Remote-SSH setup deferred to next session. Avionics components not yet procured.

---

## What's done

- Phase 0 complete — project shape, depth axis, scope, and principles all locked.
- Constraints document locked at v0.8.
- Decisions log populated: D001–D036.
- Coding standard locked: NASA JPL Power of 10.
- Ground station scope defined: multi-pane dashboard, 3D attitude visualization, fin deflection view, bidirectional C2 link, demo/flight mode toggle.
- **Phase 1 architecture review complete** (per D023). Eleven structural decisions logged as D024–D034, covering: IMU configuration, real-time path ownership, sensor bus topology, barometer inclusion, telemetry radio termination, MCU↔Pi physical link, control surface count, test stand / airframe configuration, Pi 5 role precision, power architecture, and authoritative time source.
- Architecture document populated with hardware architecture (§2) and airframe structural architecture (§3 skeleton).
- Risk register, test plan, and bibliography expanded with airframe-side entries.
- **Pi 5 bring-up complete.** DietPi v10.3.3 installed on kernel 6.18.29+rpt-rpi-2712. SSH access confirmed at 192.168.3.25 (wlan0). GPIO stack verified: lgpio 0.2.2 + gpiozero 2.0.1 on gpiochip0 (RP1). Hardware revision e04171.
- **Requirements document first pass complete** (docs/04-requirements.md). Sections 1–8 populated: system-level, estimation, control, FDIR, telemetry, ground station, power, documentation. Sections 9–11 (airframe, simulation, demonstration) deferred as stubs. Eight performance requirements carry [TBD] placeholders pending literature review (LR-1 through LR-4) — see in-progress below.

## What's in progress

- **Airframe: structural design.** Begin CAD work — first-pass body geometry, avionics bay placement, fin pivot locations, gimbal mount integration. Material/print-parameter coupon testing to follow before committing to full airframe prints.
- **Pi 5 bring-up: VS Code Remote-SSH.** Final bring-up item — connect VS Code on dev laptop to Pi via Remote-SSH extension. Deferred from 2026-05-16 session.

### Literature review tasks — required before TBD requirements can be committed

These tasks must be completed before the indicated requirements in docs/04-requirements.md can have their numerical values set. Each is a focused literature review producing a written summary and a justified number recommendation.

- **LR-1 — Control loop rate** *(unblocks REQ-SYS-011, REQ-EST-002, REQ-CTL-008)*
  Sampling theory applied to control systems. Determine the expected closed-loop bandwidth of a fin-stabilised rocket body on a bench gimbal. Derive the minimum control loop rate from that bandwidth using the Nyquist criterion and engineering margin. Produce a justified Hz recommendation.

- **LR-2 — MEMS IMU estimation accuracy** *(unblocks REQ-EST-006, REQ-CTL-007)*
  IMU noise floor, Allan deviation, bias instability for commercial-grade MEMS IMUs. Determine what steady-state attitude accuracy is realistically achievable with dual-IMU EKF fusion on a stationary bench article. Produce a justified RMS error bound in degrees.

- **LR-3 — FDIR innovation gating design** *(unblocks REQ-FDR-008)*
  Chi-squared consistency test design for dual-IMU cross-check. Window size and threshold selection. Characterise the statistical tradeoff between false-positive rate and detection latency at the chosen control loop rate. Produce a justified detection latency bound in milliseconds.

- **LR-4 — Vehicle dynamics** *(unblocks REQ-CTL-003, REQ-CTL-004, REQ-CTL-006)*
  Pitch/yaw/roll moments of inertia, fin effectiveness coefficients, achievable control bandwidth and settling time for the bench article. **Partially blocked on airframe CAD** — geometric inputs (body diameter, fin area, moment arms) come from the CAD model. Run this task in parallel with early airframe design, not sequentially after it.

LR-1, LR-2, LR-3 can begin before CAD. LR-4 requires airframe geometry to proceed.

## What's blocked

Nothing blocked.

## Next concrete tasks

1. **Complete literature reviews LR-1, LR-2, LR-3** — can begin immediately, no CAD dependency. Each produces a written summary and a justified number to commit into docs/04-requirements.md.
2. **Begin airframe CAD** — first-pass body geometry, avionics bay placement, fin pivot locations, gimbal mount integration. Unlocks LR-4. Run material coupon prints as structural design firms up.
3. **Pi 5: VS Code Remote-SSH setup.** Install Remote-SSH extension in VS Code, configure ~/.ssh/config entry for the Pi, connect and verify Pi filesystem is accessible from the laptop.
4. **Compose the formal system block diagram** referenced as deferred in `docs/05-architecture.md` §1. Choose diagramming methodology appropriate for the documentation standard.
5. **Begin component selection** once requirements firm enough to bound trade spaces (MCU per constraints §11.1, IMUs and barometer per §11.2, telemetry module per §11.3). Procurement reality (constraints §10.4) is a binding input — UAE-procurable parts only.
