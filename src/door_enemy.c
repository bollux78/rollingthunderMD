#include "rt_door_enemy.h"
/* CPU2 DCE4 rows: graphics are assigned offline to the same pose order. */
static const u8 ticks[6]={12,6,6,6,5,5};
static const u8 flags[6]={0xf6,0xf6,0xf6,0,0x24,0x25};
static void load(RT_DoorEnemy *enemy,u8 row)
{
    enemy->row=enemy->pose=row;enemy->timer=ticks[row];enemy->flags=flags[row];
}
u8 RT_doorEnemyEnterStep(RT_DoorEnemy *enemy,RT_Anchor *door)
{
    static const u8 entryFlags[5]={0x23,0,0xf6,0xf6,0xf6};
    if(!enemy || (enemy->kind!=4 && enemy->kind!=8 && enemy->kind!=12) ||
        (enemy->action&0xfc)!=0x58 || enemy->row>6)return 0;
    u8 row=enemy->row;
    if(row==6)return 1;
    if(!door)return 0;
    door->flags|=1;
    if(--enemy->timer)return 1;
    if(row>=2 && !(door->flags&4))enemy->y-=0x20;
    enemy->row=row+1;
    if(row==5){enemy->attributes=0;return 1;}
    enemy->timer=5;enemy->flags=entryFlags[row];enemy->pose=6+row;
    return 1;
}
RT_DoorEnemyResult RT_doorEnemyStep(RT_DoorEnemy *enemy,RT_Anchor *door,
    u8 parentAttributes,s32 playerX,u8 visibleActors,u8 *selector)
{
    if(!enemy || !door || !selector || (enemy->kind!=4 && enemy->kind!=8))
        return RT_DOOR_ENEMY_UNHANDLED;
    u8 state=enemy->action&0xfc;
    if(state==0x80 && (enemy->spawnAction&0xfc)==0x5c){
        door->spawnDelay--;
        if(!door->spawnDelay){
            s32 distance=enemy->x-playerX;
            if(door->visible && (u8)((door->flags&1)+door->phase)==0 &&
                visibleActors<=5 && distance>-0x600 && distance<0x600){
                enemy->action=enemy->spawnAction;load(enemy,0);state=0x5c;
            }else door->spawnDelay=0x3e;
        }
        if(state==0x80){enemy->hidden=1;enemy->action=0x80;return RT_DOOR_ENEMY_HIDE;}
    }
    if(state!=0x5c || enemy->row>5)return RT_DOOR_ENEMY_UNHANDLED;
    u8 row=enemy->row;
    if(!row){door->flags|=1;if(door->phase!=4)return RT_DOOR_ENEMY_UPDATED;}
    if(--enemy->timer)return RT_DOOR_ENEMY_UPDATED;
    if(!row)enemy->attributes=parentAttributes;
    if(row<=3 && !(door->flags&4))enemy->y+=0x20;
    u8 decision=enemy->decisions;
    if(row==3 && !(decision&1) && (decision&0x30)!=0x30){
        if(!(decision&0x20)){
            *selector=decision&8?0x08:0x3c;return RT_DOOR_ENEMY_SELECT;
        }
        enemy->action=enemy->spawnAction=decision&4?0x5d:0x5e;
    }
    if(row==4){door->flags&=0xfd;door->spawnDelay=0xbf;}
    if(row==5){
        *selector=(decision&0x30)==0x30 || (decision&1)?0x04:0x38;
        return RT_DOOR_ENEMY_SELECT;
    }
    load(enemy,row+1);return RT_DOOR_ENEMY_UPDATED;
}
