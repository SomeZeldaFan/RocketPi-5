"""Ground-station state model (G6).

Rolling current state assembled from validated frames: latest estimate, health,
actuators, modes; link statistics (rate, dropped-frame count via frame_id gaps,
last reset_cause). Exposes a read API for the dashboard.
"""
