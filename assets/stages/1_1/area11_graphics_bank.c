#include "rt_campaign_area11.h"
#include "area11_rt_agent_poses.h"
#include "area11_rt_door_poses.h"
#include "area11_rt_fan_poses.h"
#include "area11_rt_stream_stage_poses.h"
#include "area11_rt_static_stage_poses.h"
#include "area11_rt_projectile_poses.h"
#include "area11_rt_throw_poses.h"
#include "area11_rt_enemy_atlas.h"
#include "area11_rt_high_enemy_graphics.h"
const RT_CampaignGraphics rtCampaignArea11Graphics={
 &area11_rtDisplayOpening,{area11_rtAgentPoses[0],area11_rtAgentPoses[1],area11_rtAgentPoses[2],area11_rtAgentPoses[3]},
 area11_rtDoorPoses,area11_rtFanPoses,area11_rtStreamStagePoses,area11_rtStaticStagePoses,area11_rtProjectilePoses,area11_rtThrowPoses,
 area11_rtDoorPatterns,area11_rtFanPatterns,area11_rtProjectilePatterns,area11_RT_enemyGraphic,area11_RT_highEnemyGraphic,
 area11_RT_AGENT_POSE_COUNT,area11_RT_DOOR_PATTERN_COUNT,area11_RT_FAN_PATTERN_COUNT,area11_RT_PROJECTILE_PATTERN_COUNT,
 20,area11_RT_THROW_FIRST_POSE,area11_RT_THROW_POSE_COUNT
};
