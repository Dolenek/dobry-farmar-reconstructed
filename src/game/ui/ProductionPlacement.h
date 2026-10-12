#pragma once
#include "Control.h"
#include "ClickMask.h"
#include <array>
namespace farmar::recovered {
using ProductionRectangle=std::array<float,4>; // left/top/right/bottom, native 16 bytes
bool productionPointInside(const ProductionRectangle&,const Point2&) noexcept; // 004178c0
bool productionRectangleContains(const ProductionRectangle&,const Control*) noexcept; // 00417d10
bool productionRectangleTouches(const ProductionRectangle&,const Control*) noexcept; // 00417b90, original corner tests
bool productionMasksOverlap(const Control&,const Control&,const std::vector<ClickMaskColumn>*,const std::vector<ClickMaskColumn>*); // 004f0510
}
