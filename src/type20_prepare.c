#include "rt_type20.h"
/* B66E uses camera-relative signed16 fixed-point coordinates. Native world
 * coordinates already retain camera movement, so no per-actor scroll addition. */
RT_EnemyResult RT_type20Prepare(RT_Enemy *e,const RT_Type20Motion *c,u8 *counted)
{
    if(!e || !c || !c->camera || !counted || e->state.kind!=32)return RT_ENEMY_UNAVAILABLE;
    *counted=0;
    s16 x=(s16)(e->state.x-(s32)c->camera->x*16);
    s16 y=(s16)(((s32)c->camera->y+256)*16-e->state.y);
    if((e->state.action&0xfc)==0x80){
        if(y< -0x400 || y>=0x1000)return RT_ENEMY_CONTINUE;
        if(e->state.spawnAction&2){if(x<0x1500)return RT_ENEMY_CONTINUE;}
        else if(x>=-0x300)return RT_ENEMY_CONTINUE;
    }else{
        if(y< -0x400 || y>=0x1000 || x>0x1500 || x< -0x300){
            e->state.hidden=1;return RT_ENEMY_CONTINUE;
        }
        *counted=1;e->state.hidden=0;
        if(e->state.spawnAction==e->state.action)return RT_ENEMY_CONTINUE;
    }
    RT_Type20EntryResult result=RT_type20SelectInContext(e,e->state.spawnAction,c);
    return result==RT_TYPE20_ENTRY_UNAVAILABLE?RT_ENEMY_UNAVAILABLE:RT_ENEMY_CONTINUE;
}
