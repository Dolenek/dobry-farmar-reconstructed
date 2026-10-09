#pragma once
#include "GameString.h"

namespace farmar::recovered {
// Recovered text semantics of two cFileManager methods; original class ABI pending.
class PathNames {
public:
    // Original 0x00549500. Returns lowercase text after the final dot.
    static const String& getFileExt(const String& file);
    // Original 0x00549a60. Only '/' is a separator; output is lowercase.
    static const String& getFileName(const String& file);
};
}
