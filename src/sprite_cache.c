#include "rt_sprite_cache.h"
u8 RT_spriteCacheInitialize(RT_SpriteCache *c,u16 first,u8 count,u8 tiles)
{
    if(!c || !count || count>RT_SPRITE_CACHE_CAPACITY || !tiles || tiles>64 ||
        first>=1536 || (u16)count*tiles>1536-first)return 0;
    c->count=count;
    for(u16 i=0;i<count;i++){
        RT_spriteSlotInitialize(c->slots+i,first+i*tiles,tiles);c->owners[i]=255;
    }
    return 1;
}
u8 RT_spriteCachePlan(RT_SpriteCache *c,const u8 *actors,u8 count,u8 *slots)
{
    if(!c || !c->count || c->count>RT_SPRITE_CACHE_CAPACITY || count>c->count ||
        (count && (!actors || !slots)))return 0;
    u8 used=0,plan[RT_SPRITE_CACHE_CAPACITY];
    for(u16 i=0;i<count;i++){
        if(actors[i]==255)return 0;
        for(u16 j=0;j<i;j++)if(actors[j]==actors[i])return 0;
        plan[i]=255;
        for(u16 j=0;j<c->count;j++)if(c->owners[j]==actors[i]){
            plan[i]=j;used|=1<<j;break;
        }
    }
    for(u16 i=0;i<count;i++)if(plan[i]==255){
        for(u16 j=0;j<c->count;j++)if(!(used&(1<<j))){plan[i]=j;used|=1<<j;break;}
    }
    for(u16 i=0;i<count;i++){c->owners[plan[i]]=actors[i];slots[i]=plan[i];}
    return 1;
}

u8 RT_spriteCachePlanPoses(const RT_SpriteCache *c,
    const RT_SpritePose *const *poses,u8 count,u8 *slots)
{
    if(!c || !c->count || c->count>RT_SPRITE_CACHE_CAPACITY || count>80 ||
        (count && (!poses || !slots)))return 0;
    const RT_SpritePose *unique[RT_SPRITE_CACHE_CAPACITY];
    u8 mappings[80],plan[RT_SPRITE_CACHE_CAPACITY],total=0,used=0;
    for(u16 i=0;i<count;i++){
        const RT_SpritePose *pose=poses[i];
        if(!pose || !pose->patterns || !pose->tileCount)return 0;
        u8 j;
        for(j=0;j<total;j++)if(unique[j]==pose)break;
        if(j==total){
            if(total==c->count)return 0;
            unique[total++]=pose;
        }
        mappings[i]=j;
    }
    /* Reserve all already resident poses before assigning replacement slots. */
    for(u16 i=0;i<total;i++){
        plan[i]=255;
        for(u16 j=0;j<c->count;j++)if(c->slots[j].pose==unique[i] &&
            c->slots[j].capacity>=unique[i]->tileCount){
            plan[i]=j;used|=1<<j;break;
        }
    }
    for(u16 i=0;i<total;i++)if(plan[i]==255){
        for(u16 j=0;j<c->count;j++)if(!(used&(1<<j)) &&
            c->slots[j].capacity>=unique[i]->tileCount){
            plan[i]=j;used|=1<<j;break;
        }
        if(plan[i]==255)return 0;
    }
    for(u16 i=0;i<count;i++)slots[i]=plan[mappings[i]];
    return 1;
}
