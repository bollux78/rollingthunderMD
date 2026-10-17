#include "rt_campaign_area12.h"
#include "area12_rt_agent_poses.h"
#include "area12_rt_door_poses.h"
#include "area12_rt_fan_poses.h"
#include "area12_rt_stream_stage_poses.h"
#include "area12_rt_static_stage_poses.h"
#include "area12_rt_projectile_poses.h"
#include "area12_rt_throw_poses.h"
#include "area12_rt_enemy_atlas.h"
#include "area12_rt_high_enemy_graphics.h"
const RT_CampaignGraphics rtCampaignArea12Graphics={
 &area12_rtDisplayOpening,{area12_rtAgentPoses[0],area12_rtAgentPoses[1],area12_rtAgentPoses[2],area12_rtAgentPoses[3]},
 area12_rtDoorPoses,area12_rtFanPoses,area12_rtStreamStagePoses,area12_rtStaticStagePoses,area12_rtProjectilePoses,area12_rtThrowPoses,
 area12_rtDoorPatterns,area12_rtFanPatterns,area12_rtProjectilePatterns,area12_RT_enemyGraphic,area12_RT_highEnemyGraphic,
 area12_RT_AGENT_POSE_COUNT,area12_RT_DOOR_PATTERN_COUNT,area12_RT_FAN_PATTERN_COUNT,area12_RT_PROJECTILE_PATTERN_COUNT,
 20,area12_RT_THROW_FIRST_POSE,area12_RT_THROW_POSE_COUNT
};
