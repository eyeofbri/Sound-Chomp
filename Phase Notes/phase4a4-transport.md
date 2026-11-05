# Phase 4A.4 — Transport and menu refinement

This version is an **untested development candidate** based on the hardware-validated Phase 4A.3 firmware.

## Key features

- Wheel: +0.50x to +3.00x and -0.50x to -3.00x; 0.10x steps. Scrolling through zero jumps directly between ±0.50x.
- Reverse mapping is still selectable under Settings.
- Tape reels on Chomp rotate according to current audio speed while playing and at nominal speed while recording. They rest when the player is idle.
- Spectrum bands are green/yellow/red in forward and purple/yellow/red in reverse.
- Home uses two columns, filled downward first, then to the right: Chomp, Tracks, Settings, Effects, How To, About, Charge.
- Effects is a placeholder; no echo processing has been added.
- Charge displays a large battery icon. Its LCD goes dark after 12 seconds of inactivity (it does not suspend ESP32 or change charging). Tap A or click the wheel to wake, B to return to Home.
- Pixel-style wordmark retained. Serif lettering postponed until an embedded bitmap font can be assessed for legibility.

## Test checklist

- [ ] Build and upload to COM9; confirm microphone capture still works
- [ ] Verify +1.00x audio is clean (no crackling) and live speed changes work
- [ ] Verify no speed setting is below 0.50x in magnitude, including the zero crossing
- [ ] Verify normal and inverted reverse mapping
- [ ] Verify forward icon >>, reverse icon <<, and reels rotating correctly
- [ ] Verify reel direction/speed responds to scroll while playing; reels move during record
- [ ] Verify purple band bases during reverse playback; yellow/red peaks remain
- [ ] Verify main menu navigation proceeds down the left column then right
- [ ] Verify brightness can be set to 5% and retained after restart
- [ ] Verify Charge view shows percentage, turns LCD off after 12 seconds, and wakes
- [ ] Verify About credits Built for Aiden and How To colors

## Known limitations

- The reels are a pixel-art visual representation, not a simulation of tape transport mechanics.
- The waveform/spectrum remain calculated from PCM analysis; speaker audio pipeline unchanged.
- Charge page is a *display-only* screen-off feature, not system sleep or charging control.
- No real multi-track storage or audio effects yet.
- Not compiled or physically tested in this environment.
