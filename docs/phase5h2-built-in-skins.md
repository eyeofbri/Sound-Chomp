# Phase 5H.2 — Built-in skins

The firmware provides **CLASSIC**, **MINIMAL**, and **ARCADE** under Settings → SKIN. Themes GREEN/RED/BLUE work independently. Skin choice persists in Preferences.

## Editing skins

Edit only artwork in `assets/ui/sound_chomp_spritesheet.png` (Classic), `assets/skins/minimal.png`, or `assets/skins/arcade.png`. Do **not** move sprites, resize the PNG, edit yellow guide borders, or change the locked mapping (`assets/ui/sound_chomp_spritesheet.json`). The fixed mapping is reused for all three. `assets/ui/sound_chomp_spritesheet.png` is the authoritative CLASSIC source. No duplicate Classic PNG is kept.

PlatformIO generates `include/generated/ui_assets.h` and `include/generated/builtin_skins.h` before each build. No USB skin drive, PySerial, or runtime user skin installer is used. Teeth branding and tape reels remain firmware-owned.

This is an experimental build: verify firmware flash use, Settings rendering in both orientations, sprite colors, audio, and LittleFS recordings on physical hardware. Do not use `uploadfs` or erase flash.


**Phase 5H.4 convention:** yellow (`#FFFF00`) boundaries are guide-only and excluded from output. Primary-keyed sprite art uses magenta (`#FF00FF`). For background gradients use exact magenta shades `(N,0,N)` where 20 <= N <= 255; normal colored pixels are never automatically recolored. Theme-specific output is generated at build time.
