#include <genesis.h>
extern u32 rtSpriteBirthEpoch;
#include "rt_enemy_throw.h"
const RT_EnemyEntry rtEnemyThrowEntries[20]={
{6,0x1a,0x695d},
{6,0x1a,0x6976},
{6,0x1a,0x698f},
{6,0x0e,0x69a8},
{6,0x10,0x69c1},
{6,0x10,0x69da},
{6,0x10,0x69f3},
{6,0x14,0x6a0c},
{6,0x12,0x6a25},
{6,0x04,0x6a3e},
{6,0x00,0x6a57},
{6,0x00,0x6a6a},
{6,0x10,0x6a7d},
{6,0x10,0x6a96},
{6,0x10,0x6aa9},
{6,0x10,0x6ac2},
{6,0x10,0x6adb},
{6,0x00,0x6af4},
{6,0x01,0x6b0d},
{6,0x01,0x6b26},
};
RT_EnemyResult RT_enemyThrowStep(RT_Enemy *e,RT_ProjectilePool *pool,
    RT_ThrowMotion motion[RT_PROJECTILE_CAPACITY],u8 visibleProjectiles,
    RT_WeaponEvent *event,u8 *selector)
{
    static const s16 dx[3][2]={{-368,128},{-208,-32},{-496,256}};
    static const s16 dy[3]={816,848,800};
    static const s16 fx[5][2]={{16,-256},{-560,320},{-544,304},{-448,304},{-256,16}};
    static const s16 fy[5][2]={{352,352},{544,544},{720,688},{880,880},{880,880}};
    if(!e || !pool || !motion || !event || !selector ||
       ((e->state.action&0xfc)!=0x84 && (e->state.action&0xfc)!=0x88) ||
       e->state.row>9)return RT_ENEMY_UNAVAILABLE;
    event->sound=event->command=0;*selector=0;
    u8 row=e->state.row,down=(e->state.action&0xfc)==0x84,left=!!(e->state.action&2);
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    if(row==9){
        if(down)*selector=e->state.decisions&1?4:e->state.decisions&0x20?0x38:
            e->byte12&4?0x3c:(e->traits&0x80) && visibleProjectiles<16?0x84:4;
        else *selector=(e->state.decisions&1) && (e->traits&0x80)?0x88:4;
        return RT_ENEMY_SELECT;
    }
    if(down?(row<3 || row==7 || row==8):(row==0 || row==1 || row==8))event->sound=5;
    else{
        u8 slot=e->parent;
        if(row==(down?3:2)){
            for(slot=0;slot<RT_PROJECTILE_CAPACITY-1;slot++)if(!pool->slots[slot].kind)break;
            if(slot==RT_PROJECTILE_CAPACITY-1){e->state.timer=1;return RT_ENEMY_CONTINUE;}
            RT_Projectile *o=pool->slots+slot;
    rtSpriteBirthEpoch++;
            o->kind=down?RT_PROJECTILE_NEW_THROW_DOWN:RT_PROJECTILE_NEW_THROW_FORWARD;
            o->phase=o->timer=0;o->velocityX=0;o->owner=255;
            o->flags=RT_PROJECTILE_ENEMY|(left?RT_PROJECTILE_LEFT:0);
            o->layer=(e->state.attributes&0xc0)>>5;
            motion[slot]=(RT_ThrowMotion){0,e->state.attributes,e->state.action&3};
            e->parent=slot;pool->active++;
        }else if(slot>=RT_PROJECTILE_CAPACITY-1)return RT_ENEMY_UNAVAILABLE;
        RT_Projectile *o=pool->slots+slot;RT_ThrowMotion *m=motion+slot;
        if(down && row<=5){
            o->pose=20+(row-3)*2+left;o->x=e->state.x+dx[row-3][left];o->y=e->state.y-dy[row-3];
        }else if(!down && row<=6){
            o->pose=26+(row-2)*2+left;o->x=e->state.x+fx[row-2][left];o->y=e->state.y-fy[row-2][left];
        }else{
            o->kind=RT_PROJECTILE_THROWN;m->velocityY=down?48:-48;
            if(down)m->attributes+=0x40;
            o->layer=(m->attributes&0xc0)>>5;
            o->velocityX=down?(left?-8:8):(left?-40:40);o->pose=36+left;
        }
    }
    u8 index=(down?0:10)+row+1;const RT_EnemyEntry *entry=rtEnemyThrowEntries+index;
    e->state.row=row+1;e->state.timer=entry->timer;e->state.flags=entry->flags;e->state.pose=315+index;
    return RT_ENEMY_CONTINUE;
}
