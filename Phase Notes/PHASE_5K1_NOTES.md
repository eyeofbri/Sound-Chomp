# Phase 5K.1 — Audio & Controls Stability

Built on the corrected **Phase 5K Storage & Persistence** project. This is a development build, not a confirmed PlatformIO or on-device release.

## Firmware changes

1. **No Preferences/NVS writes inside Loop All's per-sample audio hot loop.** Track switches during automatic Loop All playback use `selectTrack(next, false)`; the final active track is persisted after the speaker has stopped. Manual track selection still persists immediately. This reduces the risk of flash-write stalls causing audio gaps. A power loss *during* playback may now restore the last track selected before playback rather than the current Loop All track.
2. **Speaker startup cleanup.** Speaker-begin failures end the speaker interface and report a serial error. First-play preroll is still 16 ms of silence preceded by a 60 ms stabilization delay, but its one-time success flag advances only when the silence is actually queued. An unsuccessful preroll can be retried on the next play attempt.
3. **Playback state reset.** Clear a pending long-click track advance at the beginning of each new playback, so it cannot carry from a prior run.
4. **Wheel I2C reconnection stability.** Read failures clear stale click events. Once reconnected, the button must be released before another press is accepted. The encoder's delta uses unsigned modular subtraction, handling `int32` rollover, with large jumps discarded.
5. **Preserved audio.** No changes to effect DSP algorithms, recording sample format, track storage, UI/skins, palette, recording partition, or speaker buffer size.

## Hardware test matrix

- Start immediately after reboot; check that the initial consonant/click of a recording is audible.
- Repeat Play/Stop 20 times, including during the initial preroll.
- Use Loop All with two or more recordings for several minutes. Watch for skips at track boundaries and check active track persistence after stopping and restarting.
- Repeat Loop One with speeds +0.5x, +1.0x, +3.0x and reverse -1.0x; check that switching to/from reverse remains responsive.
- Test Echo+Stutter, Echo+Robot, Warble+Telephone and Bitcrush+Robot. Check clipping, stalling, or distortion beyond the expected effect; tests cannot guarantee clean audio on all combinations.
- While playing, tap Play, tap Button A, press Button B, and long-click on Play to switch recordings; ensure the correct action happens exactly once.
- Physically unplug/replug the Unit Scroll (if safe to do so on your setup) while idle, not while recording; ensure reconnecting with its switch depressed does not generate a phantom short click, and that it accepts normal controls after release.
- Confirm all recordings, locks, ordering, names, skins and palettes survive restart.
- Check Serial Monitor for `SPEAKER BEGIN FAILED`, `SPEAKER PREROLL FAILED`, `SPEAKER PLAY FAILED`, or `Scroll disconnected` messages.

## Checks run

- `python tools/test_audio_controls_5k1.py` validates key source contracts and encoder rollover cases.
- `python tools/test_fx_slots.py` and `python tools/test_palette_remap.py` validate unchanged functionality.
- `python tools/validate_builtin_skins.py` validates sprites.
- Asset generators run without error.

**Not verified here:** Full PlatformIO build and on-device playback, latency, USB, I2C reconnection, and power-loss handling. Do not erase flash or upload a filesystem image when updating firmware containing recordings.
