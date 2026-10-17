#include "rt_scene_players.h"
u8 RT_scenePlayersInitialize(RT_ScenePlayers *players,
    const RT_SessionConfig *selection,const RT_WorldStage *stage,
    u8 checkpoint,u16 credits,RT_WorldCamera *camera)
{
    /* Source 84A7 gameplay templates: 0600/0100 layer4, or
     * 0600/0780 layer2, indexed by story/area/checkpoint. */
    static const u8 upper[2][10]={
        {0,1,0,1,0,0,0,0,0,0},
        {0,1,0,0,0,0,0,0,0,0}
    };
    if(!players || !selection || !stage || !camera || checkpoint>1 ||
       stage->story>1 || stage->area>4 || selection->mode>RT_MODE_HOME ||
       selection->game>RT_HOME_SURVIVAL || selection->players>RT_HOME_TWO_PLAYERS ||
       selection->agent[0]>RT_AGENT_LEILA || selection->agent[1]>RT_AGENT_LEILA ||
       selection->costume[0]>1 || selection->costume[1]>1)return 0;
    RT_ScenePlayers next={0};
    next.selection=*selection;
    u8 count=selection->mode==RT_MODE_HOME && selection->players==RT_HOME_TWO_PLAYERS?2:1;
    if(selection->mode==RT_MODE_ARCADE){
        next.selection.players=RT_HOME_SOLO;next.selection.game=RT_HOME_CAMPAIGN;
        next.selection.agent[0]=RT_AGENT_ALBATROSS;next.selection.costume[0]=0;
    }
    RT_WorldCamera entry=stage->entrances[checkpoint];
    u8 raised=upper[stage->story][stage->area*2+checkpoint];
    for(u8 p=0;p<count;p++){
        RT_Agent *a=next.agents+p;
        a->body.x=(s32)entry.x*16+0x600;
        a->body.y=((s32)entry.y+256)*16-(raised?0x780:0x100);
        a->body.gravity=8;a->body.layer=raised?2:4;
        a->ammoPrimary=count==2?75:50;next.combat[p].health=64;
        if(!RT_agentSelect(a,(RT_AgentCommand){RT_AGENT_IDLE,1},stage,&entry))return 0;
        /* Require a release before accepting the menu's held confirmation. */
        next.previousButtons[p]=0xFFFF;
    }
    if(!RT_homeSessionInitializePlayers(&next.home,credits,next.agents,next.combat,
        count,selection->mode==RT_MODE_HOME && selection->game==RT_HOME_SURVIVAL))return 0;
    *players=next;*camera=entry;return 1;
}
