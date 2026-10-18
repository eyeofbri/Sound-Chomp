# Phase 4A.5 — Targeted UI patch

Based on Phase 4A.4; the existing three-buffer audio pipeline and recording controls are unchanged.

- Shortened no-recording waveform prompt to prevent wrapping.
- Direction icons now use open, pixel-drawn chevrons (`<<` / `>>`) instead of filled symmetric triangles.
- Wheel turning while idle briefly spins the reels in the same direction.
- Bottom-right time label uses a new 1px-taller seven-segment numeric font.
- Cleaned up loop arrowhead junction.
- Reels shifted 2px to the right, thicker spokes, and removed joining horizontal line.
- Recording banner extends under the reels while their central hubs remain opaque black; banner ends above analyzer frame.

## Validate on-device

1. Confirm READY prompt stays on one line and status layout does not overlap.
2. Confirm rightward settings show `>>`, reverse shows `<<` (not diamonds).
3. Turn wheel while READY and observe short, correctly directed reel motion.
4. Listen to normal and reverse audio to confirm no regression in buffering.
5. Record and check that red banner extends behind both reels, does not touch analyzer.
6. Check larger timer and loop icon visually.

**Build status:** Prepared and statically checked; not yet compiled or physically tested.
