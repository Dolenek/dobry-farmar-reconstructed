// Observe only actual LoadLibrary/FreeLibrary FP effects; no Squall API calls.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
struct Environment {unsigned short control{},status{};unsigned mxcsr{};};
Environment capture() {
    Environment state{};
    __asm fnstcw state.control
    __asm fnstsw state.status
    __asm stmxcsr state.mxcsr
    return state;
}
void seed(unsigned short control,int sticky,unsigned mxcsr) {
    __asm fninit
    __asm fldcw control
    __asm ldmxcsr mxcsr
    if(sticky) {
        __asm fld1
        __asm fldz
        __asm fdivp st(1),st(0)
        __asm fstp st(0)
    }
}
void print(const Environment& e) {
    std::printf("{\"control\":%u,\"status\":%u,\"mxcsr\":%u}",unsigned(e.control),unsigned(e.status),e.mxcsr);
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=4)return 2;
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);
    const auto control=static_cast<unsigned short>(std::wcstoul(argv[2],nullptr,16));const int sticky=_wtoi(argv[3]);
    if((control&0x3f)!=0x3f||(sticky!=0&&sticky!=1))return 3;
    const unsigned mxcsr=0x1f80|((control&0xc00)<<3);
    Environment before[2]{},loaded[2]{},unloaded[2]{};
    for(unsigned i=0;i<2;++i) {
        seed(control,sticky,mxcsr);before[i]=capture();
        auto module=LoadLibraryW(argv[1]);if(!module)return 4;
        loaded[i]=capture();if(!FreeLibrary(module))return 5;unloaded[i]=capture();
    }
    seed(0x027f,0,0x1f80);
    std::printf("{\"requested_control\":%u,\"sticky\":%d,\"cycles\":[",unsigned(control),sticky);
    for(unsigned i=0;i<2;++i) {
        std::printf("%s{\"before\":",i?",":"");print(before[i]);std::printf(",\"loaded\":");print(loaded[i]);std::printf(",\"unloaded\":");print(unloaded[i]);std::printf("}");
    }
    std::printf("]}\n");return 0;
}
