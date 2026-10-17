#include "rt_enemy_throw.h"
static void explode(RT_Projectile *o,const RT_WorldCamera *camera,RT_WeaponEvent *event)
{
    s16 x=(s16)(o->x-(s32)camera->x*16);
    u16 y=(u16)(((s32)camera->y+256)*16-o->y);
    o->x=(s32)camera->x*16+(s16)((u16)x-128);
    o->y=((s32)camera->y+256)*16-(s16)((y+128)&0xff80);
    o->kind=RT_PROJECTILE_EXPLOSION;o->pose=48;o->phase=0;event->sound=0x26;
}
RT_EnemyResult RT_throwMotionStep(RT_ProjectilePool *pool,u8 slot,
    RT_ThrowMotion motion[RT_PROJECTILE_CAPACITY],const RT_WorldCamera *camera,
    RT_ThrowProbe probe,RT_ThrowHit hit,void *context,RT_WeaponEvent *event)
{
    static const s16 explosionY[13]={0,0,0,272,240,64,48,16,-16,64,0,32,32};
    if(!pool || slot>=RT_PROJECTILE_CAPACITY || !motion || !camera || !probe || !event)
        return RT_ENEMY_UNAVAILABLE;
    RT_Projectile *o=pool->slots+slot;RT_ThrowMotion *m=motion+slot;
    if(o->kind<RT_PROJECTILE_NEW_THROW_DOWN || o->kind>RT_PROJECTILE_EXPLOSION)
        return RT_ENEMY_UNAVAILABLE;
    event->sound=event->command=0;
    if(o->kind==RT_PROJECTILE_NEW_THROW_DOWN){o->kind=RT_PROJECTILE_HELD_DOWN;return RT_ENEMY_CONTINUE;}
    if(o->kind==RT_PROJECTILE_NEW_THROW_FORWARD){o->kind=RT_PROJECTILE_HELD_FORWARD;return RT_ENEMY_CONTINUE;}
    if(o->kind==RT_PROJECTILE_EXPLOSION){
        if(o->phase<=2 && hit)hit(o,m,1,context);
        o->timer=(o->timer+1)&7;if(o->timer)return RT_ENEMY_CONTINUE;
        o->pose++;o->phase++;
        if(o->phase>=13)goto removed;
        s16 y=(s16)(((s32)camera->y+256)*16-o->y);
        o->y=((s32)camera->y+256)*16-(s16)((u16)y+explosionY[o->phase]);
        return RT_ENEMY_CONTINUE;
    }
    u8 held=o->kind==RT_PROJECTILE_HELD_DOWN || o->kind==RT_PROJECTILE_HELD_FORWARD;
    if(held && !o->phase)return RT_ENEMY_CONTINUE;
    if(!held && hit && hit(o,m,0,context)){explode(o,camera,event);return RT_ENEMY_CONTINUE;}
    s16 x=(s16)((u16)(o->x-(s32)camera->x*16)+o->velocityX);
    s16 y=(s16)((u16)(((s32)camera->y+256)*16-o->y)-m->velocityY);
    o->x=(s32)camera->x*16+x;o->y=((s32)camera->y+256)*16-y;
    u8 left=o->velocityX<0,descending=(s16)(0-(u16)m->velocityY)<0;
    if((descending && y>=0x1000) || (!descending && y< -0x100))goto removed;
    if(!descending && (y<0 || y>=0xf00))goto clear;
    if((left && x< -0x200) || (!left && x>=0x1400))goto removed;
    if(x<0 || x>=0x1200)goto clear;
    u8 code,flags;
    if(!probe(o,m,&code,&flags,context))return RT_ENEMY_UNAVAILABLE;
    if(flags&3){
        m->attributes=flags&0xc0;o->layer=m->attributes>>5;
        if((code&0xee)!=0x20){explode(o,camera,event);return RT_ENEMY_CONTINUE;}
    }
clear:
    m->velocityY=(s16)((u16)m->velocityY+2);
    if(held)return RT_ENEMY_CONTINUE;
    o->timer=(o->timer+1)&3;if(o->timer)return RT_ENEMY_CONTINUE;
    o->phase++;
    if(o->phase==6){o->phase=0;o->pose=36+!!(m->direction&2);}
    else o->pose+=2;
    return RT_ENEMY_CONTINUE;
removed:
    o->kind=RT_PROJECTILE_FREE;pool->active--;return RT_ENEMY_REMOVED;
}
