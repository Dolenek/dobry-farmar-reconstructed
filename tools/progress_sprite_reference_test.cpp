// Native ProgressBar transform oracle only. Production never maps the original.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ProgressSprite.h"
#include <array>
#include <cmath>
#include <cstring>
#include <float.h>
#include <iostream>
#include <random>
#include <stdexcept>
using namespace farmar::recovered;
namespace {
template<class T> void put(void* object,unsigned offset,const T& value) {std::memcpy(static_cast<unsigned char*>(object)+offset,&value,sizeof(value));}
template<class T> T get(const void* object,unsigned offset) {T value;std::memcpy(&value,static_cast<const unsigned char*>(object)+offset,sizeof(value));return value;}
unsigned __fastcall chosenFrame(void* object,void*) {return get<unsigned>(object,0x1b4);}
void require(bool condition,const char* message) {if(!condition) throw std::runtime_error(message);}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=2 && argc!=3) return 2;
    auto module=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);if(!module) return 2;
    try {
        _control87(argc==3 ? _PC_24:_PC_53,_MCW_PC);
        auto* image=reinterpret_cast<unsigned char*>(module);
        struct Fixup {unsigned rva,address;};const Fixup fixups[]={
#include "progress_sprite_oracle_fixups.inc"
        };
        for(const auto& fixup:fixups) {
            require(get<unsigned>(image,fixup.rva)==fixup.address,"Progress oracle operand differs");
            DWORD old;require(VirtualProtect(image+fixup.rva,4,PAGE_EXECUTE_READWRITE,&old)!=0,"Progress oracle protection failed");
            put(image,fixup.rva,reinterpret_cast<unsigned>(image)+fixup.address-0x400000);
            DWORD restored;VirtualProtect(image+fixup.rva,4,old,&restored);
        }
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        const auto native=reinterpret_cast<void(__thiscall*)(void*)>(image+0xf9290);
        std::array<void*,59> table{};table[0xcc/4]=reinterpret_cast<void*>(&chosenFrame);
        std::mt19937 random(0x50524f47);unsigned cases=0;
        auto number=[&] {return float(int(random()%200001)-100000)/193.f;};
        for(unsigned trial=0;trial<100000;++trial) {
            ControlSpriteProperties p;p.x=number();p.y=number();p.width=number();p.height=number();p.angle=trial%7 ? number():float(trial%5*90);
            p.columns=1+random()%17;p.rows=1+random()%17;p.invertX=trial&1;p.invertY=trial&2;
            p.color={number()/100,number()/100,number()/100,number()/100};p.alpha=number()/300;p.rootAlpha=number()/300;
            const unsigned frame=trial%5 ? random()%(p.columns*p.rows*3):random(),side=trial%8;
            float maximum=std::fabs(number()),current=number();
            if(trial%29==0) maximum=0;
            if(trial%29==1) maximum=-maximum;
            if(trial%29==2) current=maximum;
            if(trial%29==3) current=0;
            if(trial%29==4) current=std::nextafter(maximum,INFINITY);
            std::array<unsigned char,584> object{},parent{},root{};
            auto* vtable=table.data();put(object.data(),0,vtable);put(object.data(),0x1b4,frame);
            put(object.data(),0x5c,p.x);put(object.data(),0x60,p.y);put(object.data(),0x64,p.width);put(object.data(),0x68,p.height);put(object.data(),0x54,p.angle);
            put(object.data(),0x40,p.columns);put(object.data(),0x44,p.rows);put(object.data(),0x1c4,static_cast<unsigned char>(p.invertX));put(object.data(),0x1c5,static_cast<unsigned char>(p.invertY));
            put(object.data(),0x218,p.color.r);put(object.data(),0x21c,p.color.g);put(object.data(),0x220,p.color.b);
            const float* alpha=&p.alpha;put(object.data(),0x58,alpha);
            if(trial%3) {auto* parentPointer=parent.data();auto* rootPointer=root.data();put(object.data(),0x20,parentPointer);put(parent.data(),0x20,rootPointer);put(root.data(),8,p.rootAlpha);}
            else put(object.data(),8,p.rootAlpha);
            put(object.data(),0x38,1.f);put(object.data(),0x1ac,0.f);put(object.data(),0x1b0,static_cast<unsigned char>(1));
            put(object.data(),0x238,maximum);put(object.data(),0x23c,current);put(object.data(),0x240,side);
            SpriteDefinition actual{},expected{};actual.subdivisions=expected.subdivisions=1+trial%5;
            put(object.data(),0x70,expected);
            updateProgressSprite(actual,p,frame,maximum,current,side);native(object.data());
            std::memcpy(&expected,object.data()+0x70,sizeof(expected));
            if(std::memcmp(&actual,&expected,sizeof(expected)) || std::memcmp(&current,object.data()+0x23c,4)) {
                std::cerr<<"Progress trial "<<trial<<" side "<<side<<" current "<<current<<" maximum "<<maximum<<'\n';
                for(unsigned i=0;i<4;++i) std::cerr<<i<<" source "<<std::hexfloat<<actual.corners[i].x<<','<<actual.corners[i].y<<" native "<<expected.corners[i].x<<','<<expected.corners[i].y<<'\n';
                std::cerr<<"UV source "<<actual.u0<<','<<actual.v0<<','<<actual.u1<<','<<actual.v1<<" native "<<expected.u0<<','<<expected.v0<<','<<expected.u1<<','<<expected.v1<<'\n';
                throw std::runtime_error("ProgressBar native sprite differs");
            }
            require(object[0x1b0]==0,"Native progress transform stayed dirty");
            ++cases;
        }
        FreeLibrary(module);std::cout<<"{\"sprite_comparisons\":"<<cases<<"}\n";return 0;
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';FreeLibrary(module);return 1;}
}
