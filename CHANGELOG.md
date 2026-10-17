# Changelog

## Unreleased — Phase 4A.2 controls and UI

- Tap Button A to play/stop; hold for 300 ms to record and release to finish.
- Optional automatic playback immediately after recording.
- Extend forward/reverse speed to 3x with finer steps near 1x.
- Choose conventional or inverted reverse speed mapping in Settings.
- Bright red RECORDING banner replaces speed display during capture.
- Improve battery terminal and percentage placement and add original pixel teeth glyph.
- Preserve Phase 4A.1 queued audio buffering without changing its playback algorithm.

## v0.3.0 — Playback effects and interactive UI

- Hardware verified: playback effects, smooth audio buffering, navigation, waveform and spectrum visualization, and preferences.

## Phase 4A.1 playback quality patch

- Queue adjacent playback chunks instead of waiting for each one to finish.
- Use three rotating audio buffers to avoid overwriting samples still in use.
- Reduce spectrum analysis workload during playback.
- Preserve live signed-speed control, navigation, settings, and existing UI.
- Pending hardware verification of 1.00x clarity and live wheel responsiveness.

## Phase 4A prototype

- Introduce Home, Chomp, Tracks (placeholder), and Settings navigation.
- Make Key2 the global Back button and move playback to scroll click.
- Add signed varispeed -2.0x to +2.0x; 0 pauses, negative speeds reverse.
- Stream short PCM playback chunks to apply wheel speed changes during playback.
- Add battery indicator and persistent volume / brightness settings.
- Add green/yellow/red spectrum level thresholds and purple reverse accents.
- Retain full-frame canvas-based flicker-free rendering and hold-A recording.

## Previous prototypes

See `docs/phase3a-effects.md` and `docs/phase3b-visualization.md` for earlier stages.
