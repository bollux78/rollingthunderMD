#include "rt_campaign_scene_collision.h"
u8 RT_sceneCollisionProbe(const RT_SceneActor *actor,s16 cellX,
 u8 *code,u8 *flags,void *context)
{
 (void)context;
 if(!actor || !code || !flags)return 0;
 /* Canonical source shadow reduces to direct8px cells. Upward-Y's inclusive
  * tile edge becomes ceiling division in native downward-Y coordinates. */
 u16 column=((actor->x>>7)+cellX+4)&63;
 u16 row=((actor->y>>7)+!!((u16)actor->y&127)-2)&31;
 const RT_SceneCollisionCell *cell=&rtSceneCollisionRows[rtSceneCollisionRowIds[row]][column];
 *code=cell->code;*flags=cell->flags;return 1;
}
