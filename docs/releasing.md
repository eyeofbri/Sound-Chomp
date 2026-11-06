# Release and upgrade guide — v0.8.0-rc.1

**Status:** Release candidate, not a fully hardware-certified stable release. Firmware based on Phase 5K.3, which passed 9/9 maintainer hardware checks. Fresh-install flashing and crash/power-loss recovery require separate validation.

## For M5StickS3 users (recommended)

1. Download the **Source** ZIP, extract it, and open its single `Sound-Chomp/` directory in VS Code with PlatformIO.
2. Connect Unit Scroll U186 to the Grove port; connect M5StickS3 over a USB-C data cable.
3. Select environment `m5stack-sticks3`, **Build**, then **Upload**.
4. **Do not** use Erase Flash, Upload Filesystem Image, or change flash partitions if you have recordings to keep.
5. Check the About screen shows `v0.8.0-rc.1`; test one disposable clip, reboot, and verify the clip and track metadata persist.

**Important:** The project has no recording-export function. There is no guaranteed backup for irreplaceable recordings before an upgrade. Normal application-only uploads are designed to preserve the existing LittleFS partition, but power failures, partition mismatches, or incorrect flashing commands can still lose data. Keep v0.7.1 available for fallback; downgrading does not guarantee backwards compatibility of stored metadata.

## Advanced binary distribution

On successful GitHub Actions builds, `Sound-Chomp-v0.8.0-rc.1-Firmware.zip` contains the compiled application `.bin`, and *if emitted by the toolchain*, bootloader and partition binaries. The archive includes a manifest with SHA-256 hashes.

The firmware ZIP is **not a universal one-click flasher**. Flash offsets and chip mode must be derived from the actual successful PlatformIO build and matching partition table. Do not apply generic `erase_flash`, `write_flash`, or filesystem-image commands to a board that contains recordings you want to preserve. For routine upgrades, use PlatformIO **Upload** from the source project.

## Maintainer release workflow

1. Review `CHANGELOG.md`, `docs/release-notes-v0.8.0-rc.1.md`, `docs/getting-started.md`, and `docs/hardware.md`; verify About version and `tools/release_version.txt` match.
2. Run `python tools/release_check.py` and `pio run -e m5stack-sticks3` (or use the GitHub Actions build job).
3. Examine PlatformIO's program and RAM size reports, as well as its actual build partition CSV. Do not assume source header sizes equal firmware flash usage.
4. Run `python tools/package_release.py --source` for the source release; after a successful build, run `python tools/package_release.py --firmware` to package available `.bin` artifacts.
5. Test a **normal upgrade** (no erase) on a device with *disposable* recordings, restart, and confirm track persistence, orientation modes, effects, and Loop All.
6. For fault testing, use a dedicated device or disposable recordings. Recovery after interrupted LittleFS writes is not yet a confirmed release guarantee.
7. After successful CI and hardware checks, tag the exact audited commit `v0.8.0-rc.1` and attach release assets. GitHub Actions uploads workflow artifacts; it does **not** automatically publish a GitHub release. Download the ZIPs from the successful workflow run and attach them manually to a GitHub prerelease.

## Known limitations

- No built-in USB/audio recording export, and no guaranteed recovery after unexpected power loss or flash corruption.
- A full eight-second replacement requires enough LittleFS headroom for staging and temporary PSRAM for capture; recording may be blocked to protect existing clips.
- Unit Scroll must be physically connected; no emulator or unit test can establish hardware audio quality.
- Runtime/hardware validation of the source-distribution ZIP and CI-produced firmware binaries must be performed separately.
