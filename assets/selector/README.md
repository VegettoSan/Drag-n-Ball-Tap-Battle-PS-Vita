# Vita boot-selector theme assets

These split Base64 files reconstruct a small ZIP containing **four derived PNG
elements** used only by the PS Vita data-set selector:

- `select0_background.png` — blue/cyan grid + energy background.
- `select0_header.png` — beveled blue title bar.
- `select0_button.png` — beveled cyan menu button.
- `select0_ball_1.png` — one-star Dragon Ball marker.

They were extracted/cropped from the supplied **gen.apk** resource
`assets/select0.pac` (audited PAC SHA-256:
`054157afe63699358f162eedb164f25df897d431e733e9d75f2bdcb8507bcf95`).
No character artwork is included in this selector theme.

The reconstructed ZIP SHA-256 is
`90418a27c6681ee644d5cc383e31fc73248a5c412527839d216b61bcc2516c12`.
`tools/materialize_selector_theme.py` validates the ZIP plus each PNG hash and
dimensions before any build can package them. The runtime also retains the old
plain selector as a fallback if an embedded texture cannot be loaded.

Do not replace these parts casually: the hashes are an integrity/provenance
contract for the exact theme tested by the port.


## 00.34 runtime use

The 00.34 selector must not draw the whole 512×512 background PNG. That source
contains two separate visual regions: the desired cyan/grid background occupies
the continuous top band (detected as 482×320 px), while a blue energy orb exists
later in the transparent lower section. Runtime sampling therefore crops to the
top band in both U and V and stretches only that band across the complete
960×544 Vita viewport. The header/button/ball
textures retain their independent placement and sizing.

The same background, header and button assets are reused for the
**OPENING PROFILE / LOADING GAME DATA...** transition shown immediately after
profile confirmation.

These assets are presentation-only. They do not select a codec, alter resource
paths, modify PAC bytes or change original gameplay behavior.

Current data/selector contract:
[CURRENT_RUNTIME_CONTRACT](../../docs/CURRENT_RUNTIME_CONTRACT.md).
