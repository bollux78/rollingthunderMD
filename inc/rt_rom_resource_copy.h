#ifndef RT_ROM_RESOURCE_COPY_H
#define RT_ROM_RESOURCE_COPY_H
#include <genesis.h>
/* Copy 1..256 physical ROM bytes into caller-owned RAM. Regions0..4 must
   retain their ordinary identity mapping. Higher addresses use region7 only,
   including spans crossing a512KiB boundary. Its previous mapping is restored
   before return; regions5/6 are never changed. CPU interrupts are masked around
   this bounded copy so another resource reader cannot remap7 mid-copy.
   No VDP/Z80 access. This does not permit other FAR readers to overwrite an
   audio-owned region6: an exclusive scene must use this path consistently.
   Invalid bounds leave the output and mapper untouched. */
bool RT_copyRomResource(void *output,u32 physicalAddress,u16 length,u32 romBytes);
#endif
