#include "GameColor.h"
#include "OriginalArithmetic.h"
namespace farmar::recovered {
Color Color::fromARGB(std::uint32_t value) noexcept {
    constexpr float scale=0.0039215688593685626983642578125f; // original 005ef628 bits 3b808081
    return {static_cast<float>((value>>16)&255)*scale,static_cast<float>((value>>8)&255)*scale,
            static_cast<float>(value&255)*scale,static_cast<float>(value>>24)*scale};
}
std::uint32_t Color::toARGB() const noexcept {
    const auto channel=[](float value) -> std::uint32_t {
        if(value>=1) return 255;
        if(value<0) return 0;
        return static_cast<std::uint32_t>(originalAdd(originalMultiply(value,255.0),0.5));
    };
    return (channel(a)<<24)|(channel(r)<<16)|(channel(g)<<8)|channel(b);
}
}
