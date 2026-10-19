# Phase 4D — Tape transfer, loop modes, and selected-track persistence

Built from Phase 4C's tested LittleFS and triple-buffer audio prototype.

## What's new

- Selected track (1–4) is saved in `Preferences` under `soundchomp` / `active`, and restored on startup. This preference is stored independently of each PCM track's LittleFS file.
- Long Unit Scroll press on Chomp cycles **OFF → 1 → A → OFF** (where **A** means ALL populated tracks). The transport row uses the existing loop glyph, with a `1` or `A` badge when enabled.
- LOOP 1 wraps within the active track. LOOP ALL advances to the next nonempty track in numeric order, wraps after track 4, and skips empty slots. If only one slot contains a recording, LOOP ALL repeats that slot. LOOP OFF stops at the end of the current track.
- LOOP ALL selects the new track as the active track; that selection also persists across a reboot. No clip files are modified by looping.
- Splash `SOUND-CHOMP` uses the 2× bitmap font (12 px per character) and centered text width for 240×135 landscape. The tooth impact and pixel crumble remain, and the splash can be skipped.
- Tape reels rotate counter-clockwise during forward playback and clockwise in reverse. Colored tape layers around the left reel shrink as the right grows. Tape transfer follows recorded playback position; on RECORD the tape builds toward the right based on captured samples. In READY, changing speed with the scroll nudges rotation and tape fill visually; hitting a speed limit instead flashes reel spokes red.
- Reverse accent uses purple tape; forward accent uses green.
- Offscreen canvas rendering remains unchanged.

## Hardware test checklist

1. Record different clips in slots 1, 2, and 3, select slot 3, reboot, and confirm slot 3 is active.
2. Restart again after selecting an empty slot and confirm that slot is still selected.
3. Play in LOOP OFF and confirm playback stops at the end.
4. Select LOOP 1 and confirm the same clip repeats and the timer resets each pass.
5. Select LOOP ALL, confirm playback moves through populated slots, skips slot 4 if empty, and returns to the first populated slot.
6. Verify the active slot selected through LOOP ALL survives restart.
7. Repeat LOOP 1 / ALL in reverse playback and while changing playback speed.
8. Confirm left tape ring drains/right fills in forward playback, reversing in reverse playback; check scroll nudges in READY and red reel spokes at speed limits.
9. Check splash centering and skippability and listen for any pops or stutters at track boundaries.

## Limitations / caution

- Changing tracks during LOOP ALL uses queued audio chunks. The screen can update slightly before the last queued samples from the previous track play.
- Tape rings represent normalized clip position, not an accurate physical tape pack; no reel-radius-dependent speed calculation is used.
- Flash storage still mounts with `LittleFS.begin(true)` from Phase 4C; a failed mount can format the filesystem partition. Back up important recordings before trying a new firmware version.
- This source package has not been built on the user's Windows PlatformIO system or tested on the physical M5StickS3.
