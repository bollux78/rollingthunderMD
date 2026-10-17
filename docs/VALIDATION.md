# Soft-reset save restoration validation

Cartridge: `RollingThunderMD.bin`, 33,554,432 bytes (32 MiB).

SHA256: `133fea599cb00cb5de33d9fae86b9c15b9e18e5760b4497a2f906101ca74b6b8`

- Owner-ROM source build succeeds; all 10 stage banks reproduce. The complete
  31 MiB banked art/audio region matches the preceding SRAM/pause build exactly.
- 30 real PicoDrive soft resets pass: six each in Arcade, Campaign 1P/2P and
  Survival 1P/2P. No ROM reloads or savestates are used between those resets.
- Reset from the playing sound test, title menu, paused gameplay and active
  gameplay restores best score, all initials, credits and PSG mute. Repeated
  resets and new saves/settings changes after reset pass as well.
- Reset returns to an unpaused menu; the next game updates and plays music.
  Uncommitted live score changes are discarded in favor of the last save.
  Restoring a save does not rewrite SRAM. No runtime faults occurred.
- Host tests cover both CRC-protected slots, all 65 interrupted SRAM-write
  boundaries, invalid/empty/unwritable storage, and warm restoration with SRAM
  reads explicitly forbidden. A corrupt mirror slot falls back to its partner.
- Fresh-core cold boots still restore controller-changed credits/PSG settings,
  best score and initials. Corrupting the newest SRAM slot recovers the old one.
- Campaign/Survival pause tests still pass in 1P/2P, including controller-two
  pause in co-op. Gameplay memory and screen digests remain frozen during pause;
  maximum update lag after resume is two refreshes, with no catch-up burst.

Scores/initials were seeded in work RAM to reach save boundaries without a full
campaign. Settings were changed by controller. ROM code was not patched. These
checks are not a full natural playthrough, hardware certification or a new FPS
claim. See `validation.json` for the exact ROM/core hashes and per-mode results.

## Reset implementation

Cold boot reads the existing two 64-byte, versioned, CRC32-protected SRAM slots.
Successful saves also publish a two-slot RAM mirror, committing each marker last.
The save format is unchanged, so existing SRAM files continue to work.

The linker wraps SGDK's warm-reset entry. With interrupts masked and the Z80
halted, the wrapper retains only the committed save mirror on its fresh stack,
waits for DMA, restores fixed ROM window 1, and reinitializes C data/BSS. SGDK
then rebuilds heap, DMA, display, input and audio state. The loader validates the
mirrored slots and restores the latest valid one. Old pause flags, callbacks,
audio owners and in-flight rendering state cannot leak into the new session.

This avoids runtime reads through [PicoDrive's SSF mapper](https://github.com/notaz/picodrive/blob/master/pico/carthw/carthw.c),
which replaces bank-window SRAM read handlers with direct ROM mappings. Cold
boot continues to read real SRAM; later saves continue to write real SRAM.
Reset restores the last committed save rather than saving the interrupted game.

## Reproduce

```sh
cc -Wall -Wextra -Werror -Itests/host -Iinc src/save.c tests/test_save.c -o /tmp/test-save
/tmp/test-save
python3 tests/test_soft_reset.py --core /path/to/picodrive_libretro.dylib
```

The emulator test uses `dist/RollingThunderMD.bin` and matching symbols in
`.build/game/out/release/symbol.txt`; build first with your own arcade archive.
It writes `.build/soft-reset-tests.json`. An installed PicoDrive core is required
and is not bundled. Source CI runs host tests and Python syntax checks only.
