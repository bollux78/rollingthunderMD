#include "rt_high_enemy_motion.h"
u8 RT_highEnemyGround(RT_Enemy *e,const RT_WorldCamera *camera,
    RT_EnemyProbe probe,void *context,u8 *selector)
{
    if(!e || !camera || !selector)return 0;
    *selector=0;
    s16 x=(s16)(e->state.x-(s32)camera->x*16);
    if(x< -0x100 || x>0x1300)return 1;
    u8 code,flags;
    if(!probe || !probe(0,1,&code,&flags,context))return 0;
    if(!(flags&0x20) && (!(flags&2) || (code&0xee)==0x20))*selector=0x30;
    else e->state.attributes=flags&0xc0;
    return 1;
}
RT_EnemySweep RT_highEnemyHorizontal(RT_Enemy *e,const RT_WorldCamera *camera,
    u8 phase,RT_EnemyProbe probe,void *context,u8 *hitCode)
{
    static const s8 right[4]={1,3,1,4},left[4]={-2,-4,-2,-5};
    if(!e || !camera || !hitCode)return RT_ENEMY_SWEEP_UNAVAILABLE;
    u8 type=e->state.kind&0x7c;
    if(type<0x20 || type>0x2c)return RT_ENEMY_SWEEP_UNAVAILABLE;
    s32 original=e->state.x,origin=(s32)camera->x*16;
    s16 start=(s16)(original-origin),velocity=e->velocityX;
    s16 target=(s16)((u16)start+(u16)velocity),x=start;
    if(velocity<0?target< -0x200:target>=0x1400)return RT_ENEMY_SWEEP_REMOVED;
    do{
        if(((((u8)x&0x70)+phase)&0x70)==0){
            u8 code,flags;
            if(!probe || !probe(velocity<0?left[(type-0x20)>>2]:right[(type-0x20)>>2],0,&code,&flags,context)){
                e->state.x=original;return RT_ENEMY_SWEEP_UNAVAILABLE;
            }
            if(flags&1){
                e->state.x=origin+(s16)((u16)x&0xfff0);e->byte12|=1;
                *hitCode=code;return RT_ENEMY_SWEEP_HIT;
            }
        }
        x=(s16)((u16)x+(velocity<0?0xfff0:0x10));e->state.x=origin+x;
    }while(velocity<0?x>=target:x<=target);
    e->state.x=origin+target;return RT_ENEMY_SWEEP_CLEAR;
}
RT_EnemySweep RT_highEnemyVertical(RT_Enemy *e,const RT_WorldCamera *camera,
    s16 accelerationY,u8 phase,RT_EnemyProbe probe,void *context,u16 *result)
{
    if(!e || !camera || !result)return RT_ENEMY_SWEEP_UNAVAILABLE;
    s32 original=e->state.y,origin=((s32)camera->y+256)*16;
    s16 start=(s16)(origin-original),velocity=(s16)(0-(u16)e->velocityY);
    s16 target=(s16)((u16)start+(u16)velocity),y=start;
    u8 attribute=e->state.attributes,scratch=*result>>8,state=e->state.action&0xfc;
    if(velocity>0?target>=0xf80:target< -0x380)return RT_ENEMY_SWEEP_REMOVED;
    if(velocity<=0 || (state==0x28 && !(e->byte12&0x10))){
        do{
            if(((((u8)y&0x70)+phase)&0x70)==0){
                u8 code,flags;
                if(!probe || !probe(0,velocity>0?-3:1,&code,&flags,context)){
                    e->state.y=original;e->state.attributes=attribute;
                    return RT_ENEMY_SWEEP_UNAVAILABLE;
                }
                scratch=code;
                if(velocity>0){if(flags&8){e->byte12|=0x10;break;}}
                else if((flags&6) && (state!=0x2c || (flags&4))){
                    e->state.attributes=flags&0xc0;
                    /* Source BITA retains the unmasked code for CMPA20. */
                    if(code!=0x20){
                        e->state.y=origin-(s16)((u16)y&0xfff0);e->byte12|=2;
                        *result=((u16)scratch<<8)|(flags&0xc0);return RT_ENEMY_SWEEP_HIT;
                    }
                }
            }
            y=(s16)((u16)y+(velocity>0?0x10:0xfff0));e->state.y=origin-y;
        }while(velocity>0?y<=target:y>=target);
    }
    e->state.y=origin-target;e->velocityY=(s16)((u16)e->velocityY+(u16)accelerationY);
    *result=(u16)scratch<<8;return RT_ENEMY_SWEEP_CLEAR;
}
u8 RT_highEnemySettle(RT_Enemy *e)
{
    if(!e || (u16)(0-(u16)e->velocityY)>15)return 0;
    e->velocityY=0;return 1;
}
