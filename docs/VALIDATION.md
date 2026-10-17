# Compact cartridge validation — October 17, 2026

Both images are **16,252,928 bytes (15.5 MiB)**.

| Profile | SHA256 |
| --- | --- |
| Standard emulator profile | `283f35e272fe3da63d6b16ec46f8b04ed1db46842a1eac85b978c1e2ce449274` |
| Mega EverDrive Pro profile | `0e1a62c8b159d5449eb8edb977156a54144d6a1bd49d40bb7f32e9dac2bfc2b2` |

**No physical hardware was tested.** The Pro results below use a software model
of its documented mapper in GX. Core/X3/X5 cannot fit this image; X7 and legacy
models are not supported by these profiles. See [the hardware matrix](HARDWARE_COMPATIBILITY.md).

## Verified

- All ten original stage banks and all four finite PCM clips reproduce their
  frozen hashes from source and the owner-supplied arcade ROM.
- The compact build independently compares **33,720 linked poses**, **29,790
  packed geometry records**, **808,960 background map cells**, background
  pixels/palettes and world collision data with the frozen banks. Completion
  relinking changes only pointers. Enemy counts and animation policy are unchanged.
- All four generated audio placements pass the actual runtime PCM planners.
  This catches both 32 KiB alignment and the victory clip's bank-boundary span.
- US NTSC, European PAL and Japanese NTSC opening 2P, pause/resume and
  reset/restart pass in PicoDrive and GX. Three additional GX dirty-RAM/I/O
  launches pass. Initialized C data matches its ROM image before SDK mutation.
- **60 real standard-profile soft resets** pass across Arcade, Campaign 1P/2P
  and Survival 1P/2P on the two cores. Fresh-process settings, score and initials
  restoration and corrupt-slot fallback also pass on both.
- GX Team Player 3/6-button startup and three resets per mode pass with held
  input. No physical adapter was tested.
- **60 Pro-profile resets in the software mapper model** pass across the same
  five modes, including simulated reset with the cartridge write gate left open
  and the save bank mapped. No unsafe ROM writes are observed. Fresh-process
  banked-RAM restoration and corrupt-slot fallback pass.
- Diagnostic calls into the unmodified binaries load **all ten stages** and
  enter each high-score screen. All 20 captured screens match the previous
  accepted 32 MiB build pixel for pixel, for both the standard profile and the
  Pro software model. These are loader/renderer checks, not full playthroughs.
- The long high-score PCM completes, crosses its physical banks, and restores
  the previous fixed ROM window in both profiles' software tests.
- Host tests cover all 65 interrupted save-write positions, invalid/read-only
  storage, warm restoration without SRAM reads and corrupt-mirror fallback.
  Startup tests check TMSS ordering and reject an unrecognized SDK startup.

`validation.json` records exact ROM/core identities, counts and image hashes;
`compatibility.json` includes startup and mapper traces. Score tests seed scores
and initials in work RAM; settings and pause use controller input. Stage-loader
checks use GX's CPU-register API to invoke existing functions and bypass normal
progression. They do not modify the ROM or establish full-game completion.

Both profiles rebuilt to identical checksums. A fresh source-only directory,
with no existing build outputs or research checkout, reproduced the standard
ROM exactly using the owner-supplied arcade archive and documented toolchain.

## Performance sample

One deterministic NTSC opening Campaign 2P workload, 1,800 refreshes:

| Build | Displayed presentations/second | Simulation updates/second |
| --- | ---: | ---: |
| Previous accepted 32 MiB build | 58.13 | 58.13 |
| Compact build | 57.83 | 57.83 |

This is roughly a **0.3 FPS (0.5%) cost** in this sample, not a claim of faster
rendering. The input sequence follows simulation updates; movement, combat and
normal game-flow waits are included. Stalled-refresh counters therefore must
not be interpreted solely as CPU overload. The adaptive art policy is unchanged.
This small sample does not replace the older broad performance study or certify
worst-case crowded scenes.

## Reproduce

Build with the documented toolchain and your own ROM archive. The default image
uses standard SSF2/SRAM. Add `--cart mega-everdrive-pro` for the hardware-targeted
profile. Preserve the matching `.build/game/out/release/symbol.txt` after each
profile build; compiling the other profile replaces those runtime symbols.

```sh
python tools/build.py --rom /path/to/rthunder.zip --sgdk "$GDK"
python tests/check_compatibility.py --picodrive "$PICODRIVE" --gx "$GX"
python tests/test_soft_reset.py --core "$GX"
python tests/test_cold_save.py --core "$GX"
python tests/test_teamplayer_boot.py --core "$GX"
python tests/test_banked_scenes.py --core "$GX" \
  --rom dist/RollingThunderMD.bin --symbols .build/game/out/release/symbol.txt --audio-soak

python tools/build.py --rom /path/to/rthunder.zip --sgdk "$GDK" --cart mega-everdrive-pro
python tests/test_everdrive_model.py --core "$GX" \
  --rom dist/RollingThunderMD-MegaEverDrivePro.bin \
  --symbols .build/game/out/release/symbol.txt --interrupt-save-on-reset
python tests/test_cold_save.py --everdrive-model --core "$GX" \
  --rom dist/RollingThunderMD-MegaEverDrivePro.bin \
  --symbols .build/game/out/release/symbol.txt
```

Software tests cannot establish cart-menu backup behavior, actual power-cycle
retention, analog output, physical VDP/Z80 bus timing, or compatibility with
unconnected consoles and cartridge firmware.
