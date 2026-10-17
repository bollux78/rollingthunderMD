# Startup, SRAM and reset validation — October 17, 2026

Cartridge: `RollingThunderMD.bin`, 33,554,432 bytes (32 MiB).

SHA256: `58a62d053a9f10a0f610b3e7fce55593693cca90738d3e946d6ee6c066864412`

The development cartridge passes the emulator checks below. It remains too
large for the documented retail EverDrive capacities; no physical hardware was
tested. See [hardware compatibility](HARDWARE_COMPATIBILITY.md).

- The owner-ROM build reproduces all ten frozen stage banks. All 31 MiB of
  banked graphics/audio match the preceding build byte for byte.
- US NTSC, European PAL and Japanese NTSC opening Campaign 2P, P2 pause/resume,
  reset and restart pass in both PicoDrive and Genesis Plus GX.
- Three additional GX launches with `$A5` work RAM and initialized controller
  registers pass the same checks. All 104 initialized-data bytes exactly match
  the ROM image at the first SDK reset operation, in cold and dirty launches.
- Sixty real soft resets pass: six in Arcade, Campaign 1P/2P and Survival 1P/2P,
  on each core. No ROM reloads or savestates occur between those resets.
- Playing sound-test, menu, paused-game and active-game resets restore the last
  committed best score, initials, starting credits and PSG mute. New saves and
  changed settings still work after repeated resets. Live uncommitted scores
  are discarded; restoring the save does not rewrite it.
- Fresh-process cold boots on both cores restore settings, best score and
  initials from real SRAM. Corrupting the newest slot recovers its predecessor.
- GX Team Player 3-button and 6-button modes pass startup and three resets
  each while B/Up are held. Detection, active Team Player support and received
  button state are checked. The previous runtime (with only its header corrected)
  fails reset liveness in this probe. No physical adapter was tested.
- Host tests cover all 65 interrupted SRAM-write boundaries, bad/empty/read-only
  storage, warm restoration with SRAM reads forbidden and corrupt-mirror fallback.
- Boot-adaptation tests verify that TMSS precedes the reset wrapper, unrelated
  vector/interrupt text is retained, and unrecognized SDK startup is rejected.

Scores/initials are seeded in work RAM to exercise save boundaries. Settings
are changed by controller. These are not full natural playthroughs, hardware
certification or new FPS measurements. `validation.json` and `compatibility.json`
record the exact ROM/core hashes and results.

## Implementation

The header now declares `SEGA MEGA DRIVE`. The runtime uses standard SSF2 bank
registers plus standard odd-byte SRAM, which both tested cores support. The old
`SEGA SSF` header wrongly selected GX's extended mapper and let a save-enable
write remap bank zero. The conventional work-RAM header is `$00FF0000–$00FFFFFF`.

Build-local startup assembly always performs the SDK TMSS check. Controller
registers no longer select the warm path. The wrapper recognizes a same-build
reset with a committed marker and retains only the CRC-validated save mirror.
It clears C globals, copies initialized data, assigns a deterministic interrupt
callback and lets the SDK reconstruct heap, DMA, display, input and audio.
Cold startup avoids early VDP reads; genuine warm reset drains active DMA first.

The initialized-data copy uses separate byte-read and byte-write instructions.
GCC 16 otherwise combined the C loop into an instruction whose destination
followed the incremented source register, shifting the data by one byte. Earlier
warm-reset save tests masked this defect because the save restored settings
later. The new GX probe compares the complete initial data image before SDK
mutation, and defaults are checked on both cores.

The Team Player reader now checks its optional callback before calling it,
matching the Side Arms fix. Physical adapters were not tested. Save-file format,
Start pause behavior, enemy counts, animation policy and audio assets are unchanged.

## Reproduce

```sh
cc -Wall -Wextra -Werror -Itests/host -Iinc src/save.c tests/test_save.c -o /tmp/test-save
/tmp/test-save
python3 tests/test_cart_boot.py
python3 tests/check_compatibility.py --picodrive /path/to/picodrive_libretro.dylib --gx /path/to/genesis_plus_gx_libretro.dylib
python3 tests/test_soft_reset.py --core /path/to/core_libretro.dylib
python3 tests/test_cold_save.py --core /path/to/core_libretro.dylib
python3 tests/test_teamplayer_boot.py --core /path/to/genesis_plus_gx_libretro.dylib
```

Build with an owner-supplied archive first. Emulator tests use `dist/RollingThunderMD.bin`
and matching `.build/game/out/release/symbol.txt`. The compatibility command still
returns 1 because the cartridge-size gate fails, even when all nine emulator
cases pass. Source CI runs boot/host tests and syntax/audit checks; cores and
ROMs are not bundled or provided to CI.
