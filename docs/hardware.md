# Hardware reference

Target: **M5StickS3 (StickS3)**, ESP32-S3, 8 MB flash, 8 MB PSRAM, 135x240 display, built-in microphone/speaker.

## Grove port
StickS3 HY2.0-4P: black=GND, red=5V, yellow=GPIO9, white=GPIO10.
For Unit Scroll: yellow=SDA (GPIO9), white=SCL (GPIO10). Connect using the included cable, not loose wires.

Unit Scroll U186: I2C address `0x40`; encoder position register `0x10` (4 bytes, little-endian), button register `0x20` (0=pressed), RGB LED register `0x30` (index/R/G/B). Firmware uses a low-brightness green/red status LED.

**Power:** StickS3's external 5 V output is disabled by default. Smoke-test firmware enables it via `M5.Power.setExtOutput(true)`. Do not connect an external 5 V power source to the Grove port while the device is configured as a 5 V output. Refer to M5Stack power notes.

## Sources
- https://docs.m5stack.com/en/core/StickS3
- https://docs.m5stack.com/en/unit/Unit_Scroll
- https://community.m5stack.com/topic/7027/m5-unit-scroll-arduino-example
