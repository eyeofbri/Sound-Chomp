# Phase 5K — Storage & Persistence

Based on the Phase 5J.4a source. This release is a **prototype requiring a PlatformIO build and M5StickS3 testing**.

## Main changes

- Never format LittleFS automatically on mount failure (`LittleFS.begin(false)`). Recording is blocked if persistent storage is unavailable; existing flash contents are not erased by startup.
- Reject recording *before recording begins* unless there is enough free LittleFS space to stage a full eight-second clip plus a 4 KiB allowance. This protects the previously loaded RAM recording from being overwritten when a safe on-flash save is not possible. It is intentionally conservative.
- Stage new recordings as `.tmp`; reopen and checksum **the actual file bytes** before committing.
- Rename the previous `.pcm` to `.bak`, then rename validated `.tmp` to `.pcm`. On failure, attempt to restore `.bak` immediately.
- On startup, recover interrupted saves: prefer valid `.pcm`, then valid `.bak`, then `.tmp` (if no destination); never automatically format a corrupted filesystem.
- When deleting, remove any `.bak` first so startup recovery does not resurrect the deleted track. If file deletion fails, retain the recording in RAM rather than showing it as deleted.
- Store locks and ordering together in one checksummed NVS blob. Existing individual `trkLockN` and `trkOrdN` preferences are migrated on first boot. Track names remain under their existing keys. Audio file paths and formats are unchanged.

## Limitations and recovery

- **Do not run Erase Flash or Upload Filesystem Image** on an existing device with recordings.
- There is no flash capacity expansion in this phase: four PCM slots remain.
- LittleFS does not offer a single atomic operation that spans all files, so recovery is a best-effort transactional pattern relying on rename operations and verified copies.
- If the filesystem fails to mount, recording is disabled; USB/serial log output will report the problem. A recovery tool to export files is a future project.
- A clip that fails checksum and has no valid backup is retained as an invalid file instead of deleted automatically. It is not played.
- If unexpectedly little free space remains, even a short recording may be blocked: this is deliberate to guarantee room for a full eight-second take.
- Saving metadata is still subject to NVS write failures; serial logging reports the failure. Metadata backups/export are not implemented.
- Existing on-device recordings must be confirmed after flashing, restarting, and switching tracks.

## Test checklist

1. Verify the upgrade preserves all existing four recordings, names, locks, and display order.
2. Create a fresh recording, restart, and check that it persists and plays without corruption.
3. Replace an existing *unlocked* recording and restart; confirm the new clip persists.
4. Confirm a locked recording refuses overwrite and delete.
5. Reorder tracks and restart; check that logical order and lock flags persist.
6. Confirm deletion of an unlocked track remains deleted after restart.
7. Repeat playback, FX, all three skins and palettes, landscape/portrait, and first playback after reboot.
8. Inspect serial output for `Storage:` warnings; do not deliberately corrupt or erase real recordings to test failure cases.

## Build verification

Asset generators and existing palette/FX tests were run. Full PlatformIO compilation and hardware tests were not available in this environment.
