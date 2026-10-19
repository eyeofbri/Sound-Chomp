# Phase 5C — Canonical sprite integration (experimental)

The firmware now consumes generated assets derived from `assets/ui/sound_chomp_spritesheet.png`.

## Integrated assets
- Both 1:1 background panels from the atlas
- Header teeth logo
- Player PLAY/PAUSE, SPEED and LOOP icons
- Rotating reel cover, drawn over dynamically sized tape-fill circles

Other icons/glyphs are included in the generated header but some remain drawn procedurally until subsequent integrations. This deliberately reduces visual/audio regressions. Button backgrounds and most dynamic text remain canvas-rendered.

## Regenerating after pixel edits
1. Edit the existing 512x512 RGBA PNG; **do not move the JSON rectangles**.
2. Keep the magenta guide border OUTSIDE the content region.
3. Inside the content, exact opaque `#FF00FF` is replaced with the current accent (green forward, purple reverse); `#00FFFF` maps to the secondary color (white initially).
4. Run `python -m pip install Pillow` once.
5. Run `python tools/generate_ui_assets.py` from the project root.
6. Build and upload using PlatformIO.

The `include/generated/ui_assets.h` is committed, so other contributors can build without Python/Pillow. The original `retro_background.h` is left as a rollback reference, but firmware now uses the atlas-generated background header.

### Important: graphics constraints
Background conversion is deliberately byte-swapped RGB565 for the current M5Canvas pushImage call. For dynamically drawn icon pixels, RGB565 is not swapped. This distinction is tested via source assertions.

### Testing on device
- No distorted background colors (compare Phase 5B.3)
- Teeth, play/pause, speed, loop icons render correctly
- Forward/reverse tinting reflects selected direction
- Reel windows show tape fills and rotate correctly
- Playback, echo, LittleFS storage and auto-rotation are unaffected
- Flicker-free whole-canvas frame updates remain intact
