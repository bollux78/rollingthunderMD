#include "rt_campaign_scene_art.h"
#include <string.h>
u8 RT_sceneArtInitialize(RT_SceneArtRuntime *s,u16 firstTile,u16 capacity)
{
 if(!s || !capacity || capacity>RT_SCENE_ART_TILES || firstTile<512 ||
    firstTile>1536 || capacity>1536-firstTile)return 0;
 s->firstTile=firstTile;s->capacity=capacity;s->used=s->chunkCount=0;
 s->dirtyFirst=s->dirtyEnd=0;
 s->prepared=s->dirty=0;s->preparedBatch=0;s->boundArt=0;return 1;
}
static u8 stage(RT_SceneArtRuntime *s,const RT_SceneArt *art,
 const RT_SceneArtPart *part,u8 left,u8 right,u16 cached,u16 *destination)
{
 u16 wt=(part->size>>2)+1,ht=(part->size&3)+1,amount=wt*ht;
 if(part->tile>=art->tileCount || amount>art->tileCount-part->tile)return 0;
 for(u16 i=0;i<s->chunkCount;i++){
  const RT_SceneArtChunk *key=s->chunks+i;
  if(key->tile==part->tile && key->count==amount && key->left==left && key->right==right){*destination=key->destination;return 1;}
 }
 if(s->chunkCount==RT_SCENE_ART_CHUNKS || amount>s->capacity-s->used)return 0;
 *destination=s->used;RT_SceneArtChunk *key=s->chunks+s->chunkCount;
 if(!(s->chunkCount<cached && key->tile==part->tile && key->count==amount &&
      key->left==left && key->right==right && key->destination==s->used)){
  if(!s->dirty){s->dirtyFirst=s->used;s->dirtyEnd=s->used+amount;}
  else{if(s->used<s->dirtyFirst)s->dirtyFirst=s->used;if(s->used+amount>s->dirtyEnd)s->dirtyEnd=s->used+amount;}
  s->dirty=1;
  if(!left && !right)memcpy(s->pixels+(u32)s->used*8,art->patterns+(u32)part->tile*8,amount*32);
  else for(u16 x=0;x<wt;x++){
   u16 start=x*8,end=wt*8-right;
   u16 l=left>start?left-start:0,r=start+8>end?start+8-end:0;
   if(l>8)l=8;if(r>8)r=8;
   u32 mask=(l==8?0:0xffffffffUL>>(l*4))&(r==8?0:0xffffffffUL<<(r*4));
   for(u16 row=0;row<ht*8;row++)
    s->pixels[(u32)s->used*8+x*ht*8+row]=art->patterns[(u32)part->tile*8+x*ht*8+row]&mask;
  }
 }
 *key=(RT_SceneArtChunk){part->tile,amount,s->used,left,right};
 s->used+=amount;s->chunkCount++;return 1;
}
u8 RT_sceneArtPrepareBudget(RT_SceneArtRuntime *s,const RT_SceneArt *art,
 const RT_SceneCast *cast,const RT_TransitionArt *highArt,
 RT_SpriteBatch *batch,u32 scratch[225],const u32 highRowBudget[224])
{
 if(s){s->prepared=0;s->preparedBatch=0;}
 if(!batch)return 0;
 u8 available=!batch->failed;batch->failed=1;
 if(!s || !art || !cast || !highArt || !scratch || !available || batch->count>80 ||
    !art->patterns || !art->parts || !s->capacity || s->capacity>RT_SCENE_ART_TILES ||
    s->firstTile<highArt->upperTile+(highArt->tileCount>256?highArt->tileCount-256:0) ||
    s->firstTile>1536 || s->capacity>1536-s->firstTile)return 0;
 u16 cached=s->boundArt==art?s->chunkCount:0,highCount=batch->count;s->boundArt=art;u8 population=0;
 s->used=s->chunkCount=0;
 for(u8 slot=RT_SCENE_CAST_CAPACITY;slot--;){
  const RT_SceneActor *actor=cast->actors+slot;
  if(actor->removed)continue;population++;
  if(!actor->visible)continue;
  u8 family=actor->kind==7?0:actor->kind==11?1:actor->kind==19?2:255;
  if(family==255 || actor->pose>=art->poseCount || !art->poses[family])return 0;
  const RT_SceneArtPose *pose=art->poses[family]+actor->pose;
  if(!pose->count || pose->first>art->partCount || pose->count>art->partCount-pose->first)return 0;
  for(u8 j=pose->count;j--;){
   const RT_SceneArtPart *part=art->parts+pose->first+j;
   u8 facing=!!(actor->direction&2);
   s32 cx=(actor->x>>4)+part->dx[facing],cy=((4096-actor->y)>>4)+part->dy;
   if(cx< -32 || cx>=320 || cy< -32 || cy>=224)continue;
   u16 width=((part->size>>2)+1)*8,height=((part->size&3)+1)*8;
   s16 x=(((u16)cx+67)&511)-51,y=((-(s16)(u8)cy-height)&255)-32;
   if(x+width<=16 || x>=304 || y+(s16)height<=0 || y>=224)continue;
   if(batch->count==80)return 0;
   u8 left=x<16?16-x:0,right=x+width>304?x+width-304:0,flip=part->flipX^facing;
   if(flip){u8 swap=left;left=right;right=swap;}
   u16 destination;if(!stage(s,art,part,left,right,cached,&destination))return 0;
   RT_SpriteCommand *command=batch->commands+batch->count;
   command->x=x+128;command->y=y+128;command->size=part->size;
   command->attribute=0x8000|((u16)part->palette<<13)|((u16)part->flipY<<12)|((u16)flip<<11)|(s->firstTile+destination);
   command->link=++batch->count;
  }
 }
 if(population!=cast->count)return 0;
 for(u16 i=0;i<225;i++)scratch[i]=0;
 u16 firstRow=224,lastRow=0;
 for(u16 i=highRowBudget?highCount:0;i<batch->count;i++){
  const RT_SpriteCommand *command=batch->commands+i;
  s16 top=command->y-128,bottom=top+((command->size&3)+1)*8;
  if(top<0)top=0;if(bottom>224)bottom=224;if(top>=bottom)continue;
  if(top<firstRow)firstRow=top;if(bottom>lastRow)lastRow=bottom;
  u16 width=((command->size>>2)+1)*8;
  scratch[top]+=0x10000UL+width;scratch[bottom]-=0x10000UL+width;
 }
 u32 budget=0;
 if(highRowBudget){
  for(u16 row=firstRow;row<lastRow;row++){
   budget+=scratch[row];u32 combined=budget+highRowBudget[row];
   if(combined>0x14ffffUL || (u16)combined>320)return 0;
  }
 }else for(u16 row=0;row<224;row++){budget+=scratch[row];if(budget>0x14ffffUL || (u16)budget>320)return 0;}
 if(highCount && batch->count>highCount)batch->commands[highCount-1].link=highCount;
 if(batch->count)batch->commands[batch->count-1].link=0;else batch->commands[0]=(RT_SpriteCommand){0};
 batch->failed=0;s->prepared=1;s->preparedBatch=batch;return 1;
}
u8 RT_sceneArtPrepare(RT_SceneArtRuntime *s,const RT_SceneArt *art,
 const RT_SceneCast *cast,const RT_TransitionArt *highArt,
 RT_SpriteBatch *batch,u32 scratch[225])
{return RT_sceneArtPrepareBudget(s,art,cast,highArt,batch,scratch,0);}
#ifdef __m68k__
u8 RT_sceneArtPublish(RT_SceneArtRuntime *s,RT_SpriteBatch *batch)
{
 if(!s || !batch || !s->prepared || s->preparedBatch!=batch || batch->failed)return 0;
 if(s->used && s->dirty){
  u16 end=s->dirtyEnd<s->used?s->dirtyEnd:s->used;
  if(s->dirtyFirst<end)VDP_loadTileData(s->pixels+(u32)s->dirtyFirst*8,
      s->firstTile+s->dirtyFirst,end-s->dirtyFirst,DMA);
 }
 s->dirty=0;s->prepared=0;s->preparedBatch=0;
 return RT_transitionSpritesPublish(batch);
}
#endif
