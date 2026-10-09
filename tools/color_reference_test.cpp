// Test-only original pure color routines. The original entry point/imports are
// not executed. Absolute x87 constant operands are relocated in this private
// mapped image; the reference file itself is never changed.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "GameColor.h"
#include <cstring>
#include <float.h>
#include <iostream>
#include <limits>
#include <cmath>
#include <random>
#include <stdexcept>
using namespace farmar::recovered;
namespace {
void relocateConstants(unsigned char* image,unsigned begin,unsigned end,unsigned expected) {
    DWORD old;
    if(!VirtualProtect(image+begin,end-begin,PAGE_EXECUTE_READWRITE,&old)) throw std::runtime_error("Cannot relocate oracle constants");
    unsigned changed=0;
    for(unsigned at=begin;at+6<=end;++at) {
        if(image[at]!=0xd8 || (image[at+1]!=0x1d && image[at+1]!=0x0d && image[at+1]!=0x05)) continue;
        std::uint32_t address; std::memcpy(&address,image+at+2,4);
        if(address!=0x5ef628 && address!=0x5ef1b4 && address!=0x5ef340 && address!=0x5f2854 && address!=0x5ef2c8)
            throw std::runtime_error("Unexpected original color constant");
        address=reinterpret_cast<std::uintptr_t>(image)+(address-0x400000U);
        std::memcpy(image+at+2,&address,4); ++changed; at+=5;
    }
    DWORD restored; VirtualProtect(image+begin,end-begin,old,&restored);
    FlushInstructionCache(GetCurrentProcess(),image+begin,end-begin);
    if(changed!=expected) throw std::runtime_error("Original color operand count differs");
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=2 && argc!=3) return 2;
    HMODULE original=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);
    if(!original) return 2;
    try {
        if(argc==3) _control87(_PC_24,_MCW_PC);
        auto* image=reinterpret_cast<unsigned char*>(original);
        relocateConstants(image,0x17f80,0x17fe0,4);
        relocateConstants(image,0x455e0,0x4571d,16);
        using From=void(__thiscall*)(Color*,std::uint32_t);
        using To=std::uint32_t(__thiscall*)(const Color*);
        auto from=reinterpret_cast<From>(image+0x17f80);
        auto to=reinterpret_cast<To>(image+0x455e0);
        unsigned checks=0;
        std::mt19937 random(0x434f4c52);
        auto compare=[&](const Color& input) {
            auto expected=to(&input),actual=input.toARGB();
            if(expected!=actual) throw std::runtime_error("Packed color differs");
            ++checks;
        };
        for(unsigned trial=0;trial<20000;++trial) {
            auto packed=random(); Color expected; from(&expected,packed);
            auto actual=Color::fromARGB(packed);
            if(std::memcmp(&expected,&actual,sizeof(Color))) throw std::runtime_error("RGBA float bits differ");
            ++checks; compare(actual);
            Color arbitrary;
            arbitrary.r=static_cast<int>(random()%4000000)-1000000; arbitrary.r/=1000000;
            arbitrary.g=static_cast<int>(random()%4000000)-1000000; arbitrary.g/=1000000;
            arbitrary.b=static_cast<int>(random()%4000000)-1000000; arbitrary.b/=1000000;
            arbitrary.a=static_cast<int>(random()%4000000)-1000000; arbitrary.a/=1000000;
            compare(arbitrary);
        }
        for(unsigned byte=0;byte<255;++byte) {
            const float threshold=(byte+0.5f)/255.f;
            for(float value:{threshold,std::nextafter(threshold,0.f),std::nextafter(threshold,1.f)}) compare({value,value,value,value});
        }
        compare({0,0,0,0}); compare({1,1,1,1}); compare({-1,-1,-1,-1}); compare({2,2,2,2});
        std::cout << checks << '\n'; FreeLibrary(original);
    } catch(const std::exception& error) { std::cerr << error.what() << '\n'; FreeLibrary(original); return 1; }
}
