#include "rt_combat.h"
/* BCDA preserves a distinct player record13 gate; do not use the low-family
 * close-attack collision routine here. counterattack is native record13. */
u8 RT_highEnemyPerceivePrepared(RT_Enemy *e,const RT_EnemyPerceptionTarget *target,
    RT_AgentCombat *combat,u8 *command)
{
    if(!e || !target || !combat || !command || e->state.kind<32 || e->state.kind>44)
        return 0;
    RT_enemyPerceiveGeometryPrepared(e,target);
    if(!(e->state.decisions&0x40) || combat->counterattack)return 1;
    if(((target->flags|e->state.flags)&0x80) || combat->invincibleTicks || combat->hidden || combat->removed){
        e->state.decisions&=0xfe;return 1;
    }
    combat->damagePending+=0x20;
    if((u8)(combat->damagePending+combat->priorityDamage)>=combat->health)
        combat->reaction=RT_REACTION_CLOSE_FATAL;
    else{
        *command=2;combat->counterattack=e->state.decisions;
        combat->reaction=RT_REACTION_CLOSE_HURT;
    }
    return 1;
}
