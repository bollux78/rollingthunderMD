#include "rt_motion.h"

const RT_TerrainMaterial *RT_motionProbe(const RT_WorldStage *stage,
    const RT_MotionBody *body,const RT_WorldCamera *camera,s16 dx,s16 dy)
{
    s32 fx=body->x+(s32)dx*128,fy=body->y+(s32)dy*128-1;
    s16 x=fx/128-(fx<0 && ((u16)fx&127));
    s16 y=fy/128-(fy<0 && ((u16)fy&127));
    /* The source player can fall below the display before its death exit.
     * Its probe then wraps in the 32-row published collision window. Resolve
     * that authored behavior as a coordinate, without maintaining tile RAM. */
    s16 top=camera->y/8+(camera->y>0 && ((u16)camera->y&7))+2;
    y=top+(((u16)(y-top))&31);
    return RT_worldCell(stage,x,y);
}
/* The original accumulator consumes the signed low byte after one word shift.
 * Legal player speeds are small; retaining the byte reduction also preserves
 * wrapping arithmetic on exceptional velocities, without an instruction VM. */
/* GCC's 68000 LTO can widen a masked byte load to an odd-address word
 * when specializing a single static actor. Force the actual byte access;
 * otherwise a valid fractional accumulator can trigger an address error. */
static u8 fractionByte(const u8 *value)
{
    return *(const volatile u8 *)value;
}
static u16 stepCount(u16 delta,u8 vertical)
{
    if(!delta)return 0;
    u16 byte=(delta>>1)&255;
    s16 count=byte<128?byte/8:((s16)byte-263)/8;
    if(count<0)count=-count;
    return count?count:(vertical?256:0);
}
RT_MotionResult RT_motionHorizontal(RT_MotionBody *body,
    const RT_WorldStage *stage,RT_WorldCamera *camera,u8 lockedCamera)
{
    u16 sum=(u16)(fractionByte(&body->fractionX)&15)+(u16)body->velocityX;
    u16 steps=stepCount(sum&0xfff0,0);
    body->fractionX=sum&15;
    u8 right=body->velocityX>=0;
    while(steps--){
        s16 local=(s16)(body->x-(s32)camera->x*16);
        u8 scroll=!lockedCamera && (right?(u16)local>=0x600:(u16)local<=0x600);
        if(scroll && RT_worldCameraBlocked(stage,camera,right?RT_WORLD_RIGHT:RT_WORLD_LEFT))scroll=0;
        u8 blocked=lockedCamera?(right?(s32)local+16>280*16:local<=128):
            (!scroll && (right?local>=0x1100:local<16));
        u16 alignment=scroll?((u16)camera->x&7)*16:(u16)body->x&0x70;
        if(!blocked && !alignment)
            blocked=RT_motionProbe(stage,body,camera,right?1:-2,0)->collision[body->layer>>1]&1;
        if(blocked){body->fractionX=0;body->x&=~15L;return RT_MOTION_CONTACT;}
        body->x+=right?16:-16;
        if(scroll)camera->x+=right?1:-1;
    }
    return RT_MOTION_CLEAR;
}
static u8 floorContact(RT_MotionBody *body,const RT_WorldStage *stage,
    const RT_WorldCamera *camera,s16 localUp,u8 scroll,RT_VerticalMode mode)
{
    u16 worldUp=stage->heightCells*8-camera->y-256;
    u16 alignment=scroll?(worldUp&7)*16:((worldUp&7)*16+((u16)localUp&0x70))&0x70;
    if(alignment)return 0;
    const RT_TerrainMaterial *material=RT_motionProbe(stage,body,camera,0,1);
    u8 flags=material->collision[body->layer>>1];
    if(!(flags&6))return 0;
    if((material->probeCode&0xee)==0x20){body->layer=(flags>>5)&6;return 0;}
    return flags&(mode==RT_MOTION_DROP?4:2)?flags:0;
}
RT_MotionResult RT_motionVertical(RT_MotionBody *body,
    const RT_WorldStage *stage,RT_WorldCamera *camera,RT_VerticalMode mode)
{
    u16 old=fractionByte(&body->fractionY)&15,sum=old+(u16)body->velocityY;
    /* Y runs down, so source floor-rounded upward accumulation becomes
     * ceiling-rounded downward accumulation. Fraction0 remains exact. */
    u16 steps=stepCount((u16)(((sum+15)&0xfff0)-((old+15)&0xfff0)),1);
    body->fractionY=sum&15;
    u8 up=body->velocityY<=0,hadSteps=steps!=0;
    while(steps--){
        s16 localUp=(s16)(((s32)camera->y+256)*16-body->y);
        u8 scroll,contact=0;
        if(up){
            scroll=(u16)localUp>0x780 && !RT_worldCameraBlocked(stage,camera,RT_WORLD_UP);
            if(mode==RT_MOTION_VAULT){
                u16 worldUp=stage->heightCells*8-camera->y-256;
                u16 alignment=(worldUp&7)*16;
                /* The vault's actor-only branch tests its high coordinate
                 * byte. Preserve that source rule, including fractional Y. */
                if(!scroll)alignment=(alignment+((u16)localUp>>8))&0x70;
                if(!alignment)contact=RT_motionProbe(stage,body,camera,0,-3)->collision[body->layer>>1]&8;
            }
            if(!contact && !scroll && localUp>=0xe00){body->velocityY=0;return RT_MOTION_CLEAR;}
        }else{
            scroll=(u16)localUp<0x780 && !RT_worldCameraBlocked(stage,camera,RT_WORLD_DOWN);
            if(scroll && floorContact(body,stage,camera,localUp,1,mode))scroll=0;
            if(!scroll){
                if(localUp<=-1024)return RT_MOTION_FELL_OUT;
                if(localUp>-256)contact=floorContact(body,stage,camera,localUp,0,mode);
            }
        }
        if(contact){
            if(!up)body->layer=(contact>>5)&6;
            body->fractionY=0;body->y=(body->y+15)&~15L;
            return RT_MOTION_CONTACT;
        }
        body->y+=up?-16:16;
        if(scroll)camera->y+=up?-1:1;
    }
    if(!(hadSteps && !up && body->velocityY>=64))
        body->velocityY=(s16)((u16)body->velocityY+(u16)body->gravity);
    return RT_MOTION_CLEAR;
}

