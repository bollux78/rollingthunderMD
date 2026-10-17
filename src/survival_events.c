#include "rt_survival_events.h"

typedef struct {s16 left,right,top,bottom;} Bounds;
static u8 bounds(const RT_SurvivalWorld *w,const RT_EventWindowRequest *r,u8 stage,Bounds *b)
{
    if(!w || w->stage.pages!=w->pages || !r || r->gridX>10 || r->gridY>4)return 0;
    for(u8 i=0;i<8;i++)if(w->chunks[i]>=19)return 0;
    *b=(Bounds){r->gridX-2,r->gridX,r->gridY-2,r->gridY};
    switch(r->edge){
    case RT_EVENT_WINDOW_INITIAL:break;
    case RT_EVENT_WINDOW_RIGHT:b->left=b->right=r->gridX+1;break;
    case RT_EVENT_WINDOW_LEFT:b->left=b->right=r->gridX-3;if(stage)b->top=r->gridY-3;break;
    case RT_EVENT_WINDOW_UP:b->top=b->bottom=r->gridY+1;break;
    case RT_EVENT_WINDOW_DOWN:b->top=b->bottom=r->gridY-3;break;
    default:return 0;
    }
    if(b->left<0)b->left=0;
    if(b->right>7)b->right=7;
    if(b->top<0)b->top=0;
    if(b->bottom>1)b->bottom=1;
    return 1;
}
typedef struct {s16 delta;RT_StageEventVisitor visit;void *context;} StageVisit;
static u8 stageEvent(const RT_StageEvent *event,u16 index,void *context)
{
    StageVisit *v=context;RT_StageEvent relocated=*event;
    s32 x=(s32)event->x+v->delta;if(x< -32768 || x>32767)return 0;
    relocated.x=x;return v->visit(&relocated,index,v->context);
}
typedef struct {s16 delta;RT_EnemySpawnVisitor visit;void *context;} EnemyVisit;
static u8 enemyEvent(const RT_EnemySpawn *event,u16 index,void *context)
{
    EnemyVisit *v=context;RT_EnemySpawn relocated=*event;
    s32 x=(s32)event->x+v->delta;if(x< -32768 || x>32767)return RT_ENEMY_SPAWN_ERROR;
    relocated.x=x;return v->visit(&relocated,index,v->context);
}
u8 RT_survivalVisitStageEvents(const RT_SurvivalWorld *world,
    const RT_StageEventLevel *source,const RT_EventWindowRequest *request,
    u8 *population,RT_StageEventVisitor visit,void *context)
{
    Bounds b;
    if(!source || !source->events || !source->cells || source->columns!=19 ||
       source->rows!=2 || !population || !visit || !bounds(world,request,1,&b))return 0;
    for(s16 y=b.top;y<=b.bottom;y++)for(s16 x=b.left;x<=b.right;x++){
        if(*population>=31)return 1;
        u8 chunk=world->chunks[x];
        RT_StageEventLevel cell=*source;cell.columns=cell.rows=1;
        cell.cells=source->cells+y*19+chunk;
        StageVisit v={(x-chunk)*256,visit,context};
        if(!RT_stageVisitEvents(&cell,0,0,RT_EVENT_WINDOW_INITIAL,population,stageEvent,&v))return 0;
    }
    return 1;
}
u8 RT_survivalVisitEnemySpawns(const RT_SurvivalWorld *world,
    const RT_EnemySpawnLevel *source,const RT_EventWindowRequest *request,
    const u8 *population,RT_EnemySpawnVisitor visit,void *context)
{
    Bounds b;
    if(!source || !source->events || !source->cells || source->columns!=19 ||
       source->rows!=2 || !population || !visit || !bounds(world,request,0,&b))return 0;
    for(s16 y=b.top;y<=b.bottom;y++)for(s16 x=b.left;x<=b.right;x++){
        if(*population>=40)return 1;
        u8 chunk=world->chunks[x];
        RT_EnemySpawnLevel cell=*source;cell.columns=cell.rows=1;
        cell.cells=source->cells+y*19+chunk;
        EnemyVisit v={(x-chunk)*256,visit,context};
        if(!RT_visitEnemySpawns(&cell,0,0,RT_EVENT_WINDOW_INITIAL,population,enemyEvent,&v))return 0;
    }
    return 1;
}
