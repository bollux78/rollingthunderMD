#ifndef RT_NATIVE_HOME_SESSION_H
#define RT_NATIVE_HOME_SESSION_H
#include "rt_combat.h"
typedef struct {
    u16 credits;
    u8 previousStart,continueWaiting;
    RT_Agent spawn[2];
    u8 spawnHealth[2];
    u8 players,soloRespawnHere;
} RT_HomeSession;
/* Explicit session shape: unused player slots never affect lives or Continue.
 * soloRespawnHere implements Home Survival's current-section life respawn. */
u8 RT_homeSessionInitializePlayers(RT_HomeSession *session,u16 credits,
    const RT_Agent agents[2],const RT_AgentCombat combat[2],u8 players,u8 soloRespawnHere);
void RT_homeSessionInitialize(RT_HomeSession *session,u16 credits,
    const RT_Agent agents[2],const RT_AgentCombat combat[2]);
/* Call once after both players/combat update; result bits request controller
 * edge-baseline reset for respawned players. Health zero counts as dead. */
u8 RT_homeResolveDeaths(RT_HomeSession *session,RT_Agent agents[2],
    RT_AgentCombat combat[2],u8 startMask);
void RT_homeProtectionTick(RT_AgentCombat combat[2]);
u8 RT_homeAgentVisible(const RT_AgentCombat *combat);
u8 RT_homeContinuePoll(RT_HomeSession *session,u8 deadMask,u8 startMask);
/* Co-op: one dead respawns at the survivor; both dead require Continue.
 * Solo Survival respawns in place; other solo sessions require Continue.
 * Profile state is captured at stage entry; player identity/score stay external. */
u8 RT_homeRespawnAgent(RT_HomeSession *session,RT_Agent agents[2],
    RT_AgentCombat combat[2],u8 player,u8 deadMask,u8 continuing);
typedef enum {RT_HOME_PLAY_INVALID,RT_HOME_PLAY_ACTIVE,
    RT_HOME_PLAY_CONTINUE,RT_HOME_PLAY_GAME_OVER} RT_HomePlayState;
/* Read-only scene gate: when no player lives, stop world simulation and poll
 * Continue once per refresh. A Start edge is still resolved by the existing
 * credit policy; this query never spends credits or resurrects players. */
RT_HomePlayState RT_homePlayState(const RT_HomeSession *session,
    const RT_AgentCombat combat[2]);
typedef enum {RT_HOME_AGENT_INVALID,RT_HOME_AGENT_INACTIVE,RT_HOME_AGENT_NORMAL,
    RT_HOME_AGENT_CONTACT,RT_HOME_AGENT_HIT,RT_HOME_AGENT_REMOVED} RT_HomeAgentStep;
/* Same locked-camera native service for either Home player. Reaction precedes
 * exactly one active handler; terminal death retires the player. Scene owns
 * credit resolution after both players and damage have updated. */
RT_HomeAgentStep RT_homeStepAgent(RT_Agent *agent,RT_AgentCombat *combat,
    u8 input,const RT_WorldStage *stage,RT_WorldCamera *camera,
    RT_AnchorScene *scene,u8 *sound);
/* Solo uses source camera following (lockedCamera=0); Home co-op bounds each
 * player (1) and applies horizontal framing after both have moved. */
RT_HomeAgentStep RT_sceneStepAgent(RT_Agent *agent,RT_AgentCombat *combat,
    u8 input,const RT_WorldStage *stage,RT_WorldCamera *camera,
    RT_AnchorScene *scene,u8 lockedCamera,u8 *sound);
#endif
