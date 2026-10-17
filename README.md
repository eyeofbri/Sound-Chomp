# Sound-Chomp

An open-source, pocket-sized voice recorder and sound toy inspired by the classic Yak Bak, built for **M5StickS3 + M5Stack Unit Scroll**.

> **Current development: Phase 2 audio prototype.** Phase 1 hardware, flashing, Unit Scroll rotation/button and serial were verified on physical hardware. **Phase 2 audio firmware is not yet hardware-tested.**

## Phase 2 controls

| Action | Input |
|---|---|
| Choose Record / Play | Rotate Unit Scroll |
| Activate selection | Press Unit Scroll |
| Start new recording | M5StickS3 button A |
| Stop recording | Unit Scroll press or button A |
| Play existing clip | M5StickS3 button B |
| Stop playback | Unit Scroll press or button B |

Recording replaces the previous clip. Maximum **8 seconds** of **16 kHz / 16-bit mono PCM**, stored in PSRAM (not persistent after restart). Scroll LED: blue=ready, red=recording, green=playing. No audio processing effects yet.

## Quick start

1. Clone `https://github.com/eyeofbri/Sound-Chomp.git` and open the repository directory in VS Code + PlatformIO.
2. Connect the Unit Scroll U186 using the included HY2.0 cable and connect the StickS3 over USB-C.
3. Run **PlatformIO Build**, then **Upload**, then **Monitor** (115200 baud).
4. Record a short test phrase (button A), stop with button A, and play with button B.
5. Also test selection and activation through the scroll wheel. Watch serial output for errors.

Details: [getting started](docs/getting-started.md), [hardware](docs/hardware.md), [Phase 2 testing](docs/phase2-audio.md).

## Status and roadmap

- Phase 1: hardware foundation verified on physical device.
- Phase 2: single-clip record/playback prototype (awaiting device testing).
- Later: pitch/reverse playback, clip management, and refined user interface.

## Contributing & license

Issues and pull requests are welcome; see [CONTRIBUTING.md](CONTRIBUTING.md). Licensed under MIT; see [LICENSE](LICENSE). Yak Bak belongs to its respective owners and this project is independent.
