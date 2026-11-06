# Sound-Chomp

A pocket-sized, Yak Bak-inspired audio recorder for **M5Stack M5StickS3 + Unit Scroll**, with four persistent recordings, adjustable forward/reverse playback, six assignable effects, and three built-in skins.

**Stable public release:** v0.7.1 · **Release candidate:** v0.8.0-rc.1 (based on hardware-tested Phase 5K.3; release-package verification pending).

## Hardware

- M5Stack **M5StickS3**, with 8 MB flash and PSRAM
- M5Stack **Unit Scroll U186**, plugged into the HY2.0 Grove port
- USB-C **data** cable, VS Code with PlatformIO

See [hardware connections](docs/hardware.md). No additional external components are required.

**Release candidate:** Start with the [release and upgrade guide](docs/releasing.md), including how to protect existing recordings. This project does not yet provide an audio-export workflow.

## Build and upload

1. Install VS Code and the **PlatformIO IDE** extension.
2. Open the **Sound-Chomp** directory (containing `platformio.ini`).
3. Attach Unit Scroll, then connect M5StickS3 over USB-C.
4. Use PlatformIO **Build**, then **Upload**. Wait for the reboot and Sound-Chomp player screen.
5. If upload fails, choose the correct serial port in PlatformIO's device list. Public `platformio.ini` intentionally does not hardcode `COM9`.

CLI equivalent: `pio run -e m5stack-sticks3` then `pio run -e m5stack-sticks3 -t upload`.

**Downloads:** The release source ZIP is a full PlatformIO project. GitHub Actions can also produce a *firmware binaries* ZIP (for advanced flashing workflows); it is not a one-click updater. See [release guide](docs/releasing.md).

**Protect your saved audio:** do **not** run **Erase Flash**, **Upload Filesystem Image**, or repartition the flash as part of a normal update. The firmware does not automatically format LittleFS when mounting fails. If storage is unavailable, recording is blocked. Keep valuable takes backed up by other means; the device does not include a recording export feature.

## Controls

| Control | On Player | In menus |
|---|---|---|
| Button A, tap | Play/stop | Select/confirm where applicable |
| Button A, hold ≥300 ms | Record until released | Context-dependent |
| Button B | Toggle main menu | Back/cancel |
| Scroll wheel, turn | Navigate player controls; adjust selected value in edit mode | Navigate list / edit value |
| Scroll click | Play/stop while Play focused; otherwise enter/leave editing | Select / next field |
| Scroll long-click | Next recorded track when Play focused; otherwise return focus to Play | Context-dependent; e.g. rename focus / delete confirmation |

### Tracks

Four persistent tracks of up to ~8 seconds at 16 kHz mono PCM. In **Tracks**, select a row to open actions: **Play, Rename, Delete, Lock/Unlock, Move**. Locked tracks cannot be deleted or overwritten. Delete requires an explicit hold confirmation. Names are limited to 10 characters, and logical order and lock status persist across reboots. Empty tracks still occupy a slot.

### Effects

Six available effects: **Echo, Stutter, Robot, Warble, Bitcrush, Telephone**. Assign up to three to FX:1–3 in the scrollable **Effects** menu; `--` means unassigned. Click to commit an assignment (merely scrolling previews it). On the player, each assigned FX button controls its enabled state and parameters. Effects modify speaker output only, not saved PCM.

### Skins and palettes

**Settings → SKIN:** CLASSIC / MINIMAL / ARCADE. **Settings → PALETTE:** GREEN / RED / BLUE. Both persist. Skin artwork uses fixed PNG sprite coordinates; yellow borders are editor-only, magenta is the primary icon color key, and green shades in backgrounds change with palette. The Sound-Chomp teeth logo is firmware-owned. See [skin customization](Phase%20Notes/phase5h2-built-in-skins.md).

## Troubleshooting

- **Blank/failed build:** Check the exact device, environment, USB cable, and PlatformIO log. Avoid building as an older M5StickC.
- **Unit Scroll not found:** Verify Grove cable/power, I²C address 0x40, and wiring in [hardware reference](docs/hardware.md).
- **Recording refused:** The active track might be locked, LittleFS might not mount, flash might be too full to stage a safe replacement, or PSRAM might be insufficient. Open Serial Monitor at 115200 for the reason. Do not format the storage to fix this without backing it up.
- **Recording didn't change:** Phase 5K.2 preserves the prior take if capture is empty, the mic fails, or flash save fails. If a save fails, check the serial log before retrying.
- **First playback sounds clipped:** Report firmware version, startup conditions, active FX, and reproduction steps; speaker startup is a hardware-sensitive path.

## Development and contributions

- Source: [GitHub repository](https://github.com/eyeofbri/Sound-Chomp)
- [Current setup guide](docs/getting-started.md) · [Release guide](docs/releasing.md) · [RC release notes](docs/release-notes-v0.8.0-rc.1.md) · [Contributor guide](CONTRIBUTING.md) · [Phase 5K.3 verification plan](Phase%20Notes/PHASE_5K3_NOTES.md)
- Historical phase documentation is preserved in `Phase Notes/` but **is not** current setup guidance.
- License: [MIT](LICENSE).
