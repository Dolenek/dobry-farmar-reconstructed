#pragma once
#include "GameString.h"
#include <vector>
namespace farmar::recovered {
struct ClickMaskColumn { unsigned bitCount=0; std::vector<unsigned> words; };
// 0051c040: X columns containing packed Y bits, from GPU alpha. JNG and
// native A8R8G8B8 images; conversion from other native formats remains pending.
std::vector<ClickMaskColumn> loadClickMask(const String& name);
}
