#include "rt_music_clock.h"

u8 RT_musicServicesForNtscRefresh(u32 *remainder)
{
    u32 total;
    if(!remainder || *remainder>=7874999)return 0;
    /* (2000/33) / (53693175/896040), reduced exactly. The baseline
       service is always due; carry admits one additional source service. */
    total=*remainder+89801;
    if(total>=7874999){*remainder=total-7874999;return 2;}
    *remainder=total;return 1;
}

void RT_setMusicClockBase(RT_MusicClock *clock, u8 base)
{
    if (!clock) return;
    clock->base = base;
    clock->fraction = 0;
}

void RT_setMusicClockMultiplier(RT_MusicClock *clock, u8 multiplier)
{
    if (clock) clock->multiplier = multiplier;
}

bool RT_advanceMusicClock(RT_MusicClock *clock)
{
    u32 total;
    if (!clock) return FALSE;
    total = (u32)clock->fraction + (u16)clock->base * (u16)clock->multiplier;
    clock->fraction = (u16)total;
    return total > 0xFFFF;
}
