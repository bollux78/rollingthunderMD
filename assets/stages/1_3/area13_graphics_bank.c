#include "rt_campaign_area13.h"
#include "area13_rt_agent_poses.h"
#include "area13_rt_door_poses.h"
#include "area13_rt_fan_poses.h"
#include "area13_rt_stream_stage_poses.h"
#include "area13_rt_static_stage_poses.h"
#include "area13_rt_projectile_poses.h"
#include "area13_rt_throw_poses.h"
#include "area13_rt_enemy_atlas.h"
#include "area13_rt_high_enemy_graphics.h"
const RT_CampaignGraphics rtCampaignArea13Graphics={
 &area13_rtDisplayOpening,{area13_rtAgentPoses[0],area13_rtAgentPoses[1],area13_rtAgentPoses[2],area13_rtAgentPoses[3]},
 area13_rtDoorPoses,area13_rtFanPoses,area13_rtStreamStagePoses,area13_rtStaticStagePoses,area13_rtProjectilePoses,area13_rtThrowPoses,
 area13_rtDoorPatterns,area13_rtFanPatterns,area13_rtProjectilePatterns,area13_RT_enemyGraphic,area13_RT_highEnemyGraphic,
 area13_RT_AGENT_POSE_COUNT,area13_RT_DOOR_PATTERN_COUNT,area13_RT_FAN_PATTERN_COUNT,area13_RT_PROJECTILE_PATTERN_COUNT,
 20,area13_RT_THROW_FIRST_POSE,area13_RT_THROW_POSE_COUNT
};
