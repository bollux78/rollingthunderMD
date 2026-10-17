#ifndef RT_NATIVE_FRAME_PACER_H
#define RT_NATIVE_FRAME_PACER_H
#include <genesis.h>
/* One logical update per physical refresh. Debt survives a slow draw; the
 * renderer may yield at most three consecutive updates to catching it up.
 * This restores game time, not fresh-frame throughput. Reset after loading
 * or READY so time spent outside gameplay never becomes simulation debt. */
typedef struct {u32 serviced;u8 skipped;} RT_FramePacer;
static inline void RT_framePacerReset(RT_FramePacer *p,u32 refresh)
{p->serviced=refresh-1;p->skipped=0;}
static inline u8 RT_framePacerStep(RT_FramePacer *p,u32 refresh)
{
    if(refresh==p->serviced)return 0;
    p->serviced++;return 1;
}
static inline u8 RT_framePacerDrawWithLag(RT_FramePacer *p,u32 refresh,u8 toleratedLag)
{
    if(refresh-p->serviced>toleratedLag && p->skipped<3){p->skipped++;return 0;}
    p->skipped=0;return 1;
}
/* Synchronous clients retain the original zero-debt draw policy. */
static inline u8 RT_framePacerDraw(RT_FramePacer *p,u32 refresh)
{return RT_framePacerDrawWithLag(p,refresh,0);}
#endif
