# Phase 5K.4 — Release candidate packaging

Baseline: hardware-tested Phase 5K.3 (9/9 checklist items passed on M5StickS3).

- Version changed to v0.8.0-rc.1 in About and startup serial log.
- Added release packaging script and GitHub Actions workflows with archived build artifacts.
- Updated README, setup guide, and release/upgrade warnings. No functional DSP/storage/control changes.
- Not a substitute for fresh-device test, binary flashing test, or power-loss recovery test.

Run `python tools/release_check.py` then `pio run -e m5stack-sticks3`.
