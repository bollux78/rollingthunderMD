#include "rt_type10.h"
const RT_EnemyEntry rtType10Entries[48]={
{6,0x53,0x6e70},
{6,0x53,0x6e83},
{6,0x53,0x6e96},
{6,0x53,0x6ea3},
{6,0x53,0x6eb0},
{6,0x53,0x6ec3},
{6,0x53,0x6ed6},
{6,0x53,0x6ee9},
{8,0x53,0x6efc},
{8,0x53,0x6f09},
{8,0x53,0x6f1c},
{8,0x53,0x6f2f},
{8,0x53,0x6f3c},
{8,0x53,0x6f4f},
{16,0x53,0x6f62},
{8,0x53,0x6f4f},
{8,0x53,0x6f3c},
{8,0x53,0x6f2f},
{4,0x57,0x701f},
{4,0x58,0x7038},
{4,0x58,0x704b},
{4,0x57,0x705e},
{4,0x59,0x7077},
{4,0x59,0x708a},
{6,0x5a,0x709d},
{6,0x5a,0x70aa},
{6,0x5a,0x70b7},
{6,0x5a,0x70aa},
{6,0x5a,0x709d},
{10,0x81,0x716f},
{10,0x83,0x7182},
{10,0x83,0x718f},
{10,0x83,0x719c},
{10,0xf0,0x71a9},
{10,0xf0,0x71b6},
{10,0xf0,0x71c3},
{8,0x53,0x7103},
{8,0x53,0x7110},
{8,0x53,0x7123},
{8,0x53,0x7136},
{8,0x53,0x7149},
{8,0x53,0x715c},
{8,0x53,0x7110},
{8,0x53,0x7103},
{8,0x53,0x6f75},
{16,0x53,0x6f88},
{8,0x53,0x6f75},
{8,0x53,0x6f2f},
};
enum {slowWalkRows=0,approachRows=8,releaseRows=12,walkRows=18,turnRows=24,deathRows=29,scriptedRows=36,gestureRows=44};
static u8 approach(u8 state){return state==0 || (state>=8 && state<=0x14) ||
 (state>=0x20 && state<=0x3c) || (state>=0x50 && state<=0x5c) || (state>=0x70 && state<=0x80);}
static void row(RT_Enemy *e,u8 table,u8 index){
 const RT_EnemyEntry *r=rtType10Entries+table+index;
 e->state.row=index;e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=335+table+index;
}
u8 RT_type10Enter(RT_Enemy *e,u8 selector){
 if(!e || e->state.kind!=16)return 0;
 u8 state=selector&0xfc,table;
 if(state==0x78 || state==0x7c){e->state.spawnAction=e->state.action;return 1;}
 switch(state){
 case 4:table=slowWalkRows;break;
 case 0:case 0x1c:table=approachRows;break;
 case 0x18:table=scriptedRows;break;
 case 0x4c:table=gestureRows;break;
 case 0x60:case 0x64:case 0x68:case 0x6c:table=deathRows;break;
 case 0x44:table=releaseRows;break;
 case 0x48:table=turnRows;break;
 case 0x40:table=walkRows;break;
 default:if(!approach(state))return 0;table=approachRows;break;
 }
 if(!(selector&3))selector+=e->state.action&3;
 if(state==4 || state==0x40){s16 speed=state==4?16:48;e->velocityX=selector&2?-speed:speed;}
 e->state.action=e->state.spawnAction=selector;row(e,table,0);return 1;
}
u8 RT_type10Hit(RT_Enemy *e,u8 selector,RT_Type10Effects *effects){
 u8 state=selector&0xfc;
 if(!e || !effects || e->state.kind!=16 || (state!=0x60 && state!=0x64 && state!=0x68))return 0;
 effects->sound=0;
 if(e->byte6){effects->sound=14;e->byte12=255;e->state.spawnAction=e->state.action;}
 else{effects->sharedCommand=7;RT_type10Enter(e,selector);}
 return 1;
}
RT_EnemyResult RT_type10Step(RT_Enemy *e,const RT_WorldCamera *camera,
 s32 playerX,RT_Type10Effects *effects,u8 *selector){
 if(!e || !camera || !effects || !selector || e->state.kind!=16)return RT_ENEMY_UNAVAILABLE;
 u8 state=e->state.action&0xfc,index=e->state.row;
 if(state==0x18 || state==0x1c || state==0x4c){
  u8 last=state==0x4c?3:7;if(index>last)return RT_ENEMY_UNAVAILABLE;
  *selector=0;if(--e->state.timer)return RT_ENEMY_CONTINUE;
  if(index==last)effects->scriptRequest++;else row(e,state==0x4c?gestureRows:scriptedRows,index+1);
  return RT_ENEMY_CONTINUE;
 }
 if(approach(state) && e->byte12==255){e->byte12=0;*selector=0x44;return RT_ENEMY_SELECT;}
 if(state>=0x60 && state<=0x6c){
  if(index>6)return RT_ENEMY_UNAVAILABLE;
  *selector=0;if(--e->state.timer)return RT_ENEMY_CONTINUE;
  if(index==6){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
  row(e,deathRows,index+1);return RT_ENEMY_CONTINUE;
 }
 if(approach(state)?index>3:state==4?index>7:(state==0x40 || state==0x44)?index>5:state==0x48?index>4:1)return RT_ENEMY_UNAVAILABLE;
 *selector=0;
 if(approach(state)){
  s16 distance=(s16)((u16)e->state.x-(u16)playerX);
  if(!index){if(distance<0xc00)row(e,approachRows,1);}
  else if(index==3){if(distance<0x600){*selector=0x44;return RT_ENEMY_SELECT;}}
  else if(!--e->state.timer)row(e,approachRows,index+1);
  return RT_ENEMY_CONTINUE;
 }
 if(state==0x40 || state==4){
  if(state==0x40){if(e->byte12){e->byte4=0x24;e->byte12=0;return RT_ENEMY_CONTINUE;}e->byte4=0x22;}
  s32 origin=(s32)camera->x*16;s16 x=(s16)((u16)(e->state.x-origin)+(u16)e->velocityX);
  if(e->velocityX<0?x< -0x200:x>=0x1400){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
  e->state.x=origin+x;
  if(state==4){if(!--e->state.timer)row(e,slowWalkRows,index==7?0:index+1);return RT_ENEMY_CONTINUE;}
  if(e->state.action&2?x<0x200:x>=0x1000){*selector=0x48;return RT_ENEMY_SELECT;}
  if(--e->state.timer)return RT_ENEMY_CONTINUE;
  if(index==5){if(!(e->state.decisions&1)){*selector=0x48;return RT_ENEMY_SELECT;}row(e,walkRows,0);}
  else row(e,walkRows,index+1);
  return RT_ENEMY_CONTINUE;
 }
 if(--e->state.timer)return RT_ENEMY_CONTINUE;
 if(index==(state==0x44?5:4)){*selector=0x40;return RT_ENEMY_SELECT;}
 if(state==0x48 && index==2)e->state.spawnAction=e->state.action^=3;
 row(e,state==0x44?releaseRows:turnRows,index+1);return RT_ENEMY_CONTINUE;
}
