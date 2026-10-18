# Phase 4B — Multi-track RAM prototype

## Feature overview
- 4 slots (CHOMP 01–04), each up to 8 seconds at 16-kHz mono, 16-bit signed PCM.
- Up to 1,024,000 bytes of PSRAM allocated to clips, plus playback/UI memory.
- When a track is selected, Chomp records/plays that track only.
- Each track has an independent captured length and a regenerated waveform preview.
- Tracks are volatile; reboot or loss of power clears them.
- A long wheel press in Tracks asks for deletion; wheel click confirms; B cancels.

## Controls
- Chomp: tap A = play/stop; hold A >=300ms = record until released.
- Chomp: wheel = signed speed; wheel click = play/stop; long wheel click = loop.
- B = open Home; choose Tracks; wheel = select track; A/click = load track.
- Tracks: long wheel click = delete prompt, click = confirm; B = cancel/back.

## Hardware checklist
1. Build/upload with `platformio.ini`, adjusting COM9 for your host.
2. Record distinct sounds in CHOMP 01 and CHOMP 02; verify neither is overwritten when switching.
3. Record CHOMP 03 and 04 and verify all four can be played.
4. Verify empty slots do not play stale audio.
5. Select a track and overwrite it; other tracks remain unchanged.
6. Confirm deletion works and B cancels deletion safely.
7. Confirm +1.00x plays cleanly and live speed/reverse controls remain responsive.
8. Enable looping and confirm timer returns to near zero each loop.
9. At +3.00x and the negative wheel limit, rotate further: reels remain still while spokes flash red.
10. Confirm startup splash, thicker arrows, recording banner and short idle prompt render legibly.
11. Reboot: all track slots should be empty (expected for this prototype).

## Known limitations
- Recordings are stored in RAM only, not NVS/LittleFS/SD.
- The entire audio capture/playback process is still blocking; changing screens is deliberately limited during active recording/playback.
- Firmware has not yet been compiled with PlatformIO or tested on hardware in the creation environment.
