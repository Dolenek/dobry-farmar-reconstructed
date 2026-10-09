#pragma once
#include "GameString.h"
#include "GameWString.h"

namespace farmar::recovered {
String toNarrow(const WString& text); // 0040da20
WString toWide(const String& text); // 0040daf0
}
