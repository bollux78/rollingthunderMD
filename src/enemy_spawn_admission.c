#include "rt_enemy_spawns.h"
typedef RT_EnemyAdmission Admission;
u8 RT_enemyAdmissionVisit(const RT_EnemySpawn *spawn,u16 index,void *context)
{
    Admission *a=context;
    if(a->pool->count>=RT_ENEMY_CAPACITY)return RT_ENEMY_SPAWN_FULL;
    u8 slot;
    if(!RT_enemiesSpawnGridWithFamilies(a->pool,spawn,&slot,a->effects,a->highFamilies)){
        if(a->fault)*a->fault=index;
        return RT_ENEMY_SPAWN_ERROR;
    }
    a->population++;
    if(a->coop){
        a->coop->admissionDelays[slot]=0;
        a->coop->originals++;a->coop->phase^=1;
        if(!a->coop->phase){
            if(a->pool->count>=RT_ENEMY_CAPACITY || a->population>=40)a->coop->capacityMisses++;
            else{
                /* Reuse the authored location and definition; never place an
                 * extra guard across unverified terrain or on another floor. */
                if(!RT_enemiesSpawnGridWithFamilies(a->pool,spawn,&slot,a->effects,a->highFamilies))return RT_ENEMY_SPAWN_ERROR;
                a->coop->admissionDelays[slot]=12;
                a->population++;a->coop->extras++;
            }
        }
    }
    return RT_ENEMY_SPAWN_ACCEPTED;
}
u8 RT_enemiesSpawnWindowWithType10(RT_EnemyPool *pool,const RT_EnemySpawnLevel *level,
    const RT_EventWindowRequest *request,u8 highPopulation,u16 *fault,RT_CoopPopulation *policy,RT_Type10Effects *effects)
{
    if(fault)*fault=65535;
    if(!pool || !request || pool->count>RT_ENEMY_CAPACITY || highPopulation>48)return 0;
    Admission admission={pool,(u8)(pool->count+highPopulation),fault,policy,effects,0};
    return RT_visitEnemySpawns(level,request->gridX,request->gridY,
        (RT_EventWindowEdge)request->edge,&admission.population,RT_enemyAdmissionVisit,&admission);
}

u8 RT_enemiesSpawnWindowCoop(RT_EnemyPool *pool,const RT_EnemySpawnLevel *level,
    const RT_EventWindowRequest *request,u8 highPopulation,u16 *fault,RT_CoopPopulation *policy)
{
    return RT_enemiesSpawnWindowWithType10(pool,level,request,highPopulation,fault,policy,0);
}

u8 RT_enemiesSpawnWindow(RT_EnemyPool *pool,const RT_EnemySpawnLevel *level,
    const RT_EventWindowRequest *request,u8 highPopulation,u16 *fault)
{
    return RT_enemiesSpawnWindowCoop(pool,level,request,highPopulation,fault,0);
}

u8 RT_enemiesSpawnWindowWithFamilies(RT_EnemyPool *pool,const RT_EnemySpawnLevel *level,
    const RT_EventWindowRequest *request,u16 *fault,RT_CoopPopulation *policy,
    RT_Type10Effects *effects,u8 highFamilies)
{
    if(fault)*fault=65535;
    if(!pool || !request || pool->count>RT_ENEMY_CAPACITY)return 0;
    Admission admission={pool,pool->count,fault,policy,effects,highFamilies};
    return RT_visitEnemySpawns(level,request->gridX,request->gridY,
        (RT_EventWindowEdge)request->edge,&admission.population,RT_enemyAdmissionVisit,&admission);
}
