"""Command-and-control uplink sender (G8).

Builds and signs ``command_frame_t`` (set mode flight/demo/safe-hold, reset
estimator, ack request) with cmd_seq + CRC; demo/flight toggle; ack round-trip.
"""
