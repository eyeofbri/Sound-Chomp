# Phase 4A.2 — controls and UI prototype

## Control model

| Screen | Control | Action |
|---|---|---|
| Home | wheel | choose menu item |
| Home | A or wheel click | open menu item |
| Chomp | tap A | play or stop clip |
| Chomp | hold A >= 300ms | record until released |
| Chomp | wheel click | play or stop |
| Chomp | wheel rotation | live speed/direction |
| Chomp | long wheel click | toggle loop |
| Submenu | B | home/back |

## Speed mapping

The signed wheel range goes from -3x to +3x. Zero means pause. Positive speed increases as you rotate right. Conventional reverse increases in magnitude as you rotate left; inverted reverse gets slower as you rotate further left. Inverted reverse is intentionally discontinuous across zero; test whether it feels natural before making it the default.

Speed steps: 0.25-1.00 in increments of 0.05; 1.10-2.00 in increments of 0.10; 2.25-3.00 in increments of 0.25. The audio uses the proven Phase 4A.1 rotating queue playback path, so the live update latency is affected by the queued buffer duration.

## Settings

Wheel selects a setting, click/A enters edit mode, wheel adjusts, click/A saves selection; B returns home and saves. Reverse Map and Auto Play are saved in ESP32 Preferences together with existing volume and brightness. Auto Play defaults off.

## Test checklist

1. Tap A plays then tap A stops. Hold A > 300 ms records; release ends recording without accidental tap-play.
2. Enable Auto Play and confirm recording plays automatically on release; disable it again.
3. Sweep forward and reverse speed and listen closely at +1.00x for buffer gaps.
4. Switch reverse mapping to inverted, verify wheel behavior, reboot and confirm setting persists.
5. Confirm red recording banner, tiny teeth glyph, battery terminal/percentage and flicker-free refresh.
6. Confirm scroll-click playback and Key2 Back navigation still work.

## Known limitations

- 8-second single RAM recording; Tracks is a placeholder.
- Speed resampling changes both pitch and duration, and extremes can sound distorted.
- Hardware build and runtime testing are still pending.
