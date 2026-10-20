# Phase 5H.6 — Green-shade background rollback

This restores the working Phase 5H.3 **green-dominant** background tinting algorithm after attempts at explicitly magenta-keyed backgrounds (5H.4/5H.5) failed hardware testing.

- Source backgrounds use **green shades** for palette-sensitive gradients. A GREEN palette displays the source artwork unchanged. RED and BLUE substitute their primary colors for strongly green-dominant background pixels, preserving brightness.
- Yellow (`#FFFF00`) 1-pixel editing guides remain **outside all sprite crop rectangles** and are excluded from compiled artwork.
- Exact magenta (`#FF00FF`) **inside non-background icons** still means primary palette tint; cyan (`#00FFFF`) remains the secondary key.
- Fixed-color details in backgrounds remain fixed.
- The user's edited CLASSIC sheet is included, with magenta gradient pixels converted back to green shades. The PNG layout/locked JSON coordinates are unchanged.
- No firmware audio, tracks, UI layout, or palette menu changes.

## Test

`python tools/test_palette_remap.py`

Build normally in PlatformIO. Confirm green/red/blue palette switching on CLASSIC, including both display orientations. Physical hardware verification is required before release.
