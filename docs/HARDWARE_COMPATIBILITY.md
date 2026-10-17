# Hardware compatibility — October 17, 2026

**The current 32 MiB development build is still not ready for retail EverDrives.**
The GX boot/SRAM crash, dirty-launch initialization and null Team Player callback
have been corrected. ROM capacity and device-specific mapper validation remain.
No physical console, EverDrive, MegaSD or MiSTer was connected or tested.

ROM SHA256: `58a62d053a9f10a0f610b3e7fce55593693cca90738d3e946d6ee6c066864412`.

## Verified changes

| Check | Current result |
| --- | --- |
| PicoDrive, US NTSC / European PAL / Japanese NTSC | All pass opening Campaign 2P, P2 pause/resume, soft reset and restart with music |
| Genesis Plus GX, same regions | All pass the same checks; previously failed boot |
| GX with dirty RAM and initialized controller I/O | All three regions pass; a committed same-build marker replaces the I/O warm-reset heuristic |
| C initialized-data image | All 104 bytes match ROM before SDK initialization in cold/dirty GX launches |
| Soft resets across Arcade, Campaign 1P/2P, Survival 1P/2P | 60 pass across the two cores |
| Fresh-process SRAM restore and corrupt-slot fallback | Pass on both cores |
| Graphics/audio bank comparison | All 31 MiB byte-identical to the previous build |
| GX Team Player 3/6-button held-input startup/reset | Both pass, three resets each; physical adapter untested |
| Physical hardware | Untested |

The region probes change only the header in temporary ROM copies. They test
liveness and opening gameplay, not full campaigns, physical VDP timing, analog
output or flash-cart firmware. Exact core hashes, initial-data checks and mapper
write traces are in [compatibility.json](compatibility.json); save/reset evidence
and reproduction commands are in [VALIDATION.md](VALIDATION.md).

## Mapper correction

The runtime uses standard SSF2 bank registers plus standard SRAM at `$A130F1`.
Its old `SEGA SSF` header selected GX's extended mapper, where the first save-enable
write replaced bank zero and the running code. The header now says
`SEGA MEGA DRIVE`, selecting the appropriate default bank/SRAM behavior in the
tested cores. The trace confirms save writes leave bank zero unchanged.

This is an emulator compatibility fix, not an implementation of Krikzz's banked
save protocol. A physical extended-SSF build would need a supported layout and
separate banked SRAM access. The old and current EverDrive implementations also
differ, so a GX pass must not stand in for device testing. References:
[Krikzz extended-SSF protocol](https://krikzz.com/pub/support/mega-everdrive/x3x5x7/dev/extended_ssf-v2.txt),
[published mapper](https://github.com/krikzz/mega-ed-pub/blob/master/fpga/mapper/map_ssf/map_ssf.sv),
[GX mapper implementation](https://github.com/ekeeke/Genesis-Plus-GX/blob/master/core/cart_hw/md_cart.c).

## Capacity remains the main blocker

The ROM is 33,554,432 bytes. Its linked stage banks alone occupy 22,829,484
bytes, and its last nonzero byte ends at offset 31,527,525. Trimming trailing
padding cannot make it fit. No enemies, animation frames or audio were removed.

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

## Side Arms changes reviewed and applied

| Side Arms lesson | Rolling Thunder status |
| --- | --- |
| Match header to actual mapper/SRAM protocol | Corrected for the current standard-SSF2 development build; hardware profile still pending |
| Safe entry/TMSS after a flash-cart menu | Applied through build-local SDK assembly adaptation; retained state is validated and callbacks rebuilt |
| Exact Z80 upload under GCC 16 | Already present; the same compiler hazard was newly found and fixed in reset's C-data copy |
| Protect DMA command sequences from H-interrupts | Existing project DMA guards retained; physical raster timing unverified |
| Guard null Team Player callback | Applied to the vendored reader |
| Conventional work-RAM header | Corrected to `$00FF0000–$00FFFFFF` |
| Validate copied SRAM after restoring ROM mapping | Existing behavior retained; two-slot save format unchanged |
| Disable mapper writes for a linear ROM at most 4 MiB | Cannot copy directly: this game genuinely uses banks |

Side Arms' worktree was not modified. Its hardware successes apply to its own
specific builds/devices. The original failing audit is preserved in
[commit 4002165](https://github.com/rester159/rollingthunderMD/blob/4002165/docs/HARDWARE_COMPATIBILITY.md).

## Remaining work

1. Repack the assets within the chosen cart's usable ROM space, including its
   reserved OS/save banks. Simple sharing of complete poses is insufficient;
   finer sharing or compression needs rendering-cost and RAM-budget measurements.
2. Select the actual supported mapper per hardware tier and implement its SRAM
   protocol where needed. Re-test cold load, interrupted saves, reset and bank
   restoration while audio is active.
3. Physically test US/Japanese NTSC and European PAL, Model 1 with/without TMSS,
   Model 2, Model 3 and Nomad where applicable. Record cart revision/OS, console
   revision, cold power-on, menu launch, held buttons, reset-to-game/menu, 1P/2P,
   every stage transition, long audio playback and SRAM after menu return and
   power cycling. MiSTer, MegaSD, clones and 32X/CD configurations need separate
   rows.

`tests/check_compatibility.py` deliberately still returns status 1: all nine
emulator cases pass, but the 16 MiB capacity gate fails. This is not a hardware
release certificate.
