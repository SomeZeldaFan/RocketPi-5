# Lab Notebook

**Status:** SKELETON — to be appended daily-ish as project work begins.
**Created:** 2026-05-11

Free-form chronological notes of what was done, what was observed, what surprised you, what you're stuck on. The function is not formal documentation — it's *future-you* being able to reconstruct *current-you*'s thinking.

This is the most personal document and the easiest to skip. Do not skip it. Even 5 minutes at the end of a session writing "today I did X, learned Y, am stuck on Z" is enormously valuable when in three weeks you're trying to remember why a thing is wired the way it is.

---

## Format

Reverse chronological — newest at the top — so the most recent context is immediately visible.

> ## 2026-XX-XX
> **Session goal:** What you set out to do.
> **What I did:** Brief log of actions.
> **What I observed / learned:** Anything that surprised you, any data points worth remembering.
> **Where I got stuck:** Specific obstacles, with detail.
> **Next session pickup:** What "future me" should do first when picking up.
> **Time spent:** Approximate hours.

---

## Entries

## 2026-05-16

**Session goal:** Pi 5 first bring-up — DietPi install, SSH access, GPIO verification.

**What I did:**
- Removed stale camera references from docs/01-constraints.md §3.1 and docs/02-current-state.md; committed and pushed.
- Downloaded DietPi v10.3.3 image for Raspberry Pi 5 (ARM64) and flashed to USB flash drive via Raspberry Pi Imager.
- Pre-configured dietpi.txt (locale en_US.UTF-8, timezone Asia/Dubai, WiFi country AE, headless, OpenSSH) and dietpi-wifi.txt before each boot attempt.
- Attempt 1: iPhone hotspot (SSID: S). Pi booted to kernel but never appeared on network — hotspot likely timed out before WiFi initialized.
- Attempt 2: Home WiFi RSH. Reflashed with clean config and RSH credentials. Pi booted correctly (bootloader logs confirmed USB-MSD boot, kernel loaded) but again no network response across multiple boot attempts.
- Root cause of WiFi failure: unknown. DietPi first-boot installer had run partially but not completed across early attempts.
- Resolution: on a subsequent power cycle after ethernet fallback attempts, the Pi appeared on 192.168.3.25 and SSH was reachable. DietPi-Login prompted for first-run setup completion. Completed setup interactively: passwords set, OpenSSH installed (Dropbear removed), serial/UART console kept enabled (needed for MCU UART link per D028/D029), no additional software installed.
- System upgraded kernel from 6.12.75 to 6.18.29 during first-boot setup; Pi rebooted automatically to apply.
- Installed lgpio 0.2.2 and gpiozero 2.0.1. Added dietpi user to gpio group. Verified GPIO stack: both HIGH/LOW commands on pin 17 executed without error against gpiochip0 (RP1 controller, Pi 5 40-pin header).
- VS Code Remote-SSH deferred to next session.

**What I observed / learned:**
- Pi 5 bootloader output is readable on HDMI — useful diagnostic window. Confirmed USB-MSD boot mode, correct device tree (bcm2712-rpi-5-b.dtb), and kernel load.
- Red PWR LED is always on when board is powered. Green ACT LED = active I/O. Yellow ACT LED = system idle. These are two separate LEDs — easy to confuse when watching one.
- FAT32 boot partition is untouched by DietPi's first-boot installer. All installer state lives on the ext4 root partition (not readable from macOS without tools).
- Pi 5 GPIO: the 40-pin header is on gpiochip0. gpiochip4 is a symlink to gpiochip0. lgpio must be called with chip=0 explicitly. The old RPi.GPIO library does not support Pi 5 at all — lgpio is the correct backend.
- gpio group membership required for non-root GPIO access. `usermod -aG gpio dietpi` + reboot.
- `sudo reboot` failed with dbus error on this DietPi build; `sudo shutdown -r now` worked correctly.
- DietPi v10.3.3 on Pi 5 is marked Open Beta for v10.4 — some rough edges (dbus reboot issue) expected.

**Where I got stuck:** WiFi connectivity during unattended first boot — Pi did not connect across multiple attempts with correct credentials. Resolved by interactive completion of first-boot setup after gaining SSH access on a later power cycle. Root cause of the inconsistency not fully characterized.

**Next session pickup:**
1. VS Code Remote-SSH: install Remote-SSH extension in VS Code, add `Host dietpi / HostName 192.168.3.25 / User dietpi` to ~/.ssh/config, connect, verify Pi filesystem visible.
2. After VS Code confirmed: bring-up is fully complete. Move to requirements writing (docs/04-requirements.md) or airframe CAD per the two open Phase 1 tracks.

**Versions logged:**
- DietPi: v10.3.3 (core 10, sub 3, rc 3, branch master)
- Kernel: 6.18.29+rpt-rpi-2712
- Hardware revision: e04171
- lgpio: 0.2.2
- gpiozero: 2.0.1
- Pi IP: 192.168.3.25 (wlan0, RSH network)

**Time spent:** ~4 hours.
