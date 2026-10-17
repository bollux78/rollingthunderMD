#include "rt_campaign_area14.h"
#include "area14_rt_agent_poses.h"
#include "area14_rt_door_poses.h"
#include "area14_rt_fan_poses.h"
#include "area14_rt_stream_stage_poses.h"
#include "area14_rt_static_stage_poses.h"
#include "area14_rt_projectile_poses.h"
#include "area14_rt_throw_poses.h"
#include "area14_rt_enemy_atlas.h"
#include "area14_rt_high_enemy_graphics.h"
const RT_CampaignGraphics rtCampaignArea14Graphics={
 &area14_rtDisplayOpening,{area14_rtAgentPoses[0],area14_rtAgentPoses[1],area14_rtAgentPoses[2],area14_rtAgentPoses[3]},
 area14_rtDoorPoses,area14_rtFanPoses,area14_rtStreamStagePoses,area14_rtStaticStagePoses,area14_rtProjectilePoses,area14_rtThrowPoses,
 area14_rtDoorPatterns,area14_rtFanPatterns,area14_rtProjectilePatterns,area14_RT_enemyGraphic,area14_RT_highEnemyGraphic,
 area14_RT_AGENT_POSE_COUNT,area14_RT_DOOR_PATTERN_COUNT,area14_RT_FAN_PATTERN_COUNT,area14_RT_PROJECTILE_PATTERN_COUNT,
 20,area14_RT_THROW_FIRST_POSE,area14_RT_THROW_POSE_COUNT
};
