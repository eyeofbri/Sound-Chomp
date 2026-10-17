# Phase 2 — Audio smoke test

**Not hardware-verified yet.** Confirm behavior on a physical StickS3 before tagging a release.

## Procedure
1. Build and flash; serial monitor should show `Sound-Chomp Phase 2` at 115200 baud.
2. Verify `READY`, `Scroll: OK`, and blue wheel LED.
3. Press **A**: LED should turn red; say a short phrase close to the microphone.
4. Press **A** again before 8 seconds (or allow auto-stop). Screen should show the recorded length.
5. Press **B**: LED green; listen for the recorded clip. Press **B** to stop early.
6. Rotate wheel to select `REC` or `PLAY`; press wheel to activate. During recording/playback press wheel to stop.
7. Record a second phrase and verify the old one is replaced.
8. Restart device; expect `Clip: 0.0s` because audio is RAM-only.

## Design details
- 16,000 samples/sec, signed 16-bit mono (32 KB/sec), max 8 sec = 256 KB PSRAM.
- 512-sample (~32 ms) chunks for input. Input events are checked while microphone fills each chunk.
- Microphone and speaker alternate rather than running concurrently (as recommended by M5Unified examples).
- Recording starts/stops at chunk boundaries; stopped recording can therefore include about 32 ms of additional audio.
- If `PSRAM allocation failed`, validate the board and PSRAM build flags and report available PSRAM.
- If `Mic.begin`, `Mic.record` or `playRaw` fails, inspect the serial log and open an issue with full build output.
- Device's COM port can vary; use automatic port selection or add `upload_port = COM9` / `monitor_port = COM9` to the local PlatformIO configuration if needed.
- If `No module named intelhex` appears during build, see [getting started](getting-started.md).

## Documentation
Audio APIs based on M5Stack's official StickS3 microphone sample and M5Unified `Mic_Class` / `Speaker_Class`.
- https://docs.m5stack.com/en/arduino/m5sticks3/mic
- https://github.com/m5stack/M5Unified/blob/master/examples/Basic/Microphone/Microphone.ino
