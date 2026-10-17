#include "rt_pcm_rom_driver.h"
#include "rt_segmented_pcm.h"
bool RT_startPcmRomDriver(u32 a,u32 n,u32 z){return RT_startSegmentedPcm(a,n,z);}
bool RT_startPcmRomDriverWindows(u32 a,u32 n,u32 z,u16 w){return w && w<=2 && RT_startSegmentedPcm(a,n,z);}
bool RT_rearmPcmRomDriver(u32 a,u32 n,u32 z){return RT_rearmSegmentedPcm(a,n,z);}
bool RT_replacePcmRomClipWindows(u32 a,u32 n,u32 z,u16 w){return w && w<=2 && RT_replaceSegmentedPcm(a,n,z);}
RT_PcmRomStatus RT_getPcmRomStatus(void){return (RT_PcmRomStatus)RT_segmentedPcmStatus();}
bool RT_stopPcmRomDriver(void){return RT_stopSegmentedPcm();}
