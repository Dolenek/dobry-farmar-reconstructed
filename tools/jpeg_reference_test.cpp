// Original JPEG storage and full D3DX texture reference only. Implementation libraries
// never load the original PE; CRT/device/service fixtures are explicit adapters.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "ClickMask.h"
#include "FileIndex.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "ImageTexture.h"
#include "NativeImage.h"
#include "JpegColor.h"
#include <mmintrin.h>
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
std::string diagnosticName;
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
            <<(reinterpret_cast<unsigned>(e->ExceptionRecord->ExceptionAddress)-reinterpret_cast<unsigned>(image))<<" address 0x"<<e->ExceptionRecord->ExceptionInformation[1]<<" image 0x"<<reinterpret_cast<unsigned>(image)<<'\n';
        const auto& c=*e->ContextRecord;std::cerr<<"Input "<<diagnosticName<<" EAX "<<std::hex<<c.Eax<<" ECX "<<c.Ecx<<" EDX "<<c.Edx<<" ESI "<<c.Esi<<" EDI "<<c.Edi<<" EBP "<<c.Ebp<<" ESP "<<c.Esp<<'\n';
        const auto* stack=reinterpret_cast<const unsigned*>(c.Esp);for(unsigned i=0;i<12;++i) std::cerr<<stack[i]<<' ';std::cerr<<'\n';
        std::cerr<<"patched source factory "<<field<unsigned>(image,0x1a2072)<<" expected "<<(reinterpret_cast<unsigned>(image)+0x1a1f75)<<'\n';
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
    // Require the recorded relocations to work even when the original's
    // preferred address is free in this diagnostic process. Reserve only its
    // first allocation granule: a larger reservation can fail because an
    // unrelated region overlaps its tail while 00400000 itself stays free.
    auto* preferredReservation=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x10000,MEM_RESERVE,PAGE_NOACCESS);
    HMODULE reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES),runtime=nullptr; HWND window=nullptr; IDirect3D8* d3d=nullptr;
    if(preferredReservation) VirtualFree(preferredReservation,0,MEM_RELEASE);
    if(!reference) return 2;
    try {
        image=reinterpret_cast<unsigned char*>(reference);require(image!=reinterpret_cast<unsigned char*>(0x400000),"Mask oracle must exercise recorded relocations");AddVectoredExceptionHandler(1,&diagnose);
        struct Fixup { unsigned rva,address; }; const Fixup fixups[]={
#include "click_mask_oracle_fixups.inc"
        };
        std::set<unsigned> applied;
        for(const auto& f:fixups) { if(!applied.insert(f.rva).second) continue; unsigned address; std::memcpy(&address,image+f.rva,4); require(address==f.address,"Click mask oracle operand differs"); address=reinterpret_cast<unsigned>(image)+address-0x400000;write(f.rva,&address,4); }
        redirect(0x17edb1,reinterpret_cast<void*>(&allocate)); redirect(0x17fdbe,reinterpret_cast<void*>(&allocate)); redirect(0x1803af,reinterpret_cast<void*>(&release)); redirect(0x180b56,reinterpret_cast<void*>(&release));
        redirect(0x18302c,reinterpret_cast<void*>(&allocate)); // actual D3DX Float4 row malloc, not old CRT heap startup
        redirect(0x180b5b,reinterpret_cast<void*>(&release)); // exact original CRT free entry (00580b56 is its preceding thunk)
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
        unsigned images=0;unsigned long long sourceBytes=0,gpuPixels=0,colorTriples=0;
        // Exhaust every8-bit Y/Cb/Cr input against original MMX arithmetic.
        stage=9;std::array<unsigned char,256> ys{},cbs{},crs{};std::array<unsigned char,768> rgb{};
        for(unsigned i=0;i<256;++i) ys[i]=static_cast<unsigned char>(i);
        for(unsigned cb=0;cb<256;++cb) for(unsigned cr=0;cr<256;++cr) {
            cbs.fill(static_cast<unsigned char>(cb));crs.fill(static_cast<unsigned char>(cr));
            function<void(__cdecl*)(unsigned,const void*,const void*,const void*,void*)>(0x1a79d4)(256,ys.data(),cbs.data(),crs.data(),rgb.data());_mm_empty();
            for(unsigned y=0;y<256;++y) {const auto expected=jpegYCbCrToRGB(static_cast<unsigned char>(y),static_cast<unsigned char>(cb),static_cast<unsigned char>(cr),true);require(!std::memcmp(expected.data(),rgb.data()+3*y,3),"JPEG MMX color triple differs");++colorTriples;}
        }
        {
            FileIndex index(toNarrow(WString(argv[2])),String("pack"));
            struct OriginalImage {DWORD format;unsigned char* data;DWORD pitch,left,top,width,height;void* palette;DWORD owned,paletteOwned;OriginalImage* mip;OriginalImage* face;};
            static_assert(sizeof(OriginalImage)==48);
            using Read=HRESULT(__thiscall*)(OriginalImage*,const void*,unsigned);
            using Create=HRESULT(WINAPI*)(IDirect3DDevice8*,const void*,unsigned,unsigned,unsigned,unsigned,unsigned,D3DFORMAT,D3DPOOL,unsigned,unsigned,unsigned,unsigned*,void*,IDirect3DTexture8**);
            for(const auto& entry:index.files()) {
                if(entry.first.substr(entry.first.find_last_of('.')+1)!="jpg") continue;
                diagnosticName=entry.first;stage=10;trial=images;
                auto file=index.open(String(entry.first));std::vector<unsigned char> bytes(file->size());file->read(bytes.data(),static_cast<unsigned>(bytes.size()));file.reset();
                const auto actual=decodeNativeImage(bytes.data(),bytes.size());require(actual.faces==1 && actual.mipLevels==1 && actual.planes.size()==1,"JPEG source plane count differs");
                const auto& plane=actual.planes.front();OriginalImage expected{};
                require(SUCCEEDED(function<Read>(0x1975c7)(&expected,bytes.data(),static_cast<unsigned>(bytes.size()))),"Original JPEG rejected shipped input");
                require(plane.width==expected.width && plane.height==expected.height && plane.format==expected.format && plane.pitch==expected.pitch,"JPEG original metadata differs");
                if(std::memcmp(plane.bytes.data(),expected.data,plane.bytes.size())) {
                    unsigned long long different=0;unsigned largest=0;for(unsigned i=0;i<plane.bytes.size();++i) if(plane.bytes[i]!=expected.data[i]) {++different;largest=(std::max)(largest,unsigned(std::abs(int(plane.bytes[i])-int(expected.data[i]))));}
                    std::cerr<<entry.first<<" bytes "<<different<<" largest "<<largest<<"\n";require(false,"JPEG original pixels differ");
                }
                require(jpegUsesMMX()==bool(image[0x25f63c]),"JPEG native/source CPUID path differs");
                sourceBytes+=plane.bytes.size();call(0x196ed4,&expected);
                stage=11;IDirect3DTexture8 *recovered=nullptr,*native=nullptr;TextureImageSize dimensions{};unsigned info[7]{};
                getImageTexture(device,bytes.data(),static_cast<unsigned long>(bytes.size()),recovered,dimensions);
                require(SUCCEEDED(function<Create>(0x192e55)(device,bytes.data(),static_cast<unsigned>(bytes.size()),~0U,~0U,1,0,D3DFMT_UNKNOWN,D3DPOOL_MANAGED,1,1,0,info,nullptr,&native)),"Native JPEG texture creation failed");
                D3DSURFACE_DESC sourceDesc{},nativeDesc{};recovered->GetLevelDesc(0,&sourceDesc);native->GetLevelDesc(0,&nativeDesc);
                require(dimensions.width==info[0] && dimensions.height==info[1] && dimensions.width==plane.width && dimensions.height==plane.height,"JPEG original image size differs");
                require(sourceDesc.Width==nativeDesc.Width && sourceDesc.Height==nativeDesc.Height && sourceDesc.Format==nativeDesc.Format && sourceDesc.Pool==nativeDesc.Pool && sourceDesc.Usage==nativeDesc.Usage && recovered->GetLevelCount()==native->GetLevelCount(),"JPEG texture dimensions/format/pool/mips differ");
                require(sourceDesc.Format==D3DFMT_X8R8G8B8,"Unexpected shipped JPEG format");
                D3DLOCKED_RECT sourceLock{},nativeLock{};require(SUCCEEDED(recovered->LockRect(0,&sourceLock,nullptr,D3DLOCK_READONLY)) && SUCCEEDED(native->LockRect(0,&nativeLock,nullptr,D3DLOCK_READONLY)),"JPEG texture read lock failed");
                for(unsigned y=0;y<sourceDesc.Height;++y) require(!std::memcmp(static_cast<unsigned char*>(sourceLock.pBits)+y*sourceLock.Pitch,static_cast<unsigned char*>(nativeLock.pBits)+y*nativeLock.Pitch,sourceDesc.Width*4),"JPEG whole GPU texture differs");
                recovered->UnlockRect(0);native->UnlockRect(0);gpuPixels+=static_cast<unsigned long long>(sourceDesc.Width)*sourceDesc.Height;
                recovered->Release();native->Release();++images;
            }
        }
        const unsigned nativeMMXPath=image[0x25f63c];
        for(auto* allocation:allocations) std::free(allocation);allocations.clear();device->Release();d3d->Release();DestroyWindow(window);FreeLibrary(runtime);FreeLibrary(reference);
        std::cout<<"{\"jpeg_images\":"<<images<<",\"color_triples\":"<<colorTriples<<",\"native_mmx\":"<<nativeMMXPath<<",\"source_bytes\":"<<sourceBytes<<",\"gpu_texture_pixels\":"<<gpuPixels<<",\"reference_relocated\":true,\"device_flags\":"<<(preserve ? 0x22:0x20)<<"}\n";
    } catch(const std::exception& e) { std::cerr<<e.what()<<" stage "<<stage<<" trial "<<trial<<'\n';return 1; }
}
