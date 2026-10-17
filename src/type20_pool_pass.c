#include "rt_enemy_pass.h"
#include "rt_type20.h"
u8 RT_type20PoolPass(RT_EnemyPool *pool,const RT_EnemyPassContext *c,RT_EnemyPassResult *out)
{
    if(!pool || !c || !out || !c->players.agents || !c->players.combat || !c->stage ||
        out->targetPlayer>=c->players.players || !c->camera || !c->terrain || pool->count>RT_ENEMY_CAPACITY)return 0;
    RT_Agent *target=c->players.agents+out->targetPlayer;
    RT_EnemyPerceptionTarget perception;RT_enemyPreparePerception(target,&perception);
    RT_EnemyTerrainContext probes={0,c->stage,c->camera,c->terrain};
    RT_Type20Motion motion={.camera=c->camera,.probe=RT_enemyTerrainProbeReady,
        .probeContext=&probes,.shadowXLow=(u8)(c->camera->x*16),
        .shadowYLow=(u8)(0-c->camera->y*16),.targetX=target->body.x};
    for(u8 i=0;i<RT_ENEMY_CAPACITY;i++){
        RT_Enemy *e=pool->slots+i;
        if(e->state.kind==RT_ENEMY_FREE || e->state.kind<32)continue;
        out->faultSlot=i;out->faultAction=e->state.action;
        if(e->state.kind!=32 || !e->definition)return 0;
        RT_EnemyPassEvent event={.slot=i,.player=pool->scoreOwner[i]};u8 counted=0;
        probes.enemy=e;motion.accelerationY=e->definition->accelerationY;
        motion.sound=&event.sound;motion.award=&event.scoreBCD;
        if(c->actionCoverage){u8 state=e->state.action&0xfc;c->actionCoverage[state>>7]|=1UL<<((state>>2)&31);}
        RT_EnemyResult result=RT_type20Prepare(e,&motion,&counted);
        if(result==RT_ENEMY_UNAVAILABLE || !RT_enemyPassFlush(out,&event))return 0;
        if(!e->state.hidden){
            if(c->admissionDelays && c->admissionDelays[i]){
                c->admissionDelays[i]--;e->state.hidden=1;counted=0;
            }else{
                if(!RT_highEnemyPerceivePrepared(e,&perception,c->players.combat+out->targetPlayer,&event.command))return 0;
                if(!RT_enemyPassFlush(out,&event))return 0;
                result=RT_type20Step(e,&motion);
                if(result==RT_ENEMY_UNAVAILABLE)return 0;
                if(result==RT_ENEMY_REMOVED){
                    if(!pool->count)return 0;
                    pool->count--;out->removed++;e->state.hidden=1;counted=0;
                }
            }
        }
        out->visible+=counted;
        if(!RT_enemyPassFlush(out,&event))return 0;
    }
    return 1;
}
