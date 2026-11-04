// Drive actual CreateStereoMix through channel-stop/EndChannelOfs, not a copy.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <cstring>
#include <float.h>
#include <stdexcept>
void need(bool good,const char* what){if(!good)throw std::runtime_error(what);}
int wmain(int argc,wchar_t**argv){
    if(argc!=3)return 2;SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::cout.setf(std::ios::unitbuf);
    try{
        auto module=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());need(module!=nullptr,"DLL load");
        std::array<unsigned,4> rvas{};std::ifstream config(argv[2]);for(auto& v:rvas)need(static_cast<bool>(config>>std::hex>>v),"Config");
        unsigned base=reinterpret_cast<unsigned>(module);using Mix=unsigned(__thiscall*)(void*,int);
        auto mix=reinterpret_cast<Mix>(base+rvas[0]);auto* buffer=reinterpret_cast<int*>(base+rvas[1]);
        auto* dryR=reinterpret_cast<int*>(base+rvas[2]);auto* dryL=reinterpret_cast<int*>(base+rvas[3]);
        const std::array<std::array<int,2>,6> offsets{{{{0,0}},{{0x10000000,-0x10000000}},{{0x7fffffff,0x7fffffff}},{{static_cast<int>(0x80000000u),static_cast<int>(0x80000000u)}},{{1,-1}},{{-0x08100000,0x08100000}}}};
        std::ofstream bytes("destinations.bin",std::ios::binary),states("channel-scalars.bin",std::ios::binary);unsigned cases=0;
        std::cout<<"{\"cases\":[";
        for(unsigned precision:{53u,24u})for(unsigned count:{0u,1u,2u,3u,127u,256u,511u,512u})
        for(unsigned variant=0;variant<offsets.size();++variant)for(unsigned pattern=0;pattern<3;++pattern){
            _control87(precision==24?_PC_24:_PC_53,_MCW_PC);auto control=_control87(0,0)&_MCW_PC;
            std::vector<unsigned char> object(65536,0);unsigned char sample[8]{};auto pointer=reinterpret_cast<unsigned>(sample);
            auto put=[&object](unsigned off,unsigned value){std::memcpy(object.data()+off,&value,4);};
            auto get=[&object](unsigned off){unsigned value;std::memcpy(&value,object.data()+off,4);return value;};
            put(0xd058,4);put(0xd05c,1);put(8,pointer);put(8+4,321);put(8+8,1234);put(8+0x24,0x80);put(8+0x5c,6);
            put(8+0x54,static_cast<unsigned>(offsets[variant][0]));put(8+0x58,static_cast<unsigned>(offsets[variant][1]));
            auto before=object;std::array<int,1024> initial{};unsigned random=0x17295183;
            for(unsigned i=0;i<initial.size();++i){random=random*1664525+1013904223;initial[i]=pattern==0?0:pattern==1?static_cast<int>(random):((i&1)?0x07ffffff:-0x08000000);}
            std::copy(initial.begin(),initial.end(),buffer);*dryR=*dryL=0;
            auto result=mix(object.data(),count);need(result==(count?1u:0u),"Actual stopped mixer channel count");
            need(get(8)==(count?0u:pointer),"Actual channel pointer lifecycle");
            for(unsigned i=0;i<object.size();++i)need((i>=8&&i<264)||object[i]==before[i],"Mixer changed unrelated object bytes");
            need(std::equal(initial.begin()+count*2,initial.end(),buffer+count*2),"Actual stop tail overran requested mix extent");
            need((_control87(0,0)&_MCW_PC)==control,"Mixer changed precision");
            bytes.write(reinterpret_cast<char*>(buffer),4096);states.write(reinterpret_cast<char*>(object.data()+12),252);
            std::cout<<(cases++?",":"")<<"{\"precision\":"<<precision<<",\"count\":"<<count<<",\"variant\":"<<variant<<",\"pattern\":"<<pattern
                     <<",\"returned_channels\":"<<result<<",\"current_sample_cleared\":"<<(get(8)==0?"true":"false")<<",\"right_offset\":"<<*dryR<<",\"left_offset\":"<<*dryL<<"}";
        }
        std::cout<<"],\"case_count\":"<<cases<<",\"guards_intact\":true}\n";FreeLibrary(module);return 0;
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
