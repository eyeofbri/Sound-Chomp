# Phase 4A.3 — Player polish

This prototype builds on tested Phase 4A.2 and retains the Phase 4A.1 queued-speaker-buffer fix.

## Startup & navigation
- Startup shows a large two-jaw chomp animation and lands in **Chomp**.
- Tap A or B or press the Unit Scroll wheel to skip the startup animation.
- B opens **Home** (menu), where wheel rotation selects Chomp, Tracks, Settings, How To, or About; A / wheel click opens a selection.
- Tracks still represents one RAM clip. Persistent multi-track audio is a later milestone.

## Chomp controls
- Tap A: Play / Stop. Hold A for ~300 ms: record. Release: stop recording.
- Unit Scroll rotation: live playback speed, steps of exactly 0.10×, up to ±3.00×. There is **no zero/pause detent**: between +0.10× and -0.10× the playback direction changes immediately.
- Unit Scroll short press: Play / Stop. Long press (~650 ms): Loop toggle.
- The reverse-mapping preference still controls which reverse magnitudes correspond to encoder positions.

## Visuals
- The header always contains the little two-jaw pixel teeth logo and battery meter.
- Recording replaces the status region with a red banner and a black-outlined lamp alternating red/black every 250 ms (two complete on-off cycles per second).
- Player instructions now live in **How To**, not on the player.
- Forward/reverse cassette triangles and a loop icon take the place of text; a green pixel on loop indicates enabled, dark blue indicates disabled.
- Idle waveform is drawn in a rate-adjusted timeline: speeding up compresses it, slowing down stretches and crops it. The effective duration appears on the lower-right. At playback, the live spectrum remains on screen, and the timer counts up.

## Settings
- Volume, Brightness, Reverse Map, Auto Play, **Reset Speed** (ON by default).
- Reset Speed determines whether finishing a recording resets to +1.00× or preserves the prior rate.
- Preferences saved using ESP32 Preferences upon leaving Settings or ending editing.

## Hardware smoke tests
1. Confirm the animated splash appears and can be skipped, then Chomp is shown automatically.
2. Check the teeth on every page and battery/time right alignment.
3. Hold A to record; confirm the red RECORDING banner and blinking dot; release to stop.
4. Confirm Reset Speed on/off behavior and persistence after power cycling.
5. Check +0.10× -> -0.10× transition, reverse mapping modes, all 0.10× steps, and ±3.00× caps.
6. Inspect expanded slow waveform and compressed fast waveform while idle.
7. During playback, listen for clean 1.00× sound, change speed live, verify reverse/loop and elapsed time.
8. Visit How To and About from the Home menu. Test navigation back with B.

**Unverified:** This ZIP has not been built with PlatformIO or tested on an M5StickS3 by the authoring environment.
