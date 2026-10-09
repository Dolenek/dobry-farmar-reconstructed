#pragma once
#include "Resource.h"
#include "XmlDocument.h"

namespace farmar::recovered {
class FileIndex;
// Restored XML resource ownership operations, using the recovered parser tree.
// The original 96-byte multiple-inheritance iXML/cResource ABI is still pending.
class XmlResource final : public cResource {
public:
    XmlResource(FileIndex& files,const String& name) : cResource(name),document_(files,name) {}
    std::uint32_t resourceType() const override { return 1; } // 00561160
    bool releaseReference() noexcept override { return true; } // 00561190 -> 00415340, no decrement
    const XmlDocument& document() const noexcept { return document_; }
private:
    XmlDocument document_;
};
}
