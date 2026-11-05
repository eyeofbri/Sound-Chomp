# Contributing

Thanks for helping improve Sound-Chomp!

- Discuss significant features in a GitHub issue before large changes.
- Target the M5StickS3; do not assume older Stick models have identical pins.
- Keep changes focused and update docs when behavior changes.
- For bug reports include PlatformIO output, hardware revision, and exact reproduction steps.
- Don't commit `.pio/`, passwords, private audio recordings, or proprietary assets.
- Open pull requests against `main` with a short description and test notes.

## Building and validating before a PR

Run `python tools/generate_ui_assets.py` and `python tools/generate_builtin_skins.py` when editing artwork (PlatformIO prebuild also regenerates these). Run `python tools/test_palette_remap.py`, `python tools/test_fx_slots.py`, and `python tools/test_audio_controls_5k1.py`. Build with `pio run -e m5stack-sticks3`. If a toolchain is unavailable, state that explicitly; source-level checks are not equivalent to a device build.

No fixed serial port belongs in committed `platformio.ini`. Do not add uploadfs/erase-flash steps to normal upgrade instructions. Recording safety changes require review of all error paths and hardware validation.
