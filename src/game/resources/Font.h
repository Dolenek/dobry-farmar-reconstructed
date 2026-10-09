#pragma once
#include "Resource.h"
#include <map>
#include <vector>
namespace farmar::recovered {
class FileIndex;
struct FontRange { std::uint16_t first,last,index; };
struct FontGlyph { float u0,v0,u1,v1,advance; };
static_assert(sizeof(FontRange)==6 && sizeof(FontGlyph)==20,"Original packed font records");
FileIndex*& fontFiles() noexcept;
// Original 0050bd70 data/lifetime semantics, with modern map/vector storage;
// the complete original 68-byte object/container ABI is not asserted.
class cFont final : public cResource {
public:
    explicit cFont(const String& name);
    std::uint32_t resourceType() const override { return 2; } // 0050bd60
    bool isCharAvailable(std::uint16_t character) const noexcept; // 0050b3d0, AL
    bool getGlyph(std::uint16_t character,FontGlyph& output) const noexcept; // 0050b450, untouched output on false
    const std::map<std::uint16_t,FontRange>& ranges() const noexcept { return ranges_; }
    const std::vector<FontGlyph>& glyphs() const noexcept { return glyphs_; }
private:
    std::map<std::uint16_t,FontRange> ranges_; // keyed by inclusive range end
    std::vector<FontGlyph> glyphs_;
};
}
