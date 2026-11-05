// Actual active-voice kernels from both real function tables; diagnostic only.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <array>
#include <vector>
#include <cstring>
#include <float.h>
#include <stdexcept>
using byte=unsigned char;
#include "SoundFile/track/stdafx.h"
#include "SoundFile/track/TrackDecoder.h"
void need(bool ok,const char* what){if(!ok)throw std::runtime_error(what);}
static_assert(sizeof(MODCHANNEL)==256);
static_assert(offsetof(MODCHANNEL,nFilter_A0)==72);
using Kernel=void(__cdecl*)(MODCHANNEL*,int*,int*);
int wmain(int argc,wchar_t** argv){
 if(argc!=3)return 2;SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::cout.setf(std::ios::unitbuf);
 try{
  auto module=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());need(module!=nullptr,"Load real DLL");
  std::array<unsigned,2> rvas{};std::ifstream config(argv[2]);for(auto& n:rvas)need(bool(config>>std::hex>>n),"Function table config");
  const auto base=reinterpret_cast<std::uintptr_t>(module);
  std::array<Kernel*,2> tables{reinterpret_cast<Kernel*>(base+rvas[0]),reinterpret_cast<Kernel*>(base+rvas[1])};
  std::vector<unsigned char> sample(65536);unsigned random=0x5e2718f3;
  for(auto& b:sample){random=random*1664525U+1013904223U;b=static_cast<unsigned char>(random>>24);}const auto originalSample=sample;
  std::ofstream bytes("destinations.bin",std::ios::binary),states("channel-scalars.bin",std::ios::binary);
  unsigned cases=0;std::cout<<"{\"cases\":[";
  for(unsigned table=0;table<2;++table)for(unsigned index=0;index<64;++index){
   const auto kernel=tables[table][index];need(kernel!=nullptr&&!IsBadCodePtr(reinterpret_cast<FARPROC>(kernel)),"Actual kernel pointer");
   for(unsigned precision:{53U,24U})for(unsigned count:{1U,7U,64U})for(int increment:{0x4000,0x18000,-0x8000})
   for(unsigned fraction:{0U,0xffffU})for(unsigned variant=0;variant<2;++variant){
    alignas(MODCHANNEL) std::array<unsigned char,288> channel;channel.fill(0xa7);std::memset(channel.data()+16,0,256);auto* p=reinterpret_cast<MODCHANNEL*>(channel.data()+16);
    p->pCurrentSample=reinterpret_cast<signed char*>(sample.data()+256);p->nPos=512;p->nPosLo=fraction;p->nInc=increment;p->nLength=4096;
    p->dwFlags=((index&1)?CHN_16BIT:0)|((index&2)?CHN_STEREO:0);
    const bool fastMono=table&&((index&3)<2)&&((index&0x38)==0||(index&0x38)==0x10);
    p->nRightVol=variant?255:64;p->nLeftVol=fastMono?p->nRightVol:(variant?1:127);
    p->nRightRamp=variant?-33:71;p->nLeftRamp=fastMono?p->nRightRamp:(variant?17:-53);
    p->nRampRightVol=p->nRightVol*4096;p->nRampLeftVol=p->nLeftVol*4096;p->nRampLength=count;
    p->nFilter_A0=variant?4096:8192;p->nFilter_B0=variant?4096:0;p->nFilter_B1=variant?-1024:0;
    p->nFilter_Y1=12345;p->nFilter_Y2=-23456;p->nFilter_Y3=32109;p->nFilter_Y4=-10101;
    std::vector<int> destination(count*2+16,static_cast<int>(0x5a5a5a5a));
    random=0x813f729a;for(unsigned i=0;i<count*2;++i){random=random*1664525U+1013904223U;destination[i+8]=variant?static_cast<int>(random):0;}
    const auto expectedPrecision=precision==24?_PC_24:_PC_53;_control87(expectedPrecision,_MCW_PC);
    kernel(p,destination.data()+8,destination.data()+8+count*2);
    need((_control87(0,0)&_MCW_PC)==expectedPrecision,"Actual kernel changed FPU precision");
    for(unsigned i=0;i<16;++i)need(channel[i]==0xa7&&channel[i+272]==0xa7,"Kernel changed channel guard");
    for(unsigned i=0;i<8;++i)need(destination[i]==static_cast<int>(0x5a5a5a5a)&&destination[i+8+count*2]==static_cast<int>(0x5a5a5a5a),"Kernel changed requested destination guard");
    need(sample==originalSample,"Kernel wrote input sample");need(p->pCurrentSample==reinterpret_cast<signed char*>(sample.data()+256),"Kernel changed sample pointer");
    bytes.write(reinterpret_cast<const char*>(destination.data()),static_cast<std::streamsize>(destination.size()*4));states.write(reinterpret_cast<const char*>(channel.data()+20),252);
    std::cout<<(cases++?",":"")<<"{\"table\":"<<table<<",\"index\":"<<index<<",\"kernel_rva\":"<<(reinterpret_cast<std::uintptr_t>(kernel)-base)
      <<",\"precision\":"<<precision<<",\"count\":"<<count<<",\"increment\":"<<increment<<",\"fraction\":"<<fraction<<",\"variant\":"<<variant<<",\"position\":"<<p->nPos<<",\"fraction_after\":"<<p->nPosLo<<'}';
   }
  }
  std::cout<<"],\"case_count\":"<<cases<<",\"guards_intact\":true,\"sample_read_only\":true}\n";FreeLibrary(module);return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
