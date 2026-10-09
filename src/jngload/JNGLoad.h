#pragma once
#include <cstdint>

#if defined(_WIN32)
#  if defined(JNGLOAD_BUILD_DLL)
#    define JNG_API __declspec(dllexport)
#  else
#    define JNG_API __declspec(dllimport)
#  endif
#  define JNG_CALL __cdecl
using JngULong = unsigned long;
#else
// Linux build is a decoder diagnostic, not the Windows ABI artifact.
#  define JNG_API
#  define JNG_CALL
using JngULong = std::uint32_t;
#endif

static_assert(sizeof(JngULong) == 4, "Original API uses 32-bit unsigned long");

// The original API has no input length. Supply a complete, valid image buffer.
JNG_API void JNG_CALL readMNG(const void* source, JngULong& width,
                            JngULong& height, JngULong*& pixels);
// Preserves the original contract: releases pixels, but does not null the reference.
JNG_API void JNG_CALL freeData(JngULong*& pixels);
