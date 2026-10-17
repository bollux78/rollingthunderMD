#include "rt_campaign_display.h"
u8 RT_campaignDisplayInitializeStreamed(RT_CampaignDisplay *view,
    const RT_CampaignScene *scene,const RT_DisplayStage *atlas,
    const RT_DisplayPatch patches[3][2],RT_DisplayStream *stream,u16 slots)
{
    if(!view || !scene || !scene->ready || !scene->stage || !atlas || !patches ||
       atlas->story!=scene->stage->story || atlas->area!=scene->stage->area)return 0;
    for(u8 kind=0;kind<3;kind++)for(u8 variant=0;variant<2;variant++){
        const RT_DisplayPatch *p=&patches[kind][variant];
        if(!p->cells || !p->width || p->width>64 || !p->height || p->height>32)return 0;
    }
    view->pendingBytes=0;view->doorPatches=patches;view->stream=stream;
    if(stream)return RT_displayStreamInitialize(stream,&view->display,atlas,&scene->camera,slots);
    return RT_displayInitialize(&view->display,atlas,&scene->camera);
}
u8 RT_campaignDisplayInitialize(RT_CampaignDisplay *view,const RT_CampaignScene *scene,
    const RT_DisplayStage *atlas,const RT_DisplayPatch patches[3][2])
{
    return RT_campaignDisplayInitializeStreamed(view,scene,atlas,patches,0,0);
}
u8 RT_campaignDisplayQueue(RT_CampaignDisplay *view,
    const RT_WorldCamera *camera,const RT_CampaignObjectPass *objects)
{
    if(!view || !view->doorPatches || !objects || objects->count>RT_NATIVE_STAGE_CAPACITY)return 0;
    for(u8 i=0;i<objects->count;i++)
        if(objects->events[i].kind>2 || objects->events[i].replace>1)return 0;
    if(!(view->stream?RT_displayStreamPrepare(view->stream,&view->display,camera):
        RT_displayPrepare(&view->display,camera)))return 0;
    for(u8 i=0;i<objects->count;i++){
        const RT_CampaignObjectEvent *e=objects->events+i;
        const RT_DisplayPatch *patch=&view->doorPatches[e->kind][e->replace];
        if(!(view->stream?RT_displayStreamPatch(view->stream,&view->display,patch,e->column,e->row):
            RT_displayPatch(&view->display,patch,e->column,e->row)))return 0;
    }
    u32 bytes=(u32)view->pendingBytes+view->display.queuedBytes;
    if(bytes>65535){view->display.ready=0;return 0;}
    view->pendingBytes=(u16)bytes;view->display.queuedBytes=(u16)bytes;
    return 1;
}
void RT_campaignDisplayCommitted(RT_CampaignDisplay *view)
{
    if(view){view->pendingBytes=0;view->display.queuedBytes=0;}
}
