# Architecture

**Status:** SKELETON — to be populated in Phase 1.
**Created:** 2026-05-11

This document captures the "how" of the system. Component breakdown, data flow, hardware block diagram, software module structure, interface definitions between subsystems.

The architecture must be designed to support fault tolerance from day one (see Constraints §10.2). Interfaces between estimator, controller, sensor abstraction, and health monitor must be defined such that fault-tolerance logic can be added incrementally without restructuring.

---

## Sections (to be populated)

### 1. System block diagram
### 2. Hardware architecture
- 2.1 Sensor stack
- 2.2 Compute (MCU + Pi roles)
- 2.3 Actuator stack
- 2.4 Power
- 2.5 Telemetry

### 3. Software architecture
- 3.1 Module breakdown (sensor drivers, estimator, controller, health monitor, telemetry, logging)
- 3.2 Interfaces between modules
- 3.3 Data flow
- 3.4 Threading / scheduling model
- 3.5 Error handling and fault propagation paths

### 4. Ground station architecture
### 5. Simulation architecture
### 6. Integration points

---

*Populate in Phase 1, alongside Requirements. The two documents inform each other.*
