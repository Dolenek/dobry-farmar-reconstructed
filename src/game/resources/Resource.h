#pragma once
#include "GameString.h"
#include <cstdint>
#include <functional>

namespace farmar::recovered {
#pragma pack(push,4)
class cResource {
public:
    explicit cResource(const String& name); // 0050ae80
    virtual ~cResource(); // 0050ada0
    virtual std::uint32_t resourceType() const=0; // original purecall; texture 0, sound 4
    virtual void preResetGraphics() {} // original shared empty 004bd820
    virtual void restoreGraphics() {} // original shared empty 004bd820
    virtual cResource* addReference() noexcept; // 0050ae20
    virtual bool releaseReference() noexcept; // 0050ae30; reports signed count <1, never deletes
    virtual const String& resourceName() const noexcept { return name_; } // 0050ae50
    std::int32_t referenceCount() const noexcept { return static_cast<std::int32_t>(references_); }
private:
    std::uint32_t references_=1;
    String name_;
};
#pragma pack(pop)
static_assert(sizeof(cResource)==36,"Original resource base layout");
// Original destructor calls the resource-manager singleton if it exists. Bind
// this recovered call site when the complete manager is available.
std::function<void(cResource*)>& resourceDestroyedCallback() noexcept;
}
