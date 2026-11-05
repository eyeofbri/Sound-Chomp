# Phase 5J.4 — FX assignment confirmation and Track UI refinements

- FX slot editing now previews `--` / FX:1 / FX:2 / FX:3 without displacing another effect until the user clicks to confirm the FX field. Exiting by B before confirmation discards the preview.
- Tracks heading displays LOCK or UNLOCK according to the selected track's current state.
- Locked track action: orange (palette active) fill, black padlock; redundant L/! markers removed.
- Wheel navigation skips disabled Delete for a locked track. Deletion is still guarded in click handling.
- Landscape Settings now fits six 11px-high rows within its visible area; portrait rows are unchanged.

## Suggested hardware checks
1. Navigate Echo FX:1 to FX:2 on the wheel, without clicking: the existing FX:2 effect remains assigned; back out with B and confirm nothing changed.
2. Repeat and click to confirm: previous FX:2 effect becomes unassigned, new assignment is saved.
3. Lock a track: orange lock tile appears, heading says UNLOCK when focused, delete is skipped on scrolling, no stray L/! remains.
4. Unlock it: heading reads LOCK, delete navigation returns.
5. Landscape Settings shows six rows without overlap or clipped footer.
6. Verify effects, audio, recording, and track persistence.

Validation run: FX slot assignment test, palette regression test, builtin-skin validator, asset generators, Python syntax; not compiled with PlatformIO or tested on physical hardware here.
