#ifndef RT_PCM_SCENE_FRAME_H
#define RT_PCM_SCENE_FRAME_H
#include <genesis.h>
/* Exclusive NTSC/224-line scene contract used by the finite ROM PCM driver.
   Begin waits for the next VBlank and scanline225, then allows SGDK I/O
   without taking the Z80 main bus. The PCM driver must be the sole sound
   owner and executing its verified local-RAM window during this interval.
   Queue preparation belongs before begin; bounded scene setup/VDP work,
   SYS_doVBlankProcessEx(IMMEDIATELY), and publication checks belong inside.
   Always end before returning to general scene work. This is not a general
   gameplay frame loop, a PAL schedule, or a time-budget enforcement API. */
void RT_beginExclusivePcmSceneFrame(void);
void RT_endExclusivePcmSceneFrame(void);
/* Same exclusive contract, but consumes each hardware refresh at most once.
   Zero-initialize caller-owned lastRefresh. If preparation has just crossed
   VInt, use that still-active blank instead of forcing an extra refresh.
   Interrupts must be enabled. Late entry reduces the available I/O budget;
   caller must verify its complete workload against the PCM bus windows. */
bool RT_beginExclusivePcmSceneFrameOnce(u32 *lastRefresh);
/* Deadline-aware variant for bounded transfers. latestLine225..240 is the
   cutoff for accepting the current blank, not a hard return-line bound. Late work
   waits for a real refresh even with the display disabled. Caller validates
   transfer size/cadence including the next VInt handler cost, and always pairs
   successful entry with End. */
bool RT_beginExclusivePcmSceneFrameUntil(u32 *lastRefresh,u16 latestLine);
#endif
