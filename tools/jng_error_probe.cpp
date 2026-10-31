// Diagnostic host only: original DLL is an oracle, never a shipping dependency.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <string>
#include <exception>
#include <cstdint>

using Read=void (__cdecl *)(const void*,unsigned long&,unsigned long&,unsigned long*&);
using Free=void (__cdecl *)(unsigned long*&);
struct Case {
    Read read;Free release;const void* input;
    unsigned long width=0x12345678,height=0x87654321;
    unsigned long* pixels=reinterpret_cast<unsigned long*>(0x01234560);
    std::string result,message;
    unsigned long seh=0;std::intptr_t fault=0;
    bool pointerUnchanged=false,freePreservesPointer=false;
};
static void invoke(Case* c) {
    try {c->read(c->input,c->width,c->height,c->pixels);c->result="returned";}
    catch(const char* message) {c->result="const_char_pointer";c->message=message;}
    catch(const std::exception& e) {c->result="std_exception";c->message=e.what();}
    catch(...) {c->result="other_cpp_exception";}
}
static int filter(Case* c,EXCEPTION_POINTERS* e) {
    c->seh=e->ExceptionRecord->ExceptionCode;
    if(c->seh==EXCEPTION_ACCESS_VIOLATION && e->ExceptionRecord->NumberParameters>=2)
        c->fault=static_cast<std::intptr_t>(e->ExceptionRecord->ExceptionInformation[1])-reinterpret_cast<std::intptr_t>(c->input);
    return EXCEPTION_EXECUTE_HANDLER;
}
static void guarded(Case* c) {
    __try {invoke(c);} __except(filter(c,GetExceptionInformation())) {}
}
static std::string quote(const std::string& s) {
    std::string out="\"";
    for(unsigned char ch:s) {if(ch=='\\'||ch=='\"')out+='\\';if(ch>=32&&ch<127)out+=ch;else out+='?';}
    return out+'\"';
}
int wmain(int argc,wchar_t** argv) {
    static_assert(sizeof(void*)==4 && sizeof(unsigned long)==4,"Original Win32 ABI");
    if(argc<4) {std::cerr<<"Usage: jng-error-probe DLL output-directory input...\n";return 2;}
    const auto dll=std::filesystem::absolute(argv[1]);
    const auto library=LoadLibraryExW(dll.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if(!library)return 3;
    auto read=reinterpret_cast<Read>(GetProcAddress(library,"?readMNG@@YAXPBXAAK1AAPAK@Z"));
    auto release=reinterpret_cast<Free>(GetProcAddress(library,"?freeData@@YAXAAPAK@Z"));
    if(!read||!release)return 4;
    unsigned long* nullPixels=nullptr;release(nullPixels);
    std::cout<<"{\"event\":\"free_null\",\"unchanged\":"<<(nullPixels==nullptr?"true":"false")<<"}"<<std::endl;
    const std::filesystem::path output(argv[2]);std::filesystem::create_directories(output);
    SYSTEM_INFO info{};GetSystemInfo(&info);
    for(int index=3;index<argc;++index) {
        std::ifstream file(std::filesystem::path(argv[index]),std::ios::binary);
        std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(file),{}};
        if(!file||bytes.empty()||bytes.size()>16*1024*1024)return 5;
        const size_t accessible=(bytes.size()+info.dwPageSize-1)/info.dwPageSize*info.dwPageSize;
        auto base=static_cast<unsigned char*>(VirtualAlloc(nullptr,accessible+info.dwPageSize,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));
        if(!base)return 6;
        DWORD old=0;if(!VirtualProtect(base+accessible,info.dwPageSize,PAGE_NOACCESS,&old))return 7;
        auto input=base+accessible-bytes.size();CopyMemory(input,bytes.data(),bytes.size());
        Case c{read,release,input};guarded(&c);
        c.pointerUnchanged=c.pixels==reinterpret_cast<unsigned long*>(0x01234560);
        if(!c.seh&&c.result=="returned"&&!c.pointerUnchanged) {
            if(static_cast<std::uint64_t>(c.width)*c.height>16*1024*1024)return 8;
            std::ofstream raw(output/(std::to_string(index-3)+".bgra"),std::ios::binary);
            raw.write(reinterpret_cast<const char*>(c.pixels),static_cast<std::streamsize>(c.width)*c.height*4);
            auto before=c.pixels;release(c.pixels);c.freePreservesPointer=before==c.pixels;
        }
        std::cout<<"{\"index\":"<<index-3<<",\"result\":"<<quote(c.seh?"seh":c.result)<<",\"message\":"<<quote(c.message)
                 <<",\"width\":"<<c.width<<",\"height\":"<<c.height<<",\"pointer_unchanged\":"<<(c.pointerUnchanged?"true":"false")
                 <<",\"free_preserves_pointer\":"<<(c.freePreservesPointer?"true":"false")<<",\"seh_code\":"<<c.seh<<",\"fault_offset\":"<<c.fault<<"}"<<std::endl;
        VirtualFree(base,0,MEM_RELEASE);
        // A fault leaves the DLL's state unspecified; never continue that process.
        if(c.seh)break;
    }
    FreeLibrary(library);return 0;
}
