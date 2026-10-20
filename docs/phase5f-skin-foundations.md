# Phase 5F: skin foundations (build-time, not runtime)

Phase 5F is based on the working Phase 5E firmware; no recording, audio, effects, UI navigation, LittleFS or orientation logic was intentionally changed.

## Ownership model

* Firmware-owned: `include/branding.h` contains the original 16x12 teeth graphic as RGBA-keyed pixels. It remains recolorable using the active primary theme but is **not editable** in an ordinary skin.
* Editable artwork: `assets/ui/sound_chomp_spritesheet.png` and `sound_chomp_spritesheet.json`. The 44 sprites retain the original names/sizes and include both landscape and portrait player backgrounds.
* The old reel artwork is deliberately absent. Reels remain code-drawn.

## Compact atlas and packing

* Previous canvas: 512x512 (262,144 pixels). New canvas: 244x383 (93,452 pixels), a **64.4% canvas-area reduction**. PNG bytes are smaller too, but the firmware stores the same number of **content pixels**, so expect little/no flash reduction from packing alone.
* Each sprite has a **1px solid #FFFF00** guide frame OUTSIDE the mapped rectangle; an additional transparent pixel separates neighboring frames. Magenta pixels INSIDE a crop retain their separate primary theme-key meaning.
* To repack an edited atlas, optionally install Pillow (`python -m pip install Pillow`), then run `python tools/pack_spritesheet.py`. The script repacks all crops deterministically and **updates the PNG and JSON together**. Use `--dry-run` for dimensions only. Review PNG + JSON together in version control. Packing is never triggered by normal builds.
* To validate only: `python tools/generate_ui_assets.py --validate-only` (Python standard library only). Checks dimensions, coordinates, overlaps/guide spacing, branding exclusion and guide frames. Normal PlatformIO builds run this validation and regenerate `include/generated/ui_assets.h` automatically.
* Existing artwork changes should be made in the **packed PNG**, not an extracted duplicate. The JSON provides authoritative crop coordinates. Never hand-rearrange sprites without updating JSON.

## Future skin contract (proposal)

See `skins/examples/skin.json` for a **metadata example**, not an installable skin. Eventually, each installed skin would contain `skin.json`, `spritesheet.png`, and `spritesheet.json`, with stable sprite names and native dimensions. Only the artwork/crops would change; layout and controls stay firmware-owned. Invalid skins must fall back to the bundled Classic skin.

**Not yet implemented**: runtime skin loader, USB Mass Storage device mode, an on-device skin selector, skin package installation, or a user-skin filesystem partition. LittleFS recording storage is unchanged.

## Suggested testing

1. Build and upload the unchanged Phase 5E functionality with the new generated atlas header.
2. Verify splash and header teeth logo in GREEN, RED and BLUE.
3. Check both player backgrounds, timer digits and icons against Phase 5E; positions should be unchanged.
4. Test recording, playback, Echo tail, Stutter, Robot, Loop and LittleFS persistence.
5. Edit a non-logo atlas pixel and rebuild to confirm it appears on the device.
6. Confirm validation rejects a deleted guide pixel or conflicting crop rectangle.

Validation and atlas-generation scripts were run in the packaging environment. **PlatformIO compile and on-device tests were not run here**.
