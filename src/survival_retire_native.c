#include <genesis.h>
#include "rt_survival_retire_native.h"
#include "rt_survival_attack.h"
u16 RT_survivalLocalCameraMinimum(const RT_SurvivalWindow *w)
{return w && w->initialized && !w->firstChunk && w->stream.emitted==8?0:256;}
/* Keep a full section of margin ahead of the retired boundary. The owner
 * retries after normal hidden-object retirement; no live slots are discarded
 * here to force progress. All spatial consumers and restart snapshots count. */
static u8 retained(const RT_CampaignScene *s)
{
    const s32 minimum=768*16;
    for(u8 p=0;p<s->players.home.players;p++)
        if(((s->players.home.players==1 || !s->players.combat[p].removed) &&
            s->players.agents[p].body.x<minimum) || s->players.home.spawn[p].body.x<minimum)return 0;
    for(u8 i=0;i<RT_NATIVE_STAGE_CAPACITY;i++)
        if(s->objects.objects[i].kind!=255 && s->objects.objects[i].x<minimum)return 0;
    for(u8 i=0;i<RT_ENEMY_CAPACITY;i++)
        if(s->enemies.slots[i].state.kind!=RT_ENEMY_FREE && s->enemies.slots[i].state.x<minimum)return 0;
    for(u8 i=0;i<RT_PROJECTILE_CAPACITY;i++)
        if(s->projectiles.slots[i].kind && s->projectiles.slots[i].x<minimum)return 0;
    return 1;
}
u8 RT_survivalRetireSpatiallyReady(const RT_CampaignScene *scene)
{
    return scene && scene->stage && scene->camera.x>=1024 &&
        scene->objectWindow.gridX>=5 && scene->enemyWindow.gridX>=5 &&
        scene->stage->entrances[1].x>=768 &&
        RT_survivalSceneCanShift512(scene) && retained(scene);
}
u8 RT_survivalRetireTwo(RT_SurvivalWindow *window,RT_SurvivalWorld *world,
    RT_CampaignScene *scene,RT_CampaignDisplay *view,RT_SurvivalDisplay *display,
    const RT_CampaignObjectPass *pending,const RT_WorldStage *source)
{
    if(!window || !world || !scene || !view || !display || !pending || !source ||
       !window->initialized || window->head>=8 || world->stage.pages!=world->pages ||
       scene->stage!=&world->stage || display->world!=world || display->display!=&view->display ||
       world->firstChunk!=window->firstChunk || display->firstChunk!=window->firstChunk ||
       !view->display.ready || view->pendingBytes || view->display.queuedBytes || pending->count ||
       DMA_getQueueSize() || !RT_survivalRetireSpatiallyReady(scene) ||
       view->display.scrollX!=scene->camera.x-16 || view->display.scrollY!=scene->camera.y+32 ||
       view->display.column!=view->display.scrollX/8 ||
       view->display.row!=view->display.scrollY/8-(view->display.scrollY<0 && ((u16)view->display.scrollY&7)))return 0;
    for(u8 i=0;i<8;i++)if(world->chunks[i]!=window->chunks[(window->head+i)&7])return 0;
    RT_SurvivalWindow next=*window;RT_SurvivalWorld nextWorld;
    if(!RT_advanceSurvivalWindow(&next) || !RT_advanceSurvivalWindow(&next) ||
       !RT_survivalWorldBuild(&nextWorld,source,&next))return 0;
    if(nextWorld.densityTier<world->densityTier)nextWorld.densityTier=world->densityTier;
    nextWorld.stage.entrances[1]=world->stage.entrances[1];
    nextWorld.stage.entrances[1].x-=512;
    /* Every fallible condition was checked before publication. Main-thread
     * ownership excludes DMA producers or scene mutation during this commit. */
    RT_survivalSceneShift512(scene);
    *window=next;*world=nextWorld;world->stage.pages=world->pages;
    RT_survivalAttackConfigure(scene,world);
    RT_survivalDisplayRebase(display);
    return 1;
}
