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

## 2026-05-22

**Session goal:** LR-1 — control loop rate derivation.

**What I did:** Full mass budget and loop rate derivation. Result: 1000 Hz (D052). Disturbance bandwidth ceiling fixed at 10 Hz; required control bandwidth 5× = 50 Hz; Franklin §11.2 Eq. (11.3) sampling rule (ωs/ωBW ≥ 20) gives fs ≥ 1000 Hz. ZOH delay verified at 5.6% of rise time. MOI sweep across full build-weight range (0.0587–0.0793 kg·m²) confirms recommendation is stable. See `docs/derivations/LR-1-loop-rate.md` for the complete write-up.

**What I observed / learned:**
- Shell mass (971 g at 100 mm ID / 800 mm length, 3 mm PLA wall) dominates MOI in both build scenarios — 90.4% of the lower bound, 66.9% of the upper. Servo choice barely shifts the recommendation. Geometry decisions matter more than component selection for the mass budget.
- The aerodynamics exclusion is doing the load-bearing work: with no airflow on the fins, MOI doesn't enter the bandwidth requirement at all. The loop rate is set by the human shoving the bench stand (10 Hz disturbance) and the digital-control sampling rule, full stop.
- Franklin's 20× rule (Eq. 11.3) applied cleanly. The 5× control-bandwidth margin gives ~14 dB of disturbance rejection — comfortable headroom for bench manual perturbation.
- 1000 Hz at 168 MHz with FPU leaves ~50% compute headroom per D043 (full tick < 500 μs). If the EKF or FDIR grows, there's room. 2000 Hz (Franklin's smooth upper range, 40×) adds no benefit here.

**Where I got stuck:** DjVu copy of Franklin had a corrupted page at p. 242 — exactly where Ch. 11 begins. Wasted ~30 min before noticing. Resolved by obtaining a clean PDF.

**Next session pickup:**
1. LR-3 — FDIR innovation gating (now unblocked: LR-1 result + IMU datasheets).
2. `platform.h` scrutiny session (logs D048 NVIC priority scheme; unblocks TEST-PLT-005 and TEST-PLT-HW-007).
3. The four deferred bombs: FDIR/estimator boundary (D053?), `platform_safe_state()` layer violation, `CMD_FAULT_ACTUATOR` family, `overrun_count` field. Any of these can run in parallel.

**Time spent:** ~3 hours.

---

## 2026-05-19

**Session goal:** Hardware component selection — MCU, dual IMUs, barometer, radio.

**What I did:**
- Read docs 01–03 to confirm hardware selection readiness (D037 authorised it; #1 next task in current state).
- Confirmed UAE frequency regulatory finding: 433 MHz legal per TDRA frequency sheet (D038).
- Established hardware non-negotiables for MCU, IMUs, barometer, and radio before evaluating any candidates.
- Evaluated and rejected STM32H743ZI/ZI2 Nucleo (D-cache coherency on SPI DMA); selected STM32F407ZGT6 core board (D039).
- Evaluated BMI088 (unavailable in UAE), replaced with GY-BMI160 (Bosch); selected ICM-42688-P breakout (TDK InvenSense). Both verified by pinout image — CS independently accessible on each (D040).
- Selected GY-63 MS5611 barometer; pinout verified — I2C pins, PS default high (D041).
- Selected HolyBro SiK V3 433 MHz radio pair (D042).
- Reviewed and approved full tooling list: Pinecil, UT61E+, FNIRSI 1014D, DIYUSER logic analyzer, ST-Link V2, jumper wires, pin headers, solder, flux pen.
- Identified and added missing items to order: ST-Link V2 (critical — required for flashing and debugging), flux pen, flush cutters, IPA + swabs, ESD strap.

**What I observed / learned:**
- STM32H7 D-cache is a real engineering concern for SPI DMA, not a theoretical one. Cache invalidation required around every DMA transaction on the IMU read path. F4 avoids this entirely.
- ICM-42688-P and BMI160 both expose CS independently on their breakout modules — confirmed visually before selection.
- HolyBro SiK V3 UART logic is 3.3V despite 5V supply input — directly compatible with F407. Verify on datasheet before wiring.
- Bare chip (LGA-14, LQFP) vs breakout module is a critical distinction for a first hardware build. All components selected as breakout modules or core boards — no custom PCB required at this stage.

**Where I got stuck:** BMI088 went unavailable mid-session; replaced with GY-BMI160 after pinout verification.

**Next session pickup:**
1. LR-1 — control loop rate (independent, unblocked now).
2. LR-2 — MEMS IMU accuracy (unblocked by D040; run against BMI160 and ICM-42688-P datasheets).
3. LR-3 — FDIR innovation gating (unblocked by D040; requires LR-1 first).
4. Coding framework and deliverables plan (unblocked — MCU confirmed as STM32F407ZGT6; toolchain is STM32CubeIDE + HAL; build system, repo scaffolding, and C module interface definitions can now be written).

**Time spent:** ~3 hours.

---

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
