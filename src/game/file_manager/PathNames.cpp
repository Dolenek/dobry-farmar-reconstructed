#include "PathNames.h"

namespace farmar::recovered {
const String& PathNames::getFileExt(const String& file) {
    static String result; // Original storage 0x00660610; reused across calls.
    const auto dot = file.rfind('.');
    if (dot == String::npos) result.erase();
    else result.assign(file, dot + 1);
    result.lowercase();
    return result;
}

const String& PathNames::getFileName(const String& file) {
    static String result; // Original separate storage 0x00660630.
    const auto slash = file.rfind('/');
    result.assign(file, slash == String::npos ? 0 : slash + 1);
    result.lowercase();
    return result;
}
}
