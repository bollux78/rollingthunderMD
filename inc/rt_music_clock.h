#ifndef RT_MUSIC_CLOCK_H
#define RT_MUSIC_CLOCK_H
#include <genesis.h>

typedef struct
{
    u16 fraction;
    u8 base;
    u8 multiplier;
} RT_MusicClock;

/* Native arithmetic from the source music service, not a CPU interpreter.
   Source data command 02 resets the accumulator; 03 preserves it. */
void RT_setMusicClockBase(RT_MusicClock *clock, u8 base);
void RT_setMusicClockMultiplier(RT_MusicClock *clock, u8 multiplier);
/* One source service tick. TRUE permits the next group of music events. */
bool RT_advanceMusicClock(RT_MusicClock *clock);
/* Hardware-rate adaptation, separate from source tempo arithmetic above.
   Initialize remainder to zero and call once per actual NTSC refresh
   (53693175 / 896040 Hz). Returns 1 or 2 source services at 2000/33 Hz.
   Events are quantized to refresh boundaries; no sub-frame timing is claimed.
   Null or invalid remainder returns 0 without mutation. Not a PAL scheduler. */
u8 RT_musicServicesForNtscRefresh(u32 *remainder);
#endif
