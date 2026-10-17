# Hardware compatibility audit — October 16, 2026

**The current build is not ready for an EverDrive/original-console release.**
PicoDrive success does not establish cartridge compatibility. No physical
console, EverDrive, MegaSD or MiSTer was connected or tested in this audit.

Audited source: `9c82197781b92f730ea248a440816f7c5fbc432a`.
ROM: 33,554,432 bytes (32 MiB), SHA256
`133fea599cb00cb5de33d9fae86b9c15b9e18e5760b4497a2f906101ca74b6b8`.
The audit adds tests/documentation; it does not change this ROM.

## Reproduced results

| Check | Result |
| --- | --- |
| ROM checksum / declared ROM end | Valid / matches file size |
| PicoDrive, US NTSC / European PAL / Japanese NTSC | All three pass menu, opening Campaign 2P, P2 pause/resume, soft reset and restart with music |
| Genesis Plus GX, same three regions | All three fail boot |
| GX with dirty RAM and initialized controller I/O, same regions | All three fail boot; startup wrongly selects the warm-reset path |
| Host SRAM tests | Pass, including all 65 interrupted-write boundaries and warm mirror restoration |
| Physical consoles and flash carts | Not tested |

The region probes change only the region header in temporary copies. They do
not cover complete campaigns, raster timing on different VDP revisions, analog
audio/video, power cycles, or flash-cart firmware. The menu VBlank counter
advances 120 times during a 120-frame PicoDrive observation in each region;
this is a liveness check, not a gameplay FPS measurement.

Machine-readable evidence, including exact core hashes and mapper writes:
[`compatibility.json`](compatibility.json).

## Release blockers

### Cartridge capacity and bank layout

The ROM is twice the Pro's documented 16 MB PSRAM capacity. The linked stage
banks alone occupy 22,829,484 bytes. The last nonzero ROM byte ends at offset
31,527,525, so trimming trailing padding cannot make it fit.

| Target family | Published capacity / audit status |
| --- | --- |
| Mega EverDrive Pro | 16 MB PSRAM; current image exceeds it |
| Mega EverDrive Core | 8 MB PSRAM; current image exceeds it |
| Mega EverDrive X3 / X5 | 7 MB maximum ROM; current image exceeds it |
| Mega EverDrive v2 / X7 family | Official support manual lists 16 MB PSRAM; current image exceeds it. Manual's maximum-ROM line has contradictory units, so no exact usable limit is asserted here |
| Original Mega EverDrive v1; legacy EverDrive-MD v1/v2/v3 | No support claim. Exact revision/firmware capacity and mapper behavior remain to be verified; these need their own test rows before release |
| Custom physical cartridge | Requires a specified mapper/memory design; none tested |

