## Unreleased — Phase 5B.1 (experimental)
- Explicit navigation/adjust control modes with green/amber selection indicators.
- FX1 now handles Echo; FX2/FX3 reserved for distinct future effects.
- Consolidated Echo fields into a compact settings row.
- Cleaner player button labels, timer placement and track notation.
- Native-resolution pixel-art background images for landscape and portrait.
- Locked landscape, locked portrait and IMU-driven auto orientation.
- Reordered Effects before Settings on Home.

## Unreleased — Phase 5B (`v0.6.0-dev`)

- Add bottom six-control focus strip with PLAY, SPEED, FX-1, FX-2, FX-3, LOOP.
- Scroll click on PLAY toggles playback; other focused buttons click to next function.
- Scroll long-click returns to PLAY; wheel changes the focused parameter.
- Update contextual player readouts, `PLAYING` status, home B behavior, and startup version.
- Preserve persistent tracks, LittleFS, playback buffers and non-destructive echo.

## Unreleased — Phase 4C Persistent Tracks

- Save four PCM recordings to LittleFS and restore at startup.
- Remove saved audio when a track is deleted.
- Add file integrity validation and logging for storage failures.
- Shrink the empty recording prompt and widen timer character spacing.
- Keep the triple-buffer speaker playback engine unchanged.

## Unreleased — Phase 4A.5

- Targeted reel animation, icon, banner, timer and waveform prompt corrections.
- Preserves the existing Phase 4A.4 audio pipeline and input behavior.

# Changelog

## Unreleased — Phase 4B multi-track prototype

- Implement four selectable RAM audio slots, each holding up to eight seconds.
- Add Tracks browser with active marker, duration, selection and delete confirmation.
- Keep clip data isolated between tracks and refresh waveforms on selection.
- Fix loop timer wrapping and avoid reel motion at capped wheel positions, flashing reel spokes red.
- Fix idle guidance text overflow; enhance transport arrows and recording indicator.
- Reduce splash wordmark and introduce pixel-crumble impact effect.
- Pending on-device build and testing; persistent saving is deferred.


## Unreleased — Phase 4A.2 controls and UI

- Tap Button A to play/stop; hold for 300 ms to record and release to finish.
- Optional automatic playback immediately after recording.
- Extend forward/reverse speed to 3x with finer steps near 1x.
- Choose conventional or inverted reverse speed mapping in Settings.
- Bright red RECORDING banner replaces speed display during capture.
- Improve battery terminal and percentage placement and add original pixel teeth glyph.
- Preserve Phase 4A.1 queued audio buffering without changing its playback algorithm.

## v0.3.0 — Playback effects and interactive UI

- Hardware verified: playback effects, smooth audio buffering, navigation, waveform and spectrum visualization, and preferences.

## Phase 4A.1 playback quality patch

- Queue adjacent playback chunks instead of waiting for each one to finish.
- Use three rotating audio buffers to avoid overwriting samples still in use.
- Reduce spectrum analysis workload during playback.
- Preserve live signed-speed control, navigation, settings, and existing UI.
- Pending hardware verification of 1.00x clarity and live wheel responsiveness.

## Phase 4A prototype

- Introduce Home, Chomp, Tracks (placeholder), and Settings navigation.
- Make Key2 the global Back button and move playback to scroll click.
- Add signed varispeed -2.0x to +2.0x; 0 pauses, negative speeds reverse.
- Stream short PCM playback chunks to apply wheel speed changes during playback.
- Add battery indicator and persistent volume / brightness settings.
- Add green/yellow/red spectrum level thresholds and purple reverse accents.
- Retain full-frame canvas-based flicker-free rendering and hold-A recording.

## Previous prototypes

See `docs/phase3a-effects.md` and `docs/phase3b-visualization.md` for earlier stages.

## Unreleased — Phase 4A.3 UI refinement

- Start directly in Chomp following a skippable two-jaw splash animation.
- Add How To and About menu pages (About identifies u/eyeofbri and GitHub repository).
- Redesign Chomp screen with compact top status/speed, full-width recording banner with a blinking recording lamp, and lower cassette-like playback-direction and loop indicators.
- Move help prompts off the player screen; B opens the menu.
- Provide consistent 0.10× speed steps from -3.00× to -0.10× and +0.10× to +3.00×, skipping the paused-zero position.
- Add configurable resetting of the playback speed to +1.00× after recording (default enabled; saved in Preferences).
- Stretch the idle waveform timeline for slow playback and compress it for fast playback; preserve the reverse waveform presentation.
- Display elapsed playback time in place of idle clip duration; align this time with the right-side header indicator.
- Continue using queued triple-buffer playback to prevent audio gaps.

This is a source prototype, not yet compiled or hardware validated.

## Unreleased — Phase 4A.4

- Remove heavily distorted playback speeds below 0.50x in both directions.
- Correct cassette-style forward/reverse icons and add speed-reactive tape reels.
- Add purple spectrum bands for reverse playback.
- Increase recording banner height and reduce brightness minimum to 5%.
- Introduce compact two-column Home menu with icons and an Effects placeholder.
- Add Charge screen with battery gauge and screen-off timer.
- Color-key How To help, Built for Aiden credit, and other player layout refinements.
- Preserve the proven three-buffer speaker playback pipeline.

## Unreleased — Phase 4D tape-transfer transport

- Remember and restore the selected recording slot with Preferences.
- Cycle LOOP OFF, LOOP 1, and LOOP ALL with a long scroll press; skip empty tracks in LOOP ALL.
- Center and enlarge the splash title while retaining the two-jaw pixel crumble.
- Render tape transfer with variable colored reel rings and correct rotation directions.
- Preserve Phase 4C's LittleFS clip format and three-buffer playback.

## Unreleased — Phase 5A live echo prototype

- Introduce an Effects page: echo enable, mix 0–100%, and delay 100–600 ms.
- Mix a non-destructive feedback echo into generated playback audio without modifying the LittleFS clips.
- Reassign Chomp short scroll press to toggle Speed / Echo control, including during playback.
- Keep A tap for Play/Stop, A hold for Record, and long scroll press for looping.
- Show `ALL(n)` for Loop All, with the active track number; continue skipping empty slots.
- Persist echo configuration in NVS; preserve Phase 4D's three-buffer speaker queue and saved track format.
- See [Phase 5A test guide](docs/phase5a-echo.md). This package has not been compiled or tested on hardware.

## Unreleased — Phase 5B.2 UI hotfix
- Correct embedded RGB565 artwork byte order to address red/green gradient inversion.
- Remove opaque backgrounds behind transport controls.
- Expand landscape transport widths and recenter the seven-segment timer.
- Show live playback rate on speed control and hide battery percentage in portrait.
