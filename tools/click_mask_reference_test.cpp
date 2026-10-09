// Original GPU JNG click-mask reference only. Implementation libraries
// never load the original PE; CRT/device/service fixtures are explicit adapters.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ClickMask.h"
#include "FileIndex.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "ImageTexture.h"
#include "Settings.h"
#include "StringManager.h"
#include "StringConversion.h"
#include "XmlDocument.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <float.h>
#include <iostream>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>
using namespace farmar::recovered;
namespace {
unsigned char* image=nullptr;
unsigned stage=0,trial=0,action=0;
std::set<void*> allocations;
FileIndex* files=nullptr; IDirect3DDevice8* device=nullptr;
template<class T> T& field(void* object,unsigned offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(object)+offset); }
template<class T> T function(unsigned rva) { return reinterpret_cast<T>(image+rva); }
void call(unsigned rva,void* object) { function<void(__thiscall*)(void*)>(rva)(object); }
void require(bool result,const char* message) { if(!result) throw std::runtime_error(message); }
void* __cdecl allocate(unsigned bytes) { auto* p=std::malloc(bytes); if(!p) throw std::bad_alloc(); allocations.insert(p); return p; }
void __cdecl release(void* p) { allocations.erase(p); std::free(p); }
LONG CALLBACK diagnose(EXCEPTION_POINTERS* e) {
    if(e->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION) {
        std::cerr<<"Text control AV stage "<<stage<<" trial "<<trial<<" action "<<action<<" RVA 0x"<<std::hex
            <<(reinterpret_cast<unsigned>(e->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image))<<" address 0x"<<e->ExceptionRecord->ExceptionInformation[1]<<'\n';
        TerminateProcess(GetCurrentProcess(),1);
    } return EXCEPTION_CONTINUE_SEARCH;
}
void write(unsigned rva,const void* p,unsigned size) { DWORD old,ignored; require(VirtualProtect(image+rva,size,PAGE_EXECUTE_READWRITE,&old)!=0,"Oracle protection failed"); std::memcpy(image+rva,p,size); VirtualProtect(image+rva,size,old,&ignored); }
void redirect(unsigned rva,void* p) { unsigned char instruction[5]={0xe9}; unsigned delta=reinterpret_cast<unsigned>(p)-reinterpret_cast<unsigned>(image+rva+5); std::memcpy(instruction+1,&delta,4); write(rva,instruction,5); }
void putPointer(unsigned rva,void* p) { std::memcpy(image+rva,&p,4); }
struct Node { Node *left,*parent,*right; void* key; unsigned char color,nil,pad[2]; };
void initializeSet(unsigned rva) { auto* h=static_cast<Node*>(allocate(20)); *h={h,h,h,nullptr,1,1,{0,0}}; std::memset(image+rva,0,12); putPointer(rva+4,h); }
struct NativeColumn { unsigned bitCount,allocator; unsigned* words; unsigned count,capacity; };
struct NativeMask { unsigned allocator=0; NativeColumn *first=nullptr,*last=nullptr,*end=nullptr; };
static_assert(sizeof(NativeColumn)==20 && sizeof(NativeMask)==16);
iFile* __fastcall open(void*,void*,const String* name) { return files->open(*name).release(); }
void __fastcall getJng(void*,void*,const void* source,const unsigned long* size,IDirect3DTexture8** texture,TextureImageSize* dimensions) { getJngTexture(device,source,*size,*texture,*dimensions); }
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3 && argc!=4) return 2;
    HMODULE reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES),runtime=nullptr; HWND window=nullptr; IDirect3D8* d3d=nullptr;
    if(!reference) return 2;
    try {
        image=reinterpret_cast<unsigned char*>(reference); AddVectoredExceptionHandler(1,&diagnose);
        struct Fixup { unsigned rva,address; }; const Fixup fixups[]={
#include "click_mask_oracle_fixups.inc"
        };
        std::set<unsigned> applied;
        for(const auto& f:fixups) { if(!applied.insert(f.rva).second) continue; unsigned address; std::memcpy(&address,image+f.rva,4); require(address==f.address,"Click mask oracle operand differs"); address=reinterpret_cast<unsigned>(image)+address-0x400000;write(f.rva,&address,4); }
        redirect(0x17edb1,reinterpret_cast<void*>(&allocate)); redirect(0x17fdbe,reinterpret_cast<void*>(&allocate)); redirect(0x1803af,reinterpret_cast<void*>(&release)); redirect(0x180b56,reinterpret_cast<void*>(&release));
        redirect(0x17ffc0,reinterpret_cast<void*>(&std::memmove)); // diagnostic CRT boundary, including its internal jump tables
        redirect(0x10f580,reinterpret_cast<void*>(&getJng));
        std::array<void*,3> fileTable{}; fileTable[2]=reinterpret_cast<void*>(&open); auto* fp=fileTable.data(); putPointer(0x25f72c,&fp);
        FlushInstructionCache(GetCurrentProcess(),image,0x200000);
        window=CreateWindowW(L"STATIC",L"Click mask comparison",WS_POPUP,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);require(window!=nullptr,"Mask test window unavailable");
        wchar_t system[MAX_PATH]; GetSystemDirectoryW(system,MAX_PATH);runtime=LoadLibraryW((std::wstring(system)+L"\\d3d8.dll").c_str());require(runtime!=nullptr,"D3D8 unavailable");
        d3d=reinterpret_cast<IDirect3D8*(WINAPI*)(UINT)>(GetProcAddress(runtime,"Direct3DCreate8"))(120);require(d3d!=nullptr,"D3D8 SDK120 unavailable");
        D3DPRESENT_PARAMETERS parameters{};parameters.BackBufferWidth=parameters.BackBufferHeight=64;parameters.BackBufferFormat=D3DFMT_X8R8G8B8;parameters.BackBufferCount=1;
        parameters.SwapEffect=D3DSWAPEFFECT_DISCARD;parameters.Windowed=TRUE;parameters.hDeviceWindow=window;
        const auto controlBefore=_control87(0,0);const bool preserve=argc==3;
        require(SUCCEEDED(d3d->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING|(preserve ? D3DCREATE_FPU_PRESERVE:0),&parameters,&device)),"Mask device failed");
        unsigned masks=0,columns=0; unsigned long long bits=0;
        {
            FileIndex index(toNarrow(WString(argv[2])),String("pack"));files=&index;
            textureEnvironment().files=&index;textureEnvironment().load=[](const void* source,const unsigned long& size,IDirect3DTexture8*& texture,TextureImageSize& dimensions) { getJngTexture(device,source,size,texture,dimensions); };
            std::array<unsigned char,20> graphics{};field<IDirect3DDevice8*>(graphics.data(),16)=device;
            NativeMask expected;
            // Reuse the original vector across differently sized masks to cover
            // growth, shrink, bit-array resizing, and overwritten active bits.
            for(const auto& entry:index.files()) {
                const String name(entry.first.c_str());auto file=index.open(name);unsigned magic=0;if(file->size()<4) continue;file->read(&magic,4);file.reset();
                if(magic!=0x474e4a8bU) continue;
                stage=1;trial=masks; const auto actual=loadClickMask(name);
                function<void(__thiscall*)(void*,const String*,NativeMask*)>(0x11c040)(graphics.data(),&name,&expected);
                stage=2;require(actual.size()==unsigned(expected.last-expected.first),"Original mask column count differs");
                for(unsigned x=0;x<actual.size();++x) {
                    const auto& column=actual[x];const auto& native=expected.first[x];require(column.bitCount==native.bitCount,"Original mask bit count differs");
                    for(unsigned y=0;y<column.bitCount;++y) require(bool(column.words[y>>5]&(1U<<(y&31)))==bool(native.words[y>>5]&(1U<<(y&31))),"Original GPU mask alpha bit differs");
                    ++columns;bits+=column.bitCount;
                } ++masks;
            }
            stage=3;call(0xf5200,&expected);textureEnvironment()={};files=nullptr;
        }
        for(auto* allocation:allocations) std::free(allocation);allocations.clear();device->Release();d3d->Release();DestroyWindow(window);FreeLibrary(runtime);FreeLibrary(reference);
        std::cout<<"{\"masks\":"<<masks<<",\"columns\":"<<columns<<",\"alpha_bits\":"<<bits<<",\"device_flags\":"<<(preserve ? 0x22:0x20)<<",\"x87_control_before\":"<<controlBefore<<",\"x87_control_after\":"<<_control87(0,0)<<"}\n";
    } catch(const std::exception& e) { std::cerr<<e.what()<<" stage "<<stage<<" trial "<<trial<<'\n';return 1; }
}
