// Differential numerical kernels on owned fixtures; never linked into the game.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <float.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <cstdint>
#include <cstring>
#include <stdexcept>
struct Module {
    HMODULE h;
    Module(const wchar_t* path):h(LoadLibraryW(std::filesystem::absolute(path).c_str())){if(!h)throw std::runtime_error("DLL load");}
    ~Module(){FreeLibrary(h);}
    template<class F> F at(unsigned rva){return reinterpret_cast<F>(reinterpret_cast<std::uintptr_t>(h)+rva);}
};
int wmain(int argc,wchar_t** argv){
    if(argc<7||argc>8)return 2;
    try{
        Module native(argv[1]),source(argv[2]);std::string kind=std::filesystem::path(argv[3]).string();
        unsigned nrva=std::stoul(argv[4],nullptr,16),srva=std::stoul(argv[5],nullptr,16),init=std::stoul(argv[6],nullptr,16);
        _control87(argc==8?_PC_24:_PC_53,_MCW_PC);
        using Init=void(__thiscall*)(void*);using Transform=void(__thiscall*)(void*,float*,float*);using Inplace=void(__thiscall*)(void*,float*);using Window=void(__thiscall*)(void*,float*,int,short*);
        std::vector<unsigned char>a(47744),b(47744);native.at<Init>(0xd620)(b.data());source.at<Init>(init)(a.data());
        if(a!=b)throw std::runtime_error("Initializer tables differ");
        std::uint32_t rng=0x96420011;unsigned failed=0,first=0,different=0;std::uint64_t fnv=14695981039346656037ULL;std::array<float,512>x,y;alignas(16) std::array<unsigned char,320>u,v;
        const unsigned total=4096;
        for(unsigned run=0;run<total;++run){
            for(unsigned i=0;i<x.size();++i){rng=rng*1664525U+1013904223U;x[i]=float(int((rng>>8)&65535)-32768)/float(1U<<((run%8)+2));}y=x;const auto initial=x;
            u.fill(0x5a);v=u;
            unsigned short beforeCW,beforeSW,afterCW,afterSW;
            __asm { fnstcw beforeCW }
            __asm { fnstsw beforeSW }
            if(kind=="fdct32"){
                source.at<Transform>(srva)(a.data(),x.data(),reinterpret_cast<float*>(u.data()+16));native.at<Transform>(nrva)(b.data(),y.data(),reinterpret_cast<float*>(v.data()+16));
            }else if(kind=="imdct18"||kind=="imdct6_3"){
                source.at<Inplace>(srva)(a.data(),x.data());native.at<Inplace>(nrva)(b.data(),y.data());std::memcpy(u.data()+16,x.data(),18*4);std::memcpy(v.data()+16,y.data(),18*4);
            }else if(kind=="window"||kind=="window_dual"){
                source.at<Window>(srva)(a.data(),x.data(),(run%16)*32,reinterpret_cast<short*>(u.data()+16));native.at<Window>(nrva)(b.data(),y.data(),(run%16)*32,reinterpret_cast<short*>(v.data()+16));
            }else throw std::runtime_error("Unknown kernel");
            __asm { fnstcw afterCW }
            __asm { fnstsw afterSW }
            if(beforeCW!=afterCW||((beforeSW^afterSW)&0x3800))throw std::runtime_error("Kernel control word or x87 stack changed");
            const unsigned outputBytes=kind=="fdct32"?128:kind=="window"?64:kind=="window_dual"?128:72;
            for(unsigned i=0;i<u.size();++i)if((i<16||i>=16+outputBytes)&&(u[i]!=0x5a||v[i]!=0x5a))throw std::runtime_error("Kernel guard changed");
            const unsigned inputStart=(kind=="imdct18"||kind=="imdct6_3")?18:0;
            if(std::memcmp(x.data()+inputStart,initial.data()+inputStart,(x.size()-inputStart)*4)||std::memcmp(y.data()+inputStart,initial.data()+inputStart,(y.size()-inputStart)*4))throw std::runtime_error("Input guard changed");
            if(u!=v){if(!failed){first=run;std::ofstream("first-source-output.bin",std::ios::binary).write(reinterpret_cast<char*>(u.data()),u.size());std::ofstream("first-native-output.bin",std::ios::binary).write(reinterpret_cast<char*>(v.data()),v.size());std::ofstream("first-input.bin",std::ios::binary).write(reinterpret_cast<const char*>(initial.data()),initial.size()*4);}++failed;for(unsigned i=0;i<u.size();++i)different+=u[i]!=v[i];}
            for(unsigned char z:u){fnv^=z;fnv*=1099511628211ULL;}
        }
        std::cout<<"{\"cases\":"<<total<<",\"failed_cases\":"<<failed<<",\"different_bytes\":"<<different<<",\"first_failed_case\":"<<first<<",\"source_fnv1a64\":"<<fnv<<"}\n";return failed?1:0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
