#include "rt_combat.h"
u8 RT_playerBulletHitEnemies(const RT_Projectile *shot,void *context)
{
    RT_EnemyTargets *targets=context;
    if(!shot || (shot->flags&RT_PROJECTILE_ENEMY) || shot->kind!=RT_PROJECTILE_SHOT ||
        !targets || !targets->enemies)return 0;
    u8 remaining=targets->visibleActors;
    for(u16 i=0;i<RT_ENEMY_CAPACITY && remaining;i++){
        RT_Enemy *e=targets->enemies->slots+i;RT_DoorEnemy *a=&e->state;
        if(a->kind==RT_ENEMY_FREE || a->hidden)continue;
        remaining--;
        if(a->attributes!=(u8)(shot->layer<<5) || (a->flags&0x80))continue;
        const RT_ContactBox *box=rtContactBoxes+a->flags,*bullet=&rtBulletContactBox;
        u16 y=(u16)(shot->y-a->y-box->yOffset+bullet->yOffset);
        if((s16)y<0){if((u16)(0-y)>box->height)continue;}
        else if(y>bullet->height)continue;
        u16 x=(u16)(a->x+box->xOffset[!!(a->action&2)]-shot->x-bullet->xOffset[shot->velocityX<0]);
        if((s16)x<0){if((u16)(0-x)>box->width)continue;}
        else if(x>bullet->width)continue;
        a->spawnAction=shot->velocityX<0?0x62:0x61;
        targets->enemies->scoreOwner[i]=shot->owner<2?shot->owner:0;
        if(e->byte6)e->byte6--;
        return 1;
    }
    return 0;
}
