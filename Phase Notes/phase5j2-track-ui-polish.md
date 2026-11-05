# Phase 5J.2 — Track UI polish

Based on Phase 5J.1; no filesystem repartitioning and no audio file migration.

- The player status line shows the currently selected track's persisted display name. It truncates safely to fit a single line and favors the name if FX/loop suffixes would overflow in portrait.
- Track actions are code-drawn 9x9 pixel icons: Play, serif T (Rename), Trash, Padlock Open/Closed, and bold Move Up arrow. No extra sprite coordinates or third-party icon assets.
- Locked track: Delete remains in its normal position but is dimmed and cannot open a confirmation dialog. Lock/Unlock icon changes immediately.
- Reorder popup is 75% of display width and right aligned, exposing the left portion of the track list. Live list order still updates as you scroll. CLICK and B finish the move (previous implementation persists each swap immediately).
- Four tracks, saved names, saved order, lock checks, skins, palettes, and effects remain unchanged.

## Hardware checks

1. Rename a track to 10 characters; verify its name appears beneath READY/PLAYING in portrait and landscape.
2. Enable FX 1–3 and loop; verify player status never wraps or covers the reel area.
3. Verify five icons on each track row and active yellow focus outline.
4. Lock a track: delete action dims and click cannot delete. Unlock restores action.
5. Reorder in portrait/landscape: track list remains visible left of the move popup and visually updates during scrolling.
6. Restart; verify names, lock states, and order persisted. Test recording/playback and effects remain working.

**Status:** Source-level and asset checks only, not a verified PlatformIO compilation or hardware test.
