# Sound-Chomp

An open-source, pocket-sized voice recorder and sound toy inspired by the classic Yak Bak, built with the **M5StickS3** and **M5Stack Unit Scroll**.

> **Status: Phase 1 — hardware smoke test (unverified on physical hardware).** Audio recording is not implemented yet.

## Hardware
- M5Stack **StickS3** (not StickC / StickC Plus)
- M5Stack **Unit Scroll U186** (I2C, address `0x40`)
- Included four-wire Grove cable and USB-C data cable

## Quick start
1. Install VS Code and the **PlatformIO IDE** extension.
2. Clone this repository: `git clone https://github.com/eyeofbri/Sound-Chomp.git`.
3. Open the cloned folder in VS Code.
4. Connect Unit Scroll to the StickS3 HY2.0-4P port and connect USB-C.
5. In PlatformIO, select **Build**, then **Upload**, then **Monitor** (115200 baud).
6. Verify screen, both hardware buttons, scroll movement and scroll-button state. If the wheel isn't detected, see [troubleshooting](docs/getting-started.md).

See [getting started](docs/getting-started.md) and [hardware](docs/hardware.md).

## Project roadmap
- **0.1.x**: PlatformIO setup, display/buttons, wheel and LED test
- **0.2.x**: Microphone recording and speaker playback experiments
- **0.3.x**: Playback controls using Unit Scroll
- Later: clip management, audio effects, and polished interface

Roadmap items are tentative. Version `0.1.0` should only be tagged **after** a successful hardware test.

## Contributing
Issues and pull requests are welcome. This is an early-stage project; see [CONTRIBUTING.md](CONTRIBUTING.md).

## License
MIT — see [LICENSE](LICENSE). The Yak Bak name belongs to its respective owners; this is an independent fan-inspired project.
