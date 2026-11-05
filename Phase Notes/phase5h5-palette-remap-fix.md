> Historical development note: **superseded by Phase 5H.6**. Current green-background conventions are documented in `phase5h6-green-background-rollback.md`.

# Phase 5H.5 — Magenta background shading fix

The background palette remapper now accepts nearly-neutral magenta ramp shades, including editor-generated `(R,1,R)` pixels and minor red/blue rounding discrepancies. Intensity is preserved for GREEN, RED and BLUE palettes. Non-magenta artwork remains untouched.

The supplied edited CLASSIC PNG is now the canonical `assets/ui/sound_chomp_spritesheet.png`. Yellow guides and fixed sprite coordinates remain unchanged. MINIMAL and ARCADE use the same corrected recoloring helper.

This fix affects generated player backgrounds only; audio and storage are unchanged. Use `python tools/test_palette_remap.py` to verify all generated palette variants.
