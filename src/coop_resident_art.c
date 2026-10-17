#include "rt_coop_resident_art.h"
static RT_SpriteSlot resident[10];
static u8 ready,stage,clearTicks;
volatile u8 rtCoopCompact;
void RT_coopDetailPressure(void){rtCoopCompact=1;clearTicks=0;}
void RT_coopDetailUpdate(u8 players,u8 enemies,u8 projectiles,u8 late)
{
    if(players!=2){rtCoopCompact=clearTicks=0;return;}
    /* Enter immediately; restore after half a second of light simulation.
     * The gap prevents rapid toggling at the crowd boundary. */
    if(enemies>=6 || projectiles>=8 || late){RT_coopDetailPressure();return;}
    if(rtCoopCompact){
        if(enemies<=3 && projectiles<=4){if(++clearTicks>=30)rtCoopCompact=clearTicks=0;}
        else clearTicks=0;
    }
}
u8 RT_coopResidentBegin(RT_CampaignSprites *s,u8 players)
{
    ready=0;rtCoopCompact=clearTicks=0;
    if(players!=2)return 1;
    if(!s || !s->ready || !s->graphics || !s->graphics->stage)return 0;
    const RT_DisplayStage *d=s->graphics->stage;
    if(d->story>1 || d->area>=5 || s->arena.blocks<=RT_COOP_RESIDENT_TILES/16)return 0;
    stage=d->story*5+d->area;
    u16 first=s->arena.firstTile;
    /* Immutable guard atlas owns a disjoint VRAM range until the next stage.
     * Loaded only while display is disabled: compact guard poses need no DMA.
     * Full-detail poses use the dynamic arena. Tightly cropped resident pieces
     * may share tile runs; their per-pose spans need not fit a dynamic slot.
     * Other actors/stage objects retain the existing dynamic upload policy. */
    if(!RT_spriteArenaInitialize(&s->arena,first+RT_COOP_RESIDENT_TILES,
        s->arena.blocks-RT_COOP_RESIDENT_TILES/16))return 0;
    VDP_loadTileData(rtCoopPatterns[stage],first,RT_COOP_RESIDENT_TILES,CPU);
    for(u8 i=0;i<10;i++)resident[i]=(RT_SpriteSlot){rtCoopArt[stage][i].pose,
        first+rtCoopOffsets[stage][i],rtCoopArt[stage][i].pose->tileCount};
    ready=1;return 1;
}
u8 RT_coopResidentIndex(const RT_Enemy *e)
{
    if(!ready || (e->state.kind!=4 && e->state.kind!=8 && e->state.kind!=12))return 255;
    u8 state=e->state.action&0xfc,index=0;
    if(state==0x60 || state==0x64 || state==0x68)index=4;
    else if(state==0x1c || (e->state.pose>=199 && e->state.pose<=203))index=3;
    else if(state==0x18)index=2;
    else if(e->state.pose>=91 && e->state.pose<=96)index=(e->state.pose-91)>=3;
    return index+(e->state.kind==8?5:0);
}
const RT_PackedSpritePose *RT_coopResidentGraphic(u8 index){return rtCoopArt[stage]+index;}
RT_SpriteSlot *RT_coopResidentSlot(u8 index){return resident+index;}
