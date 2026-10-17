# Public build validation

Final cartridge: `RollingThunderMD.bin`, 33,554,432 bytes (32 MiB).

SHA256: `ecfd1197b63637af134888e833e53a90a1c023f99fa356128d1768c5a5ee279d`

- Built successfully from an independent clean local Git clone with only the
  supplied arcade archive and documented toolchain/dependencies. No copied
  `.build` directory, previous cartridge, research assets or network input.
- The working checkout and clean clone produced identical cartridge bytes.
- All 10 stage banks and the complete 31 MiB banked region match the frozen
  adaptive build exactly, including completion/high-score art and audio.
- Four regenerated audio C files match the frozen source byte for byte.
- Eight unmodified PicoDrive runs, 4,200 refreshes each: Campaign and Survival,
  both characters, 1P and 2P. Correct session selection, no runtime faults.
- 1P stays out of compact mode; all four 2P smoke runs exercised both modes.
- The boot capture shows the game menu directly. The intro function, chant and
  Namcot image symbols are absent from the final link; nontrivial old asset
  byte samples are absent from the cartridge.
- Source-package audit: no tracked ROM/archive files, compiled outputs,
  generated audio clips, forbidden intro files or machine-specific source paths.
- Missing owner-ROM input fails before building.

Only boot intro removal and the cartridge title differ in the native runtime
source. Existing graphics/layout tables remain in the source package. Full
natural-game completion and physical hardware compatibility remain unverified.
See `validation.json` for smoke-test details. These are development-build checks,
not a new performance claim or hardware certification.
