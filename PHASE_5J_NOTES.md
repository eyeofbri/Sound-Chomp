# Phase 5J — Track Manager prototype

Based on the Phase 5I.1 compile-fix project. This is a development build; it has **not** been compiled in PlatformIO or tested on the M5StickS3.

## New interaction
- Settings/Home > TRACKS: wheel selects a track; click opens its Play/Rename/Delete actions.
- Wheel moves between actions. Press B to go back to track selection.
- PLAY selects the track and plays it if it contains audio.
- RENAME enters a character-picker popup, wheel cycles A-Z/0-9/space/dash/underscore/<. Click appends the character; `<` acts as backspace. Hold wheel click to save. Press B to cancel. Names have a 10-character maximum and are persisted in Preferences, not as audio filenames.
- DELETE opens a confirmation popup. The default is non-destructive: B cancels; **hold the wheel click** to delete.
- Current audio-file storage and existing Preferences keys remain in place.
- Shared filled primary-color menu heading, red row focus, yellow active fields; Charging page has no extra secondary title.

## Limitations
- This build intentionally maintains **4 recording slots**. Six complete 8-second, 16kHz, 16-bit recordings would require 1,536,000 bytes raw, plus headers, filesystem overhead, and a full-size temporary save file. The existing 8MB partition table gives LittleFS around 1.5MB. Expanding slots safely requires storage migration (different partition layout, compression or smaller per-track maxima) and a backup procedure. **Do not use erase-flash or uploadfs on an existing device with important recordings.**
- Pencil and trash artwork are text-symbol placeholders (`T` and `X`) for now.
- Ordering, track locking and wider documentation are planned for 5J.1. Phase 5K will focus on storage/audio/system stability, tests, and a release candidate.
- First test in both orientations; particularly test rename persistence, cancel/back, deletion safeguard, effect and audio regression.

## Roadmap
1. 5J.1: lock and logical order metadata, recording/delete guards, UI buttons, migration design for six tracks.
2. 5K: verify partition/PSRAM capacity on hardware, test all full-length recordings, power-loss safety, menu regressions, and release docs.
