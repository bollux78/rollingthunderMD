#ifndef RT_NATIVE_SCENE_PLAYERS_H
#define RT_NATIVE_SCENE_PLAYERS_H
#include "rt_home_mode.h"
#include "rt_home_session.h"
typedef struct {
    RT_SessionConfig selection;
    RT_Agent agents[2];
    RT_AgentCombat combat[2];
    RT_HomeSession home;
    u16 previousButtons[2];
} RT_ScenePlayers;
/* New game/stage entry, not a diagnostic placement. Uses the authored camera
 * entrance and source 84A7 player template. Home P2 shares that safe entrance.
 * Renderer consumes selection.agent/costume; physics is shared. Invalid input
 * leaves outputs unchanged. Lives/credit count is owned by the application.
 * This does not construct terrain, enemies, Survival chunks or Arcade credits. */
u8 RT_scenePlayersInitialize(RT_ScenePlayers *players,
    const RT_SessionConfig *selection,const RT_WorldStage *stage,
    u8 checkpoint,u16 credits,RT_WorldCamera *camera);
#endif
