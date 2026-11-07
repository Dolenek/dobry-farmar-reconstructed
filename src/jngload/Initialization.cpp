// Native1003ad92 calls1003afb5(0) before C/C++ initializer arrays.
// Its1003a8a2 callback skips the precision change for this DLL argument,
// then executes FNCLEX at1003a8bd. Keep the caller's control word intact.
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error The original DLL startup requires the MSVC Win32 x87 environment.
#endif
namespace {
int __cdecl initializeNativeFloatingPoint() {
    __asm fnclex
    return 0;
}
}
#pragma section(".CRT$XIB",read)
extern "C" {
__declspec(allocate(".CRT$XIB")) int (__cdecl* nativeJngFpuInitializer)()=initializeNativeFloatingPoint;
}