Sources: [Pro specifications](https://krikzz.com/our-products/cartridges/mega-everdrive-pro.html),
[Core specifications](https://krikzz.com/our-products/cartridges/mega-everdrive-core.html),
[X3 specifications](https://krikzz.com/our-products/cartridges/mega-everdrive-x3.html),
[X5 specifications](https://krikzz.com/our-products/cartridges/mega-everdrive-x5.html),
[v2 manual in the X3/X5/X7 support directory](https://krikzz.com/pub/support/mega-everdrive/x3x5x7/Mega%20Everdrive-v2%20user%20manual%20%20EN.pdf).

Meeting a file-size limit alone is insufficient: reserved OS/save banks and
supported mapper registers also constrain the usable layout. Older Krikzz
extended-SSF documentation mentions possible 16/32 MB memory implementations;
that is not evidence of 32 MB support in the retail models above.

### SRAM protocol conflicts with the mapper

`SEGA SSF` selects extended banking in GX. `src/save.c` calls SGDK's standard
SRAM API, which writes `$A130F1`. On the audited GX core, the first write of
`1` at frame 25 (US/Japan) or 22 (PAL) changes bank zero's ROM base from
`$000000` to `$080000`. The running instructions/vectors are replaced and boot
fails. Subsequent RAM fault values are corruption, not meaningful game error
codes. The trace forwards all writes unchanged.

Krikzz's extended-SSF protocol also uses banked save memory instead of this
standard SRAM overlay. Its bank-zero protection and save mapping differ from
GX, so the precise GX failure must not be described as an observed EverDrive
failure. Both the older protocol and current published FPGA source must inform
the implementation; neither is emulated by the existing standard-SRAM code.
See [Krikzz's extended-SSF specification](https://krikzz.com/pub/support/mega-everdrive/x3x5x7/dev/extended_ssf-v2.txt)
and [published SSF mapper implementation](https://github.com/krikzz/mega-ed-pub/blob/master/fpga/mapper/map_ssf/map_ssf.sv).

PicoDrive accepts the current combination, which explains the earlier passing
save/reset tests. Its behavior is insufficient for the intended hardware target.

### Flash-cart startup can mistake another program's RAM for a saved session

The SDK uses controller control registers to distinguish cold and warm startup.
The dirty-launch probe fills work RAM with `$A5` and sets I/O registers 4 and 6
to `$40`, following the Side Arms test. The first observed bank write comes
from `__wrap__reset_entry`, and `intCB=$A5A5A5A5` plus save availability `$A5`
survive into SDK reset initialization. This confirms that arbitrary old state
is trusted. TMSS initialization is also skipped by the selected SDK path.

The independent mapper failure prevents attributing the eventual boot failure
solely to this startup defect. Fixing startup needs a validated same-game reset
marker, deterministic callbacks and TMSS handling, while preserving committed
saves on a genuine soft reset.

## Comparison with Side Arms

Reviewed local history and `docs/sound_port.md`, plus the build guards and QA
scripts in Side Arms. Its worktree was not modified.

| Side Arms change | Rolling Thunder finding |
| --- | --- |
| Linear cartridge header and no-op mapper for a ROM at most 4 MiB | Cannot copy directly: Rolling Thunder genuinely needs banking. Repack data and choose a supported mapper/save protocol first |
| Prevent MiSTer small-ROM SRAM aliasing from SDK bank writes | Relevant lesson, but its small-ROM bus model does not describe this 32 MiB cartridge |
| Unconditional safe boot/TMSS after flash-cart menu | Missing equivalent; dirty-launch probe exposes invalid retained state |
| Exact Z80 driver upload under GCC 16 | Already present in project `sgdk_z80_ctrl.c`; linked disassembly confirms separate source-read and destination-write instructions |
| H-interrupt protection around VDP DMA command sequences | Project DMA flush and CPU-copy paths already mask interrupts; linked flush routine includes the guard. Physical raster timing remains unverified |
| Guard null Team Player callback during detection/reset | Missing at `src/sgdk_joy.c` Team Player reader: `if (change) joyEventCB(...)` can call a null pointer. Code-review finding; no physical Team Player test performed |
| Conventional work-RAM header range | Header uses `$E0FF0000–$E0FFFFFF`; normalize metadata to `$00FF0000–$00FFFFFF`. The 68000 address alias itself is not proof of a runtime fault |
| Copy SRAM to RAM before validating ROM-dependent structures | Current save code already validates after disabling its SRAM window, but the selected SRAM protocol is incompatible |

Side Arms' reported hardware successes apply to those specific builds/devices,
not to Rolling Thunder or all later Side Arms changes.

## Work needed before physical testing

1. Repack/deduplicate or compress stage assets and revise bank access so the
   image fits the chosen cart tier, including its reserved save/OS space.
   A single unchanged 32 MiB image cannot satisfy the listed retail capacities.
2. Implement the appropriate banked save protocol and regression-test cold
   load, reset, interrupted saves and bank restoration while audio is active.
3. Harden startup, guard the Team Player callback, normalize header metadata,
   and run GX plus PicoDrive regressions against the rebuilt cartridge.
4. Test actual flash carts across US/Japanese NTSC and European PAL consoles:
   Model 1 with/without TMSS, Model 2, Model 3 and Nomad where applicable.
   Record cart revision/OS, console revision, cold power-on, menu launch, held
   buttons, reset-to-game/menu, 1P/2P, all stage transitions, long audio playback,
   and SRAM persistence after returning to the cart menu and power cycling.
   MiSTer, MegaSD, clones and attached 32X/CD configurations need separate rows.

## Reproduce locally

Build with an owner-supplied arcade archive first, then run:

```sh
python3 tests/check_compatibility.py \
  --picodrive /path/to/picodrive_libretro.dylib \
  --gx /path/to/genesis_plus_gx_libretro.dylib
```

Results go to `.build/compatibility/results.json`. The current build intentionally
returns exit status 1 because the compatibility gate fails. Each emulator case
runs in its own process with a timeout. The cores and ROM are not distributed.
