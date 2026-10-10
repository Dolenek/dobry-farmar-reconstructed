#pragma once
#include "Resource.h"
namespace farmar::recovered {
class cSound final : public cResource {
public:
    explicit cSound(const String& name); // 00559570
    ~cSound() override; // 00559970
    std::uint32_t resourceType() const override { return 4; } // 00559960
    int sampleID() const noexcept { return sample_; }
private:
    int sample_=0;
};
static_assert(sizeof(cSound)==40,"Original cSound resource layout");
}
