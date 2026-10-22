# Phase 5I.1 — UI & controls stabilization

Based on Phase 5I. This is a development build, not an official release.

## Changes

- Long-press the Unit Scroll wheel on the focused **Play** control to switch to the next *recorded* track, wrapping past empty tracks. When playback is active, this stops it before changing track. A long press from any other player control still returns focus to Play.
- Runtime-drawn time labels replace legacy digit glyphs, eliminating their opaque dark backgrounds. Removed the old number and symbol glyph sprites from the editable atlas.
- Former timer-glyph cells 0–5 now contain six transparent, individually editable effect glyphs: Echo, Stutter, Robot, Warble, Bitcrush, and Telephone. Icon artwork is carried across all three built-in skin sheets, using fixed JSON coordinates and yellow guide frames.
- For normal transport icons, only *edge-connected* opaque black sprite pixels are made transparent; enclosed black marks are preserved. Reverse speed uses the dedicated reverse-chevron atlas sprite.
- Added consistent menu headings below the branding header, including Home, Tracks, Effects, Settings, How-To, About, and Charging.
- Portrait Tracks uses a second line for duration; footer shows `CLICK: OPEN` and `HOLD: CLEAR` on separate rows.
- Effects uses `FX:1` / `FX:2` / `FX:3` / `--` instead of SLOT / UNUSED. Focused editable field is highlighted using palette warning yellow; list is scrollable.
- Portrait Settings fills full width when no scrollbar is needed, aligns values right, and uses `REV MAP`.
- How-To uses orientation-specific row arrangement to prevent collisions and spells out `B: TOGGLE MENU`.
- Charging suppresses the header's small battery, centers status and percent around a wide battery, and uses proper portrait geometry.
- Removed About page's duplicate SOUND-CHOMP prefix and the recurring B: MENU footers.

## Artwork rules

- Keep yellow guide frames outside the sprite crops, unchanged.
- Keep primary `#FF00FF` and secondary `#00FFFF` palette keys inside sprites.
- Background panels retain the tested green-shade palette conversion (do not revert to experimental magenta background ramp).
- Built-in skins have the same locked atlas coordinates; skin authors edit PNG pixels only.

## Hardware checklist

1. Compile and upload from PlatformIO (COM9 may differ).
2. Check reverse speed icon changes from >> to << and old opaque black icon patches are gone.
3. Cycle through all six FX assignments; check each icon in all three skin modes and both orientations.
4. Long-press wheel on Play with more than one saved track; repeat while audio is playing and when another control has focus.
5. Verify portrait Tracks/Effects/Settings/How-To/About/Charging with menus full and scrolling.
6. Check all three palettes and confirm recordings and existing audio FX remain unchanged.

This package was validated using the Python asset generator and slot tests. It has **not** been compiled by PlatformIO or hardware-tested on an M5StickS3 in this environment.
