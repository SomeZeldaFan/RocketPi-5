# Current State

**Last updated:** 2026-05-12

Live project status. Updated at the end of every work session.

---

## Where we are

**Phase:** Phase 0 complete. Phase 1 (requirements + architecture) not yet started.

**Constraints doc:** v0.7 locked.
**Domain:** Locked — bench-only model rocketry GNC suite.
**Depth axis:** Locked — fault-tolerant graceful degradation under sensor compromise.
**Flight:** Excluded.

Nothing has been built yet. Pi 5 kit is physically on hand, not yet configured.

---

## What's done

- Phase 0 complete — project shape, depth axis, scope, and principles all locked.
- Constraints document locked at v0.7.
- Decisions log populated: D001–D022.
- Coding standard locked: NASA JPL Power of 10.
- Ground station scope defined: multi-pane dashboard, 3D attitude visualization, fin deflection view, bidirectional C2 link, demo/flight mode toggle.

## What's in progress

Nothing active. Between Phase 0 and Phase 1.

## What's blocked

Nothing blocked.

## Next concrete tasks

1. **Pi 5 bring-up (Thursday).** DietPi install, SSH access, GPIO and camera verification, hello-world hardware interaction.
2. **Open Phase 1.** Begin requirements doc (docs/04-requirements.md) — functional and non-functional requirements, fault tolerance requirements baked in from the start.
