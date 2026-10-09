// Test-only original sprite vertex/index writer. Own fixture object storage and
// buffers are supplied; no game entry point, file I/O, audio or draw code runs.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "SpriteGeometry.h"
#include <array>
#include <cstring>
#include <float.h>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>
using namespace farmar::recovered;
namespace {
struct Fixup { std::uint32_t rva,address; };
const Fixup fixups[]={
#include "sprite_oracle_fixups.inc"
};
template<class Value> void write(std::array<unsigned char,680>& object,unsigned offset,const Value& value) {
    std::memcpy(object.data()+offset,&value,sizeof(value));
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=2 && argc!=3) return 2;
    auto module=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES); if(!module) return 2;
    try {
        if(argc==3) _control87(_PC_24,_MCW_PC);
        auto* image=reinterpret_cast<unsigned char*>(module);
        for(const auto& fixup:fixups) {
            std::uint32_t address; std::memcpy(&address,image+fixup.rva,4);
            if(address!=fixup.address) throw std::runtime_error("Sprite oracle operand differs");
            DWORD old; if(!VirtualProtect(image+fixup.rva,4,PAGE_EXECUTE_READWRITE,&old)) throw std::runtime_error("Sprite oracle protection failed");
            address=reinterpret_cast<std::uintptr_t>(image)+(address-0x400000U); std::memcpy(image+fixup.rva,&address,4);
            DWORD restored; VirtualProtect(image+fixup.rva,4,old,&restored);
        }
        FlushInstructionCache(GetCurrentProcess(),image+0x117f40,0x865);
        using Render=void(__thiscall*)(void*,const SpriteDefinition*,const void*,unsigned);
        const auto render=reinterpret_cast<Render>(image+0x117f40);
        std::mt19937 random(0x53505254); unsigned comparisons=0;
        auto number=[&] { return static_cast<float>(static_cast<int>(random()%400001)-200000)/197.f; };
        for(unsigned trial=0;trial<10000;++trial) {
            SpriteDefinition sprite{}; sprite.subdivisions=1+random()%8;
            for(auto& corner:sprite.corners) { corner.x=number(); corner.y=number(); corner.color=random(); }
            sprite.u0=number()/3000; sprite.v0=number()/3000; sprite.u1=number()/3000; sprite.v1=number()/3000;
            const unsigned base=trial%3==0 ? random()%4096 : 0;
            const bool fullscreen=trial%2==0;
            DesktopImagePlacement placement; placement.scaleX=1.5f; placement.scaleY=1.5f; placement.offsetX=240; placement.offsetY=0;
            TextureScale scale{trial%4==0 ? 1.f : 0.75f,trial%4==0 ? 1.f : 0.875f};
            const unsigned count=(sprite.subdivisions+1)*(sprite.subdivisions+1),indexCount=sprite.subdivisions*sprite.subdivisions*6;
            std::vector<SpriteVertex> actual(count),expected(base+count);
            std::vector<std::uint16_t> actualIndices(indexCount),expectedIndices(indexCount);
            std::array<unsigned char,680> object{};
            write(object,0x110,static_cast<unsigned char>(fullscreen)); write(object,0x112,static_cast<unsigned char>(1));
            write(object,0x174,expected.data()); write(object,0x178,expectedIndices.data()); write(object,0x1ac,base);
            write(object,0x248,placement.scaleX); write(object,0x24c,placement.scaleY); write(object,0x250,placement.offsetX); write(object,0x254,placement.offsetY);
            // Test fixture for fields read from the texture, not a runtime cTexture.
            std::array<unsigned char,72> texture{};
            std::memcpy(texture.data()+0x28,&scale.u,4); std::memcpy(texture.data()+0x2c,&scale.v,4);
            render(object.data(),&sprite,trial%4==0 ? nullptr:texture.data(),0);
            writeSpriteGeometry(sprite,scale,fullscreen,placement,actual.data(),actualIndices.data(),base);
            if(std::memcmp(actual.data(),expected.data()+base,count*sizeof(SpriteVertex))) {
                for(unsigned vertex=0;vertex<count;++vertex) if(std::memcmp(&actual[vertex],&expected[base+vertex],sizeof(SpriteVertex))) {
                    const auto& a=actual[vertex]; const auto& e=expected[base+vertex];
                    std::cerr << "trial " << trial << " vertex " << vertex << " divisions " << sprite.subdivisions << " fullscreen " << fullscreen
                        << " actual " << std::hexfloat << a.x << ',' << a.y << ',' << a.u << ',' << a.v << " color " << std::hex << a.color
                        << " expected " << std::hexfloat << e.x << ',' << e.y << ',' << e.u << ',' << e.v << " color " << std::hex << e.color << '\n';
                    break;
                }
                throw std::runtime_error("Original sprite vertex differs");
            }
            if(actualIndices!=expectedIndices) throw std::runtime_error("Original sprite indices differ");
            comparisons+=2;
        }
        FreeLibrary(module); std::cout << comparisons << '\n';
    } catch(const std::exception& error) { std::cerr << error.what() << '\n'; FreeLibrary(module); return 1; }
}
