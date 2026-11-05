# Phase 3B — Hold recording and audio visualization

This is an **experimental source candidate**, not yet compiled or tested on an M5StickS3. Preserve a copy of the known-good Phase 3A firmware before flashing.

## Changes

- Button A: **press and hold** to record; **release** to stop. Recording still ends at 8 seconds. Unit Scroll press remains an emergency/manual stop while recording.
- Enlarged READY/REC/PLAY state and speed readouts.
- During recording or playback, show an 18-band Goertzel-based frequency visualization computed from the actual PCM recording or processed playback buffer (not randomized graphics). This is a lightweight **approximate audio spectrum**, not a calibrated scientific FFT/spectrum analyzer.
- At idle, show a cached waveform amplitude preview of the recorded clip. The preview reverses left-to-right when Reverse is enabled, and occupies more or less of the fixed 8-second timeline based on adjusted playback speed.
- Canvas-based full-frame composition is retained to avoid flashing.

## Controls

| Control | Action |
|---|---|
| Hold Button A | Record while held (up to 8 seconds) |
| Release Button A | Stop recording and update waveform |
| Button B | Play/stop |
| Scroll rotate | Change speed 0.50x to 2.00x |
| Scroll short press while idle | Reverse/forward |
| Scroll long press | Loop on/off |
| Scroll short press during playback | Stop playback |

## Tests

- [ ] Build passes in PlatformIO.
- [ ] Upload succeeds to COM9.
- [ ] Button A **starts recording immediately** when held, and stops when released.
- [ ] Very short button presses don't cause lockups.
- [ ] Eight-second maximum length still works.
- [ ] Input during recording remains responsive.
- [ ] Moving bars react to speech when recording, and to stored audio during playback.
- [ ] Idle waveform appears after the first recording.
- [ ] Reverse flips waveform visually and sounds backward.
- [ ] Speed changes visibly affect waveform timescale and playback duration.
- [ ] Loop on/off and stopping work normally.
- [ ] Display remains flicker-free and legible.

## Limitations / interpretation

- Playback-spectrum location is **estimated from elapsed play time** rather than read from an actual DAC playhead. During buffer startup or heavy load, bars may lag/lead the speaker slightly.
- Analysis uses 256-sample Goertzel windows at 18 approximately logarithmic center frequencies. Short windows favor responsiveness over fine bass-frequency resolution.
- The stored waveform is a peak-envelope visualization; not a time-domain sample-by-sample oscilloscope.
- Changing speed still applies on the next playback/loop cycle, not halfway through a clip.
- Sound is stored in volatile PSRAM and is lost at power-off.
- On-device performance and exact text layout require physical validation.
