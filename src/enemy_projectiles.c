#include <genesis.h>
extern u32 rtSpriteBirthEpoch;
#include "rt_enemies.h"
u8 RT_enemyProjectileRequest(RT_ProjectilePool *pool,const RT_Enemy *enemy,
    u8 lowPose,RT_WeaponEvent *event,u8 *firstSlot)
{
    if(!pool || !enemy || !event)return 0;
    event->sound=event->command=0;
    u8 slot;
    for(slot=0;slot<RT_PROJECTILE_CAPACITY-1;slot++)
        if(!pool->slots[slot].kind && !pool->slots[slot+1].kind)break;
    if(slot==RT_PROJECTILE_CAPACITY-1){event->sound=0x2b;return 0;}
    u8 left=!!(enemy->state.action&2);
    s32 y=enemy->state.y-(lowPose?0xf0:0x240);
    s16 x=lowPose?-0x80:left?-0x180:0x80;
    s16 flashX=lowPose?(left?-0x180:0x80):(left?-0x280:0x180);
    RT_Projectile *shot=pool->slots+slot,*flash=shot+1;
    rtSpriteBirthEpoch++;
    *shot=(RT_Projectile){enemy->state.x+x,y,left?-44:44,
        RT_PROJECTILE_NEW_SHOT,0,0,rtEnemyShotCyclePoses[left][0],
        RT_PROJECTILE_ENEMY|(left?RT_PROJECTILE_LEFT:0),255,(enemy->state.attributes&0xc0)>>5,0};
    *flash=(RT_Projectile){enemy->state.x+flashX,y,0,RT_PROJECTILE_NEW_FLASH,0,0,
        (u8)(12+left),RT_PROJECTILE_ENEMY|(left?RT_PROJECTILE_LEFT:0),255,flash->layer,0};
    pool->active+=2;event->sound=1;if(firstSlot)*firstSlot=slot;return 1;
}
