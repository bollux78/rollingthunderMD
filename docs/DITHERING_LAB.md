# Rolling Thunder MD — Dithering Lab

Experimental visual color-conversion lab for [rester159's Rolling Thunder MD](https://github.com/rester159/rollingthunderMD), developed in the [bollux78 fork](https://github.com/bollux78/rollingthunderMD).

## Version 0.1 — sprite preview

A standalone Tkinter + Pillow viewer that uses the upstream `tools/extract_assets.py` decoder to read sprites directly from a user-owned **Rolling Thunder arcade revision 3** `rthunder.zip` archive.

- Compare **Original**, **Bayer 4×4**, **Bayer Balanced**, and **Bayer Adaptive**.
- Browse the 1024 original arcade 32×32 sprite frames.
- Manually select among 128 original sprite palette groups.
- Export the side-by-side 1024×512 pixel comparison to PNG.
- Preserve the original fork's C files, converted assets, build system and ROM checks.

**Note:** Palette selection is manual in this first build. The tool does not infer the palette group assigned to a frame in-game, so it is a color experiment, not yet a faithful screenshot renderer. Bayer color pairs are constrained to a single sprite palette group, pen 15 remains transparent, and comparisons are approximations of the RGB333 conversion.

## Run (Linux / Fish)

From a clone of the complete fork:

```fish
cd ~/Projetos/rollingthunderMD
python -m venv .venv
source .venv/bin/activate.fish
python -m pip install Pillow
python tools/rolling_dither_lab.py --self-test
python tools/rolling_dither_lab.py
```

Or pass your legally obtained original arcade ROM directly:

```fish
python tools/rolling_dither_lab.py --rom /path/to/rthunder.zip
```

No ROM files, sprite graphics or samples are included or uploaded. All ROM chip hashes are checked against the upstream extraction tool. **This version is preview-only; it does not generate or patch Mega Drive ROMs.**

## Next steps

1. Correctly associate sprites with their in-game palette groups.
2. Decode background tiles and reconstruct the stage maps.
3. Add Black Tiger-style per-tile Bayer parameters, color pair copy/paste and JSON profiles.
4. Introduce a separate experimental build option for modified graphics, without weakening the upstream baseline bank validation.

## Credits

Original Mega Drive port: [rester159](https://github.com/rester159/rollingthunderMD). Dithering Lab adaptation: bollux78, building upon the Black Tiger MD Dithering Lab experiments. Rolling Thunder and its copyrighted art/audio belong to their owners. Please retain the upstream notices.
