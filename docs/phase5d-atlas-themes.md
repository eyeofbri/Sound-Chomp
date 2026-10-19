# Phase 5D — Atlas and theme completion (development candidate)

## Scope
- Builds on tested Phase 5C. Audio buffering, four LittleFS clip slots and storage format remain unchanged.
- Player uses atlas landscape/portrait backdrops. Other menus deliberately use a flat dark background.
- Atlas-based glyphs: teeth logo, transport, direction/loop, FX icons, and custom time digits. Frame sprites are used when the native frame dimensions fit the current button slot; otherwise the existing vector borders remain. Dynamic battery, tape fill, progress, waveform, and text remain code-drawn.
- Reworked native-size reel cover with larger transparent windows. Tape circle is clipped to the cover diameter and rotates by source pixel lookup without geometric scaling.
- Three named color themes. See roles below. Reverse uses the chosen theme's secondary color rather than a fixed purple.
- PlatformIO pre-build script regenerates `include/generated/ui_assets.h` directly from the canonical PNG/JSON every build, using only Python standard library.

## Theme roles

| Name | Primary | Reverse/secondary | Warning | Active |
|---|---|---|---|---|
| GREEN | `#00FF00` | `#FF00FF` | `#FFFF00` | `#FF9900` |
| RED | `#FF0000` | `#00FFAE` | `#FFFF00` | `#FF9900` |
| BLUE | `#00FFF7` | `#FF0044` | `#FFFF00` | `#FF9900` |

The build script also derives green/red/blue background gradients from the one original atlas background without duplicating the editable PNG. Color remapping preserves neutral metallic framing.

Primary is used for forward accents, teeth/title/battery, waveform and selected menu items. Reverse maps to secondary. Warning is for peaks/limit indicators; active marks enabled or editing state.

## Editing
Edit `assets/ui/sound_chomp_spritesheet.png`, **do not move the 46 JSON rectangles**, and build in PlatformIO. Guide pixels (`#FF00FF`) outside rectangles are ignored. Inside crop rectangles the same magenta triggers the primary theme role; opaque `#00FFFF` triggers secondary. The generated header is committed for reference and updated automatically during the build.

## Test plan
1. Build and flash; confirm automatic atlas regeneration appears in build output.
2. Check reel covers are crisp and tape never grows past the outer rim in landscape and portrait.
3. Verify backgrounds appear only in the player, not the menus.
4. Verify timer, PLAY/SPEED/FX/LOOP icons and selection borders are legible, especially in portrait.
5. Select GREEN, RED and BLUE and confirm normal/reverse palettes, title, teeth, battery and menu highlights change immediately and persist after restart.
6. Confirm recordings persist, loop-one/all work and echo remains clean at 1.0x.
7. Confirm splash teeth and text follow saved theme color.

## Status
Static checks and PNG asset generation only; **not yet compiled in PlatformIO or tested on hardware**. Keep Phase 5C as a known-good rollback.
