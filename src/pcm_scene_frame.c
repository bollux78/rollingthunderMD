#include "rt_pcm_scene_frame.h"
#include "rt_z80_bus_policy.h"

void RT_beginExclusivePcmSceneFrame(void)
{
    VDP_waitVBlank(TRUE);
    while(GET_VCOUNTER<225);
    RT_setZ80LocalBusOnly(TRUE);
}

void RT_endExclusivePcmSceneFrame(void)
{
    RT_setZ80LocalBusOnly(FALSE);
}

bool RT_beginExclusivePcmSceneFrameOnce(u32 *lastRefresh)
{
    if(!lastRefresh)return FALSE;
    /* A preparation that crosses VInt can still use that refresh's blank.
       Consume each refresh once, including while display is disabled. */
    while(vtimer==*lastRefresh);
    VDP_waitVBlank(FALSE);
    while(GET_VCOUNTER<225);
    *lastRefresh=vtimer;
    RT_setZ80LocalBusOnly(TRUE);
    return TRUE;
}

bool RT_beginExclusivePcmSceneFrameUntil(u32 *lastRefresh,u16 latestLine)
{
    if(!lastRefresh || latestLine<225 || latestLine>240)return FALSE;
    while(vtimer==*lastRefresh);
    if(GET_VCOUNTER>latestLine){
        if(VDP_isEnable())VDP_waitVBlank(TRUE);
        else {
            /* Disabled video forces the blank bit: use a real IRQ instead. */
            u32 missed=vtimer;while(vtimer==missed);
        }
    }
    /* A next-refresh entry includes the installed VInt handler's cost. Do not
     * repeatedly reject it: that would starve transfers behind palette IRQs. */
    while(GET_VCOUNTER<225);
    *lastRefresh=vtimer;
    RT_setZ80LocalBusOnly(TRUE);
    return TRUE;
}
