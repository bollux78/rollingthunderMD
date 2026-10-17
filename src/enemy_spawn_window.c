#include "rt_enemy_spawns.h"
u8 RT_visitEnemySpawns(const RT_EnemySpawnLevel *level,u8 gridX,u8 gridY,
    RT_EventWindowEdge edge,const u8 *population,RT_EnemySpawnVisitor visit,void *context)
{
    if(!level || !level->events || !level->cells || !level->columns || !level->rows ||
        level->columns>125 || level->rows>125 || gridX>level->columns+2 ||
        gridY>level->rows+2 || !population || !visit)return 0;
    s16 left=(s16)gridX-2,right=gridX,top=(s16)gridY-2,bottom=gridY;
    switch(edge){
    case RT_EVENT_WINDOW_INITIAL:break;
    case RT_EVENT_WINDOW_RIGHT:left=right=(s16)gridX+1;break;
    case RT_EVENT_WINDOW_LEFT:left=right=(s16)gridX-3;break;
    case RT_EVENT_WINDOW_UP:top=bottom=(s16)gridY+1;break;
    case RT_EVENT_WINDOW_DOWN:top=bottom=(s16)gridY-3;break;
    default:return 0;
    }
    if(left<0)left=0;
    if(top<0)top=0;
    if(right>=level->columns)right=level->columns-1;
    if(bottom>=level->rows)bottom=level->rows-1;
    for(s16 y=top;y<=bottom;y++)for(s16 x=left;x<=right;x++){
        if(*population>=40)return 1;
        const RT_StageEventCell *cell=level->cells+(u16)y*level->columns+x;
        if(cell->first>level->eventCount || cell->count>level->eventCount-cell->first)return 0;
        for(u16 i=0;i<cell->count;i++){
            u16 index=cell->first+i;u8 result=visit(level->events+index,index,context);
            if(result==RT_ENEMY_SPAWN_FULL)break;
            if(result!=RT_ENEMY_SPAWN_ACCEPTED)return 0;
        }
    }
    return 1;
}
