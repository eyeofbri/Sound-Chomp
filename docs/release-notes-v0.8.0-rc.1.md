# Sound-Chomp v0.8.0-rc.1 — Release candidate

**Status:** Candidate for maintainer testing; v0.7.1 remains the last published stable release.

## Highlights

- Four persistent ~8-second recordings, with names, locking, deletion confirmation, and track ordering.
- Effects: Echo, Stutter, Robot, Warble, Bitcrush, Telephone; three configurable FX slots.
- Loop Off/One/All, reverse playback, 0.5x–3.0x speed adjustment.
- CLASSIC / MINIMAL / ARCADE skins; GREEN / RED / BLUE palettes.
- Portrait and landscape modes, plus accelerometer AUTO mode with orientation-correct startup splash.
- Safety hardening: staged/checksummed LittleFS saves, temporary PSRAM recording capture, preservation of previous recording after failed saves, and no automatic LittleFS formatting on mount failure.
- Updated installation documentation and automated packaging workflow.

## Install / upgrade

See [Release and upgrade guide](releasing.md). For normal upgrades, open the complete source project in VS Code + PlatformIO and use **Upload**. Avoid Erase Flash, Upload Filesystem Image, and partition changes to preserve existing recordings.

## Validation and limitations

The maintainer reported all 9 Phase 5K.3 hardware checks passing. Release ZIP packaging and source-level regression checks passed locally; **a full PlatformIO compilation, a clean-device install, CI binary generation, and power-loss recovery fault testing have not yet been verified in this packaging pass**. There is no audio export/backup feature. The firmware binary archive is for advanced users, not a one-click upgrade.

## For testers

Confirm About reports `v0.8.0-rc.1`; test installation without erasing data, storage persistence after restart, 3 orientation modes, rapid control navigation, first playback, FX combinations, Loop All, and repeat recording. Use disposable recordings for destructive failure tests. Report board revision, build log, serial logs, and reproduction steps.
