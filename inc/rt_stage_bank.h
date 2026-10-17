#ifndef RT_NATIVE_STAGE_BANK_H
#define RT_NATIVE_STAGE_BANK_H
#include "rt_world.h"
#define RT_STAGE_BANK_BASE 0x100000UL
#define RT_STAGE_BANK_BYTES 0x300000UL
/* Six consecutive512KiB pages replace only CPU windows2..7. The builder must
 * prove code, interrupts, stack initializer and persistent data stay below1MiB,
 * and the complete referenced bank is physically present in the cartridge.
 * Header ROM-end must record the physical page-aligned image end, not the
 * CPU-visible4MiB window. Out-of-image banks are rejected before mapping.
 * Stage pointers/caches become invalid across this call. Before switching,
 * terminate stage callbacks and release their references, including audio.
 * Current native gameplay does NOT yet use this experimental layout. */
#ifdef __m68k__
u8 RT_stageBankSelect(u8 firstPage);
#endif
#endif
