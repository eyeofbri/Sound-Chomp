# Phase 5E — Echo Tail, Stutter & Robot

Experimental update based on hardware-validated Phase 5D.1. **Not yet built or hardware-tested** on M5StickS3.

## New audio effects

- **FX-1 — Echo:** Mixes delayed samples into playback with gentle feedback. When LOOP OFF reaches the end of the recording, queued silent input allows the feedback to decay for up to 3 seconds (five delay cycles, capped at 3 seconds). Button A or Scroll PLAY can interrupt the tail.
- **FX-2 — Stutter:** Samples a small grain (80–400 ms) and repeats it while enabled. The saved source clip is never changed. Rotate upward from OFF to enable, then adjust grain duration in 20-ms steps; rotate downward from the minimum to disable. The original clip transport continues advancing underneath the effect.
- **FX-3 — Robot:** 55 Hz square-wave ring/amplitude modulation, 0–100% mix. Rotate upward from OFF to enable and adjust intensity in 5% steps; rotate downward from the minimum to disable. This intentionally produces a buzzy robotic timbre.

## Player & menu controls

Player controls are unchanged: wheel rotation navigates; click enters Adjust; rotation adjusts; click returns to Navigate; long-click focuses PLAY. Button A tap plays/stops and hold records.

In the **Effects** menu, rotate to choose Echo, Stutter, or Robot. Click begins editing: enabled state, then intensity/duration (Echo also includes delay). Click again to finish. B returns to the main menu. Settings are persisted in NVS; original LittleFS PCM recordings stay unchanged.

## UI cleanup

- Keep `icon_teeth`: header and splash still use it.
- Remove unused `icon_reel` from the atlas and regenerate its header. The earlier procedural tape reels remain in use.
- Remove redundant `assets/backgroundLandscape.png` and `assets/backgroundPortrait.png`. The canonical atlas provides backgrounds.
- Remove single charging-color pixel inside the battery fill; theme-colored fill remains.

## Validation checklist

1. Build/upload using COM9, confirm normal playback sounds as clear as Phase 5D.1 with all FX off.
2. Record a 2-second clip, enable Echo (35%, 250ms), LOOP OFF, confirm audible decaying repeats after the clip ends. Verify tap A or wheel PLAY stops tail.
3. Verify Echo remains smooth at different speeds, reverse, LOOP ONE and LOOP ALL.
4. Activate FX-2, alter 80–400ms slices during playback, disable again. Verify no new audio gaps.
5. Activate FX-3 and vary intensity, then disable; verify volume and quality at normal speed.
6. Confirm combining effects doesn't create excessive distortion; reduce volume/mix if needed.
7. Confirm FX1/2/3 indicator lights and the one-line status row.
8. Restart: effect preferences and all four saved tracks survive.
9. Confirm no reel visual regression or battery stray pixels.
10. Edit the atlas and build without manually generating assets.

## Notes

The experimental FX-2 grain and FX-3 ring-modulator are computationally simple, by design. They are not studio-quality effects, and effect combinations can distort or clip. The three-buffer M5Unified output schedule is unchanged.
