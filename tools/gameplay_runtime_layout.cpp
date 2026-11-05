// Compiler-produced metadata only. No Game/Gameplay construction or game calls.
#include "Gameplay.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
using namespace farmar::recovered;

// Explicit template instantiation permits naming the private data member.
// Read the compiler's actual single-inheritance MSVC member-pointer offset;
// do not modify access specifiers or construct an uninitialized game object.
struct CoefficientTag {
    using type = std::vector<float> Gameplay::*;
    friend type member(CoefficientTag);
};
template<class Tag, typename Tag::type Pointer> struct ExposeMember {
    friend typename Tag::type member(Tag) { return Pointer; }
};
template struct ExposeMember<CoefficientTag, &Gameplay::coefficients_>;

int main() {
    const auto pointer = member(CoefficientTag{});
    static_assert(sizeof(pointer) == 4 && sizeof(void*) == 4,
                  "Read-only observer requires actual Win32 single-inheritance metadata");
    std::uint32_t offset = 0;
    std::memcpy(&offset,&pointer,sizeof(offset));
    if(offset+sizeof(std::vector<float>) > sizeof(Gameplay)) return 2;
    std::printf("{\"gameplay_size\":%zu,\"coefficient_vector\":%u,\"vector_bytes\":%zu,\"event_type\":%zu,\"event_data\":%zu,\"contract_requirements\":%zu,\"contract_bonus\":%zu,\"contract_penalty\":%zu}\n",
        sizeof(Gameplay),offset,sizeof(std::vector<float>),offsetof(GameplayEvent,type),offsetof(GameplayEvent,data),
        offsetof(ContractEvent,requirements),offsetof(ContractEvent,bonus),offsetof(ContractEvent,penalty));
}
