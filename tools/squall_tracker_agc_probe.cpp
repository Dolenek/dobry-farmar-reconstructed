// Actual AGC/Read code with owned initialized mixer states and module data.
// Writes diagnostic data only, never code/IAT; no production replacement calls.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <float.h>
#include <stdexcept>
void need(bool condition,const char* text){if(!condition)throw std::runtime_error(text);}
int wmain(int argc,wchar_t**argv){
    if(argc!=4)return 2;SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::cout.setf(std::ios::unitbuf);
    try{
        auto module=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());need(module!=nullptr,"DLL load");
        std::array<unsigned,10> rvas{};std::ifstream config(argv[2]);for(auto& value:rvas)need(static_cast<bool>(config>>std::hex>>value),"Config RVAs");
        auto base=reinterpret_cast<unsigned>(module);
        using Read=unsigned(__thiscall*)(void*,void*,unsigned,int);using AGC=void(__thiscall*)(void*,int);
        auto read=reinterpret_cast<Read>(base+rvas[0]);auto agc=rvas[1]?reinterpret_cast<AGC>(base+rvas[1]):nullptr;
        auto* buffer=reinterpret_cast<int*>(base+rvas[2]);auto* gain=reinterpret_cast<unsigned*>(base+rvas[3]);
        auto* hz=reinterpret_cast<unsigned*>(base+rvas[4]);auto* channels=reinterpret_cast<unsigned*>(base+rvas[5]);
        auto* vu=reinterpret_cast<unsigned*>(base+rvas[6]);auto* setup=reinterpret_cast<unsigned*>(base+rvas[7]);
        auto* dryRight=reinterpret_cast<int*>(base+rvas[8]);auto* dryLeft=reinterpret_cast<int*>(base+rvas[9]);
        bool all=std::wstring(argv[3])==L"all";need(!all||agc,"Actual AGC method missing");
        std::vector<unsigned char> object(65536,0);std::ofstream output("destinations.bin",std::ios::binary);
        unsigned cases=0,kernels=0,reads=0,recovery=0;
        auto reset=[&](){*gain=512;*vu=0;*hz=44100;*channels=2;buffer[0]=0;if(agc)agc(object.data(),1);};
        auto prefix=[&](const char* phase,unsigned precision,unsigned count,unsigned seed,unsigned variant){
            std::cout<<(cases++?",":"")<<"{\"phase\":\""<<phase<<"\",\"precision\":"<<precision<<",\"count\":"<<count<<",\"seed\":"<<seed<<",\"variant\":"<<variant;
        };
        auto end=[&](){std::cout<<",\"gain\":"<<*gain<<",\"vu\":"<<*vu<<"}";};
        std::cout<<"{\"cases\":[";
        for(unsigned precision:{53u,24u}){
            _control87(precision==24?_PC_24:_PC_53,_MCW_PC);auto control=_control87(0,0)&_MCW_PC;
            if(all){
                for(unsigned count:{1u,2u,3u,127u,511u,512u,1023u,1024u})
                for(unsigned seed:{0u,1u,128u,256u,511u,512u,1024u,0x7fffffffu,0xffffffffu})
                for(unsigned variant=0;variant<6;++variant)for(unsigned meter:{0u,255u}){
                    reset();*gain=seed;*vu=meter;unsigned random=0x17295183;
                    std::fill(buffer,buffer+1024,0x17295183);
                    for(unsigned i=0;i<count;++i){random=random*1664525+1013904223;
                        int value=0;if(variant==1)value=static_cast<int>(random);
                        if(variant==2)value=(i&1)?0x08100001:-0x08100001;
                        if(variant==3)value=static_cast<int>(i%5)-2;
                        if(variant==4)value=i%17==0?0x7fffffff:0;
                        if(variant==5)value=static_cast<int>(0x80000000u);
                        buffer[i]=value;
                    }
                    agc(object.data(),count);need(std::all_of(buffer+count,buffer+1024,[](int v){return v==0x17295183;}),"Actual AGC overran extent");
                    output.write(reinterpret_cast<char*>(buffer),4096);prefix("kernel",precision,count,seed,variant*2+(meter!=0));end();++kernels;
                    need((_control87(0,0)&_MCW_PC)==control,"AGC changed precision");
                }
                for(unsigned frequency:{11025u,44100u})for(unsigned channelCount:{1u,2u})for(unsigned seed:{0u,1u,256u,511u}){
                    reset();*gain=seed;*hz=frequency;*channels=channelCount;
                    for(unsigned step=0;step<200;++step){std::fill(buffer,buffer+1024,0);agc(object.data(),512);
                        prefix("recovery",precision,512,seed,step);std::cout<<",\"frequency\":"<<frequency<<",\"mix_channels\":"<<channelCount;end();++recovery;}
                }
            }
            const std::array<std::array<int,2>,6> ramps{{{{0,0}},{{0x10000000,-0x10000000}},{{0x7fffffff,0x7fffffff}},{{static_cast<int>(0x80000000u),static_cast<int>(0x80000000u)}},{{1,-1}},{{-0x08100000,0x08100000}}}};
            for(unsigned outChannels:{1u,2u})for(unsigned count:{1u,64u,512u,1024u,4096u})
            for(unsigned seed:{1u,256u,512u,1024u})for(unsigned enabled:{0u,4u})for(unsigned variant=0;variant<ramps.size();++variant){
                reset();*gain=seed;*setup=enabled;*dryRight=ramps[variant][0];*dryLeft=ramps[variant][1];
                std::fill(object.begin(),object.end(),0);unsigned moduleChannels=4;
                std::memcpy(object.data()+0xd058,&moduleChannels,4);std::memcpy(object.data()+0xd064,&count,4);
                std::vector<unsigned char> pcm(32+count*outChannels*2,0x5a);
                auto returned=read(object.data(),pcm.data()+16,count*outChannels*2,outChannels);need(returned==count,"Read frame count");
                for(unsigned i=0;i<16;++i)need(pcm[i]==0x5a&&pcm[pcm.size()-1-i]==0x5a,"Actual Read guard");
                output.write(reinterpret_cast<char*>(pcm.data()),pcm.size());prefix("read",precision,count,seed,variant);
                std::cout<<",\"out_channels\":"<<outChannels<<",\"enabled\":"<<enabled<<",\"returned_frames\":"<<returned<<",\"right_offset\":"<<*dryRight<<",\"left_offset\":"<<*dryLeft;end();++reads;
                need((_control87(0,0)&_MCW_PC)==control,"Read changed precision");
            }
        }
        std::cout<<"],\"case_count\":"<<cases<<",\"kernels\":"<<kernels<<",\"reads\":"<<reads<<",\"recovery_steps\":"<<recovery<<",\"guards_intact\":true}\n";
        FreeLibrary(module);return 0;
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
