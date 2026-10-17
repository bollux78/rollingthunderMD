#include "rt_enemies.h"
const RT_EnemyEntry rtEnemySlopeEntries[12]={
 {6,1,0x62ab},{6,1,0x62be},{6,1,0x62d1},{6,1,0x62e4},{6,1,0x62f7},{6,1,0x630a},
 {6,1,0x6239},{6,3,0x624c},{6,0,0x625f},{6,1,0x6272},{6,3,0x6285},{6,0,0x6298}
};
RT_EnemyResult RT_enemySlopeStep(RT_Enemy *enemy,const RT_WorldCamera *camera,
    RT_EnemyProbe probe,void *context,u8 *selector)
{
    if(!enemy || !camera || !probe || !selector)return RT_ENEMY_UNAVAILABLE;
    RT_DoorEnemy *a=&enemy->state;u8 state=a->action&0xfc,row=a->row;
    if((state!=0x4c && state!=0x50) || row>5)return RT_ENEMY_UNAVAILABLE;
    *selector=0;if(--a->timer)return RT_ENEMY_CONTINUE;
    u8 descending=state==0x50,left=enemy->velocityX<0,next=row==5?0:row+1;
    if(descending?(row==0 || row==3):(row==2 || row==5)){
        s32 x=a->x+(left?-0xc0:0xc0),originX=(s32)camera->x*16;
        if((left && x<originX-0x200) || (!left && x>=originX+(descending?0x1200:0x1400)))goto removed;
        a->x=x;
        s32 y=a->y+(descending?0x80:-0x80),originY=((s32)camera->y+256)*16;
        if(descending?y>originY+0x300:y<=originY-0x1000)goto removed;
        a->y=y;
        if(!descending && row==5)a->row=255;
        u8 code,flags;
        if(!probe(descending?(left?1:-1):(left?-1:1),descending?1:0,&code,&flags,context))return RT_ENEMY_UNAVAILABLE;
        if(!(flags&0x20)){*selector=descending?(left?6:5):4;return RT_ENEMY_SELECT;}
    }
    u8 index=descending*6+next;const RT_EnemyEntry *entry=rtEnemySlopeEntries+index;
    a->row=next;a->timer=entry->timer;a->flags=entry->flags;a->pose=97+index;
    return RT_ENEMY_CONTINUE;
removed:
    a->kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;
}
