#include "rt_mcu_input.h"
bool RT_filterMcuInputs(const u8 scan[37],u8 records[111],u8 combined[74])
{
 static const u8 select[37]={
  8,0,9,1,10,2,11,3,12,4,13,5,14,6,15,7,
  24,16,25,17,26,18,27,30,33,28,20,19,32,22,36,29,21,34,35,23,31};
 u16 i;
 if(!scan || !records || !combined)return FALSE;
 for(i=0;i<37;i++){
  u8 *p=records+i*3;
  u8 previous=p[1],latched=p[2],index=select[i];
  u8 current=(scan[index] ^ ((index==18 || index==26)?0:255))&1;
  u8 next=((current|previous)&latched)|(current&previous);
  p[0]=(~latched)&next;p[1]=current;p[2]=next;
  combined[i*2]=p[0];combined[i*2+1]=current;
 }
 combined[1]|=combined[35];combined[35]=combined[1];
 return TRUE;
}
bool RT_getMcuCreditInputs(const u8 combined[74],u8 inputs[5])
{
 static const u8 indices[5]={21,20,22,19,18};
 u16 i;
 if(!combined || !inputs)return FALSE;
 for(i=0;i<5;i++)inputs[i]=combined[indices[i]*2];
 return TRUE;
}

bool RT_publishMcuInputs(const u8 combined[74],u8 sharedInputs[70],
 volatile u8 *ready1188)
{
 if(!combined || !sharedInputs || !ready1188)return FALSE;
 *ready1188=0;
 memcpy(sharedInputs,combined,70);
 *ready1188=1;
 return TRUE;
}
bool RT_serviceMcuInputScan(const u8 scan[37],u8 records[111],u8 combined[74],
 u8 sharedInputs[70],volatile u8 *ready1188)
{
 if(!scan || !records || !combined || !sharedInputs || !ready1188)return FALSE;
 RT_publishMcuInputs(combined,sharedInputs,ready1188);
 return RT_filterMcuInputs(scan,records,combined);
}
