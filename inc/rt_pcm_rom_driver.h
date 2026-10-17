#ifndef RT_PCM_ROM_DRIVER_H
#define RT_PCM_ROM_DRIVER_H
#include "rt_pcm_rom_plan.h"

typedef enum {
    RT_PCM_ROM_INACTIVE = 0,
    RT_PCM_ROM_STARTING = 1,
    RT_PCM_ROM_PLAYING = 2,
    RT_PCM_ROM_COMPLETE = 3,
    RT_PCM_ROM_OWNERSHIP_LOST = 4
} RT_PcmRomStatus;

/* Experimental, exclusive single-lane backend. Replaces the SGDK sound driver
   and owns mapper region 5 until stop. No other sound driver or region-5 writer
   may run while active. Caller must not already own the Z80 bus at start/stop.
   Invalid descriptors and an already-active backend fail without taking over.
   v4 finite ring cadence is 227 Z80 cycles per sample. Timed frame service
   is verified only in isolated NTSC probes; production scheduling remains unresolved. NOT enabled in live gameplay. */
bool RT_startPcmRomDriver(u32 physicalAddress, u32 sampleCount, u32 romBytes);
/* Opt-in long-clip mode. windows=1 is identical to start above; windows=2
   owns mapper regions5 AND6 until stop. Both prior mappings are restored.
   Caller must keep code/data/resource mappers out of these windows throughout
   playback. The ordinary game's current resource layout does NOT reserve6;
   do not enable this mode there without changing that ownership/layout. */
bool RT_startPcmRomDriverWindows(u32 physicalAddress,u32 sampleCount,
    u32 romBytes,u16 windows);
/* Start another clip after COMPLETE without reloading/resetting the driver.
   May select a new physical bank; retains the original mapping for final stop.
   Rejects incomplete playback, invalid descriptors and ownership mismatches.
   Not a gapless chaining API: setup/publication time separates the clips. */
bool RT_rearmPcmRomDriver(u32 physicalAddress, u32 sampleCount, u32 romBytes);
/* Interrupt/restart a clip using the SAME owned resident driver and window
   count. No code/chip reload and no synthetic COMPLETE for the old clip.
   Requires an intentional source replacement boundary; holds the Z80 bus and
   resets its CPU, so old sample continuity ends here. Descriptor, generation,
   mapping and signature failures leave the old device untouched. Original
   mapper values remain saved for final stop. Does not mix or switch windows. */
bool RT_replacePcmRomClipWindows(u32 physicalAddress,u32 sampleCount,
    u32 romBytes,u16 windows);
/* Active polling reads a volatile 68000 RAM mailbox without taking the Z80
   bus. Driver-load generation and mapper ownership are checked each time.
   COMPLETE is published after final silence; only then does polling acquire
   the bus to verify the signature and completed state. Not a score event. */
RT_PcmRomStatus RT_getPcmRomStatus(void);
/* Idempotent when inactive. Stops playback, loads SGDK's null driver, and
   restores the previous region-5 mapping. Does not restore the previous sound.
   The null-driver reset masks interrupts for multiple refreshes. Stop any
   scanline raster owner and blank its display before this blocking teardown,
   or retain the completed driver for rearm at the next scene boundary.
   Ownership mismatch fails without unloading another driver or remapping. */
bool RT_stopPcmRomDriver(void);
#endif
