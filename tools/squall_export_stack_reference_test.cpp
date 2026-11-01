// All97 real exports: declared C++ contracts plus measured native/source ESP.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "Squall.h"
#include <array>
#include <filesystem>
#include <float.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
namespace {
void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
struct Api {
#define SQUALL_ENTRY(name) decltype(&name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    HMODULE module{};
    explicit Api(const wchar_t* path){module=LoadLibraryW(std::filesystem::absolute(path).c_str());need(module!=nullptr,"Load DLL");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));need(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    }
    ~Api(){if(module){SQUALL_Free();SQUALL_Sample_UnloadAll();FreeLibrary(module);}}
};
HWND window{};
const char* activeExport="";
void* __cdecl allocate(unsigned size){return HeapAlloc(GetProcessHeap(),0,size);}
void __cdecl release(void* pointer){if(pointer)HeapFree(GetProcessHeap(),0,pointer);}
unsigned __cdecl open(const char*){return ~0U;}
int __cdecl read(unsigned,void*,int){return 0;}
int __cdecl seek(unsigned,int,int){return -1;}
void __cdecl close(unsigned){}
void __cdecl listener(void*){}
int __cdecl worker(int,void*){return 1;}
template<typename T> T argument(){
    if constexpr(std::is_same_v<T,int>)return -1;
    else if constexpr(std::is_same_v<T,unsigned int>)return 100;
    else if constexpr(std::is_same_v<T,float>)return 1;
    else if constexpr(std::is_same_v<T,squall_callback_alloc_t>)return allocate;
    else if constexpr(std::is_same_v<T,squall_callback_free_t>)return !std::strcmp(activeExport,"SQUALL_Listener_SetWorker")?listener:release;
    else if constexpr(std::is_same_v<T,squall_callback_open_t>)return open;
    else if constexpr(std::is_same_v<T,squall_callback_read_t>)return read;
    else if constexpr(std::is_same_v<T,squall_callback_seek_t>)return seek;
    else if constexpr(std::is_same_v<T,squall_callback_close_t>)return close;
    else if constexpr(std::is_same_v<T,squall_callback_listener_t>)return listener;
    else if constexpr(std::is_same_v<T,squall_callback_channel_t>)return worker;
    else if constexpr(std::is_same_v<T,void*>){static unsigned words[64]{};return words;}
    else if constexpr(std::is_same_v<T,char*>){static char file[512]="sample.wav";return file;}
    else if constexpr(std::is_same_v<T,float*>){static float words[64]{};return words;}
    else if constexpr(std::is_same_v<T,int*>){static int words[64]{};return words;}
    else if constexpr(std::is_same_v<T,squall_parameters_t*>){static squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=4;p.UseAlg=1;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;return &p;}
    else if constexpr(std::is_same_v<T,squall_sample_default_t*>){static squall_sample_default_t p{};p.SampleGroupID=17;p.Volume=0;p.Pan=50;p.MinDist=1;p.MaxDist=1000;return &p;}
    else if constexpr(std::is_pointer_v<T>){static std::remove_pointer_t<T> p{};return &p;}
    else {static_assert(!sizeof(T),"Unhandled ABI argument type");}
}
struct Result {int value;std::intptr_t stackDelta;};
template<typename R,typename... Args,std::size_t... I>
__declspec(noinline) Result invoke(R(__cdecl* function)(Args...),std::index_sequence<I...>){
    static_assert((...&&(sizeof(Args)==4)),"All native API argument slots must be DWORD");
    static_assert(std::is_void_v<R>||std::is_same_v<R,int>,"Native public return register contract");
    volatile std::uint32_t canaries[16];for(unsigned i=0;i<16;++i)canaries[i]=0x71ac0000+i;
    std::uintptr_t before{},after{};volatile int value{};
    __asm {mov before,esp}
    if constexpr(std::is_void_v<R>)function((static_cast<void>(I),argument<Args>())...);
    else value=function((static_cast<void>(I),argument<Args>())...);
    __asm {mov after,esp}
    // Keep the diagnostic frame usable even if a tested target pops arguments.
    __asm {mov esp,before}
    for(unsigned i=0;i<16;++i)need(canaries[i]==0x71ac0000+i,"ABI stack canary changed");
    return {value,static_cast<std::intptr_t>(after-before)};
}
template<typename R,typename... Args>std::string signature(R(__cdecl*)(Args...)){return __FUNCSIG__;}
unsigned entries{};
template<typename R,typename... Args>void check(const char* name,R(__cdecl* source)(Args...),R(__cdecl* native)(Args...)){
    activeExport=name;
    const auto a=invoke(source,std::index_sequence_for<Args...>{}),b=invoke(native,std::index_sequence_for<Args...>{});
    if(a.stackDelta||b.stackDelta){std::cerr<<name<<" sourceESP="<<a.stackDelta<<" nativeESP="<<b.stackDelta<<'\n';throw std::runtime_error("Caller cleanup stack differs");}
    if(entries++)std::cout<<',';
    std::cout<<"{\"name\":\""<<name<<"\",\"declared_signature\":\""<<signature(source)<<"\",\"argument_count\":"<<sizeof...(Args)<<",\"declared_stack_argument_bytes\":"<<(0U+...+unsigned(sizeof(Args)))<<",\"declared_return_bytes\":"<<(std::is_void_v<R>?0:4)<<",\"source_stack_delta\":"<<a.stackDelta<<",\"native_stack_delta\":"<<b.stackDelta<<",\"stack_canaries_per_call\":16,\"source_return_word\":"<<a.value<<",\"native_return_word\":"<<b.value<<'}';
}
}
int wmain(int argc,wchar_t** argv){
    if(argc<3||argc>4)return 2;
    try{
        Api native(argv[1]),source(argv[2]);const auto precision=argc==4?_PC_24:_PC_53;_control87(precision,_MCW_PC);
        window=CreateWindowExW(0,L"STATIC",L"Squall export ABI",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);need(window!=nullptr,"Create Init window");
        std::cout<<"{\"export_lookups\":194,\"measured_exports\":[";
#define SQUALL_ENTRY(name) if(std::strcmp(#name,"SQUALL_Init"))check(#name,source.name,native.name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
        // Channel calls above deliberately use invalid IDs before Init. Valid
        // channel behavior is covered by the separate real-buffer harnesses.
        check("SQUALL_Init",source.SQUALL_Init,native.SQUALL_Init);
        need(entries==97,"Missing export stack measurement");need((_control87(0,0)&_MCW_PC)==precision,"Precision changed");
        source.SQUALL_Free();native.SQUALL_Free();DestroyWindow(window);window=nullptr;
        std::cout<<"],\"export_count\":"<<entries<<",\"actual_init_window\":true,\"native_argument_count_statically_proved\":false}\n";return 0;
    }catch(const std::exception& error){if(window)DestroyWindow(window);std::cerr<<error.what()<<'\n';return 1;}
}
