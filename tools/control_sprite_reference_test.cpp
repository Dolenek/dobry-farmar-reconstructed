// Original cControl::makeSprite is a test oracle on owned geometry/parent data.
// No virtual UI methods, original constructor, imports or entry point are used.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ControlSprite.h"
#include "ControlAnimation.h"
#include <array>
#include <cstring>
#include <cmath>
#include <float.h>
#include <iostream>
#include <random>
#include <stdexcept>
using namespace farmar::recovered;
namespace {
struct Fixup { std::uint32_t rva,address; };
const Fixup fixups[]={
#include "control_oracle_fixups.inc"
};
template<class T,std::size_t N> void put(std::array<unsigned char,N>& object,unsigned offset,const T& value) { std::memcpy(object.data()+offset,&value,sizeof(value)); }
void __fastcall ended(void* object,void*) {
    unsigned value; auto* bytes=static_cast<unsigned char*>(object); std::memcpy(&value,bytes+0x1b4,4); ++value; std::memcpy(bytes+0x1b4,&value,4);
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=2 && argc!=3) return 2;
    auto module=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES); if(!module) return 2;
    try {
        if(argc==3) _control87(_PC_24,_MCW_PC);
        auto* image=reinterpret_cast<unsigned char*>(module);
        for(const auto& fixup:fixups) {
            std::uint32_t address; std::memcpy(&address,image+fixup.rva,4); if(address!=fixup.address) throw std::runtime_error("Control oracle operand differs");
            DWORD old; if(!VirtualProtect(image+fixup.rva,4,PAGE_EXECUTE_READWRITE,&old)) throw std::runtime_error("Oracle protection failed");
            address=reinterpret_cast<std::uintptr_t>(image)+(address-0x400000U); std::memcpy(image+fixup.rva,&address,4);
            DWORD restored; VirtualProtect(image+fixup.rva,4,old,&restored);
        }
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        using MakeSprite=void(__thiscall*)(void*,SpriteDefinition*,unsigned,float);
        const auto makeSprite=reinterpret_cast<MakeSprite>(image+0xef3b0);
        using MakeTransform=void(__thiscall*)(void*); const auto makeTransform=reinterpret_cast<MakeTransform>(image+0xf0b70);
        std::array<void*,59> table{}; table[0xcc/4]=image+0x13a10; table[0xdc/4]=reinterpret_cast<void*>(&ended);
        std::mt19937 random(0x434f4e54); unsigned comparisons=0,animationComparisons=0;
        auto number=[&] { return static_cast<float>(static_cast<int>(random()%200001)-100000)/193.f; };
        for(unsigned trial=0;trial<100000;++trial) {
            ControlSpriteProperties p;
            p.x=number(); p.y=number(); p.width=number(); p.height=number(); p.angle=trial%7 ? number():float((trial%5)*90);
            p.columns=1+random()%17; p.rows=1+random()%17; p.invertX=trial&1; p.invertY=trial&2;
            p.color={number()/100,number()/100,number()/100,number()/100}; p.rootAlpha=number()/300; p.alpha=number()/300;
            const float opacity=number()/300; const unsigned frame=trial%5 ? random()%(p.columns*p.rows*3):random();
            std::array<unsigned char,0x228> object{},parent{},root{};
            put(object,0x5c,p.x); put(object,0x60,p.y); put(object,0x64,p.width); put(object,0x68,p.height); put(object,0x54,p.angle);
            put(object,0x40,p.columns); put(object,0x44,p.rows); put(object,0x1c4,static_cast<unsigned char>(p.invertX)); put(object,0x1c5,static_cast<unsigned char>(p.invertY));
            put(object,0x218,p.color.r); put(object,0x21c,p.color.g); put(object,0x220,p.color.b);
            const auto* alpha=&p.alpha; put(object,0x58,alpha);
            if(trial%3) { auto* parentPointer=parent.data(); auto* rootPointer=root.data(); put(object,0x20,parentPointer); put(parent,0x20,rootPointer); put(root,0x8,p.rootAlpha); }
            else put(object,0x8,p.rootAlpha);
            SpriteDefinition actual{},expected{}; actual.subdivisions=expected.subdivisions=1+trial%5;
            updateControlSprite(actual,p,frame,opacity); makeSprite(object.data(),&expected,frame,opacity);
            if(std::memcmp(&actual,&expected,76)) {
                std::cerr << "trial " << trial << " angle " << std::hexfloat << p.angle << " frame " << frame << " columns " << p.columns << " rows " << p.rows << '\n';
                for(unsigned corner=0;corner<4;++corner) std::cerr << "corner " << corner << " actual " << actual.corners[corner].x << ',' << actual.corners[corner].y << " expected " << expected.corners[corner].x << ',' << expected.corners[corner].y << '\n';
                std::cerr << "UV actual " << actual.u0 << ',' << actual.v0 << ',' << actual.u1 << ',' << actual.v1 << " expected " << expected.u0 << ',' << expected.v0 << ',' << expected.u1 << ',' << expected.v1 << '\n';
                throw std::runtime_error("Original Control sprite differs");
            }
            ++comparisons;
            if(trial<20000) {
                ControlAnimationState animation; animation.length=1.f+std::fabs(number()); animation.time=number()*3;
                if(trial%11==0) animation.time=animation.length;
                if(trial%11==1) animation.time=-animation.length;
                if(trial%11==2) animation.time=0;
                if(trial%11==3) animation.time=std::nextafter(animation.length,INFINITY);
                if(trial%11==4) animation.time=std::nextafter(animation.length,0.f);
                animation.frames=1+random()%(p.columns*p.rows); animation.loop=trial&4; animation.doubleRender=trial&8;
                put(object,0x34,animation.frames); put(object,0x38,animation.length); put(object,0x3c,static_cast<unsigned char>(animation.loop));
                put(object,0x1ac,animation.time); put(object,0x1b0,static_cast<unsigned char>(1)); put(object,0x1b4,0U);
                put(object,0x1c6,static_cast<unsigned char>(animation.doubleRender)); auto* tablePointer=table.data(); put(object,0,tablePointer);
                SpriteDefinition first{},second{}; first.subdivisions=second.subdivisions=1;
                std::memcpy(object.data()+0x70,&first,76); std::memcpy(object.data()+0xbc,&second,76);
                const auto resolved=resolveControlAnimation(animation); updateAnimatedControlSprites(first,second,p,resolved);
                makeTransform(object.data()); unsigned endCount; std::memcpy(&endCount,object.data()+0x1b4,4);
                if(std::memcmp(object.data()+0x1ac,&animation.time,4) || std::memcmp(object.data()+0x70,&first,76)
                    || std::memcmp(object.data()+0xbc,&second,76) || endCount!=unsigned(resolved.ended) || object[0x1b0]) {
                    std::cerr << "Animation trial " << trial << " time " << animation.time << " length " << animation.length << " frames " << animation.frames << " loop " << animation.loop << " double " << animation.doubleRender << '\n';
                    throw std::runtime_error("Original Control animation differs");
                }
                ++animationComparisons;
            }
        }
        FreeLibrary(module); std::cout << "{\"sprite_comparisons\":" << comparisons << ",\"animation_scenarios\":" << animationComparisons << "}\n";
    } catch(const std::exception& error) { std::cerr << error.what() << '\n'; FreeLibrary(module); return 1; }
}
