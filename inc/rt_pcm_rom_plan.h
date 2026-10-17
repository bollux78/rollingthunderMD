#ifndef RT_PCM_ROM_PLAN_H
#define RT_PCM_ROM_PLAN_H
#include <genesis.h>

#define RT_PCM_ROM_WINDOW_REGION 5
#define RT_PCM_ROM_WINDOW_BASE 0x280000UL
#define RT_PCM_ROM_BANK_BYTES 0x80000UL
#define RT_PCM_ROM_GUARD_BYTES 16

typedef struct {
    u16 mapperBank;
    u16 z80Bank;
    u16 samplePointer;
    u16 initialRemaining;
    u16 fullGroups;
    u16 tailRemaining;
    u32 sampleCount;
} RT_PcmRomPlan;

/* Pure descriptor validation for the experimental buffered ROM driver.
   The clip starts on a 32 KiB boundary; all samples and 16 readable guard
   bytes must fit a single 512 KiB cartridge bank and the supplied ROM size.
   Leaves output unchanged on failure. Does not map a bank, acquire the Z80,
   start playback, or grant a bus-policy exemption. */
bool RT_planPcmRomClip(u32 physicalAddress, u32 sampleCount, u32 romBytes,
                       RT_PcmRomPlan *output);
/* Explicitly reserve one or two consecutive CPU windows beginning at region5.
   Two windows permit a clip/guard to cross one physical512KiB bank boundary.
   The caller must reserve region6 as well: ordinary FAR resource mapping there
   would violate audio ownership. The default planner remains single-window. */
bool RT_planPcmRomClipWindows(u32 physicalAddress,u32 sampleCount,u32 romBytes,
    u16 windows,RT_PcmRomPlan *output);
#endif
