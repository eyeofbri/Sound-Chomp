# Phase 4C — Persistent recordings (development prototype)

Sound-Chomp saves each of four 16 kHz, 16-bit mono, up-to-eight-second PCM tracks to the onboard flash filesystem and restores them after reboot.

## Important flash-storage warning

This firmware calls `LittleFS.begin(true)`. On a first-time or incompatible filesystem mount, LittleFS may format the existing **filesystem partition**. It is **not** intended to erase the full device firmware, but any older filesystem data within that partition can be lost. Make a backup of any important existing stored content before flashing. The current Phase 4B firmware used RAM-only recordings.

This build retains `default_8MB.csv` and adds `board_build.filesystem = littlefs`; no custom partition table is required. It does not require a filesystem image upload.

## Recording storage

- Slot filenames: `/chomp01.pcm` through `/chomp04.pcm`.
- File format: 20-byte header (magic, format version, sample rate, sample count, checksum), followed by little-endian signed 16-bit PCM.
- Tracks are loaded on boot; corrupt or incompatible files are skipped and reported over the Serial Monitor.
- Finishing recording attempts a save. Track deletion removes its corresponding flash file.
- Saves first write a temporary file, check its length, then replace the destination. Power loss during final file replacement is not fully transactional.
- Recording works in RAM even if flash fails; watch Serial Monitor for `WARNING` messages.
- When flash is nearly full, a replacement save may fail because writing the new clip temporarily needs additional space. The last flash-saved copy may remain unchanged while the currently recorded audio exists only in RAM.
- Do not repeatedly record in a tight loop unnecessarily; flash has finite write endurance.

## Screen refinements

- The empty-track text is rendered in the smallest built-in bitmap font with a centered short line (`TAP A: PLAY  HOLD A: REC`), eliminating prior oversized wrapping.
- Transport time digits have an additional 1px horizontal advance for readability.

## Test checklist

1. Build and flash; record a short sound into each of the four slots.
2. Verify playback is smooth at +1.0x and speed changes remain responsive.
3. Power-cycle the M5StickS3, open Tracks, and confirm all four recordings persist and play.
4. Delete a slot, reboot, and verify it stays empty; other slots should remain.
5. Overwrite one slot, reboot, and verify the new audio loads.
6. Verify no unexpected flash initialization or save failures in Serial Monitor.
7. Confirm the empty prompt stays on a single line and timer digits are legible.

## Next: Phase 5 — Echo

After hardware validation, add an optional real-time echo effect that does not modify stored source audio. Start with a fixed delay, feedback amount, and on/off toggle. Keep the existing triple-buffer playback method.
