// Offline asset tool only; ymfm never runs in the cartridge.
#include "ymfm_opm.h"
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <string>
extern "C" {
#include "rt_music_sequence.h"
}
static void writeRegister(u8 reg,u8 value,void *context){
 auto *chip=static_cast<ymfm::ym2151 *>(context);
 chip->write_address(reg);chip->write_data(value);
}
int main(int argc,char **argv){
 if(argc<3 || (argc>4 && argc%2))return 2;
 unsigned limit=argc>=4?std::strtoul(argv[3],nullptr,10):10000;
 unsigned age=0;
 if(!limit || limit>10000)return 2;
 for(int i=4;i<argc;i+=2){
  unsigned track=std::strtoul(argv[i],nullptr,10),count=std::strtoul(argv[i+1],nullptr,10);
  if(track>13 || !count || count>10000 || age+count>100000)return 2;
  age+=count;
 }
 unsigned track=std::strtoul(argv[2],nullptr,10);if(track<1||track>13)return 2;
 FILE *output=std::fopen(argv[1],"wb");if(!output)return 3;
 ymfm::ymfm_interface interface;ymfm::ym2151 chip(interface);chip.reset();
 RT_MusicSequence sequence={};u8 ack=0;
 uint64_t warmSamples=0;int16_t warmRing[256][2]={};
 unsigned warmServices=0;
 for(int i=4;i<argc;i+=2){
  unsigned prior=std::strtoul(argv[i],nullptr,10),count=std::strtoul(argv[i+1],nullptr,10);
  RT_publishMusicCommand(&sequence.control,prior,&ack);
  for(unsigned service=0;service<count;service++){

   if(RT_serviceMusicSequence(&sequence,&rtArcadeMusicBank,1024,writeRegister,&chip)==RT_MUSIC_CONTROL_INVALID)return 4;
   uint64_t boundary=uint64_t(++warmServices)*3579545*33/(64*2000);
   while(warmSamples<boundary){
    ymfm::ym2151::output_data frame;chip.generate(&frame);
    for(unsigned channel=0;channel<2;channel++){
     if(frame.data[channel]<-32768 || frame.data[channel]>32767)return 5;
     warmRing[warmSamples%256][channel]=frame.data[channel];
    }
    warmSamples++;
   }
  }
 }

 unsigned prefix=warmSamples<256?warmSamples:256;
 for(unsigned i=0;i<prefix;i++)for(unsigned channel=0;channel<2;channel++){
  int16_t v=warmRing[(warmSamples-prefix+i)%256][channel];
  unsigned char bytes[2]={static_cast<unsigned char>(v),static_cast<unsigned char>(v>>8)};
  if(std::fwrite(bytes,1,2,output)!=2)return 6;
 }
 // Canonical big-endian state bytes for comparison with a 68000 snapshot.
 unsigned char state[sizeof sequence];std::memcpy(state,&sequence,sizeof sequence);
 auto put16=[&](size_t at,unsigned value){state[at]=value>>8;state[at+1]=value;};
 put16(offsetof(RT_MusicSequence,control.clock.fraction),sequence.control.clock.fraction);
 put16(offsetof(RT_MusicSequence,control.cursor),sequence.control.cursor);
 put16(offsetof(RT_MusicSequence,control.returnCursor),sequence.control.returnCursor);
 for(unsigned i=0;i<8;i++)put16(offsetof(RT_MusicSequence,control.loopKey)+2*i,sequence.control.loopKey[i]);
 std::string priorHex;const char*digits="0123456789abcdef";
 for(auto value:state){priorHex+=digits[value>>4];priorHex+=digits[value&15];}
 RT_publishMusicCommand(&sequence.control,track,&ack);
 if(RT_serviceMusicSequence(&sequence,&rtArcadeMusicBank,1024,writeRegister,&chip)==RT_MUSIC_CONTROL_INVALID)return 4;
 uint64_t samples=0;unsigned tick=0,terminal=0;
 while(tick<limit && (argc>=4 || !terminal || tick<terminal+2)){
  uint64_t boundary=uint64_t(age+tick+1)*3579545*33/(64*2000)-warmSamples;
  while(samples<boundary){
   ymfm::ym2151::output_data frame;chip.generate(&frame);
   for(unsigned channel=0;channel<2;channel++){
    int32_t v=frame.data[channel];if(v<-32768||v>32767)return 5;
    unsigned char bytes[2]={static_cast<unsigned char>(v),static_cast<unsigned char>(v>>8)};
    if(std::fwrite(bytes,1,2,output)!=2)return 6;
   }
   samples++;
  }
  tick++;
  if(RT_serviceMusicSequence(&sequence,&rtArcadeMusicBank,1024,writeRegister,&chip)==RT_MUSIC_CONTROL_INVALID)return 4;
  if(!sequence.control.status && !terminal)terminal=tick;
 }
 if(std::fclose(output))return 7;
 std::printf("{\"prior_sequence_hex\":\"%s\",\"warm_prefix_samples\":%u,\"samples\":%llu,\"ticks\":%u,\"terminal_service\":%u,\"status\":%u}\n",priorHex.c_str(),prefix,(unsigned long long)samples,tick,terminal,sequence.control.status);
 return (terminal || argc>=4)?0:8;
}
