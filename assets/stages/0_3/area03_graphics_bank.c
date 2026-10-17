#include "rt_campaign_area03.h"
#include "area03_rt_agent_poses.h"
#include "area03_rt_door_poses.h"
#include "area03_rt_fan_poses.h"
#include "area03_rt_stream_stage_poses.h"
#include "area03_rt_static_stage_poses.h"
#include "area03_rt_projectile_poses.h"
#include "area03_rt_throw_poses.h"
#include "area03_rt_enemy_atlas.h"
#include "area03_rt_high_enemy_graphics.h"
const RT_CampaignGraphics rtCampaignArea03Graphics={
 &area03_rtDisplayOpening,{area03_rtAgentPoses[0],area03_rtAgentPoses[1],area03_rtAgentPoses[2],area03_rtAgentPoses[3]},
 area03_rtDoorPoses,area03_rtFanPoses,area03_rtStreamStagePoses,area03_rtStaticStagePoses,area03_rtProjectilePoses,area03_rtThrowPoses,
 area03_rtDoorPatterns,area03_rtFanPatterns,area03_rtProjectilePatterns,area03_RT_enemyGraphic,area03_RT_highEnemyGraphic,
 area03_RT_AGENT_POSE_COUNT,area03_RT_DOOR_PATTERN_COUNT,area03_RT_FAN_PATTERN_COUNT,area03_RT_PROJECTILE_PATTERN_COUNT,
 20,area03_RT_THROW_FIRST_POSE,area03_RT_THROW_POSE_COUNT
};
