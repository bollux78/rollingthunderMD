# Public build validation

The runtime is frozen at the adaptive 2P development snapshot. Only the boot
intro call/assets and cartridge title change in the public build. The standalone
build uses no earlier cartridge image or research directory.

The initial standalone build reproduced all ten stage banks, the complete
banked portion of the 32 MiB cartridge, and all four generated voice/music C
files byte for byte against the accepted local snapshot. The finite audio clips
are regenerated from the owner-supplied arcade music data and verified by SHA256.

A clean-checkout reproduction and intro-free emulator smoke tests are recorded
below once complete. These checks do not certify a full natural playthrough or
physical hardware support.
