# Sound-Chomp

Open-source Yak Bak-inspired voice recorder/sound toy for **M5StickS3 + M5Stack Unit Scroll**.

> **Phase 3A prototype (`v0.3.0` candidate):** Playback speed, reverse, loop, and flicker-free Winamp-inspired display. **Not yet compiled or tested on hardware.** Phase 2 recording/playback tests passed 7/7 on real hardware.

## Phase 3A controls

| Action | Input |
|---|---|
| Record / stop recording | Button A |
| Play / stop playback | Button B |
| Change speed | Rotate Unit Scroll (0.50×–2.00×) |
| Toggle forward/reverse | Short press scroll wheel **while idle** |
| Toggle loop | Hold scroll wheel ~0.65 seconds |
| Stop playback | Short press scroll wheel or Button B |
| Stop recording | Scroll press or Button A |

The effects are applied to a separate PSRAM output buffer; the original recorded clip remains unchanged. Speed changes during playback apply on the next loop cycle or playback start. At the 0.50× setting an 8-second recording takes approximately 16 seconds to play. Changing speed also changes pitch, just like classic variable-speed tape playback.

## Installation

1. Clone <https://github.com/eyeofbri/Sound-Chomp> and open it with VS Code + PlatformIO.
2. Copy this package into your clone, preserving your existing LICENSE and Git history.
3. The included `platformio.ini` is the user's provided local configuration with `COM9` specified; contributors should change or remove those port lines.
4. Build, upload, and open the 115200-baud serial monitor.
5. Follow [the Phase 3A test plan](docs/phase3a-effects.md).

## Display

The UI is inspired by the **classic Winamp default skin**: dark beveled frame, neon-green digital text and small level bars. To prevent visible flashing, each full frame is drawn to an off-screen `M5Canvas` sprite and pushed to the display in one operation. The bars are **decorative** in this prototype, not a real-time audio analyzer.

## Roadmap

- Phase 1: hardware/Unit Scroll validation ✅
- Phase 2: 8-second 16 kHz mono RAM record/playback ✅ (7/7 on device)
- Phase 3A: variable speed, reverse, loop, Winamp-inspired non-flashing display — hardware validation pending
- Phase 3B: playback/control refinements, responsive UI, optional visualization

## License

MIT — see `LICENSE` in the original repository. Independent of the owners of the Yak Bak and Winamp trademarks.
