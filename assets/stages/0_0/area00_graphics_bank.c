#include "rt_campaign_area00.h"
#include "area00_rt_agent_poses.h"
#include "area00_rt_door_poses.h"
#include "area00_rt_fan_poses.h"
#include "area00_rt_stream_stage_poses.h"
#include "area00_rt_static_stage_poses.h"
#include "area00_rt_projectile_poses.h"
#include "area00_rt_throw_poses.h"
#include "area00_rt_enemy_atlas.h"
#include "area00_rt_high_enemy_graphics.h"
const RT_CampaignGraphics rtCampaignArea00Graphics={
 &area00_rtDisplayOpening,{area00_rtAgentPoses[0],area00_rtAgentPoses[1],area00_rtAgentPoses[2],area00_rtAgentPoses[3]},
 area00_rtDoorPoses,area00_rtFanPoses,area00_rtStreamStagePoses,area00_rtStaticStagePoses,area00_rtProjectilePoses,area00_rtThrowPoses,
 area00_rtDoorPatterns,area00_rtFanPatterns,area00_rtProjectilePatterns,area00_RT_enemyGraphic,area00_RT_highEnemyGraphic,
 area00_RT_AGENT_POSE_COUNT,area00_RT_DOOR_PATTERN_COUNT,area00_RT_FAN_PATTERN_COUNT,area00_RT_PROJECTILE_PATTERN_COUNT,
 20,area00_RT_THROW_FIRST_POSE,area00_RT_THROW_POSE_COUNT
};
