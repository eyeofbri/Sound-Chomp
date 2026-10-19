# Phase 5A — Echo and live control switching

Experimental firmware based on the hardware-tested Phase 4D build. **Compile and test on a StickS3 before publishing a release.**

## Chomp controls

| Input | Behavior |
|---|---|
| Tap A | Start/stop playback |
| Hold A | Record; release to finish |
| Scroll click | Switch the wheel between SPEED and ECHO MIX (also while playing) |
| Scroll rotation (Speed) | ±0.50x to ±3.00x at 0.10x intervals |
| Scroll rotation (Echo) | Adjust echo wet mix 0–100%, 5% per encoder tick |
| Long scroll click | Cycle loop OFF → 1 → ALL |
| B | Home/menu (stops playback) |

The player displays `ECHO NN% ON/OFF` in amber when ECHO is the selected control. The effect must be enabled separately in the Effects menu; setting the wet mix by itself does not automatically enable it.

## Effects menu

From Chomp, press B → EFFECTS. Rotate wheel to select a setting; tap A or wheel click to edit and tap again to finish. B exits and saves:

- **ECHO ENABLE:** OFF / ON (default OFF)
- **ECHO MIX:** 0–100% in 5% steps (default 35%)
- **ECHO DELAY:** 100–600 ms in 50 ms steps (default 250 ms)

Preferences persist in ESP32 NVS, independently of the track files. The current speed/echo wheel selection does not persist across reboot.

## Audio implementation

The unchanged four-track LittleFS `.pcm` format remains the source of truth. A feedback delay line (16 kHz PCM, 600 ms capacity) is allocated in PSRAM. Each generated playback sample adds a delayed signal at the chosen mix, with 35% feedback; the output is clamped to signed 16-bit and delivered by the existing three rotating 512-sample audio buffers. The source recording is **never overwritten by the effect**.

Echo history resets at the beginning of playback. It continues across a loop boundary, so the tail can carry naturally between loops or between tracks in LOOP ALL. Delay or wet mix changes made during playback affect newly generated chunks, with latency from the existing speaker queue. Echo is not applied to microphone capture.

**Known tradeoffs:** Very high wet mix can clip loud source recordings; changing delay live can cause small pitch/click artifacts due to read-pointer jumps. Echo tails do not continue after explicitly stopping playback. The analyzer represents the post-effect PCM output. If echo PSRAM allocation fails, dry audio will still work but no echo will be heard.

## Hardware regression checklist

1. Compile and upload using your existing `platformio.ini` (COM9 on the developer's machine).
2. Play at +1.00x with ECHO OFF; compare audio clarity with Phase 4D.
3. Enable echo in Effects, set 250 ms and 35%; verify delayed repeats.
4. During playback short-click the wheel; turning it adjusts echo mix without stopping audio.
5. Short-click again; speed and direction adjust live as before.
6. Long-click cycles OFF / 1 / ALL; `ALL(n)` shows the track currently playing.
7. Verify loop-all skips empty slots, and echo sounds continuous across track boundaries.
8. Record and restart; all four recordings and last selected track should survive.
9. Verify recording, splash, battery, menus, teeth and reels render without flicker.
10. Change echo settings, restart, and verify the settings persist.

Do not upload a LittleFS filesystem image or reformat flash to install this firmware. Flash the firmware with PlatformIO's normal Upload action only. Existing saved tracks should be preserved.
