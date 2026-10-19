# Phase 5B.2 — RGB565 and player layout fixes

Incremental update to the Phase 5B.1 UI implementation. No audio, track-storage or effects pipeline changes.

- Correct RGB565 byte order for embedded portrait/landscape bitmap arrays. If colors are still swapped on hardware, check the drawing driver byte-swap configuration instead of applying a second swap.
- Remove solid control-button fills and opaque button text backgrounds, so embedded art shows through.
- Widen the landscape transport buttons into previously unused space and center the narrower right-hand time display.
- Restore the compact custom seven-segment timer digits.
- Show current signed speed (e.g. `+1.0x`, `-2.5x`) in the speed control in place of SPD.
- In portrait, hide battery percentage but retain battery icon, avoiding overlap with SOUND-CHOMP title.

## Testing on physical StickS3
1. Confirm that the gradient is green, not red, in both landscape and portrait.
2. Check +1.0x, -3.0x, +3.0x speed labels and all six transport buttons for clipping.
3. Verify selected-button highlight and amber adjustment mode remain readable on the background.
4. Verify the custom timer fits in the timer box and stays centered for short and long durations.
5. Verify portrait title and battery icon never overlap.
6. Confirm audio, recording, saved tracks, live echo and loop modes behave identically to Phase 5B.1.

Source packaged but not compiled or tested on device; keep your previous working commit as a rollback.
