#include "rt_enemies.h"
const RT_EnemyEntry rtEnemyPauseEntries[17]={
 {8,0x25,0x6b98},{10,0x25,0x6bab},{10,0x25,0x6b72},
 {10,0x25,0x6bbe},{8,0x25,0x6b72},{8,0x25,0x6b85},
 {8,0x25,0x6b72},{8,0x25,0x6bab},{8,0x25,0x6b98},
 {8,0,0x694a},{10,0x1a,0x6687},{10,0x1a,0x669a},{10,0x1a,0x66b3},
 {8,0x1a,0x66cc},{8,0x1a,0x66e5},{8,0x1a,0x6937},{8,0,0x694a}
};
RT_EnemyResult RT_enemyPauseStep(RT_Enemy *enemy,s32 playerX,u8 projectileCount,
    RT_EnemyProbe probe,void *context,u8 *selector)
{
    if(!enemy || !selector)return RT_ENEMY_UNAVAILABLE;
    *selector=0;
    RT_DoorEnemy *actor=&enemy->state;u8 state=actor->action&0xfc;
    if(state!=8 && state!=0x3c)return RT_ENEMY_UNAVAILABLE;
    u8 last=state==8?8:7,row=actor->row;
    if(row>last)return RT_ENEMY_UNAVAILABLE;
    if(--actor->timer)return RT_ENEMY_CONTINUE;
    if(row==last){
        if(actor->decisions&1)enemy->byte12&=0xfb;
        else if(!(enemy->byte12&0x20)){*selector=0x38;return RT_ENEMY_SELECT;}
        *selector=enemy->byte12&1?0x48:4;return RT_ENEMY_SELECT;
    }
    if(row==5 && !(actor->decisions&0x21) && enemy->delay){enemy->delay--;row=3;}
    else if(row==4){
        s32 distance=actor->x-playerX;
        u8 shouldProbe=!!(enemy->byte12&1);
        if(!shouldProbe && !(actor->decisions&(state==8?0x21:0x20))){
            shouldProbe=(enemy->variation&0x40) ||
                (enemy->variation&3)==(distance< -0x200?1:distance>=0x400?3:2);
            if(!shouldProbe && state==0x3c && (enemy->traits&0x40) && projectileCount<16){
                *selector=0x84;return RT_ENEMY_SELECT;
            }
        }
        if(shouldProbe){
            u8 code,flags;
            if(!probe || !probe(0,state==8?1:-3,&code,&flags,context))return RT_ENEMY_UNAVAILABLE;
            if(state==8?(code!=0x23 && code!=0x2f):!!(flags&0x10)){
                *selector=state==8?0x28:0x2c;return RT_ENEMY_SELECT;
            }
            enemy->byte12|=0x20;
        }
    }
    actor->row=row+1;
    u8 entry=(state==8?0:9)+actor->row;
    actor->timer=rtEnemyPauseEntries[entry].timer;actor->flags=rtEnemyPauseEntries[entry].flags;
    actor->pose=60+entry;
    return RT_ENEMY_CONTINUE;
}
