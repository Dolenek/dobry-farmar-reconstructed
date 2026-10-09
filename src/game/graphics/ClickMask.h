#pragma once
#include "GameString.h"
#include <vector>
namespace farmar::recovered {
struct ClickMaskColumn { unsigned bitCount=0; std::vector<unsigned> words; };
// 0051c040 JNG branch: X columns containing packed Y bits, from GPU alpha.
std::vector<ClickMaskColumn> loadClickMask(const String& name);
}
