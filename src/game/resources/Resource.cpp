#include "Resource.h"
namespace farmar::recovered {
namespace { std::function<void(cResource*)> destroyed; }
std::function<void(cResource*)>& resourceDestroyedCallback() noexcept { return destroyed; }
cResource::cResource(const String& name) : name_(name) {}
cResource::~cResource() { if(destroyed) destroyed(this); }
cResource* cResource::addReference() noexcept { ++references_; return this; }
bool cResource::releaseReference() noexcept {
    --references_;
    return static_cast<std::int32_t>(references_)<1;
}
}
