# Changelog

## v0.8.0-rc.1 — Release candidate (Phase 5K.4)
- Package hardware-tested Phase 5K.3 behavior without changes to audio/storage algorithms.
- Synchronize firmware/About version, setup documentation, and release instructions.
- Add a release checklist, repeatable source/firmware ZIP packaging, and GitHub Actions artifact builds.
- No partition changes or recording-format migration; power-loss recovery remains unverified on device.

## Unreleased — Phase 5K.3 (v0.8.0 development)
- Initialize device orientation before the startup splash and canvas creation.
- Sample accelerometer in AUTO with fallback for ambiguous readings.
- Collect all historical phase notes in `Phase Notes/` and update current documentation links.

## Unreleased — Phase 5K.2 (v0.8.0 development)
- Preserve previous recording through temporary PSRAM capture and verified flash save.
- Handle empty recording / mic failure / save failure without replacing the current take.
- Block recording if temporary capture buffer cannot be allocated.
- Suppress auto-play after failed saves.
- Remove developer-specific COM port settings and refresh user-facing guides.
- Include the repository's MIT license and a 5K.2 test plan.

## v0.7.1 — Public stable release
The last published release as reported by project maintainer. For full history see GitHub releases and historical documentation under `Phase Notes/`.
