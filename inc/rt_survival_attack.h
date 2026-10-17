#ifndef RT_NATIVE_SURVIVAL_ATTACK_H
#define RT_NATIVE_SURVIVAL_ATTACK_H
#include "rt_survival_scene.h"
/* Configure only at construction, spawn-window movement or retirement.
 * Never scan the pool just to scale timers on every gameplay update. */
static inline void RT_survivalAttackConfigure(RT_CampaignScene *scene,const RT_SurvivalWorld *world)
{
    u8 tier=world->densityTier,coop=scene->players.home.players==2;
    u8 rate=coop?4+3*tier:2*tier;
    scene->attackClockEnabled=tier!=0;
    for(u8 i=0;i<RT_ENEMY_CAPACITY;i++){
        u8 *phase=scene->attackPhases+i;
        if(!tier){if(coop){if(*phase>1)*phase=0;}else *phase=2;}
        else *phase=(rate<<3)|(*phase>=8?*phase&7:0);
    }
}
#endif