void RT_motionStairCamera(const RT_MotionBody *body,const RT_WorldStage *stage,
    RT_WorldCamera *camera,u8 *flags)
{
    u16 local=(u16)(body->x-(s32)camera->x*16);
    u8 hold=0,right=body->velocityX>=0;
    if(right){
        if(!(*flags&1)){if(local<0x600){*flags|=1;hold=1;}}
        else if(local<0xa00)hold=1;
        else *flags&=2;
    }else{
        if(!(*flags&1)){if(local>=0xa00){*flags|=1;hold=1;}}
        else if(local>0x600)hold=1;
        else *flags&=2;
    }
    if(!hold && !RT_worldCameraBlocked(stage,camera,right?RT_WORLD_RIGHT:RT_WORLD_LEFT))
        camera->x+=right?1:-1;

    local=(u16)(((s32)camera->y+256)*16-body->y);
    u8 up=body->velocityY<=0;hold=0;
    if(up){
        if(!(*flags&2)){if(local<0x400){*flags|=2;hold=1;}}
        else if(local<0x800)hold=1;
        /* Original 9456 assigns the low coordinate byte to these flags. */
        else *flags=(u8)local;
    }else{
        if(!(*flags&2)){if(local>=0x800){*flags|=2;hold=1;}}
        else if(local>0x400)hold=1;
        else *flags&=1;
    }
    if(!hold && !RT_worldCameraBlocked(stage,camera,up?RT_WORLD_UP:RT_WORLD_DOWN))
        camera->y+=up?-1:1;
}
