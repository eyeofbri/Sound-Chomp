# Sound-Chomp

An open-source pocket-sized Yak Bak-inspired voice recorder and sound toy for the **M5StickS3** + **M5Stack Unit Scroll**. Developed with PlatformIO and Arduino.

## Phase 4A.1 playback quality patch (not yet hardware verified)

- Main menu with Chomp, Tracks, Settings.
- A: hold-to-record in Chomp; select on Home.
- Key2 / B: Back to Home.
- Wheel: signed varispeed (-2x to +2x), including live changes while playing; 0 pauses.
- Wheel click: play/stop inside Chomp; open menu entries elsewhere.
- Wheel long-click: toggle loop inside Chomp.
- Improved playback chunk queueing (three rotating buffers) to reduce choppiness at 1.00x.
- Live audio spectrum; idle waveform; purple reverse accent; Winamp-inspired flicker-free display.
- Battery indicator, speaker volume and brightness settings (persist across reboots).
- Single 8-second 16-kHz 16-bit mono RAM recording. Tracks currently displays a placeholder; saved tracks will arrive later.

### Build and upload

1. Open the `Sound-Chomp` folder in VS Code with PlatformIO installed.
2. Connect M5StickS3 by USB. This package retains the developer's local `COM9` configuration; adjust/remove the port settings on other machines.
3. Build with PlatformIO, then upload and test. Check `docs/phase4a-navigation.md` for controls and the hardware test checklist.

Prior phase guides remain in `docs/`. MIT licensed; see `LICENSE` in the existing repository.


## Phase 4A.2 experimental changes

- **Chomp:** tap A to play/stop; hold A ~300 ms to record and release to stop.
- Scroll click also plays/stops; long scroll click toggles looping.
- Scroll rotation controls playback speed from -3x to +3x; zero pauses.
- Settings include Volume, Brightness, Reverse Map (Normal/Inverted), and Auto Play.
- Auto Play automatically starts the last recording after releasing A; off by default.
- Inverted reverse mapping means further into negative wheel positions slows reverse playback; transitioning through zero into reverse jumps initially to fast reverse.
- Tracks is still a placeholder; recordings reside in RAM.

**Verification:** This package has not been compiled or physically tested. Preserve your validated Phase 4A.1 commit for rollback.

### Current experimental prototype — Phase 4A.3

Adds a skippable animated splash, Chomp-first boot, revised cassette/Winamp UI,
0.10× speed steps with no zero detent, reset-speed-on-recording preference, and
How To / About pages. See [`docs/phase4a3-ui.md`](docs/phase4a3-ui.md).
