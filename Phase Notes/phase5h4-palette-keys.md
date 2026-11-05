> Historical development note: **superseded by Phase 5H.6**. Current green-background conventions are documented in `phase5h6-green-background-rollback.md`.

# Phase 5H.4 — Palette-keyed background gradients and yellow atlas guides

The 5H.3 hardware-tested implementation is preserved except for asset conversion.

## Editing convention

- **Yellow `#FFFF00`**: the one-pixel **OUTSIDE** guide frame around each sprite. Never included in its crop. Keep this frame intact and do not move or resize sprites; the JSON coordinate map is firmware-owned.
- **Magenta `#FF00FF`**: primary color token. For normal UI sprites this maps to the selected palette primary color.
- **Background magenta shading ramp**: `#140014` through `#FF00FF`, specifically `RGB(N,0,N)` with N in [20,255]. Brightness `N/255` multiplies the primary palette color. For example, `#800080` generates a half-bright primary highlight. This preserves shaded gradients without fuzzy detection of ordinary green objects.
- **Cyan `#00FFFF`**: secondary color token in icons (reserved in background design).
- Any other pixel is fixed artwork. Theme changes leave it alone. Yellow **inside** a sprite is a valid fixed warning yellow; only guides outside crop rectangles are excluded.

## Authoring process

1. Edit `assets/ui/sound_chomp_spritesheet.png` (CLASSIC), `assets/skins/minimal.png`, or `assets/skins/arcade.png`.
2. Only paint *within* existing yellow frames. Do not reposition frames or edit the locked JSON manifest.
3. In the backgrounds, use magenta-shaded values for recolorable light/gradient elements, and ordinary RGB colors for static decorations.
4. Click Build/Upload in PlatformIO. The prebuild hook validates all guide frames, generates all three built-in skins and GREEN/RED/BLUE variations automatically.
5. Optional: `python tools/validate_builtin_skins.py` to regenerate/validate on the desktop.

**Compatibility:** The saved skin and palette Preferences keys, recorder, audio effects, layout, branding and tape reels are unchanged. The PNG editing convention is changed; any outside older skin PNGs must be updated to yellow guides before use.

## Testing

- CLASSIC/MINIMAL/ARCADE each render in both orientations.
- GREEN/RED/BLUE recolor only keyed background gradients and keyed icon accents.
- Fixed green or decorative artwork remains unchanged when choosing RED or BLUE.
- Gradient shading remains visible and the transparent regions render correctly.
- Existing recordings, effects, palette and skin preference persist after reboot.
