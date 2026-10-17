#include "rt_segmented_pcm.h"
bool RT_planSegmentedPcm(u32 address,u32 count,u32 romBytes,RT_SegmentedPcmPlan *out)
{
 if(!out || !count || count>2097151UL || (address&32767) ||
    !romBytes || romBytes>0x2000000UL || (romBytes&0x7ffffUL) ||
    address>=romBytes || count>romBytes-address || romBytes-address-count<16)return FALSE;
 u16 preload=count<256?count:256;
 u32 remaining=count-preload;
 RT_SegmentedPcmPlan p={address>>19,0x10+((address&0x7ffffUL)>>15),preload,
     remaining>>8,remaining&255,preload&255,(address+count-1)>>19};
 *out=p;return TRUE;
}
