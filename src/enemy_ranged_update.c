#include "rt_enemies.h"
RT_EnemyResult RT_enemyRangedUpdate(RT_Enemy *enemy,RT_ProjectilePool *pool,
    const RT_WorldCamera *camera,u8 workspaceFlags,RT_WeaponEvent *event)
{
    if(!enemy || !pool || !camera || !event)return RT_ENEMY_UNAVAILABLE;
    event->sound=event->command=0;
    u8 shot,selector,low=(enemy->state.action&0xfc)==0x1c;
    RT_EnemyResult result=RT_enemyRangedStep(enemy,workspaceFlags,&shot,&selector);
    if(shot)RT_enemyProjectileRequest(pool,enemy,low,event,0);
    if(result==RT_ENEMY_SELECT && !RT_enemySelect(enemy,camera,selector,workspaceFlags))
        return RT_ENEMY_UNAVAILABLE;
    return result;
}
