// Compare actual tracker effect methods on complete owned diagnostic objects.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <array>
#include <string>
#include <cstring>
#include <float.h>
#include <stdexcept>
using byte=unsigned char;
#include "SoundFile/track/stdafx.h"
#include "SoundFile/track/TrackDecoder.h"
static_assert(sizeof(CDecompressTrack)==0xef0c);
static_assert(sizeof(MODCHANNEL)==256);
static_assert(offsetof(CDecompressTrack,m_dwSongFlags)==0xd058);
static_assert(offsetof(CDecompressTrack,m_nType)==0xd06c);

int wmain(int argc,wchar_t** argv) {
    if(argc!=4)return 2;
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    std::cout.setf(std::ios::unitbuf);
    try {
        auto native=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());
        auto source=LoadLibraryW(std::filesystem::absolute(argv[2]).c_str());
        if(!native||!source)throw std::runtime_error("LoadLibrary");
        std::ifstream manifest(argv[3]);if(!manifest)throw std::runtime_error("Manifest");
        using ChannelCall=void(__thiscall*)(void*,MODCHANNEL*,unsigned);
        using GlobalCall=void(__thiscall*)(void*,unsigned);
        using EffectsCall=int(__thiscall*)(void*);
        alignas(CDecompressTrack) std::array<unsigned char,sizeof(CDecompressTrack)+32> left,right;
        const unsigned types[]={MOD_TYPE_MOD,MOD_TYPE_S3M,MOD_TYPE_IT,MOD_TYPE_XM,MOD_TYPE_MT2,MOD_TYPE_STM,MOD_TYPE_AMF,MOD_TYPE_MOD|MOD_TYPE_IT};
        unsigned long long total=0,different=0,controlDifferent=0,guardDifferent=0,saved=0;
        std::string name;unsigned nativeRva,sourceRva,kind;
        while(manifest>>name>>std::hex>>nativeRva>>sourceRva>>std::dec>>kind) {
            auto n=reinterpret_cast<std::uintptr_t>(native)+nativeRva;
            auto s=reinterpret_cast<std::uintptr_t>(source)+sourceRva;
            unsigned long long cases=0,differences=0;
            for(unsigned precision:{53u,24u})for(unsigned type:types)for(unsigned variant=0;variant<8;++variant)for(unsigned param=0;param<256;++param) {
                left.fill(0xa7);std::memset(left.data()+16,0,sizeof(CDecompressTrack));
                auto* a=reinterpret_cast<CDecompressTrack*>(left.data()+16);
                a->m_nType=type;a->m_nChannels=4;
                a->m_dwSongFlags=((variant&1)?SONG_FIRSTTICK:0)|((variant&2)?SONG_LINEARSLIDES:0)|((variant&4)?SONG_FASTVOLSLIDES:0);
                a->m_nTickCount=(variant&1)?0:3;a->m_nMusicSpeed=6;a->m_nMusicTempo=(variant<2)?32:((variant<6)?125:255);
                a->m_nGlobalVolume=(variant<2)?0:((variant<6)?128:256);a->m_nOldGlbVolSlide=0x3a;
                auto& c=a->Chn[0];c.nPeriod=(variant<2)?32:((variant<4)?1712:((variant<6)?65535:0));
                c.nVolume=(variant<2)?0:((variant<6)?128:256);c.nPan=(variant<2)?0:((variant<6)?128:256);
                c.nGlobalVol=(variant<2)?0:((variant<6)?32:64);c.nC4Speed=8363;
                c.nPortamentoDest=(variant&2)?1000:2000;c.nPortamentoSlide=28;
                c.nOldPortaUpDown=0x3a;c.nOldFinePortaUpDown=5;c.nOldFineVolUpDown=7;c.nOldVolumeSlide=0x3a;c.nOldPanSlide=0x2f;c.nOldChnVolSlide=0xf3;
                c.nVibratoDepth=9;c.nVibratoSpeed=7;c.nTremoloDepth=11;c.nTremoloSpeed=3;c.nPanbrelloDepth=6;c.nPanbrelloSpeed=5;
                if(kind>=256){c.nRowCommand=static_cast<BYTE>(kind-256);c.nRowParam=static_cast<BYTE>(param);}
                right=left;auto* b=reinterpret_cast<CDecompressTrack*>(right.data()+16);
                auto requested=precision==24?_PC_24:_PC_53;
                int nr=0,sr=0;
                _control87(requested,_MCW_PC);auto nb=_control87(0,0);
                if(kind>=256)nr=reinterpret_cast<EffectsCall>(n)(a);else if(kind)reinterpret_cast<GlobalCall>(n)(a,param);else reinterpret_cast<ChannelCall>(n)(a,&a->Chn[0],param);
                auto na=_control87(0,0);
                _control87(requested,_MCW_PC);auto sb=_control87(0,0);
                if(kind>=256)sr=reinterpret_cast<EffectsCall>(s)(b);else if(kind)reinterpret_cast<GlobalCall>(s)(b,param);else reinterpret_cast<ChannelCall>(s)(b,&b->Chn[0],param);
                auto sa=_control87(0,0);
                bool cw=nb!=sb||na!=sa||((nb&_MCW_PC)!=requested)||((na&_MCW_PC)!=requested)||((sa&_MCW_PC)!=requested);
                bool guards=false;for(unsigned i=0;i<16;++i)guards|=left[i]!=0xa7||right[i]!=0xa7||left[i+sizeof(CDecompressTrack)+16]!=0xa7||right[i+sizeof(CDecompressTrack)+16]!=0xa7;
                bool diff=nr!=sr||std::memcmp(left.data(),right.data(),left.size())!=0;
                ++cases;++total;controlDifferent+=cw;guardDifferent+=guards;
                if(diff||cw||guards){++different;++differences;
                    if(saved++<32){auto stem=name+"-"+std::to_string(precision)+"-"+std::to_string(type)+"-"+std::to_string(variant)+"-"+std::to_string(param);
                        std::ofstream(stem+"-native.bin",std::ios::binary).write(reinterpret_cast<char*>(left.data()),left.size());
                        std::ofstream(stem+"-source.bin",std::ios::binary).write(reinterpret_cast<char*>(right.data()),right.size());
                    }
                }
            }
            std::cout<<"{\"method\":\""<<name<<"\",\"pairs\":"<<cases<<",\"different_pairs\":"<<differences<<"}\n";
        }
        std::cout<<"{\"total_pairs\":"<<total<<",\"different_pairs\":"<<different<<",\"control_differences\":"<<controlDifferent<<",\"guard_differences\":"<<guardDifferent<<",\"object_bytes\":"<<sizeof(CDecompressTrack)<<",\"whole_object_bytes_compared\":"<<total*sizeof(CDecompressTrack)<<"}\n";
        FreeLibrary(source);FreeLibrary(native);return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
