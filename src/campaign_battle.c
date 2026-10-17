#include "rt_campaign_scene.h"
typedef struct {
    RT_PlayerTargets players;
    RT_EnemyTargets enemies;
    RT_CampaignBattlePass *result;
} BattleTargets;
static u8 hit(const RT_Projectile *shot,void *context)
{
    BattleTargets *targets=context;u8 struck;
    if(shot->flags&RT_PROJECTILE_ENEMY){
        struck=RT_enemyBulletHitPlayers(shot,&targets->players);
        targets->result->playerHits+=struck;
    }else{
        struck=RT_playerBulletHitEnemies(shot,&targets->enemies);
        targets->result->enemyHits+=struck;
    }
    return struck;
}
u8 RT_campaignBattleStep(RT_CampaignScene *scene,u8 tick,u8 refresh,
    RT_CampaignBattlePass *result)
{
    if(!scene || !scene->ready || !result ||
       (scene->players.home.players!=1 && scene->players.home.players!=2))return 0;
    RT_campaignBattlePassClear(result);
    RT_PlayerTargets targets={scene->players.agents,scene->players.combat,scene->players.home.players};
    if(!RT_terrainWindowMove(&scene->terrain,scene->stage,&scene->camera))return 0;
    u8 visibleObjects=0,remainingObjects=scene->objects.count;
    for(u8 i=0;i<RT_NATIVE_STAGE_CAPACITY && remainingObjects;i++){
        const RT_Anchor *object=scene->objects.objects+i;
        if(object->kind==255)continue;
        remainingObjects--;
        if(object->visible && object->kind<=3)visibleObjects++;
    }
    if(remainingObjects)return 0;
    if(RT_enemyTargetPlayer(&targets)!=255){
        u8 coop=targets.players==2;
        RT_EnemyPassContext context={.stage=scene->stage,.camera=&scene->camera,
            .terrain=&scene->terrain,.objects=&scene->objects,.players=targets,
            .projectiles=&scene->projectiles,.visibleObjects=visibleObjects,
            .decisionCounter=tick,.objectCounter=tick,
            .coopAttackPhases=(coop || scene->attackClockEnabled)?scene->attackPhases:NULL,
            .admissionDelays=coop?scene->population.admissionDelays:NULL,
            .throwMotion=scene->throwMotion,.visibleProjectiles=scene->visibleProjectiles,
            .type10Effects=scene->effects,.vblankPhase=refresh};
        if(!RT_enemiesUpdateWithHigh(&scene->enemies,&context,&result->enemies,
            scene->hasHighEnemies?RT_highFamiliesPoolPass:NULL))return 0;
        /* CPU2 CE9B increments the shared completion byte only after the
         * type10 death animation retires. Ordinary removal is not victory. */
        scene->bossProgress+=result->enemies.type10Deaths;
        BattleTargets battle={targets,{&scene->enemies,result->enemies.visible},result};
        RT_ProjectilePassContext projectiles={scene->stage,&scene->camera,
            {&scene->terrain,targets},scene->throwMotion,hit,&battle};
        if(!RT_projectilesUpdateAll(&scene->projectiles,&projectiles,&result->projectiles))return 0;
        scene->visibleProjectiles=result->projectiles.visible;
    }
    for(u8 p=0;p<targets.players;p++)
        RT_agentDamageTick(targets.agents+p,&scene->camera,targets.combat+p,0);
    return 1;
}
