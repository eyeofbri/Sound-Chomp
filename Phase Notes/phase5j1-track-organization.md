# Phase 5J.1 — Track Organization and Rename Editor

## Track actions

Four recording files are retained; this update does **not** repartition flash or increase track count.

Select a track with the wheel, then click to focus its actions. Turn to choose one of five actions:

- `>` plays the track and opens the player.
- `T` opens the rename editor.
- `X` opens deletion confirmation; hold the scroll button to delete. Locked tracks cannot be deleted.
- `L` locks, or `U` unlocks, the selected physical track. A locked track cannot be overwritten by holding A on the player.
- `^` enters move mode. Turn to swap the track's displayed position with adjacent rows, and click to finish. Reordering does not rename or move PCM files.

Press B to back out of an action/popup. The active track indicator (`*`) follows the underlying recording if it is reordered.

## Rename editor

The popup displays five characters (`AB C DE` pattern) around the currently selected character, with `_` representing a space. Focus advances among **CHAR**, **BACKSPACE**, **SAVE**, and **CANCEL** using a **long scroll click**. When CHAR is focused, turning the wheel selects a character and clicking adds it; for the other three actions clicking executes them. B cancels. Names remain at most ten characters and are stored in Preferences without renaming PCM files.

## Persistence

Locks use Preferences `trkLock0..3`, order uses `trkOrd0..3`, and names retain `trkNm0..3`. Invalid or duplicate stored order entries trigger the default order. The four underlying `/chompNN.pcm` files remain unchanged.

## Safety and limitations

- A lock blocks recording, but it does not provide a password or secure write protection against flashing/reformatting.
- The existing 5J temporary-save scheme remains unchanged and is **not** power-loss atomic when replacing a recording.
- This prototype has not been compiled by PlatformIO or tested on M5StickS3 hardware here. Test with disposable recordings first.
- The existing red selected-row and yellow focused-action colors are retained. New editor action focus follows the same yellow convention.
