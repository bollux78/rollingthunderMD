#include "rt_combat.h"
#ifdef __m68k__
#include "rt_perception_68000.h"
#else
u8 RT_enemyPerceiveGeometryPrepared(RT_Enemy *enemy,const RT_EnemyPerceptionTarget *player)
{
    RT_DoorEnemy *a=&enemy->state;
    const RT_ContactBox *e=rtContactBoxes+(a->flags&127);
    u8 flags=((a->decisions<<1)|a->decisions)&2;
    /* Source Y is upward. Retain its signed word comparisons, including
     * the 8000 magnitude edge, over native downward world coordinates. */
    u16 delta=(u16)(a->y+e->yOffset-player->y);
    if((s16)delta<0){
        delta=(u16)(0-delta);
        if((s16)delta<=player->height)flags|=0x61;
    }else if((s16)delta>0x500)flags|=8;
    else if((s16)delta>(s16)e->height)flags|=0x21;
    else flags|=0x61;
    delta=(u16)(player->x-
        a->x-e->xOffset[!!(a->action&2)]);
    u8 same=a->attributes==player->attributes;
    if((s16)delta<0){
        delta=(u16)(0-delta);
        if((s16)delta>player->width){
            if(a->action&2)flags=same?flags&0x2f:(flags|0x80)&0xaa;
            else flags&=0x2a;
        }else flags=same?flags|0x10:(flags|0x80)&0xba;
    }else if((s16)delta>(s16)e->width){
        if(a->action&1)flags=same?(flags|4)&0x2f:(flags|0x84)&0xae;
        else flags=(flags|4)&0x2e;
    }else flags=same?flags|0x14:(flags|0x94)&0xbe;
    a->decisions=flags;return 1;
}
#endif

u8 RT_enemyPerceivePrepared(RT_Enemy *enemy,const RT_EnemyPerceptionTarget *player)
{
    RT_enemyPerceiveGeometryPrepared(enemy,player);
    if((enemy->state.decisions&0x40) && ((player->flags|enemy->state.flags)&0x80))
        enemy->state.decisions&=0xfe;
    return 1;
}

u8 RT_enemyPerceive(RT_Enemy *enemy,const RT_Agent *player)
{
    if(!enemy || !player || enemy->state.kind==RT_ENEMY_FREE)return 0;
    return RT_enemyPerceiveReady(enemy,player);
}

void RT_enemyPreparePerception(const RT_Agent *player,RT_EnemyPerceptionTarget *target)
{
    const RT_ContactBox *box=rtContactBoxes+(player->contactPose&127);
    target->x=(u16)(player->body.x+box->xOffset[player->direction==2]);
    target->y=(u16)(player->body.y+box->yOffset);
    target->width=box->width;target->height=box->height;
    target->attributes=player->body.layer<<5;target->flags=player->contactPose;
}
u8 RT_enemyPerceiveReady(RT_Enemy *enemy,const RT_Agent *player)
{
    RT_EnemyPerceptionTarget target;RT_enemyPreparePerception(player,&target);
    return RT_enemyPerceivePrepared(enemy,&target);
}
