"""Wire protocol: codec (G2) + parser/validator (G5).

Single source of truth for the on-wire byte layout of ``telemetry_frame_t`` and
``command_frame_t``. Checks ``protocol_version == AVIONICS_PROTOCOL_VERSION`` and
verifies CRC-16/CCITT on every frame; the future MCU packer must conform to this.
"""
