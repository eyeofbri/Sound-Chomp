# Changelog

## Unreleased — Phase 3A / v0.3.0 candidate

- Add speed-controlled playback at 0.50×–2.00× using interpolation.
- Add reverse playback while keeping the source clip unchanged.
- Add optional repeated playback / looping.
- Introduce Winamp-inspired dark/green UI, rendered with off-screen `M5Canvas` to avoid full-screen redraw flashing.
- Preserve Phase 2 record/play hardware buttons, I²C pin mapping, and microphone/speaker calls.
- Use user-supplied PlatformIO configuration (COM9).
- **Pending hardware build and test; not an official release yet.**

## v0.2.0 — Voice recording & playback

- Tested on physical device: build, flash, screen, record, play, Unit Scroll and overwrite functionality passed.
