# Hardware compatibility — October 17, 2026

The compact ROM is **15.5 MiB (16,252,928 bytes)**, down from 32 MiB. Two
profiles are built from the same assets and gameplay code:

- `RollingThunderMD.bin`: standard SSF2 banking and odd-byte SRAM, tested in
  PicoDrive and Genesis Plus GX.
- `RollingThunderMD-MegaEverDrivePro.bin`: extended SSF banking and banked
  battery RAM, intended for Mega EverDrive Pro. Software protocol tests pass;
  **no physical cart or console has been tested**.

The Pro profile is ready for physical testing, not a hardware compatibility
certificate. Neither profile supports every EverDrive.

## Target matrix

| Target | Status |
| --- | --- |
| Mega EverDrive Pro | Fits its published 16 MB memory capacity. Dedicated extended-SSF profile; ROM stops before bank 31, which the published mapper assigns to battery RAM. Physical launch, menu backup and power-cycle persistence remain unverified. |
| Mega EverDrive Core | Unsupported by this build: exceeds its 8 MB memory capacity. |
| Mega EverDrive X3 / X5 | Unsupported by this build: exceeds the published 7 MB ROM limit. |
| Mega EverDrive v2 / X7 family | No support claim. Older extended-SSF documentation reserves banks 30–31 for the OS; this layout uses bank 30. The Pro profile must not be treated as an X7 profile. |
| Original Mega EverDrive v1; legacy EverDrive-MD v1/v2/v3 | Unverified and not supported by the supplied profiles; exact revision/firmware limits need separate evidence. |
| Model 1 with/without TMSS, Model 2, Model 3, Nomad; NTSC/PAL | CPU/VDP startup measures and emulator region checks are covered. No physical console tested. |
| MegaSD, MiSTer, clones, 32X/CD combinations, custom cartridges | No compatibility claim; require separate mapper and device testing. |

Sources: [Pro specifications](https://krikzz.com/our-products/cartridges/mega-everdrive-pro.html),
[Core specifications](https://krikzz.com/our-products/cartridges/mega-everdrive-core.html),
[X3 specifications](https://krikzz.com/our-products/cartridges/mega-everdrive-x3.html),
[X5 specifications](https://krikzz.com/our-products/cartridges/mega-everdrive-x5.html),
[older extended-SSF protocol](https://krikzz.com/pub/support/mega-everdrive/x3x5x7/dev/extended_ssf-v2.txt),
[Pro mapper implementation](https://github.com/krikzz/mega-ed-pub/blob/master/fpga/mapper/map_ssf/map_ssf.sv).

## Lossless layout

The builder first reproduces all ten frozen stage banks. It then interns repeated
geometry, uses the existing 16-cell map-block representation, and shares raw pose
pixels across stages. Identical suffix/prefix tiles can overlap in the physical
pixel pool. Every pose remains contiguous raw pixels; no image decompression or
new decode buffer runs during gameplay.

General enemy geometry is omitted only where the gameplay renderer exclusively
uses its equivalent packed geometry. Player and stage-object general geometry
is retained. Enemy counts, animation selection policy, audio samples and every
rendered pose remain unchanged. The build compares linked pixels and geometry,
background maps/palettes and collision data against the frozen banks. Completion
assets are relinked with pointer-only changes; their pixels remain unchanged.

Shared-pose DMA temporarily maps window 7, transfers the pixels, and restores the
previous mapping. Existing 128 KiB DMA-boundary splitting is retained. The fixed
runtime stays below 1 MiB; compact stage metadata stays below window 7. UI and
short victory PCM occupy spare space around the fixed bank. All four finite
clips are checked with the actual runtime alignment/span planners during build.

## Mapper and saves

The standard profile declares `SEGA MEGA DRIVE` and uses `$A130F1` SRAM control
at `$200001–$20FFFF`. This avoids the previous GX extended-mapper boot crash.

The Pro profile declares `SEGA SSF`. Saves temporarily map physical bank 31 into
CPU window 4 and access its upper half through `$240001`. Protected word writes
`$A000` / `$8000` at `$A130F0` open/close the write gate while keeping bank zero
fixed. Interrupts and Z80 bus consumers are held during the transaction. The
previous mapping is restored before they resume. Reset closes the write gate
before restoring application state. The two-slot CRC/commit-last format remains
unchanged.

GX does not implement this cartridge's complete write-gate/banked-RAM behavior.
`tests/test_everdrive_model.py` supplies a software bus model based on Krikzz's
published register logic and runs the actual Pro binary. It checks save writes,
reset, an interrupted-save reset condition and absence of writes to ROM. Fresh
processes also test persistence and corrupt-slot fallback. This tests our
protocol usage; it does not test the FPGA, firmware, electrical timing or OS save
backup behavior on a real cartridge.

## Side Arms measures retained

- Unconditional TMSS/startup path after a flash-cart menu; validated warm-reset
  marker instead of controller-I/O heuristics.
- Exact initialized-data and Z80 copies under GCC 16.
- DMA control-port writes guarded against raster interrupts and conflicting
  Z80 bus activity; DMA triggers use memory operands.
- Null Team Player callback guard, tested with held inputs on 3/6-button modes.
- Conventional `$00FF0000–$00FFFFFF` work-RAM header.
- Save validation after restoring ROM mapping; committed mirrors survive reset.

Side Arms was inspected without modifying its worktree. Its device results do
not certify this game. Its linear-ROM mapper workaround cannot apply to a game
that still requires bank switching.

## Physical validation still required

Record cartridge model/revision and OS, console revision/region and attached
hardware. Check cold power-on, launch from the cart menu, held buttons, reset to
game/menu, 1P/2P, all stage transitions, long audio playback, and saves after menu
return and full power cycling. Confirm the Pro OS actually backs up the banked
battery data. Smaller cartridges need a substantially smaller, separately
validated build; a header change alone cannot make this image fit.

Exact software results and artifact hashes are in [VALIDATION.md](VALIDATION.md)
and [validation.json](validation.json). The earlier 32 MiB audit is preserved in
[commit f035d7e](https://github.com/rester159/rollingthunderMD/blob/f035d7e/docs/HARDWARE_COMPATIBILITY.md).
