# Getting started — current Sound-Chomp firmware

This guide applies to **Phase 5K.2**. The older phase notes in `docs/` are development history.

1. Connect **Unit Scroll U186** to the M5StickS3 Grove connector with the matching HY2.0 cable. Use a USB-C data cable for the board.
2. Install VS Code and PlatformIO, then open the `Sound-Chomp/` directory.
3. Build for `m5stack-sticks3`; upload via PlatformIO. The port is autodetected when supported, otherwise select the correct port manually.
4. The display should start in the Sound-Chomp player, with a theme-colored header and reel animation. **B** toggles the menu. The old Scroll: FOUND diagnostic screen is no longer part of current firmware.
5. Hold **A** on the player to capture audio; release to stop. Tap **A** to play. Use **Tracks** to rename, lock, reorder, or delete recordings. Use **Effects** to map six effects to three player FX slots. Use **Settings** for skin/palette and player preferences.

**Avoid** Erase Flash / Upload Filesystem Image / changing partitions if recordings already exist. LittleFS won't automatically format on a mount failure, and recording is disabled if persistent storage cannot be safely accessed.

## Quick troubleshooting

- If the upload port isn't found, check your USB-C **data** cable and PlatformIO serial device list. Windows COM numbers vary.
- If the wheel doesn't respond, verify the Unit Scroll model, Grove connection, and 5V power. See `hardware.md`.
- If recording is blocked, check whether the track is locked and look for storage/PSRAM messages at **115200 baud**.
- If PlatformIO reports a missing `intelhex` Python module on Windows, install it into PlatformIO's Python environment, not necessarily system Python: `& "$env:USERPROFILE\.platformio\penv\Scripts\python.exe" -m pip install intelhex`.
- For bug reports attach build logs, exact device details, and reproduction steps. Avoid uploading private recordings to public issues.
