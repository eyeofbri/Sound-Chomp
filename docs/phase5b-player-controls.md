# Phase 5B — focused transport player

Development version: `v0.6.0-dev` (not an official release). Based on the tested Phase 5A firmware.

## Player controls

- Button A tap: play or stop. Button A hold: record until released.
- Button B on Chomp: open Home. Button B on Home: return to Chomp. Button B on a submenu: return to Home.
- Scroll click when PLAY is focused: play or stop.
- Scroll rotation while PLAY is focused: advance focus to SPEED (positive) or stay on PLAY (negative).
- Scroll click on any other control: advance focus to next, wrapping LOOP to PLAY.
- Scroll long click: immediately reset focused control to PLAY.
- Scroll rotation with a different focused control changes that setting while idle or playing:
  - SPEED: signed -3.0x through -0.5x, +0.5x through +3.0x, increments 0.1x.
  - FX-1: echo enabled on/off.
  - FX-2: echo level 0-100% in 5% steps.
  - FX-3: echo delay 100-600ms in 50ms steps.
  - LOOP: off, one track, all populated tracks.
- Echo settings persist after a short inactivity window; source clips remain unchanged.

## Layout

- Small fixed header with teeth glyph, wordmark, battery.
- Status and context-sensitive focused control value on left; animated tape reels on right.
- Waveform when idle; analyzer while recording or playing.
- Six 27px-tall transport buttons at bottom with active green lamp and outlined selection; time in dedicated right-most readout.
- PLAYING replaces PLAY during audio playback; the red RECORDING banner still takes precedence while recording.
- Splash screen and About both derive the version from a single `kFirmwareVersion` constant.
- Uses existing off-screen sprite to prevent flashing.

## Physical validation checklist

- [ ] Builds in PlatformIO and uploads to COM9.
- [ ] No regression to smooth playback at +1.0x.
- [ ] A tap/hold record controls work from any focused button.
- [ ] PLAY click starts/stops playback and PLAY rotation moves focus to SPEED.
- [ ] Other click targets advance focus including LOOP -> PLAY.
- [ ] Long wheel click jumps immediately to PLAY.
- [ ] Speed, echo enabled, level, delay and loop each adjust with the wheel during playback.
- [ ] B on Home returns to player; B in submenu returns to Home.
- [ ] LOOP ALL still skips empty tracks; LittleFS recordings survive reboot.
- [ ] Version appears on splash and About.
- [ ] Interface remains flicker-free and all labels fit on the physical display.

## Known points to verify

- Speaker queue latency means changes take effect in subsequent queued audio blocks, not zero latency.
- PLAY focus is deliberately special: clicking acts on playback, while rotating clockwise enters SPEED focus.
- Live effect/delay adjustments may produce artifacts; test across loop and track boundaries.
- Compiling/physical QA is required before tagging an official release.
