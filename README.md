# Rolling Thunder MD

Native Mega Drive / Genesis port of Rolling Thunder, frozen at the October 16,
2026 development build. Includes Arcade, Home Campaign and Survival, two-player
co-op, the HUD and shirt-colour fixes, increased co-op ammunition, and adaptive
2P enemy animation. Enemy counts are unchanged.

In lighter 2P scenes, enemies use their full banked poses. Crowds, rendering lag
or sprite-memory pressure select compact poses; full detail returns after 30
light simulation updates. 1P keeps its existing animation policy.

## What is and is not included

This repository contains the native engine, converted **graphics/layout tables**
(in C and JSON), build tools, and a small vendored offline audio synthesizer.
It is **not an asset-free repository**. The native art includes the existing
Leila/RT2 adaptation and project-specific artwork; no RT2 executable is included.

No original ROM archives, executable ROM images, built `.bin` files, extracted
voice samples, or rendered music clips are included. You must supply your own
**Rolling Thunder revision 3 arcade `rthunder.zip`** to regenerate the audio.
There are no ROM downloads in the build process. Input chip hashes are checked.
The Sega splash/chant and Namcot splash/logo are omitted entirely; the binary
boots directly into the game menu. The `SEGA SSF` cartridge-header identifier
remains because it declares the mapper to emulators/hardware, not a logo asset.

## Build

Tested on macOS with:

- Python 3.12+ and NumPy (see `requirements.txt`)
- SGDK **2.12**, built with bank switching enabled
- GNU m68k-elf GCC **16.1.0** and binutils **2.46.1**
- A host C/C++17 compiler (`cc` / `c++`), GNU Make, and Java for SGDK's rescomp

The frozen bank checks require the tested cross compiler: generated lookup
functions and fixed completion-bank exports are layout-sensitive. A different
compiler may fail the bank checksum checks rather than silently producing an
untested layout. SGDK and its cross compiler are external dependencies, not
bundled here. Set `GDK` to the directory containing `makefile.gen`, `inc`, `lib`
and the `bin/m68k-elf-*` tools. Put a suitable `java` on `PATH`.

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install -r requirements.txt
export GDK=/path/to/sgdk
python tools/build.py --rom /path/to/your/rthunder.zip --sgdk "$GDK"
```

Or use `make build ROM=/path/to/rthunder.zip GDK=/path/to/sgdk`.
Do not put ROMs in Git. `roms/`, `.build/`, `dist/` and common ROM/archive formats
are ignored. The build reads the supplied archive; it never modifies it.

Outputs:

- `dist/RollingThunderMD.bin` — 32 MiB SSF-mapper development cartridge
- `dist/SHA256SUMS` — binary checksum
- `dist/build.json` — build identity and validation metadata

All intermediates are generated under `.build/`. No research checkout, previous
Mega Drive ROM, local extracted-audio directory, emulator, or network download is
needed to build. `make clean` removes only generated build/output directories.
`CC` and `CXX` can select the host compilers; `--jobs` controls stage compilation.

The audio build decodes the arcade samples, converts the source music records,
and synthesizes the finite ending/high-score clips with pinned ymfm source.
Every stage bank and finite audio clip is checked against the frozen baseline.
Native runtime and Z80 drivers are compiled from source, then the cartridge
banks are assembled from those new outputs.

## Playing and validation

Use an emulator with **32 MiB SSF mapper support**. The tested core is PicoDrive.
A 4 MiB-only loader cannot run this cartridge. Physical-cartridge compatibility
has not been certified. This is a development build, not a full-game release
certification. Standard Genesis controller inputs are used; Home's setup lets
player two join before choosing the characters.

The frozen adaptive build passed 48 opening performance runs, 20 stage/checkpoint
entry probes, 3,550 matched gameplay-state comparisons and 862 unchanged 1P
image comparisons. Average 2P displayed rates were 54.49 FPS in Campaign and
43.09 FPS in Survival. These include presentations reusing artwork; they are not
fresh-animation rates or a guarantee of 60 FPS. Restoring light-scene animation
costs about 1–1.5 FPS compared with permanently compact detail.

**Start pauses/resumes gameplay** in Arcade and Home. Either controller can
pause a 2P game; controller two is ignored for pause in solo. The screen dims,
gameplay and its timer freeze, and music continues. Menu confirmation and
Continue retain their existing Start behavior.

**Battery-backed SRAM** stores the best score, the five-record initials table,
starting credits and the sound test's PSG mute setting. Settings save when
changed; scores save at life/stage boundaries, game over, completed initials
entry and when you pause. Pause before powering off to save an ongoing run's
best score. Progress/lives are not resumed from SRAM. Empty or invalid SRAM
uses defaults; two CRC-checked slots protect against interrupted writes. Enable
battery saves in your emulator and retain its `.srm` file. In the tested PicoDrive
core, reload the ROM to restore saves; soft reset does not restore SRAM after
SSF bank remapping. The cartridge requires
SSF banking **and** standard odd-byte SRAM at `$200001–$20FFFF`.

The public build removes boot intros, updates the cartridge title, and adds
SRAM saves and Start pause. The frozen banked art/audio remain unchanged. See
`docs/VALIDATION.md` for checks specific to the public build.

## Source layout

- `src/`, `inc/`: runtime, Z80 drivers and native tables
- `assets/stages/`: stage data and generated lookup source, independently linked
- `assets/completion/`, `assets/ui/`: native completion/menu/high-score data
- `tools/`: owner-ROM audio conversion and portable build orchestration
- `vendor/ymfm/`: offline YM2151 synthesis source; never runs in the cartridge

## Notices

Rolling Thunder and its original characters/art/music belong to their respective
rights holders. This is an unofficial port. No license to the original game
assets is granted by this repository. Third-party source licenses and notices
are retained under `licenses/` and `vendor/ymfm/LICENSE`; see `NOTICE.md`.
