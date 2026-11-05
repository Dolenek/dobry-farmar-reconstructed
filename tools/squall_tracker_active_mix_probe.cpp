// Complete actual CreateStereoMix dispatch, boundaries and channel lifecycle.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <cstring>
#include <float.h>
#include <stdexcept>
using byte=unsigned char;
#include "SoundFile/track/stdafx.h"
#include "SoundFile/track/TrackDecoder.h"
void need(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
static_assert(sizeof(MODCHANNEL)==256);
int wmain(int argc,wchar_t** argv){
 if(argc!=3)return 2;SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::cout.setf(std::ios::unitbuf);
 try{
  auto module=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());need(module!=nullptr,"Load actual DLL");
  std::array<unsigned,5> rvas{};std::ifstream config(argv[2]);for(auto& n:rvas)need(bool(config>>std::hex>>n),"Active mixer config");
  const auto base=reinterpret_cast<std::uintptr_t>(module);using Mix=unsigned(__thiscall*)(void*,int);
  auto mix=reinterpret_cast<Mix>(base+rvas[0]);auto* buffer=reinterpret_cast<int*>(base+rvas[1]);auto* dryR=reinterpret_cast<int*>(base+rvas[2]);auto* dryL=reinterpret_cast<int*>(base+rvas[3]);auto* setup=reinterpret_cast<unsigned*>(base+rvas[4]);
  std::vector<unsigned char> sample(65536);unsigned random=0x712f853a;for(auto& b:sample){random=random*1664525U+1013904223U;b=static_cast<unsigned char>(random>>24);}const auto originalSample=sample;
  std::ofstream bytes("destinations.bin",std::ios::binary),states("channel-scalars.bin",std::ios::binary);unsigned cases=0;
  std::cout<<"{\"cases\":[";
  for(unsigned precision:{53U,24U})for(unsigned count:{0U,7U,64U,257U})for(unsigned channels:{1U,2U,33U})
  for(unsigned mode:{0xffffffffU,0U,0x10U,0x410U,0x10010U})for(unsigned variant=0;variant<8;++variant)
  for(int increment:{0x8000,0x18000,-0x8000})for(unsigned loop=0;loop<3;++loop){
   std::vector<unsigned char> object(65536,0);auto put=[&](unsigned off,unsigned value){std::memcpy(object.data()+off,&value,4);};
   put(0xd058,4);put(0xd05c,channels);*setup=mode==0xffffffffU?0:mode;*dryR=*dryL=0;
   std::array<int,1024> initial{};random=0x7219a38f;
   for(auto& value:initial){random=random*1664525U+1013904223U;value=(variant&1)?static_cast<int>(random):0;}
   std::copy(initial.begin(),initial.end(),buffer);
   for(unsigned j=0;j<channels;++j){
    put(0x5008+j*4,channels-1-j);auto* p=reinterpret_cast<MODCHANNEL*>(object.data()+8+j*256);
    p->pCurrentSample=reinterpret_cast<signed char*>(sample.data()+256);p->nPos=std::array<unsigned,6>{0,7,8,55,56,63}[(variant+j)%6];p->nPosLo=(variant&1)?0xffff:0;p->nInc=increment;
    const unsigned format=(variant+j)&3;p->dwFlags=((format&1)?CHN_16BIT:0)|((format&2)?CHN_STEREO:0)|(mode==0xffffffffU?CHN_NOIDO:0);
    p->nLength=loop?56:64;p->nLoopStart=8;p->nLoopEnd=56;if(loop)p->dwFlags|=CHN_LOOP;if(loop==2)p->dwFlags|=CHN_PINGPONGLOOP;
    const bool mute=variant==0&&j%3==0;p->nRightVol=mute?0:64;p->nLeftVol=(variant&1)?p->nRightVol:127;
    if(mute)p->nLeftVol=0;p->nRightRamp=37;p->nLeftRamp=(variant&1)?37:-51;
    p->nRampRightVol=p->nRightVol*4096;p->nRampLeftVol=p->nLeftVol*4096;p->nNewRightVol=81;p->nNewLeftVol=113;p->nRampLength=(variant&4)?7:0;
    if(variant>=6){p->dwFlags|=CHN_NOTEFADE;p->nFadeOutVol=0;}p->nMasterChn=1;
   }
   const auto before=object;const auto fp=precision==24?_PC_24:_PC_53;_control87(fp,_MCW_PC);
   const auto result=mix(object.data(),count);need((_control87(0,0)&_MCW_PC)==fp,"Active mixer changed FPU precision");need(result==(count?channels:0),"Actual active mixer channel count");
   for(unsigned i=0;i<object.size();++i)need((i>=8&&i<8+channels*256)||object[i]==before[i],"Mixer changed unrelated object byte");
   need(std::equal(initial.begin()+count*2,initial.end(),buffer+count*2),"Active mixer overran requested global buffer");need(sample==originalSample,"Active mixer wrote sample data");
   bytes.write(reinterpret_cast<char*>(buffer),static_cast<std::streamsize>((count*2+8)*4));
   std::cout<<(cases++?",":"")<<"{\"precision\":"<<precision<<",\"count\":"<<count<<",\"channels\":"<<channels<<",\"mode\":"<<mode<<",\"variant\":"<<variant<<",\"increment\":"<<increment<<",\"loop\":"<<loop<<",\"returned_channels\":"<<result<<",\"dry_right\":"<<*dryR<<",\"dry_left\":"<<*dryL<<",\"sample_alive\":[";
   for(unsigned j=0;j<channels;++j){auto* p=reinterpret_cast<MODCHANNEL*>(object.data()+8+j*256);need(p->pCurrentSample==nullptr||p->pCurrentSample==reinterpret_cast<signed char*>(sample.data()+256),"Unexpected sample ownership pointer");if(j)std::cout<<',';std::cout<<(p->pCurrentSample?"true":"false");states.write(reinterpret_cast<const char*>(object.data()+12+j*256),252);}
   std::cout<<"]}";
  }
  std::cout<<"],\"case_count\":"<<cases<<",\"guards_intact\":true,\"sample_read_only\":true}\n";FreeLibrary(module);return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
