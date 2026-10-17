#include "rt_campaign_area01.h"
#include "area01_rt_agent_poses.h"
#include "area01_rt_door_poses.h"
#include "area01_rt_fan_poses.h"
#include "area01_rt_stream_stage_poses.h"
#include "area01_rt_static_stage_poses.h"
#include "area01_rt_projectile_poses.h"
#include "area01_rt_throw_poses.h"
#include "area01_rt_enemy_atlas.h"
#include "area01_rt_high_enemy_graphics.h"
const RT_CampaignGraphics rtCampaignArea01Graphics={
 &area01_rtDisplayOpening,{area01_rtAgentPoses[0],area01_rtAgentPoses[1],area01_rtAgentPoses[2],area01_rtAgentPoses[3]},
 area01_rtDoorPoses,area01_rtFanPoses,area01_rtStreamStagePoses,area01_rtStaticStagePoses,area01_rtProjectilePoses,area01_rtThrowPoses,
 area01_rtDoorPatterns,area01_rtFanPatterns,area01_rtProjectilePatterns,area01_RT_enemyGraphic,area01_RT_highEnemyGraphic,
 area01_RT_AGENT_POSE_COUNT,area01_RT_DOOR_PATTERN_COUNT,area01_RT_FAN_PATTERN_COUNT,area01_RT_PROJECTILE_PATTERN_COUNT,
 20,area01_RT_THROW_FIRST_POSE,area01_RT_THROW_POSE_COUNT
};
