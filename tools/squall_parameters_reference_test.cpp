// Original instructions are isolated test oracles. The DLL entry point is not
// executed, no sound devices are initialized, and production calls no original.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "Parameters.h"
#include <array>
#include <cstring>
#include <float.h>
#include <iostream>
#include <random>
#include <stdexcept>
using namespace farmar::recovered;
namespace {
int devices=0;unsigned deviceCalls=0;
int __cdecl deviceCount() { ++deviceCalls;return devices; }
void require(bool condition,const char* error) { if(!condition) throw std::runtime_error(error); }
void redirect(unsigned char* image,unsigned rva,void* target) {
    image[rva]=0xe9;const unsigned offset=reinterpret_cast<unsigned>(target)-reinterpret_cast<unsigned>(image+rva+5);
    std::memcpy(image+rva+1,&offset,4);
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc<2 || argc>3) return 2;
    if(argc==3) _control87(_PC_24,_MCW_PC);
    HMODULE reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);
    if(!reference) return 2;
    unsigned char* image=nullptr;void* unreadable=nullptr;HWND window=nullptr;
    try {
        auto* loaded=reinterpret_cast<unsigned char*>(reference);
        auto* headers=reinterpret_cast<IMAGE_NT_HEADERS*>(loaded+reinterpret_cast<IMAGE_DOS_HEADER*>(loaded)->e_lfanew);
        require(headers->OptionalHeader.ImageBase==0x10000000 && headers->FileHeader.Machine==IMAGE_FILE_MACHINE_I386,"Unexpected Squall reference ABI");
        image=static_cast<unsigned char*>(VirtualAlloc(nullptr,headers->OptionalHeader.SizeOfImage,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE));
        require(image!=nullptr,"Oracle allocation failed");std::memcpy(image,loaded,headers->OptionalHeader.SizeOfImage);
        // Apply the actual PE relocation directory to the private test mapping.
        // The loader has already relocated the source mapping if necessary.
        const auto directory=headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
        require(directory.Size>0,"Missing reference relocation directory");
        unsigned offset=0,relocations=0;
        while(offset<directory.Size) {
            auto* block=reinterpret_cast<IMAGE_BASE_RELOCATION*>(image+directory.VirtualAddress+offset);
            require(block->SizeOfBlock>=8 && offset+block->SizeOfBlock<=directory.Size,"Invalid relocation block");
            auto* entries=reinterpret_cast<WORD*>(block+1);
            for(unsigned i=0;i<(block->SizeOfBlock-8)/2;++i) {
                const unsigned type=entries[i]>>12;if(type==IMAGE_REL_BASED_ABSOLUTE) continue;
                require(type==IMAGE_REL_BASED_HIGHLOW,"Unexpected relocation type");
                auto* value=reinterpret_cast<unsigned*>(image+block->VirtualAddress+(entries[i]&0xfff));
                *value+=reinterpret_cast<unsigned>(image)-reinterpret_cast<unsigned>(loaded);++relocations;
            }
            offset+=block->SizeOfBlock;
        }
        *reinterpret_cast<void**>(image+0x48024)=reinterpret_cast<void*>(&IsBadReadPtr);
        *reinterpret_cast<void**>(image+0x48148)=reinterpret_cast<void*>(&IsWindow);
        redirect(image,0x7e40,reinterpret_cast<void*>(&deviceCount));
        FlushInstructionCache(GetCurrentProcess(),image,headers->OptionalHeader.SizeOfImage);
        const auto native=reinterpret_cast<void(__cdecl*)(const SquallParameters*,SquallParameters*)>(image+0x30a0);
        window=CreateWindowExW(0,L"STATIC",L"Squall parameter oracle",0,0,0,1,1,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
        require(window!=nullptr,"Window fixture creation failed");
        SYSTEM_INFO system;GetSystemInfo(&system);
        unreadable=VirtualAlloc(nullptr,system.dwPageSize*2,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
        require(unreadable!=nullptr,"Pointer fixture allocation failed");DWORD protection;
        require(VirtualProtect(static_cast<unsigned char*>(unreadable)+system.dwPageSize,system.dwPageSize,PAGE_NOACCESS,&protection)!=0,"Pointer fixture protection failed");
        unsigned checks=0,enumerations=0,invalidInputs=0,overlaps=0;
        auto compare=[&](const SquallParameters* input) {
            SquallParameters actual{},expected{};deviceCalls=0;
            normalizeSquallParameters(input,&actual,&deviceCount);const auto calls=deviceCalls;
            deviceCalls=0;native(input,&expected);
            require(!std::memcmp(&actual,&expected,52),"Normalized parameter bytes differ");
            require(deviceCalls==calls,"Device enumeration short-circuit differs");++checks;enumerations+=calls;
        };
        std::mt19937 random(0x5351414c);
        const std::array<int,18> integers={0,1,-1,2,3,4,8,16,17,255,256,257,44100,48000,48001,5000,5001,0x7fffffff};
        const std::array<unsigned,20> floats={0,0x80000000,0x3f800000,0x41200000,0x41200001,0x411fffff,0x7f800000,0xff800000,
            0x7fc00000,0xffc00001,0x7f800001,0xff800001,1,0x80000001,0x007fffff,0x00800000,0x7f7fffff,0xff7fffff,0x3a83126f,0xbf800000};
        for(unsigned test=0;test<100000;++test) {
            SquallParameters input;std::array<unsigned,13> words;
            for(unsigned i=0;i<13;++i) words[i]=test<2000 ? static_cast<unsigned>(integers[(test+i*7)%integers.size()]):random();
            words[0]=test%3==0 ? reinterpret_cast<unsigned>(window):test%3==1 ? 0:0xdeadbeef;
            for(unsigned i=10;i<13;++i) words[i]=test<2000 ? floats[(test+i*3)%floats.size()]:random();
            std::memcpy(&input,words.data(),52);devices=integers[test%integers.size()];compare(&input);
        }
        // All fields separately at every integer/float boundary, retaining the
        // valid engine setup in the other fields rather than masking branches.
        for(unsigned field=1;field<13;++field) for(unsigned value=0;value<40;++value) {
            const SquallParameters defaults{reinterpret_cast<unsigned>(window),1,44100,16,16,0,0,3,200,0,1.0f,0.001f,1.0f};
            SquallParameters input=defaults;const unsigned word=field>=10 ? floats[value%floats.size()]:static_cast<unsigned>(integers[value%integers.size()]);
            std::memcpy(reinterpret_cast<unsigned char*>(&input)+4*field,&word,4);devices=2;compare(&input);
        }
        for(int count:integers) {
            devices=count;
            compare(nullptr);compare(reinterpret_cast<const SquallParameters*>(1));
            compare(reinterpret_cast<const SquallParameters*>(static_cast<unsigned char*>(unreadable)+system.dwPageSize));
            compare(reinterpret_cast<const SquallParameters*>(static_cast<unsigned char*>(unreadable)+system.dwPageSize-48));
            invalidInputs+=4;
        }
        for(int shift=-12;shift<=12;++shift) {
            alignas(SquallParameters) std::array<unsigned char,256> actual{},expected{};
            for(auto& byte:actual) byte=static_cast<unsigned char>(random());expected=actual;
            const SquallParameters input{reinterpret_cast<unsigned>(window),1,44100,16,16,0,0,3,200,0,1.0f,0.001f,1.0f};
            std::memcpy(actual.data()+80,&input,52);std::memcpy(expected.data()+80,&input,52);devices=3;
            deviceCalls=0;normalizeSquallParameters(reinterpret_cast<SquallParameters*>(actual.data()+80),reinterpret_cast<SquallParameters*>(actual.data()+80+4*shift),&deviceCount);
            const auto calls=deviceCalls;deviceCalls=0;
            native(reinterpret_cast<SquallParameters*>(expected.data()+80),reinterpret_cast<SquallParameters*>(expected.data()+80+4*shift));
            require(actual==expected && deviceCalls==calls,"Forward overlapping parameter copy differs");++overlaps;++checks;enumerations+=calls;
        }
        std::cout<<"{\"checks\":"<<checks<<",\"device_enumerations\":"<<enumerations<<",\"invalid_inputs\":"<<invalidInputs
                 <<",\"overlap_cases\":"<<overlaps<<",\"relocations\":"<<relocations<<"}\n";
        DestroyWindow(window);VirtualFree(unreadable,0,MEM_RELEASE);VirtualFree(image,0,MEM_RELEASE);FreeLibrary(reference);return 0;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';if(window) DestroyWindow(window);if(unreadable) VirtualFree(unreadable,0,MEM_RELEASE);
        if(image) VirtualFree(image,0,MEM_RELEASE);FreeLibrary(reference);return 1;
    }
}
