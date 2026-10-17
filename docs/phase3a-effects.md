# Phase 3A — Audio effects & screen test plan

## Before uploading

- Confirm you're flashing the M5StickS3 and that `platformio.ini` points to the correct serial port (currently COM9).
- Run **Build**. This code has not been compiled in the authoring environment and may need API adjustments for the locally installed M5Unified version.
- Run **Upload**, then monitor at 115200 baud.

## Device tests

- [ ] App shows Winamp-inspired dark screen with green text and no visible whole-screen black flashing.
- [ ] A records a phrase, and A or wheel press stops it.
- [ ] B plays the phrase at 1.00× and B stops playback.
- [ ] Wheel rotation while idle changes speed in steps, between 0.50× and 2.00×.
- [ ] Slower playback has lower pitch and longer duration; faster playback has higher pitch and shorter duration.
- [ ] Short wheel press while idle switches FWD / REV. Reverse playback sounds backward.
- [ ] Long wheel press (~650 ms) toggles looping; playback repeats until stopped.
- [ ] Rotating the wheel while audio plays changes speed on the **next loop** (or next Play), not immediately.
- [ ] A second recording replaces the first; playback options remain available.
- [ ] Long loop playback doesn't freeze the UI or prevent stopping.
- [ ] Serial monitor shows speed, direction, loop changes and playback information.

## Limitations and known behavior

- Single clip, maximum ~8 seconds at 16 kHz / 16-bit mono; stored in volatile PSRAM.
- Speed shift changes **pitch and duration**; this is not independent pitch shifting.
- UI mini bars are decorative (not audio-derived).
- No persistent clip storage, playlist, EQ or true spectrum visualization.
- Scroll short click action has intentionally changed from Phase 2's Record/Play selector to reverse toggle. A/B remain Record/Play.
- Long pressing wheel during recording stops it rather than toggling loop.
- Speed changes during playback are applied on the next loop iteration; audio is pre-rendered before each `playRaw` call.
- Off-screen canvas allocates RAM; if creation fails, an error is shown on the display and logged to serial.
