#ifndef RT_NATIVE_CAMPAIGN_SCORE_H
#define RT_NATIVE_CAMPAIGN_SCORE_H
#include "rt_display.h"
#include "rt_enemy_pass.h"
/* Decimal score units represent10 displayed points, matching the arcade's
 * appended zero. No packed BCD or emulated memory layout at runtime. */
typedef struct {u32 high,players[2];u8 bonusIndex[2];} RT_CampaignScore;
typedef struct {u16 remaining;u8 divider;} RT_CampaignTimer;
typedef enum {RT_TIMER_INVALID,RT_TIMER_IDLE,RT_TIMER_CHANGED,RT_TIMER_EXPIRED} RT_TimerResult;
u8 RT_campaignTimerBegin(RT_CampaignTimer *timer,u8 setting);
/* CPU1 $9539 service: one logical gameplay tick,64 ticks per timer unit.
 * Paused while the player is in the scripted exit. EXPIRED requests the
 * source60 fatal selector; the caller owns reaction dispatch, not this clock. */
RT_TimerResult RT_campaignTimerStep(RT_CampaignTimer *timer,u8 exiting);
typedef enum {RT_SCORE_INVALID,RT_SCORE_ADDED,RT_SCORE_EXTRA_LIFE} RT_ScoreResult;
RT_ScoreResult RT_campaignAward(RT_CampaignScore *score,u8 player,u16 award,
    u16 secondaryAward,u8 setting,u8 *reserves);
/* Consume an ordered enemy phase exactly once, including a terminal update.
 * Converts immutable source award values at the event boundary. Validates the
 * whole batch before changing scores. Home supplies a zeroed temporary reserve
 * count and adds earned lives to its shared credit pool. */
u8 RT_campaignScoreEnemies(RT_CampaignScore *score,const RT_EnemyPassResult *pass,
    u8 players,u8 setting,u8 *reserves);
typedef struct {u16 remaining;u8 clock,ending,done;} RT_CampaignBonus;
enum {RT_BONUS_SOUND=1,RT_BONUS_EXTRA_LIFE_SOUND=2,RT_BONUS_SCORE_HUD=4,RT_BONUS_TIMER_HUD=8};
typedef enum {RT_BONUS_INVALID,RT_BONUS_WAIT,RT_BONUS_AWARDED,RT_BONUS_COMPLETE} RT_BonusResult;
u8 RT_campaignBonusBegin(RT_CampaignBonus *bonus,u16 timer,u8 ending,u8 clock);
/* Advance with physical refreshes, independently from skipped presentations. */
void RT_campaignBonusRefresh(RT_CampaignBonus *bonus,u32 elapsed);
/* One original bonus service:12score units per remaining timer unit. Caller
 * consumes event bits for sound/HUD, never loses the final award on COMPLETE. */
RT_BonusResult RT_campaignBonusStep(RT_CampaignBonus *bonus,RT_CampaignScore *score,
    u8 player,u8 setting,u8 *reserves,u8 *events);
#endif
