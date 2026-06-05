# Bibliography / Reference Stash

**Status:** SKELETON — append references as they prove useful.
**Created:** 2026-05-11

Links to datasheets, papers, tutorials, videos, forum threads that proved useful during the project. Each entry has a brief note on what it covers and why it's useful.

The function is to not lose useful sources. When you're debugging a sensor at 2am in July, you don't want to spend an hour re-finding the one Stack Overflow thread that explained the I²C quirk.

---

## Format

> **[Author, Title, Source]** — Link.
> *What it covers:* One sentence.
> *Why it's useful here:* One sentence.
> *Date added:* 2026-XX-XX.

---

## Seed references (worth bookmarking even before they're needed)

- **Roger Labbe, "Kalman and Bayesian Filters in Python"** — https://github.com/rlabbe/Kalman-and-Bayesian-Filters-in-Python
  *Free, Jupyter-notebook based, builds intuition before formalism. The reference for learning Kalman filtering from scratch.*

- **BPS.space YouTube channel** — https://www.youtube.com/@BPSspace
  *Hobbyist-but-serious rocket avionics work, including test stand methodology and active control implementations.*

- **NAR (National Association of Rocketry) safety code & motor classification** — https://www.nar.org
  *Universal vocabulary for hobby rocket motors; useful even though we're not flying.*

## Aerospace structures references (added 2026-05-15 per D031)

- **Megson, T.H.G., *Aircraft Structures for Engineering Students*** — Butterworth-Heinemann.
  *Standard undergraduate aerospace structures textbook. Covers semi-monocoque construction (skin / frame / stringer load paths) at the right level for the bench-load envelope we are designing against. Primary reference for the airframe chapter.*

- **Bruhn, E.F., *Analysis and Design of Flight Vehicle Structures*** — Jacobs Publishing.
  *Canonical aerospace structures reference. Over-precise for our bench-only load envelope but useful for completeness and credibility, and as a deeper reference if specific load paths warrant detailed analysis.*

- **NASA Technical Reports on additively-manufactured structural elements** — https://ntrs.nasa.gov
  *NASA Glenn and Langley have published on additively-manufactured (FDM and other) structural test results. Directly relevant to the FDM-printed airframe construction approach. Specific reports to be cataloged here as identified during airframe design.*

## Control systems references (added 2026-05-22 per LR-1)

- **Franklin, G. F., Powell, J. D., Workman, M. L., *Digital Control of Dynamic Systems*, 3rd ed.** — Addison-Wesley Longman, 1998. ISBN 0-201-33153-5.
  *Authoritative reference for digital control of continuous-time systems. Ch. 11 (§11.2 pp. 451–453) covers sampling rate selection: Nyquist absolute bound (Eq. 11.1), practical smooth-response range 20 < ωs/ωBW < 40 (Eq. 11.2), and the ≤ 10% rise-time ZOH delay criterion (Eq. 11.3). Used in LR-1 to derive the 1000 Hz loop rate (D052). Will be re-referenced in LR-3 (FDIR innovation gating) and during control law design.*

- **Beer, F. P., Johnston, E. R. Jr., *Vector Mechanics for Engineers: Dynamics*.**
  *Standard rigid-body dynamics reference. Used in LR-1 for hollow-cylinder MOI formulae (transverse: $I_\perp = m[(r_o^2+r_i^2)/4 + L^2/12]$; axial; point-mass parallel-axis) during the airframe mass budget. Will be re-referenced for airframe structural analysis.*

---

*Add references here as they're found.*
