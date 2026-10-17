#include "rt_enemies.h"
#include "rt_enemy_attack_clock.h"
const u16 rtEnemyWalkGraphics[6]={0x6013,0x6026,0x6039,0x604c,0x605f,0x6072};
RT_EnemySweep RT_enemySweepHorizontal(RT_Enemy *enemy,const RT_WorldCamera *camera,
    RT_EnemyProbe probe,void *context,u8 *selector)
{
    static const u8 codes[8]={0x19,0x22,0x24,0x25,0x26,0x27,0x2c,0x2d};
    static const u8 responses[2][8]={{0x48,0x48,0x24,8,0x24,0x28,0x24,0x3c},
        {0x48,0x48,0x0c,8,0x0c,0x28,0x0c,0x3c}};
    if(!enemy || !camera || !probe || !selector || enemy->state.kind==RT_ENEMY_FREE)
        return RT_ENEMY_SWEEP_UNAVAILABLE;
    *selector=0;
    s32 initial=enemy->state.x,target=initial+enemy->velocityX;
    u8 left=enemy->velocityX<0;
    if((!left && target>=(s32)camera->x*16+0x1400) ||
        (left && target<(s32)camera->x*16-0x200)){
        enemy->state.kind=RT_ENEMY_FREE;return RT_ENEMY_SWEEP_REMOVED;
    }
    if(enemy->state.kind<0x10){
        u16 phase=(u16)initial&0x70;
        s32 position=initial+(left?-(s16)phase:(s16)((0-phase)&0x70));
        while(left?position>=target:position<=target){
            u8 code,flags;enemy->state.x=position;
            if(!probe(left?-2:1,0,&code,&flags,context)){
                enemy->state.x=initial;return RT_ENEMY_SWEEP_UNAVAILABLE;
            }
            if(flags&1){
                enemy->state.x=position-((u16)position&15);enemy->byte12|=1;*selector=0x24;
                for(u16 i=0;i<8;i++)if(codes[i]==code){*selector=responses[enemy->variation>>7][i];break;}
                return RT_ENEMY_SWEEP_HIT;
            }
            position+=left?-128:128;
        }
    }
    enemy->state.x=target;return RT_ENEMY_SWEEP_CLEAR;
}
u8 RT_enemyWalkTransitions(RT_Enemy *enemy,const RT_WorldCamera *camera,
    RT_EnemyProbe probe,void *context,u8 *selector)
{
    if(!enemy || !camera || !probe || !selector)return 0;
    *selector=0;u8 code,flags,left=!!(enemy->state.action&2);
    /* The native callback reads an immutable ring: compute its base cell and
     * collision layer once. Keep generic callbacks and their side effects. */
    if(probe==RT_enemyTerrainProbeReady && context &&
       ((const RT_EnemyTerrainContext*)context)->enemy==enemy){
        const RT_TerrainWindow *window=((const RT_EnemyTerrainContext*)context)->window;
        if(!window || !window->ready)return 0;
        u16 column=((u16)enemy->state.x>>7)&63;
        u16 row=(((u16)((u16)enemy->state.y-1)>>7)&31)*64;
        u16 below=(row+64)&2047;
        u8 layer=(enemy->state.attributes&0xc0)>>6;
        const RT_TerrainMaterial *m=rtWorldMaterials+
            window->materials[(left?row:below)|((column-1)&63)];
        code=m->probeCode;flags=m->collision[layer];
        if((flags&0x20) && !(code&1)){*selector=left?0x4e:0x51;return 1;}
        m=rtWorldMaterials+window->materials[(left?below:row)|((column+1)&63)];
        code=m->probeCode;flags=m->collision[layer];
        if((flags&0x20) && (code&1)){*selector=left?0x52:0x4d;return 1;}
        s32 localX=enemy->state.x-(s32)camera->x*16;
        if(localX< -0x100 || localX>0x1300)return 1;
        m=rtWorldMaterials+window->materials[below|column];
        code=m->probeCode;flags=m->collision[layer];
        if((flags&0x20) || ((flags&2) && (code&0xee)!=0x20))enemy->state.attributes=flags&0xc0;
        else *selector=0x30;
        return 1;
    }
    if(!probe(-1,left?0:1,&code,&flags,context))return 0;
    if((flags&0x20) && !(code&1)){*selector=left?0x4e:0x51;return 1;}
    if(!probe(1,left?1:0,&code,&flags,context))return 0;
    if((flags&0x20) && (code&1)){*selector=left?0x52:0x4d;return 1;}
    s32 x=enemy->state.x-(s32)camera->x*16;
    if(x< -0x100 || x>0x1300)return 1;
    if(!probe(0,1,&code,&flags,context))return 0;
    if((flags&0x20) || ((flags&2) && (code&0xee)!=0x20))enemy->state.attributes=flags&0xc0;
    else *selector=0x30;
    return 1;
}
u8 RT_enemyWalkAnimation(RT_Enemy *enemy)
{
    if(!enemy || enemy->state.row>5)return 0;
    RT_DoorEnemy *a=&enemy->state;u8 state=a->action&0xfc;
    if(state!=4 && state!=0x40 && state!=0x9c && state!=0xa0 && state!=0xa4)return 0;
    if(--a->timer)return 1;
    a->row=a->row==5?0:a->row+1;enemy->byte12&=0x20;
    a->timer=state==4?6:4;a->flags=1;a->pose=91+a->row;return 1;
}
u8 RT_enemyWalkingChoice(RT_Enemy *enemy,s32 playerX,u8 sourceCounter,u8 *coopAttackPhase)
{
    static const u8 attacks[8]={0,0,0x18,0x18,0x0c,0x0c,0x18,0x0c};
    if(!enemy || (enemy->byte12&0x20))return 0;
    u8 flags=enemy->state.decisions;
    if(flags&0x80){if(--enemy->delay)return 0;}
    else if(flags&1){
        s32 distance=enemy->state.x-playerX;
        if(distance> -0x180 && distance<0x180)return 0x94;
        enemy->delay--;
        RT_enemyAttackClock(&enemy->delay,coopAttackPhase);
        if(enemy->delay)return 0;
        u8 choice=enemy->traits&0x0c;
        if(!choice)return enemy->traits&0x40?0x88:0;
        choice=attacks[(choice+(sourceCounter&2))>>1];
        if(choice==0x0c)enemy->byte12|=8;
        return choice;
    }else if(!(enemy->variation&0x40)){
        if(--enemy->delay)return 0;
        if(!(enemy->variation&0x0c)){enemy->delay=enemy->variation&0x10?30:20;return 0;}
        if(!(enemy->variation&8))return 0x38;
    }
    if(flags&0x20)return 0x38;
    return flags&8?8:0x3c;
}
u8 RT_enemyFindWalkObject(const RT_Enemy *enemy,const RT_StagePool *objects,
    u8 visibleObjects,u8 sourceCounter,u8 *selector,u8 *slot)
{
    if(!enemy || !objects || !objects->level || !selector || !slot)return 0;
    *selector=0;*slot=255;
    if(!(enemy->traits&0x20) || (sourceCounter&15) || !visibleObjects)return 1;
    for(u16 i=0;i<RT_NATIVE_STAGE_CAPACITY;i++){
        const RT_Anchor *object=objects->objects+i;
        if(!object->visible || object->kind>3)continue;
        if(!(object->flags&0x38)){
            s32 distance=object->x+0xe0-enemy->state.x;
            if(distance> -0x80 && distance<0x80){
                u16 definition=objects->definitions[i];
                if(definition>=objects->level->eventCount)return 0;
                u8 same=objects->level->events[definition].parameter==enemy->state.attributes;
                u16 height=!same || (object->flags&4)?0x3f0:0x470;
                if(enemy->state.y==object->y+height){
                    *slot=i;if(object->kind==3)*selector=same?0xa8:0xac;
                    return 1;
                }
            }
        }
        if(!--visibleObjects)return 1;
    }
    return 0;
}
RT_EnemyResult RT_enemyWalkStep(RT_Enemy *enemy,const RT_WorldCamera *camera,
    RT_EnemyProbe probe,void *context,const RT_StagePool *objects,u8 visibleObjects,
    s32 playerX,u8 decisionCounter,u8 objectCounter,u8 *selector,u8 *objectSlot,
    u8 *coopAttackPhase)
{
    if(!enemy || !camera || !probe || !objects || !selector || !objectSlot)return RT_ENEMY_UNAVAILABLE;
    *selector=0;*objectSlot=255;
    u8 state=enemy->state.action&0xfc;
    if(enemy->state.kind==RT_ENEMY_FREE || enemy->state.row>5 ||
        (state!=4 && state!=0x40 && state!=0x9c && state!=0xa0 && state!=0xa4))return RT_ENEMY_UNAVAILABLE;
    RT_EnemySweep movement=RT_enemySweepHorizontal(enemy,camera,probe,context,selector);
    if(movement==RT_ENEMY_SWEEP_REMOVED)return RT_ENEMY_REMOVED;
    if(movement==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(movement==RT_ENEMY_SWEEP_HIT)return RT_ENEMY_SELECT;
    if(!RT_enemyWalkTransitions(enemy,camera,probe,context,selector))return RT_ENEMY_UNAVAILABLE;
    if(*selector)return RT_ENEMY_SELECT;
    if(!RT_enemyFindWalkObject(enemy,objects,visibleObjects,objectCounter,selector,objectSlot))return RT_ENEMY_UNAVAILABLE;
    if(*selector)return RT_ENEMY_SELECT;
    *selector=RT_enemyWalkingChoice(enemy,playerX,decisionCounter,coopAttackPhase);
    if(*selector)return RT_ENEMY_SELECT;
    return RT_enemyWalkAnimation(enemy)?RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
}
