// Original1003e814 runs1003cf10 before its C/C++ initializer arrays. Modern
// shared CRT startup does not supply that old DLL's observable FP setup.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This initialization implements the original MSVC Win32 x87 environment.
#endif
namespace {
unsigned savedMxcsr;
bool hasSse;
int __cdecl initializeNativeFloatingPoint() {
    // The original only changes x87 PC to53 and clears exception flags. It
    // preserves rounding/masks, the x87 stack and the caller's SSE state.
    hasSse=IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE)!=FALSE;
    if(hasSse) { __asm stmxcsr savedMxcsr }
    unsigned short control;
    __asm fwait
    __asm fnstcw control
    control=static_cast<unsigned short>((control&~0x0300u)|0x0200u);
    __asm fldcw control
    __asm fnclex
    return 0;
}
}
// C initialization precedes the user C++ constructors that compute the sound
// tables. This object is linked only into Squall.dll, not the static probes.
#pragma section(".CRT$XIB",read)
extern "C" {
__declspec(allocate(".CRT$XIB")) int (__cdecl* nativeSquallFpuInitializer)()=initializeNativeFloatingPoint;
BOOL WINAPI DllMain(HINSTANCE,DWORD reason,LPVOID) {
    // Modern CRT math used by the constructors can set SSE exception flags;
    // original x87-only startup did not expose those changes to the caller.
    if(reason==DLL_PROCESS_ATTACH && hasSse) { __asm ldmxcsr savedMxcsr }
    return TRUE;
}
}
