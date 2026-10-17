# SRAM and Start-pause build validation

Cartridge: `RollingThunderMD.bin`, 33,554,432 bytes (32 MiB).

SHA256: `c7efdd789711c4bc88a1cabc781070cac25586594221b53dafad38c3afeb02ef`

- Owner-ROM source build succeeded; all 10 stage banks reproduced. The complete
  31 MiB banked region is byte-identical to the previous public build.
- Host save tests cover roundtrips, unchanged-save suppression, empty/corrupt
  data, unavailable SRAM, and all 65 interrupted-write boundaries.
- PicoDrive settings changed through the menu survive a fresh core process:
  seven starting credits and PSG muted.
- Best score and all initials survive cold boot through real SRAM, with no
  savestate. This test seeded score/table RAM, then used the production pause
  save path; it does not claim a naturally completed high-score run.
- Corrupting the newest slot recovers the preceding valid save.
- Campaign and Survival, 1P/2P: Start pauses/resumes twice, including a held
  Start and controller-two pause/resume in co-op. Simulation memory, update
  counts and framebuffer digests stay constant throughout each pause. Maximum
  update lag is two refreshes, with no catch-up burst or runtime faults.
- Arcade pause/resume passes; player two cannot pause a solo game.
- Eight controller-only opening smoke runs, 4,200 refreshes each, cover both
  agents and both Home games in 1P/2P. No runtime faults or heap-end corruption.
  Solo stays in full detail; every co-op run exercises both detail modes.
- Source audit excludes ROMs, compiled outputs, boot intro assets and local paths.

## SRAM integration

Two 64-byte slots contain a version, sequence, settings, best score, five
seven-byte records, CRC32 and a magic marker committed last. Byte 128 is a
startup write probe. Scratch/entry controls are never persisted.

The runtime checks reads at startup and writes after draining ROM DMA, with
interrupts and Z80 ROM consumers excluded while SRAM is mapped. Runtime
read-back is deliberately avoided: [PicoDrive's SSF mapper](https://github.com/notaz/picodrive/blob/master/pico/carthw/carthw.c)
replaces bank-window read mappings with ROM, while SRAM writes remain available.
The next cold boot validates slot checksums. A separate PicoDrive soft-reset
probe did not restore SRAM after bank remapping; reload content to restore saves.
Unsupported SRAM leaves gameplay
functional with defaults; saves require an emulator/cartridge with SRAM support.

Run the portable save tests with:

```sh
cc -Wall -Wextra -Werror -Itests/host -Iinc src/save.c tests/test_save.c -o /tmp/test-save
/tmp/test-save
```

The earlier public-source build was independently reproduced from a clean clone.
This revision was rebuilt through the same owner-ROM pipeline and tested as
above. Full natural-game completion and physical hardware remain unverified.
These checks make no new performance claim. See `validation.json` for details.
