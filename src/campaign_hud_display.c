#include "rt_campaign_hud_display.h"
extern VoidCallback *vintCB;
static VoidCallback *savedVInt;
static RT_CampaignHudDisplay *volatile owner;
#ifdef RT_HUD_TRACE
volatile u16 rtHudHLine,rtHudVLine,rtHudHLines[256];
#endif
static HINTERRUPT_CALLBACK hudHInt(void)
{
    if(!owner)return;
#ifdef RT_HUD_TRACE
    rtHudHLine=GET_VCOUNTER;
#endif
    /* Enabling HInt in VInt can fire a latched countdown as soon as VInt
     * returns; if that is after vblank, it lands above the real split line.
     * Ignore it: the counter still fires at line 23. */
    u16 line=GET_VCOUNTER;
    if(line>=224 || line<16)return;
#ifdef RT_HUD_TRACE
    rtHudHLines[vtimer&255]=GET_VCOUNTER+1;
#endif
    VDP_setWindowVPos(TRUE,27);VDP_setHInterrupt(FALSE);
}
static void hudVInt(void)
{
    if(savedVInt)savedVInt();
    if(!owner)return;
#ifdef RT_HUD_TRACE
    rtHudVLine=GET_VCOUNTER;
#endif
    VDP_setWindowVPos(FALSE,3);VDP_setHIntCounter(23);VDP_setHInterrupt(TRUE);
}
static u16 attribute(const RT_CampaignHudArt *art,RT_HudCell cell,u16 first)
{
    for(u8 i=0;i<art->groupCount;i++)if(art->groups[i]==cell.group){
        u16 value=art->attributes[i][cell.glyph];
        if(value==65535 || (value&2047)>=art->patternCount)return 65535;
        return value+first;
    }
    return 65535;
}
u8 RT_campaignHudDisplayBegin(RT_CampaignHudDisplay *d,const RT_CampaignHudArt *art,u16 first)
{
    if(!d || owner || !art || !art->patterns || !art->attributes || !art->initial ||
       !art->groups || !art->groupCount || !art->patternCount ||
       first>=1536 || art->patternCount>1536-first || DMA_getQueueSize() ||
       (VDP_getReg(1)&0x40) || (VDP_getReg(0)&0x10))return 0;
    d->ready=0;
    for(u16 i=0;i<144;i++)if(attribute(art,art->initial[i],first)==65535)return 0;
    u16 blank=attribute(art,(RT_HudCell){255,3},first);if(blank==65535)return 0;
    VDP_loadTileData(art->patterns,first,art->patternCount,CPU);
    u16 row[40];
    for(u8 r=0;r<4;r++){
        for(u8 x=0;x<40;x++)row[x]=x<2 || x>=38?blank:attribute(art,art->initial[r*36+x-2],first);
        DMA_transfer(CPU,DMA_VRAM,row,0xd000+(r==3?27:r)*128,40,2);
    }
    d->art=art;d->firstTile=first;memcpy(d->previous.cells,art->initial,sizeof d->previous);
    VDP_setWindowAddress(0xd000);VDP_setWindowHPos(FALSE,0);VDP_setWindowVPos(FALSE,3);
    SYS_disableInts();savedVInt=vintCB;owner=d;
    SYS_setVIntCallback(hudVInt);SYS_setHIntCallback(hudHInt);d->ready=1;
    SYS_enableInts();return 1;
}
u8 RT_campaignHudDisplayQueue(RT_CampaignHudDisplay *d,const RT_CampaignHud *hud)
{return RT_campaignHudDisplayQueueRows(d,hud,15);}
u8 RT_campaignHudDisplayQueueRows(RT_CampaignHudDisplay *d,const RT_CampaignHud *hud,u8 dirtyRows)
{
    if(!d || !hud || owner!=d || !d->ready || dirtyRows>15)return 0;
    if(!dirtyRows)return 1;
    u16 rows[144];
    u8 starts[4],ends[4];
    for(u8 r=0;r<4;r++){
        starts[r]=36;ends[r]=0;
        if(!(dirtyRows&(1<<r)))continue;
        u8 first=36,last=0;
        for(u8 x=0;x<36;x++){
            u16 i=r*36+x;
            if(hud->cells[i].glyph!=d->previous.cells[i].glyph || hud->cells[i].group!=d->previous.cells[i].group){
                if(first==36)first=x;last=x+1;
            }
        }
        starts[r]=first;ends[r]=last;
        for(u8 x=first;x<last;x++){
            u16 i=r*36+x;
            rows[i]=attribute(d->art,hud->cells[i],d->firstTile);
            if(rows[i]==65535)return 0;
        }
    }
    /* Validate every changed span before enqueueing any DMA. Unchanged cells
     * outside these spans need no glyph lookup or conversion. */
    for(u8 r=0;r<4;r++){
        u8 first=starts[r],last=ends[r];
        if(first==36)continue;
        if(!DMA_transfer(DMA_QUEUE_COPY,DMA_VRAM,rows+r*36+first,
            0xd000+(r==3?27:r)*128+(first+2)*2,last-first,2)){d->ready=0;return 0;}
        memcpy(d->previous.cells+r*36+first,hud->cells+r*36+first,(last-first)*sizeof(RT_HudCell));
    }
    return 1;
}
void RT_campaignHudDisplayEnd(RT_CampaignHudDisplay *d)
{
    if(!d || owner!=d)return;
    SYS_disableInts();VDP_setHInterrupt(FALSE);SYS_setHIntCallback(NULL);
    owner=0;d->ready=0;SYS_setVIntCallback(savedVInt);
    VDP_setWindowVPos(FALSE,0);VDP_setHIntCounter(255);SYS_enableInts();
}
