# Phase 5I — Configurable FX slots (development build)

This version builds on hardware-verified Phase 5H.6. Its configurable effects need a new hardware validation pass before releasing.

## Effects menu

Six scrollable effects: ECHO, STUTTER, ROBOT, WARBLE, BITCRUSH, TELEPHONE. Wheel scrolls list; click begins editing `SLOT` (0=UNUSED, 1..3). Subsequent clicks step through that effect's parameters, then finish and save. Wheel edits the active field. The scroll indicator shows your place in the list. The Effects menu **never** enables or disables an effect.

Each slot accepts one effect. Reassigning an occupied slot makes the formerly assigned effect UNUSED and OFF. Leaving an effect UNUSED turns it OFF. Empty FX player buttons are hidden without changing the button layout; wheel focus skips empty slots. The original default assignments are ECHO→1, STUTTER→2, ROBOT→3. Existing 5H.6 enable states and audio parameters migrate into new preferences the first time the new firmware boots.

## Player

FX slots remain in the existing positions. Select a populated FX slot with the wheel and click to edit; adjust with the wheel as before. Turn up from OFF to enable and then adjust the primary effect parameter. Turn down to the minimum and once more to disable. The name shown beneath PLAYING/READY follows the assigned effect. The short labels on buttons identify what is assigned; the on-light still indicates enabled status. The original audio is not altered in storage.

## Audio processing

- ECHO: delay and wet mix; retains fading one-shot tail.
- STUTTER: repeating grain (80–400 ms).
- ROBOT: square-wave ring-like amplitude modulation.
- WARBLE: sinusoidal playback-rate modulation, adjustable Hz and depth; pitch varies naturally.
- BITCRUSH: adjustable quantization bits and sample-and-hold interval.
- TELEPHONE: lightweight high-pass/low-pass filtering.

Three assigned effects may be enabled simultaneously. Processing order is fixed: Warble (reading rate), Stutter, Robot, Bitcrush, Telephone, Echo. Extreme effect combinations can distort. No additional recording storage is used.

## Upgrade / safe testing

Build and upload using the standard PlatformIO environment; don't run `uploadfs` or erase flash. Phase 5H.6 remains the rollback build. Check all six assignments, displacement, persistence after reboot, disabled/hidden slot navigation, all three palettes and skins, portrait menu scroll, and all audio controls. PlatformIO compilation and on-device audio testing were not possible in this environment.
