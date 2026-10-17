#ifndef RT_NATIVE_ENEMY_ATTACK_CLOCK_H
#define RT_NATIVE_ENEMY_ATTACK_CLOCK_H
#include <genesis.h>
/* Byte encoding:0/1 retains Campaign co-op's alternating extra decrement;
 * 2 disables extras for solo Survival. Values>=8 store additional eighths
 * per decision tick in bits3..7 and fractional carry in bits0..2. */
static inline void RT_enemyAttackClock(u8 *delay,u8 *phase)
{
    if(!phase)return;
    u8 value=*phase;
    if(value<2){*phase=value^1;if(!*phase && *delay)--*delay;}
    else if(value>=8){
        u8 sum=(value&7)+(value>>3),extra=sum>>3;
        *phase=(value&0xf8)|(sum&7);
        *delay=extra>=*delay?0:*delay-extra;
    }
}
#endif
