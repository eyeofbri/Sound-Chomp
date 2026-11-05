# Phase 5K.2 — Recording safety and release readiness

## Changes

- Capture new PCM into a **temporary 256 KiB PSRAM buffer**, leaving the previous recording intact until a verified LittleFS transaction succeeds.
- Mic initialization failure, capture failure, zero-sample recording, PSRAM allocation failure, and save failure **leave the previous in-memory recording untouched**.
- Only on successful save: activate the new buffer, release old PCM, refresh waveform, and apply reset-speed-on-record preference.
- Auto-play after recording is suppressed when save failed.
- Removed developer-only COM9 port configuration.
- Added MIT license matching public repository, refreshed README and Getting Started, and provided tests/verification checklist.

## Explicit limits

- No on-device export/backups; recordings still depend on the flash chip.
- One extra 256 KiB PSRAM buffer must be available while recording. Failure blocks new recording safely.
- Current LittleFS layout and PCM format unchanged; do not run `uploadfs`, erase flash, or repartition when preserving recordings.
- Full PlatformIO/device build, playback/audio tests, and real interrupted-power tests remain necessary before a public release.

## Hardware test plan

1. Verify all four prior recordings, names, lock flags, order, palettes, and skins after flashing.
2. Record on an empty track, restart, confirm playback; overwrite an unlocked track and restart.
3. Press/release A quickly so no recording starts. Begin recording and release immediately; ensure old take remains if zero audio is captured.
4. Lock a track: recording and deletion must be blocked. Unlock and verify normal operation.
5. Fill the storage close to its limit with long recordings and verify a blocked recording does not replace its prior take.
6. Test Echo, Stutter, Robot, Warble, Bitcrush, Telephone, Loop Off/One/All, reverse playback, and first playback after reboot.
7. Verify all skin/palette pairs and portrait/landscape navigation.
8. Monitor serial output for `Recording save failed: previous take preserved`, storage errors, microphone errors, and PSRAM allocation issues.
9. Test repeated record/replace cycles and reboot persistence. Any forced power-loss test should use disposable recordings only.
10. Confirm clean build and upload with no COM-specific setting, and review PlatformIO firmware binary size versus partition capacity.

## Release gate

Do not tag v0.8.0 until PlatformIO compilation and real-device regression testing pass. This package is a development candidate only.
