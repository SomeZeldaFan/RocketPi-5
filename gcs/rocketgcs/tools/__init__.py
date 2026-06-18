"""Dev tooling.

Home of the fake-MCU frame emitter (G3): generates physically-plausible
telemetry streams (incl. the five fault scenarios) over loopback or a real SiK
radio, so the whole GCS can be exercised end-to-end without an MCU. Lives here,
not in ``sim/`` (HIL was removed from scope — sim/ stays an empty placeholder).
"""
