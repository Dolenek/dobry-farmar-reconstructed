// Recovered application wrapper; see docs/JNGLOAD.md for original addresses.
// LIBMNG remains a source dependency; matching its original build is unfinished.
#include "JNGLoad.h"
#include <libmng.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <new>
#if defined(_WIN32)
// Keep Windows INT32 (int) separate from JPEG 6b's INT32 (long).
// The wrapper needs only this Kernel32 declaration, with the original ABI.
extern "C" __declspec(dllimport) unsigned long __stdcall GetTickCount();
#endif

namespace {
struct DecodeState {
    const unsigned char* source;
    std::uint32_t offset;
    JngULong width;
    JngULong height;
    JngULong* pixels;
};

// Original globals at 0x10055340..0x10055350: preserve its non-reentrant behavior.
DecodeState state{};

mng_bool MNG_DECL readData(mng_handle handle, mng_ptr output,
                           mng_uint32 requested, mng_uint32p read) {
    if (requested != 0) {
        auto* input = static_cast<DecodeState*>(mng_get_userdata(handle));
        std::memcpy(output, input->source + input->offset, requested);
        *read = requested;
        input->offset += requested;
    }
    return MNG_TRUE;
}

mng_ptr MNG_DECL canvasLine(mng_handle handle, mng_uint32 line) {
    auto* image = static_cast<DecodeState*>(mng_get_userdata(handle));
    return image->pixels + static_cast<std::uint32_t>(image->width * line);
}

mng_bool MNG_DECL setTimer(mng_handle, mng_uint32) { return MNG_TRUE; }
mng_bool MNG_DECL refresh(mng_handle, mng_uint32, mng_uint32,
                          mng_uint32, mng_uint32) { return MNG_TRUE; }

mng_uint32 MNG_DECL ticks(mng_handle) {
#if defined(_WIN32)
    return ::GetTickCount();
#else
    auto elapsed = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<mng_uint32>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
#endif
}

mng_ptr MNG_DECL allocate(mng_size_t size) { return std::calloc(1, size); }
void MNG_DECL release(mng_ptr memory, mng_size_t) { std::free(memory); }

void require(mng_retcode result, const char* message) {
    // Original exports throw a C++ const char* exception, not std::exception.
    if (result != MNG_NOERROR) throw message;
}
}

void JNG_CALL readMNG(const void* source, JngULong& width,
                      JngULong& height, JngULong*& pixels) {
    state.source = static_cast<const unsigned char*>(source);
    state.offset = 0;
    mng_handle handle = mng_initialize(&state, allocate, release, nullptr);
    if (!handle) throw "Can't initialize LIBMNG.";

    require(mng_setcb_readdata(handle, readData), "Error mng_setcb_readdata.");
    // The repeated registration and first error text are present in the original.
    require(mng_setcb_getcanvasline(handle, canvasLine), "Error mng_setcb_readdata.");
    require(mng_setcb_gettickcount(handle, ticks), "Error mng_setcb_gettickcount.");
    require(mng_setcb_settimer(handle, setTimer), "Error mng_setcb_settimer.");
    require(mng_setcb_getcanvasline(handle, canvasLine), "Error mng_setcb_getcanvasline.");
    require(mng_setcb_refresh(handle, refresh), "Error mng_setcb_refresh.");
    require(mng_read(handle), "Error mng_read.");

    state.width = mng_get_imagewidth(handle);
    state.height = mng_get_imageheight(handle);
    std::uint32_t bytes = state.width * state.height * 4U;
    // Original operator new ultimately uses the CRT heap and freeData uses free.
    // Explicit malloc preserves that allocation ownership without mismatched C++ new/free.
    state.pixels = static_cast<JngULong*>(std::malloc(bytes == 0 ? 1 : bytes));
    if (!state.pixels) throw std::bad_alloc();
    static_assert(MNG_CANVAS_BGRA8 == 0x1001, "Original canvas format");
    mng_set_canvasstyle(handle, MNG_CANVAS_BGRA8);
    mng_display(handle);
    pixels = state.pixels;
    width = state.width;
    height = state.height;
    mng_cleanup(&handle);
}

void JNG_CALL freeData(JngULong*& pixels) { std::free(pixels); }
