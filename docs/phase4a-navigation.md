# Phase 4A — Navigation and Live Varispeed (experimental)

## Controls

| Screen | A (main button) | B (Key2) | Wheel rotate | Wheel short click | Wheel long click |
| --- | --- | --- | --- | --- | --- |
| Home | Open selected page | — | Choose Chomp / Tracks / Settings | Open selected page | — |
| Chomp | Hold to record, release to stop | Back to Home, stops playback | Playback speed: -2.0x to +2.0x; 0 pauses | Play / Stop | Toggle loop |
| Tracks | Open Chomp | Back to Home | — | Open Chomp | — |
| Settings | Enter / finish editing selected value | Back to Home | Select setting / change value | Enter / finish editing | — |

## Live varispeed

The scroll wheel selects signed speeds from -2x (reverse) through 0 (pause) to +2x (forward). Playback is rendered in 384-sample chunks at 16 kHz. Wheel changes are read while playback is active and applied at the next generated chunk boundary. Existing speaker buffering can add latency; test on hardware. Negative playback reverses from the current source cursor. Speed changes alter pitch and duration; this is **not** time-stretching / pitch preservation.

Boundary condition: reaching either end stops playback unless loop is enabled. The play cursor starts at the end for negative speeds. Changing direction mid-clip reverses at the current cursor.

## Screen

- Retains off-screen `M5Canvas` sprite rendering, pushed to LCD as a completed frame.
- The Chomp screen shows the same waveform at rest and active spectrum during recording / playback.
- Spectrum bands are green below 72%, yellow above 72%, and red above 89% of display height.
- Negative speed uses a purple accent and reverses the waveform preview.
- Battery estimation (percentage and charge status) appears at top-right.
- Settings include speaker volume and display brightness; saved to ESP32 Preferences.
- Tracks is a placeholder for now; only one recording lives in PSRAM and is lost on reboot.
- Rotation/orientation settings and persistent multi-track storage are deferred to Phase 4B/5.

## Hardware verification checklist

1. Boot opens Home; wheel chooses items and A/scroll click enters.
2. Chomp: hold A to record; release stops. B returns Home.
3. Scroll click plays/stops; B during playback stops and returns Home.
4. Rotate wheel during playback and listen for **immediate** change at chunk boundaries.
5. Cross through 0: audio pauses; rotate into negative for reverse.
6. With loop on, listen through both clip ends at both speed signs.
7. Settings change brightness and volume and persist after reboot.
8. Check spectrum, waveform, battery level, and flicker-free UI.

## Notes

- Keep a copy of working Phase 3B until testing completes.
- This source package is uncompiled and not hardware verified.
- Embedded serial monitor is configured for COM9 in platformio.ini (local Windows setup); contributors can remove the two COM9 lines for automatic port detection.


### Phase 4A.1 playback quality test

1. Record a 4–8 second phrase, then play it at +1.00x. Confirm that speech is as clear as Phase 3B.
2. Try +0.50x, +1.50x and −1.00x; confirm live changes still work without resets.
3. Loop a clip while adjusting speed and direction. Check for unintended silences and sharp clicks.
4. Confirm the spectrum and menus remain responsive and flicker-free.
5. If audio still skips, note whether the skips occur at regular intervals (~32ms), only when the display redraws, or only when turning the wheel.

This patch has not been compiled or tested on real hardware.
