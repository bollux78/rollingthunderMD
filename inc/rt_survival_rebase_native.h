#ifndef RT_NATIVE_SURVIVAL_REBASE_H
#define RT_NATIVE_SURVIVAL_REBASE_H
#include "rt_campaign_scene.h"
/* Coordinate-only part of a Survival rebase. It does not retire chunks.
 * Owner must establish retention for every terrain/spawn/display consumer,
 * finish queued DMA/events, and update safe Continue spawn snapshots before
 * retiring their original section. Use preflight before publishing any part
 * of the combined world/display/scene operation. No callbacks occur here. */
u8 RT_survivalSceneCanShift512(const RT_CampaignScene *scene);
/* Atomic on failure. Shifts configured players, restart snapshots, live
 * anchors/enemies/projectiles, camera, collision-window camera and event-grid
 * centers. Slot links, velocities, fractions, timers and collision-ring cells
 * remain unchanged. Retired co-op player records are not spatial consumers:
 * respawn uses the survivor, so their obsolete coordinates stay untouched.
 * 512px is exactly the collision/display ring X period. */
u8 RT_survivalSceneShift512(RT_CampaignScene *scene);
#endif
