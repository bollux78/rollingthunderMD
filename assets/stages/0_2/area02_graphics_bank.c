#include "rt_campaign_area02.h"
#include "area02_rt_agent_poses.h"
#include "area02_rt_door_poses.h"
#include "area02_rt_fan_poses.h"
#include "area02_rt_stream_stage_poses.h"
#include "area02_rt_static_stage_poses.h"
#include "area02_rt_projectile_poses.h"
#include "area02_rt_throw_poses.h"
#include "area02_rt_enemy_atlas.h"
#include "area02_rt_high_enemy_graphics.h"
const RT_CampaignGraphics rtCampaignArea02Graphics={
 &area02_rtDisplayOpening,{area02_rtAgentPoses[0],area02_rtAgentPoses[1],area02_rtAgentPoses[2],area02_rtAgentPoses[3]},
 area02_rtDoorPoses,area02_rtFanPoses,area02_rtStreamStagePoses,area02_rtStaticStagePoses,area02_rtProjectilePoses,area02_rtThrowPoses,
 area02_rtDoorPatterns,area02_rtFanPatterns,area02_rtProjectilePatterns,area02_RT_enemyGraphic,area02_RT_highEnemyGraphic,
 area02_RT_AGENT_POSE_COUNT,area02_RT_DOOR_PATTERN_COUNT,area02_RT_FAN_PATTERN_COUNT,area02_RT_PROJECTILE_PATTERN_COUNT,
 20,area02_RT_THROW_FIRST_POSE,area02_RT_THROW_POSE_COUNT
};
