# Current State

**Last updated:** 2026-05-15

Live project status. Updated at the end of every work session.

---

## Where we are

**Phase:** Phase 1 — top-level system architecture review **complete**. Phase 1 now opens two parallel tracks (avionics requirements + airframe structural design).

**Constraints doc:** v0.8 locked.
**Domain:** Locked — bench-only model rocketry GNC suite.
**Depth axis:** Locked — fault-tolerant graceful degradation under sensor compromise.
**Flight:** Excluded.
**System architecture:** Locked (D024–D034). Custom semi-monocoque airframe path adopted (D031); D011 retired.

Pi 5 kit is physically on hand, not yet configured. Avionics components not yet procured.

---

## What's done

- Phase 0 complete — project shape, depth axis, scope, and principles all locked.
- Constraints document locked at v0.8.
- Decisions log populated: D001–D034.
- Coding standard locked: NASA JPL Power of 10.
- Ground station scope defined: multi-pane dashboard, 3D attitude visualization, fin deflection view, bidirectional C2 link, demo/flight mode toggle.
- **Phase 1 architecture review complete** (per D023). Eleven structural decisions logged as D024–D034, covering: IMU configuration, real-time path ownership, sensor bus topology, barometer inclusion, telemetry radio termination, MCU↔Pi physical link, control surface count, test stand / airframe configuration, Pi 5 role precision, power architecture, and authoritative time source.
- Architecture document populated with hardware architecture (§2) and airframe structural architecture (§3 skeleton).
- Risk register, test plan, and bibliography expanded with airframe-side entries.

## What's in progress

Two parallel tracks now open under Phase 1:

- **Avionics: requirements writing.** Open `docs/04-requirements.md` and write system-level, estimation, control, FDIR, telemetry, simulation, documentation, and demonstration requirements rooted in the now-explicit system concept.
- **Airframe: structural design.** Begin CAD work — first-pass body geometry, avionics bay placement, fin pivot locations, gimbal mount integration. Material/print-parameter coupon testing to follow before committing to full airframe prints.

## What's blocked

Nothing blocked.

## Next concrete tasks

1. **Open `docs/04-requirements.md`** and write the first pass of system-level requirements rooted in the architecture concept in `docs/05-architecture.md` §2.
2. **Begin airframe CAD** — first-pass body geometry, avionics bay placement, fin pivot locations, gimbal mount integration. Run material coupon prints as the structural design firms up.
3. **Pi 5 bring-up** (parallel track — does not block the other two). DietPi install, SSH access, GPIO and camera verification, hello-world hardware interaction.
4. **Compose the formal system block diagram** referenced as deferred in `docs/05-architecture.md` §1. Choose diagramming methodology appropriate for the documentation standard.
5. **Begin component selection** once requirements firm enough to bound trade spaces (MCU per constraints §11.1, IMUs and barometer per §11.2, telemetry module per §11.3). Procurement reality (constraints §10.4) is a binding input — UAE-procurable parts only.
