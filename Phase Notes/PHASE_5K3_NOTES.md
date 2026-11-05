# Phase 5K.3 — Splash orientation and phase-note organization

- On startup, load the saved orientation preference before drawing any splash pixels.
- LANDSCAPE / PORTRAIT use the matching display rotation immediately.
- AUTO samples M5StickS3 accelerometer 12 times (~216 ms), using the established X/Y axis thresholds and selecting a clear majority; ambiguous readings fall back to landscape.
- The same auto-orientation decision helper is used later in the ordinary runtime loop, which preserves its existing stability debounce.
- Create the sprite canvas only after setting rotation, then play the existing themed splash unchanged.
- Move historical `PHASE_*.md` and `docs/phase*.md` documents into `Phase Notes/`.
- No changes to PCM files, LittleFS partitions, FX, or navigation behavior.

## Hardware acceptance

1. Set Settings > Orientation to LAND, restart held vertically: splash stays landscape.
2. Set VERT, restart held vertically: splash starts portrait and is centered.
3. Set AUTO, restart held vertically and then horizontally: splash should start in the held orientation each time.
4. Test AUTO held upside down / rotated 180 degrees, and at a 45-degree tilt (should default landscape when uncertain).
5. Check that selected GREEN/RED/BLUE palette still colors splash correctly.
6. Verify normal orientation rotation, audio/FX and four saved tracks after startup.

Verification here: static source check and toolchain-independent tests only; not a full PlatformIO build or device test.
