# Getting started

## Prerequisites
- VS Code + PlatformIO IDE extension
- USB-C **data** cable
- StickS3 and Unit Scroll U186 attached to the Grove connector

## Build and upload
From PlatformIO's toolbar use Build, Upload and Monitor. Or in a terminal with PlatformIO CLI installed:

```sh
pio run -e m5stack-sticks3
pio run -e m5stack-sticks3 -t upload
pio device monitor -b 115200
```

**Expected:** the screen displays `SOUND-CHOMP`, a `Scroll: FOUND` indicator, wheel count that changes when scrolled, and a button indicator that reads `DOWN` when the wheel is pressed. Built-in Btn A and B states are shown too. Scroll LED should be dim green and turn red while pressed.

## Troubleshooting
- **Upload fails:** check that your USB cable supports data; try another port. Consult StickS3 documentation for download mode (hold side reset until internal green LED flashes).
- **Unit Scroll not found:** check the Grove cable and Unit Scroll model; check device's external 5V power setting; the code uses SDA GPIO9/SCL GPIO10 and address `0x40`.
- **Blank display or compilation errors:** capture full PlatformIO logs and open an issue; StickS3 uses vendor-specific integration and must not be built as an older StickC.
- **Serial output missing:** reopen the monitor at 115200 after uploading.

This initial project hasn't been physically verified. Please report your build log and on-device results before publishing `v0.1.0`.

## Windows build dependency troubleshooting
If you see `ModuleNotFoundError: No module named intelhex`, in PowerShell run:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\python.exe" -m pip install intelhex
```

Rebuild. If needed, clean the PlatformIO project and rebuild.
