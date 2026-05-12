# Current State

**Last updated:** 2026-05-12

Live project status. Updated at the end of every work session.

---

## Where we are

**Phase:** Phase 1 open — top-level system architecture review in progress.

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

Structured top-level system architecture options review — output establishes the explicit system concept that Phase 1 requirements and architecture decisions sequence from.

## What's blocked

Nothing blocked.

## Next concrete tasks

1. **Pi 5 bring-up.** DietPi install, SSH access, GPIO and camera verification, hello-world hardware interaction. (Parallel track — does not block Phase 1.)
2. **Complete structured architecture review** → make executive decisions on structural questions → document explicit system concept.
3. **Log Phase 1 decision sequencing (D024)** → open requirements doc (docs/04-requirements.md).
