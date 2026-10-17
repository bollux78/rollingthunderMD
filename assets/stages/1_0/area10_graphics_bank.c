#include "rt_campaign_area10.h"
#include "area10_rt_agent_poses.h"
#include "area10_rt_door_poses.h"
#include "area10_rt_fan_poses.h"
#include "area10_rt_stream_stage_poses.h"
#include "area10_rt_static_stage_poses.h"
#include "area10_rt_projectile_poses.h"
#include "area10_rt_throw_poses.h"
#include "area10_rt_enemy_atlas.h"
#include "area10_rt_high_enemy_graphics.h"
const RT_CampaignGraphics rtCampaignArea10Graphics={
 &area10_rtDisplayOpening,{area10_rtAgentPoses[0],area10_rtAgentPoses[1],area10_rtAgentPoses[2],area10_rtAgentPoses[3]},
 area10_rtDoorPoses,area10_rtFanPoses,area10_rtStreamStagePoses,area10_rtStaticStagePoses,area10_rtProjectilePoses,area10_rtThrowPoses,
 area10_rtDoorPatterns,area10_rtFanPatterns,area10_rtProjectilePatterns,area10_RT_enemyGraphic,area10_RT_highEnemyGraphic,
 area10_RT_AGENT_POSE_COUNT,area10_RT_DOOR_PATTERN_COUNT,area10_RT_FAN_PATTERN_COUNT,area10_RT_PROJECTILE_PATTERN_COUNT,
 20,area10_RT_THROW_FIRST_POSE,area10_RT_THROW_POSE_COUNT
};
