// Test-only original storage readers, with allocator redirected into this
// diagnostic's CRT. The original entry point and CRT initialization never run.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "FileIndex.h"
#include "NativeImage.h"
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace farmar::recovered;
namespace {
std::vector<void*> allocations;
void* __cdecl allocate(unsigned count) {
    auto* value=std::malloc(count); if(!value) throw std::bad_alloc();
    std::memset(value,0xcd,count); allocations.push_back(value); return value;
}
void clearAllocations() { for(auto* value:allocations) std::free(value); allocations.clear(); }
String path(const wchar_t* input) {
    const int count=WideCharToMultiByte(CP_ACP,0,input,-1,nullptr,0,nullptr,nullptr);
    std::string bytes(count,'\0'); WideCharToMultiByte(CP_ACP,0,input,-1,bytes.data(),count,nullptr,nullptr); return String(bytes.c_str());
}
struct OriginalImage {
    DWORD format; unsigned char* data; DWORD pitch,left,top,width,height;
    void* palette; DWORD owned,paletteOwned; OriginalImage* mip; OriginalImage* face;
};
static_assert(sizeof(OriginalImage)==48,"Original image source");
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3) return 2;
    auto module=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES); if(!module) return 2;
    try {
        auto* original=reinterpret_cast<unsigned char*>(module); auto* entry=original+0x17edb1;
        DWORD protection; if(!VirtualProtect(entry,5,PAGE_EXECUTE_READWRITE,&protection)) throw std::runtime_error("allocator oracle protection failed");
        const DWORD distance=reinterpret_cast<DWORD>(&allocate)-reinterpret_cast<DWORD>(entry+5);
        entry[0]=0xe9; std::memcpy(entry+1,&distance,4); DWORD ignored; VirtualProtect(entry,5,protection,&ignored);
        FlushInstructionCache(GetCurrentProcess(),entry,5);
        using Read=HRESULT(__thiscall*)(OriginalImage*,const void*,unsigned);
        const auto readDDS=reinterpret_cast<Read>(original+0x19847a),readTGA=reinterpret_cast<Read>(original+0x19781c),readBMP=reinterpret_cast<Read>(original+0x196f2d);
        FileIndex files(path(argv[2]),String("pack")); unsigned images=0; std::uint64_t compared=0;
        for(const auto& indexed:files.files()) {
            const auto& name=indexed.first;
            const auto extension=name.substr(name.find_last_of('.')+1);
            if(extension!="dds" && extension!="tga" && extension!="bmp") continue;
            auto file=files.open(String(name.c_str())); const unsigned size=file->size(); std::vector<unsigned char> bytes(size);
            file->read(bytes.data(),size); file.reset();
            DWORD magic=0; if(size>=4) std::memcpy(&magic,bytes.data(),4); if(magic==0x474e4a8b) continue;
            auto actual=decodeNativeImage(bytes.data(),size); OriginalImage expected{};
            const auto reader=extension=="dds" ? readDDS:(extension=="tga" ? readTGA:readBMP);
            const unsigned skip=extension=="bmp" ? 14:0;
            const HRESULT result=reader(&expected,bytes.data()+skip,size-skip);
            if(FAILED(result)) throw std::runtime_error("Original image source rejected shipped input: "+name);
            const OriginalImage* face=&expected; unsigned planeIndex=0,faces=0;
            for(;face;face=face->face) {
                ++faces; unsigned mipCount=0;
                for(auto* plane=face;plane;plane=plane->mip) {
                    ++mipCount; if(planeIndex>=actual.planes.size()) throw std::runtime_error("Original image plane count differs");
                    const auto& recovered=actual.planes[planeIndex++];
                    if(recovered.format!=plane->format || recovered.width!=plane->width || recovered.height!=plane->height || recovered.pitch!=plane->pitch)
                        throw std::runtime_error("Original image source metadata differs: "+name);
                    if(std::memcmp(recovered.bytes.data(),plane->data,recovered.bytes.size())) throw std::runtime_error("Original image source bytes differ: "+name);
                    compared+=recovered.bytes.size();
                }
                if(mipCount!=actual.mipLevels) throw std::runtime_error("Original image mip count differs");
            }
            if(faces!=actual.faces || planeIndex!=actual.planes.size()) throw std::runtime_error("Original image face count differs");
            ++images; clearAllocations();
        }
        FreeLibrary(module); std::cout << "{\"images_compared\":" << images << ",\"source_bytes_compared\":" << compared << "}\n";
    } catch(const std::exception& error) { clearAllocations(); FreeLibrary(module); std::cerr << error.what() << '\n'; return 1; }
}
