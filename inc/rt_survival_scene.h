#ifndef RT_NATIVE_SURVIVAL_SCENE_H
#define RT_NATIVE_SURVIVAL_SCENE_H
#include "rt_survival_spawns_native.h"
#include "rt_campaign_scene.h"
/* Native generated entry. Storage for scene/window/world must remain stable.
 * Invalid selection preserves all outputs; constructor failures leave ready0.
 * Countdown is disabled: service this with a Survival loop, not Campaign's
 * timeout/exit handler. This does not initialize VDP, music or menu state. */
RT_CampaignInit RT_survivalSceneInitialize(RT_CampaignScene *scene,
    RT_SurvivalWindow *window,RT_SurvivalWorld *world,const RT_SessionConfig *selection,
    u32 seed,u8 enemySet,u16 credits);
/* After Home death resolution has charged exactly the player in paidMask,
 * rebuild live pools at the moving checkpoint, preserving generated terrain,
 * selection and remaining credits. Other dead players still need a credit.
 * Caller drains old graphics/events first and reinitializes the view after.
 * Invalid arguments preserve scene; construction faults leave ready0. */
RT_CampaignInit RT_survivalSceneContinue(RT_CampaignScene *scene,
    const RT_SurvivalWorld *world,u8 paidMask,const u16 pads[2]);
/* After ordinary player/battle/object services: source-order culling and
 * generated-strip admission. World bounds constrain native camera movement;
 * owner retires chunks after graphics publication. */
u8 RT_survivalSceneScroll(RT_CampaignScene *scene,const RT_SurvivalWorld *world,
    const RT_WorldCamera *previous);
/* visibilityReady requires a successful ObjectsStep at the same camera with
 * unchanged object membership, positions and visibility since that step. */
u8 RT_survivalSceneScrollPrepared(RT_CampaignScene *scene,const RT_SurvivalWorld *world,const RT_WorldCamera *previous,u8 visibilityReady);
#endif
