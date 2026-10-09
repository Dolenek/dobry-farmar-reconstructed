#pragma once
#include <cstdint>

namespace farmar::recovered {
// Original four floats in RGBA order, D3DXCOLOR-compatible storage.
struct Color {
    float r=0,g=0,b=0,a=0;
    static Color fromARGB(std::uint32_t value) noexcept; // 00417f80
    std::uint32_t toARGB() const noexcept; // 004455e0, clamp +255*x+0.5 then truncate
};
static_assert(sizeof(Color)==16,"Original color layout");
}
