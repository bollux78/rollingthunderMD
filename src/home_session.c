#include "rt_home_session.h"
u8 RT_homeSessionInitializePlayers(RT_HomeSession *s,u16 credits,
    const RT_Agent agents[2],const RT_AgentCombat combat[2],u8 players,u8 soloRespawnHere)
{
    if(!s || !agents || !combat || (players!=1 && players!=2) || soloRespawnHere>1)return 0;
    s->credits=credits;s->previousStart=s->continueWaiting=0;
    s->players=players;s->soloRespawnHere=players==1 && soloRespawnHere;
    for(u8 i=0;i<2;i++){s->spawn[i]=agents[i];s->spawnHealth[i]=i<players?combat[i].health:0;}
    return 1;
}
void RT_homeSessionInitialize(RT_HomeSession *s,u16 credits,
    const RT_Agent agents[2],const RT_AgentCombat combat[2])
{(void)RT_homeSessionInitializePlayers(s,credits,agents,combat,2,0);}
void RT_homeProtectionTick(RT_AgentCombat combat[2])
{
    for(u8 i=0;i<2;i++)if(combat[i].invincibleTicks)combat[i].invincibleTicks--;
}
u8 RT_homeAgentVisible(const RT_AgentCombat *combat)
{
    return combat && combat->health && !combat->removed && !combat->hidden &&
        (!combat->invincibleTicks || !((combat->invincibleTicks/6)&1));
}
u8 RT_homeContinuePoll(RT_HomeSession *s,u8 deadMask,u8 startMask)
{
    if(!s || (s->players!=1 && s->players!=2))return 255;
    u8 mask=(1<<s->players)-1;
    u8 edge=(startMask&mask)&~s->previousStart;s->previousStart=startMask&mask;
    if(deadMask!=mask || !s->credits){s->continueWaiting=0;return 255;}
    if(!s->continueWaiting){s->continueWaiting=1;return 255;}
    return edge&1?0:edge&2?1:255;
}
u8 RT_homeRespawnAgent(RT_HomeSession *s,RT_Agent agents[2],
    RT_AgentCombat combat[2],u8 player,u8 deadMask,u8 continuing)
{
    if(!s || !agents || !combat || player>=s->players || (s->players!=1 && s->players!=2) || !s->credits ||
        (continuing?deadMask!=((1<<s->players)-1):deadMask!=(1<<player)))return 0;
    RT_Agent fresh=s->spawn[player];
    if(!continuing){
        if(s->players==1 && !s->soloRespawnHere)return 0;
        u8 anchor=s->players==1?player:player^1;
        fresh.body.x=agents[anchor].body.x;fresh.body.y=agents[anchor].body.y;
        fresh.body.layer=agents[anchor].body.layer;
    }
    fresh.ammoAlternate=0;fresh.weaponMode=0;fresh.shotRequest=0;
    fresh.fireLatch=fresh.jumpLatch=0;fresh.soundCount=0;
    agents[player]=fresh;combat[player]=(RT_AgentCombat){0};
    combat[player].health=s->spawnHealth[player];combat[player].invincibleTicks=180;
    s->credits--;if(continuing)s->continueWaiting=0;return 1;
}
u8 RT_homeResolveDeaths(RT_HomeSession *session,RT_Agent agents[2],
    RT_AgentCombat combat[2],u8 startMask)
{
    if(!session || !agents || !combat || (session->players!=1 && session->players!=2))return 0;
    u8 dead=0;
    for(u8 p=0;p<session->players;p++)if(combat[p].removed || !combat[p].health)dead|=1<<p;
    u8 choice=RT_homeContinuePoll(session,dead,startMask);
    if(session->players==1 && session->soloRespawnHere && dead==1)
        return RT_homeRespawnAgent(session,agents,combat,0,dead,0)?1:0;
    if(session->players==2 && (dead==1 || dead==2)){
        u8 player=dead==1?0:1;
        return RT_homeRespawnAgent(session,agents,combat,player,dead,0)?1<<player:0;
    }
    if(dead==((1<<session->players)-1) && choice<session->players)
        return RT_homeRespawnAgent(session,agents,combat,choice,dead,1)?1<<choice:0;
    return 0;
}

RT_HomePlayState RT_homePlayState(const RT_HomeSession *session,
    const RT_AgentCombat combat[2])
{
    if(!session || !combat || (session->players!=1 && session->players!=2))return RT_HOME_PLAY_INVALID;
    for(u8 i=0;i<session->players;i++)if(!combat[i].removed && combat[i].health)
        return RT_HOME_PLAY_ACTIVE;
    if(session->players==1 && session->soloRespawnHere && session->credits)return RT_HOME_PLAY_ACTIVE;
    return session->credits?RT_HOME_PLAY_CONTINUE:RT_HOME_PLAY_GAME_OVER;
}
