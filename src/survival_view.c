#include "rt_survival_view.h"
u8 RT_survivalViewInitialize(RT_CampaignDisplay *view,RT_SurvivalDisplay *stream,
    const RT_CampaignScene *scene,const RT_SurvivalWorld *world,
    const RT_DisplayStage *atlas,const RT_DisplayPatch patches[3][2])
{
    if(!view || !stream || !scene || !scene->ready || !world ||
       scene->stage!=&world->stage || !patches ||
       scene->players.selection.mode!=RT_MODE_HOME ||
       scene->players.selection.game!=RT_HOME_SURVIVAL)return 0;
    for(u8 kind=0;kind<3;kind++)for(u8 variant=0;variant<2;variant++){
        const RT_DisplayPatch *p=&patches[kind][variant];
        if(!p->cells || !p->width || p->width>64 || !p->height || p->height>32)return 0;
    }
    view->pendingBytes=0;view->doorPatches=patches;view->stream=0;
    return RT_survivalDisplayInitialize(stream,&view->display,atlas,world,&scene->camera);
}
u8 RT_survivalViewQueue(RT_CampaignDisplay *view,RT_SurvivalDisplay *stream,
    const RT_WorldCamera *camera,const RT_CampaignObjectPass *objects)
{
    if(!view || !stream || stream->display!=&view->display || view->stream ||
       !view->doorPatches || !objects || objects->count>RT_NATIVE_STAGE_CAPACITY)return 0;
    for(u8 i=0;i<objects->count;i++)
        if(objects->events[i].kind>2 || objects->events[i].replace>1)return 0;
    if(!RT_survivalDisplayPrepare(stream,camera))return 0;
    for(u8 i=0;i<objects->count;i++){
        const RT_CampaignObjectEvent *e=objects->events+i;
        if(!RT_displayPatch(&view->display,&view->doorPatches[e->kind][e->replace],e->column,e->row)){
            view->display.ready=0;return 0;
        }
    }
    u32 bytes=(u32)view->pendingBytes+view->display.queuedBytes;
    if(bytes>65535){view->display.ready=0;return 0;}
    view->pendingBytes=(u16)bytes;view->display.queuedBytes=(u16)bytes;
    return 1;
}
