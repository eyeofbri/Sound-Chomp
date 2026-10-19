# Phase 5F.1 — Portrait layout and first-play reliability

Base: tested Phase 5F (44-sprite packed atlas) with no changes to LittleFS tracks,
recording sample data, or the effect processing algorithms.

## Portrait player

- Transport button grid starts 3px lower (`h-62` instead of `h-65`).
- Time is now rendered with the same theme-colored atlas glyphs as landscape;
  its whole label is centered horizontally above the transport grid.
- Visualizer moves from y=97 to y=99 and shrinks from 65px to 62px high,
  keeping the bottom border clear of the new clock.
- Recording banner shrinks by 1px in height (36px to 35px).
- Normal compact status row under READY moves 1px up (y=51 to y=50).
- During player FX/speed/loop editing, portrait now displays the selected
  setting's name and current value instead of continuing to show compact status.

## Landscape player

- Edit status spells out `ECHO`, `STUTTER`, and `ROBOT`, replacing FX1/FX2/FX3.
- Both orientations now share one edit-status rendering routine.

## Startup audio mitigation

- On the first speaker startup since boot, pause for 60ms after initializing
  the speaker, then queue 16ms of silence ahead of the *unchanged* clip PCM.
- No samples from the recorded clip are skipped by this pre-roll.
- This is a targeted mitigation for missing attack transients on the first
  playback after reboot, not a confirmed root-cause fix. Confirm on device,
  using a clip with a sharp transient exactly at its start.

## Hardware validation

1. Flash through PlatformIO and inspect portrait at 135x240.
2. Check centered atlas clock and its clearance from waveform border.
3. Verify two-row portrait transport has more room at the screen bottom.
4. Edit all FX controls in portrait and landscape and check live values.
5. Reboot repeatedly; play a recording with a loud onset first each time.
6. Verify smooth loop playback, stutter, robot, echo tails, and track persistence.

No runtime skin loading or USB MSC is added in this maintenance build. Those
remain planned for the dedicated Phase 5G milestone.
