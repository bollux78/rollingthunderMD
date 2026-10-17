#include "rt_campaign_area04.h"
#include "area04_rt_agent_poses.h"
#include "area04_rt_door_poses.h"
#include "area04_rt_fan_poses.h"
#include "area04_rt_stream_stage_poses.h"
#include "area04_rt_static_stage_poses.h"
#include "area04_rt_projectile_poses.h"
#include "area04_rt_throw_poses.h"
#include "area04_rt_enemy_atlas.h"
#include "area04_rt_high_enemy_graphics.h"
const RT_CampaignGraphics rtCampaignArea04Graphics={
 &area04_rtDisplayOpening,{area04_rtAgentPoses[0],area04_rtAgentPoses[1],area04_rtAgentPoses[2],area04_rtAgentPoses[3]},
 area04_rtDoorPoses,area04_rtFanPoses,area04_rtStreamStagePoses,area04_rtStaticStagePoses,area04_rtProjectilePoses,area04_rtThrowPoses,
 area04_rtDoorPatterns,area04_rtFanPatterns,area04_rtProjectilePatterns,area04_RT_enemyGraphic,area04_RT_highEnemyGraphic,
 area04_RT_AGENT_POSE_COUNT,area04_RT_DOOR_PATTERN_COUNT,area04_RT_FAN_PATTERN_COUNT,area04_RT_PROJECTILE_PATTERN_COUNT,
 20,area04_RT_THROW_FIRST_POSE,area04_RT_THROW_POSE_COUNT
};
