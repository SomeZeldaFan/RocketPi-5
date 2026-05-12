# Requirements

**Status:** SKELETON — to be populated in Phase 1.
**Created:** 2026-05-11

This document captures the formal specification of what the system must do. Functional requirements (what it does), performance requirements (how well), interface requirements (how it connects to other things). It is the thing we test against to know if the project is done.

Without this document, "done" is a vibe. With it, "done" is a checklist.

---

## Format (to be used when populating)

Each requirement gets a unique ID, a statement, a category, a priority, a rationale, and a verification method.

Example structure:

> **REQ-EST-001 — Attitude estimation update rate**
> The attitude estimator shall produce attitude estimates at a minimum rate of 50 Hz.
> *Category:* Performance.
> *Priority:* Must.
> *Rationale:* Closed-loop control stability requires sensor-to-actuator latency below 40 ms.
> *Verification:* Bench measurement of estimator output timestamps over a 60-second run.

---

## Sections (to be populated)

### 1. System-level requirements
### 2. Estimation subsystem requirements
### 3. Control subsystem requirements
### 4. Fault detection & graceful degradation requirements
### 5. Telemetry & ground station requirements
### 6. Simulation requirements
### 7. Documentation requirements
### 8. Demonstration requirements

---

*Populate in Phase 1, after architecture is roughed out and the trade space of specific components is bounded.*
