#include <assert.h>
#include <stdio.h>
#include "rt_save.h"
static u8 memory[256];static int writes,limit=-1;static bool enabled;
void SYS_disableInts(void){} void SYS_enableInts(void){}
bool Z80_getAndRequestBus(bool wait){(void)wait;return false;}void Z80_releaseBus(void){}
void SRAM_enable(void){assert(!enabled);enabled=true;}void SRAM_disable(void){enabled=false;}
u8 SRAM_readByte(u32 offset){assert(enabled&&offset<256);return memory[offset];}
void SRAM_writeByte(u32 offset,u8 value){assert(enabled&&offset<256);if(limit<0||writes<limit)memory[offset]=value;writes++;}
static u8 records[35]={0,0x30,0,5,10,15,41,0,0x20,0,4,10,15,20,0,0x10,0,3,10,15,20,0,8,0,2,10,15,20,0,4,0,1,10,15,20};
static void load(u32 expected,u8 credit,u8 muted){u8 r[35],c,m;u32 b;limit=-1;assert(RT_saveLoad(r,&b,&c,&m));assert(b==expected&&c==credit&&m==muted&&!memcmp(r,records,35));}
int main(void){
    u8 r[35],c=6,m=0;u32 b=3000;
    memset(memory,255,sizeof memory);assert(!RT_saveLoad(r,&b,&c,&m));assert(c==6&&b==3000);
    assert(RT_saveStore(records,3000,6,0));load(3000,6,0);
    writes=0;assert(RT_saveStore(records,3000,6,0));assert(!writes);
    u8 baseline[128];memcpy(baseline,memory,128);
    /* Every possible interrupted write must retain the preceding valid slot. */
    for(int cut=0;cut<65;cut++){
        memcpy(memory,baseline,128);load(3000,6,0);writes=0;limit=cut;
        assert(RT_saveStore(records,654321,9,1));load(3000,6,0);
    }
    limit=-1;assert(RT_saveStore(records,654321,9,1));load(654321,9,1);
    memory[64+22]^=1;load(3000,6,0); /* newest CRC invalid */
    assert(!RT_saveStore(records,3000,0,0));assert(!RT_saveStore(records,1000000,6,0));
    memset(memory,0,sizeof memory);assert(!RT_saveLoad(r,&b,&c,&m));
    limit=0;writes=0;assert(!RT_saveLoad(r,&b,&c,&m));assert(!rtSaveAvailable);
    assert(!RT_saveStore(records,3000,6,0));
    puts("SRAM: roundtrip, unchanged writes, 65 interrupted writes, corruption, defaults and validation passed.");
}
