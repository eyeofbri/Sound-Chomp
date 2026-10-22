## Phase 5J (development)
- Introduce track-list actions, on-device 10-character display-name editing and safe hold-to-delete modal.
- Persist names in NVS Preferences separately from LittleFS audio files.
- Add filled palette heading on non-player menu pages and consistent action focus.
- Keep storage at four slots pending a safe six-track migration.
- Hardware compilation and device tests still required.

## Phase 5I.1 — UI & Controls Polish (development)

See `PHASE_5I1_NOTES.md` for source-level changes and hardware test checklist.

## Phase 5I — Configurable FX slots (development)

- Six FX library entries with one-to-one assignment to three player slots; UNUSED support and displacement.
- New non-destructive Warble, Bitcrush, Telephone effects.
- Scrollable Effects menu, retained player effect editing, and persistent assignments.
- Built on 5H.6 artwork and green-shade background recoloring.

## Phase 5H.6 — Background color rollback

- Restore proven green-dominant background recoloring for all three built-in skins.
- Recolor edited CLASSIC magenta background gradients back to green shades.
- Retain yellow atlas editing guide frames and magenta icon color keys.
- Add regression test. No functional audio or UI changes.

## Unreleased — Phase 5H.3 (palette terminology and header alignment)
- Rename Settings THEME to PALETTE (PAL on portrait) without changing stored palette selections.
- Lower player-only header artwork, title, and battery by 1px in landscape and portrait.
- Preserve skin generation, all audio processing, layout and recording behavior.

## Phase 5H.2 — Built-in skins
- Replace experimental USB / runtime skins with three compiled choices: CLASSIC, MINIMAL, ARCADE.
- Preserve fixed canonical manifest, themes, protected branding and clean player backgrounds.
- Keep LittleFS audio and effects unchanged.

## Phase 5H.1 — USB media + skin background hotfix

- Correct multi-sector USB MSC callback range handling to address Windows no-media failure.
- Mark MSC RAM disk writable and reject writes after eject.
- Show skin backgrounds under player header and visualizer instead of solid panel fills.
- Add visible narrow Settings scrollbar and tighten Settings item width.
- Document compiled USER.SKN format and Python-only skin build workflow.

## Phase 5H (USB Skin Drive prototype)
- Add PSRAM FAT12 USB Mass Storage drive for drag-and-drop skin installation.
- Reuse Phase 5G skin validator and retain private LittleFS recordings partition.
- Add Settings USB SKINS action and Eject/press-A flow.

## Phase 5F.2 — Portrait layout refinement

- Lower portrait reels by 1px without modifying landscape reel positioning.
- Shorten portrait visualizer by 4px (62px to 58px) and move it down 4px (y=99 to y=103), retaining the same bottom edge to preserve timer/button spacing.
- Horizontally center portrait edit-detail status strings beneath READY/PLAYING using measured text width. Normal direction/track/effect status remains centered.
- Preserve Phase 5F.1 audio-start mitigation, themes, sprites, effects, and recording format.

## Unreleased — Phase 5E (experimental)

- Added FX-2 Stutter and FX-3 Robot as non-destructive playback effects.
- Allowed Echo to decay after a one-shot clip without looping.
- Added effect preferences and streamlined Effects menu.
- Removed obsolete standalone backgrounds and unused reel sprite; retained teeth art.
- Removed isolated charging pixel inside battery bar.

## Unreleased — Phase 5C

- Integrate v1.1 canonical sprite atlas for native backgrounds and selected transport icons.
- Support in-sprite theme-key recoloring, guide-border-safe cropping, and reel overlay composition.
- Add deterministic PNG-to-C++ asset generator.

## Phase 5B.3 — UI layout corrections (prototype)

- Remove opaque main player heading/detail text backgrounds and timer suffix background.
- Correct automatic portrait/landscape axis mapping.
- Shift landscape status 2 px lower and grow analyzer by 3 px.
- Make portrait buttons 2 px taller while preserving lower edge, center status/details, and separate reels from waveform.
- No changes to stored recordings or playback engine.

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

## Unreleased — Phase 5D (atlas and themes)
- Add three four-role palettes (GREEN/RED/BLUE) and persist user choice.
- Complete atlas rendering for FX icons and custom time glyphs; conditionally use atlas frames when they fit.
- Improve native-size reel cover and constrain tape-fill circles behind its windows.
- Use retro bitmap backgrounds only in player; menus return to neutral surfaces.
- Automatically regenerate UI assets during PlatformIO builds using the Python standard library.
- The four recorded tracks and audio effects processing are unchanged.

## Unreleased — Phase 5D.1 (player micro-refinements)
- Restore the earlier Phase 4D procedural reels with circular colored tape bands, clean rotating spokes, and black centers; atlas reel artwork stays available for later edits but is not rendered by the player.
- Replace duplicate speed and multi-line echo details with one compact status line containing direction, track, enabled FX1, and loop state.
- Center the portrait recording dot and REC label as a unit; adjust READY / PLAYING status down by 1px.
- Give landscape transport buttons an additional pixel of width without changing icon or text sizes, and shift Play/Pause icon down 1px.
- Remove the timer border, using warning color while idle and active color during playback.
- Remove the low-contrast menu selection fills; retain themed focus borders.
- Move the charging indicator pixel inside the battery frame to prevent the white/bright pixel above the battery.

## Phase 5F — Skin foundations (unreleased development)

- Move theme-aware Sound-Chomp teeth logo to firmware-owned `include/branding.h`; exclude it from community-editable atlases.
- Deterministically pack 44 remaining sprite crops into 244x383 PNG with magenta guide borders and JSON coordinates automatically rewritten.
- Add optional `tools/pack_spritesheet.py`, atlas-guide validation and `--validate-only` mode.
- Draft future skin metadata contract; no runtime loading or USB functionality yet.
- Keep audio effects, recording format and saved-track handling unchanged.

## Phase 5F.1 — Portrait player polish and first-play audio mitigation
- Lowered portrait controls, centered theme-colored atlas clock, and reduced visualizer height.
- Added live edit details in portrait and full effect names in landscape.
- Shortened portrait REC banner by 1px and raised status text by 1px.
- Added one-time post-boot speaker settle and silent preroll to protect initial clip samples.

## Phase 5G — Runtime user skins (v0.8.0-dev)

- Added validated external `User` skin stored in LittleFS and rendered from PSRAM, with compiled `Classic` fallback.
- Added Settings > SKIN (Classic/User), persisted through Preferences.
- Added standard-library Python custom-skin compiler with full native dimensions / magenta guide validation.
- Added USB-serial installer that stages a `.skin` file without bulk filesystem upload or recording replacement.
- Kept firmware-owned teeth branding, procedural reels, audio path, recording filesystem and Phase 5F.2 portrait geometry unchanged.
- No USB Mass Storage or multi-skin browser yet; this is a hardware-unverified development build.

## Phase 5H.4 (development)
- Replace 1px magenta atlas guide borders with yellow (`#FFFF00`) borders across all built-in skins.
- Use exact magenta-keyed shading for recolorable background pixels; remove fuzzy green-dominant hue detection.
- Update PNG validators, optional packer, manifest and customization documentation.
- Preserve hardware-tested Phase 5H.3 functionality; device build/test pending.
