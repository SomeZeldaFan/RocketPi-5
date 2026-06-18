"""RocketPi5 ground control station.

Runs on the Pi 5 (DietPi, headless) — telemetry reception, frame logging,
dashboard, and C2 uplink only; never in the real-time path (D025/D032).

Mirrors the MCU module decomposition (D022):
    transport -> protocol -> state -> dashboard / c2 / logger
The fake-MCU frame emitter (G3) lives under ``tools``.

Bench-only static GNC suite — see ``docs/`` for constraints and decisions.
"""

__version__ = "0.0.1"
