# Phase 5B.1 — player UI and controls (experimental)

This build keeps four LittleFS clips and the established three-buffer playback. **No new effects** are added; FX-2 and FX-3 are reserved placeholders.

## Wheel controls
- Rotate in **navigation** mode: change the highlighted button. Short-click **PLAY** to start/stop sound.
- Short-click a different implemented button (SPD, FX1, LOOP): enter **adjust** mode, indicated by amber. Rotate to adjust. Short-click again to return to navigation (green).
- Long-click wheel: select PLAY and exit adjust mode.
- Button A: tap to play/stop, hold to record; B: main menu/back.
- FX1: zero/off at minimum; turn clockwise to enable Echo then increase delay; counterclockwise eventually turns it off. Echo mix stays on Effects page.
- FX2/FX3 are deliberately non-functional placeholders.

## Screens
- Effects precedes Settings on Home. Echo ON/OFF, mix and delay occupy one row in landscape. In portrait the compact layout may need further tuning.
- Settings → ORIENTATION: LAND (locked), VERT (locked), AUTO (based on IMU). Auto mode uses hysteresis and avoids changing rotation while recording or playing.
- Both backgrounds are original, embedded static 16-bit RGB565 artwork at native 240×135 and 135×240 resolutions. Real data is drawn over it with M5Canvas to prevent flashing.
- Locked portrait uses LCD rotation 0; AUTO uses accelerometer axis checks. You may need to tune axis mapping for the device's natural orientation.

## Hardware test list
1. Confirm build/upload and baseline clear playback.
2. Navigate bottom strip without accidentally changing speed.
3. Short click PLAY to start/stop, tap/hold A to play/record.
4. Select SPD, click to edit (amber), rotate, click to exit.
5. Select FX1, enable/disable Echo and adjust delay; confirm FX2/3 do not change Echo.
6. Open Effects row and cycle enable/mix/delay fields.
7. Check LOOP OFF/1/ALL including while playing and saved tracks after reboot.
8. Test locked landscape, locked portrait and auto in every physical orientation.
9. Check text clipping, background visibility and canvas flicker.

**Status:** Not yet compiled in PlatformIO or tested on hardware. Keep v0.5.0 available as rollback.
