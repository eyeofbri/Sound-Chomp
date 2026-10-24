# Phase 5J.3 — Track interaction feedback

Based on Phase 5J.2. This is a narrow UI-only patch; PCM storage, track metadata, reordering behavior, and audio processing are unchanged.

## Changes
- Tracks heading: when a row action has focus, shows yellow `SELECT`, `EDIT`, `DELETE`, `LOCK`, or `MOVE` at the upper-right of the colored heading under the battery (on a contrasting dark inset).
- Reorder overlay: displays `POSITION: X` with X representing the live one-based position within the currently reordered track list. Continues updating as the wheel moves the item.
- Track Select icon: changes the previous diamond-shaped symbol into a native-pixel right-facing filled triangle.

## Hardware test suggestions
1. Verify action heading labels remain legible in landscape and portrait with GREEN, RED and BLUE palettes.
2. Enter Move and scroll an item across all four positions. Confirm Position counts 1–4 and row remains partially visible behind the overlay.
3. Confirm the first action looks like a small right-facing Play icon rather than a diamond.
4. Verify save/rename/lock/delete and all audio effects remain unaffected.

## Verification
Asset generation and script checks completed locally; full PlatformIO compilation and M5StickS3 testing have not been performed here.
