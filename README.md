> Phase 5B.1 experimental UI update: see [docs/phase5b1-ui-controls.md](docs/phase5b1-ui-controls.md).

> **Phase 5B development candidate (`v0.6.0-dev`):** Focusable PLAY/SPEED/FX-1/FX-2/FX-3/LOOP player controls, Winamp-inspired transport strip, Main Menu B navigation, and versioned splash. See [Phase 5B guide](docs/phase5b-player-controls.md). This candidate has not been compiled or hardware-tested.

> **Phase 4C development prototype:** Four RAM-backed tracks now save to LittleFS and reload after reboot. See [persistent storage notes](docs/phase4c-persistent-tracks.md) for important first-run formatting warnings and limitations.

# Sound-Chomp

An open-source pocket-sized Yak Bak-inspired voice recorder and sound toy for the **M5StickS3** + **M5Stack Unit Scroll**. Developed with PlatformIO and Arduino.

## Project overview and build

- Main menu with Chomp, Tracks, Settings.
- A: hold-to-record in Chomp; select on Home.
- Key2 / B: Back to Home.
- Wheel: signed varispeed (-2x to +2x), including live changes while playing; 0 pauses.
- Wheel click: play/stop inside Chomp; open menu entries elsewhere.
- Wheel long-click: toggle loop inside Chomp.
- Improved playback chunk queueing (three rotating buffers) to reduce choppiness at 1.00x.
- Live audio spectrum; idle waveform; purple reverse accent; Winamp-inspired flicker-free display.
- Battery indicator, speaker volume and brightness settings (persist across reboots).
- Four separate 8-second 16-kHz 16-bit mono RAM recording slots (Phase 4B). Clips are lost on reboot.

### Build and upload

1. Open the `Sound-Chomp` folder in VS Code with PlatformIO installed.
2. Connect M5StickS3 by USB. This package retains the developer's local `COM9` configuration; adjust/remove the port settings on other machines.
3. Build with PlatformIO, then upload and test. Check `docs/phase4a-navigation.md` for controls and the hardware test checklist.

Prior phase guides remain in `docs/`. MIT licensed; see `LICENSE` in the existing repository.


## Phase 4A.2 experimental changes

- **Chomp:** tap A to play/stop; hold A ~300 ms to record and release to stop.
- Scroll click also plays/stops; long scroll click toggles looping.
- Scroll rotation controls playback speed from -3x to +3x; zero pauses.
- Settings include Volume, Brightness, Reverse Map (Normal/Inverted), and Auto Play.
- Auto Play automatically starts the last recording after releasing A; off by default.
- Inverted reverse mapping means further into negative wheel positions slows reverse playback; transitioning through zero into reverse jumps initially to fast reverse.
- Tracks is still a placeholder; recordings reside in RAM.

**Verification:** This package has not been compiled or physically tested. Preserve your validated Phase 4A.1 commit for rollback.

### Current experimental prototype — Phase 4A.3

Adds a skippable animated splash, Chomp-first boot, revised cassette/Winamp UI,
0.10× speed steps with no zero detent, reset-speed-on-recording preference, and
How To / About pages. See [`docs/phase4a3-ui.md`](docs/phase4a3-ui.md).


## Phase 4A.5 preview (unverified)

- Playback speeds ±0.50x to ±3.00x in 0.10x increments, skipping the noisy region near zero.
- Animated stereo tape reels rotate according to playback speed and direction; they also turn during recording.
- Green/yellow/red live spectrum, switching to purple/yellow/red in reverse.
- Two-column Home menu with icon hints and reserved Effects page.
- Charge page with large battery and 12-second screen dim to off; button A or Unit Scroll press wakes it, B returns Home. This turns off only the **display**, not the device or charger.
- Brightness minimum 5%; About includes Built for Aiden.

See `docs/phase4a4-transport.md` for details and a test checklist.


## Current prototype — Phase 4B multi-track

- Four separate 8-second tracks in PSRAM, with individually selectable recording buffers.
- B opens Home, then choose Tracks. Rotate wheel to highlight CHOMP 01–04; tap A or wheel click to select and return to Chomp.
- An asterisk identifies the active slot; recorded lengths appear next to each track.
- Hold the Unit Scroll click on the highlighted track to ask for delete confirmation; click again to erase the track or B to cancel.
- Recording replaces only the currently active track. **Recordings are not saved to flash.**
- At ±3.00x limits, extra wheel turns flash reel spokes red instead of animating.
- Playback timer now wraps for looping, alongside minor banner, icon and splash polish.

See `docs/phase4b-tracks.md` for full controls, technical notes, and verification checklist.

### Phase 4D prototype

Phase 4D adds last-selected-track persistence, three loop modes (OFF / 1 / ALL),
and tape-transfer reel animation. See [Phase 4D test notes](docs/phase4d-tape-transfer.md).
This is an unverified development package; use the last tested Phase 4C build for rollback.

## Current development package — Phase 5A (experimental)

Based on the hardware-verified Phase 4D persistent-track build, Phase 5A adds a **non-destructive echo effect** with adjustable mix and delay. Use **B → Effects** to enable echo and change its settings. In Chomp, **short scroll click switches between Speed and Echo mix**, scroll rotation adjusts the highlighted mode, **long scroll click cycles loop modes**, **tap A plays/stops**, and **hold A records**. Loop All displays `ALL(n)` for the current slot.

Four stored recordings and track selection survive device restarts. Echo preferences are stored in NVS, not written into PCM files. This source package has not yet been built or hardware-tested: see [Phase 5A guide](docs/phase5a-echo.md) for the definitive current controls and tests. Earlier historical sections above describe older behavior and should not be used as current instructions.
