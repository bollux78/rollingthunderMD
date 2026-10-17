#include "rt_survival_spawns_native.h"
typedef struct {RT_EnemyAdmission admission;u8 densityTier;} SurvivalAdmission;
static u8 admit(const RT_EnemySpawn *spawn,u16 index,void *context)
{
    SurvivalAdmission *s=context;
    u8 result=RT_enemyAdmissionVisit(spawn,index,&s->admission);
    if(result!=RT_ENEMY_SPAWN_ACCEPTED)return result;
    /* Quarter-step density growth, using only authored constructor locations.
     * Co-op's existing additional guard policy applies to both admissions,
     * retaining its1.5x target over the Survival base. Pool limits win. */
    if((index&3)<s->densityTier && s->admission.population<40 &&
       s->admission.pool->count<RT_ENEMY_CAPACITY){
        result=RT_enemyAdmissionVisit(spawn,index,&s->admission);
        if(result==RT_ENEMY_SPAWN_ERROR)return result;
    }
    return RT_ENEMY_SPAWN_ACCEPTED;
}
u8 RT_survivalSpawnObjects(const RT_SurvivalWorld *world,RT_StagePool *pool,
    const RT_EventWindowRequest *request)
{
    if(!pool || !pool->level || pool->count>RT_NATIVE_STAGE_CAPACITY)return 0;
    return RT_survivalVisitStageEvents(world,pool->level,request,&pool->count,
        RT_stagePoolEventVisitor,pool);
}
u8 RT_survivalSpawnEnemies(const RT_SurvivalWorld *world,RT_EnemyPool *pool,
    const RT_EnemySpawnLevel *source,const RT_EventWindowRequest *request,
    u16 *fault,RT_CoopPopulation *policy,RT_Type10Effects *effects,u8 highFamilies)
{
    if(fault)*fault=65535;
    if(!pool || pool->count>RT_ENEMY_CAPACITY)return 0;
    if(!world || world->densityTier>4)return 0;
    /* Add25% scheduled constructors per16 retired256px sections, capped at
     * double density to respect the fixed native actor pool. No speed/damage
     * changes. This is density only; attack-rate progression is separate. */
    u8 tier=world->densityTier;
    SurvivalAdmission admission={{pool,pool->count,fault,policy,effects,highFamilies},tier};
    return RT_survivalVisitEnemySpawns(world,source,request,&admission.admission.population,
        admit,&admission);
}
