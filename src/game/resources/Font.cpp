#include "Font.h"
#include "FileIndex.h"
#include <cstring>
#include <stdexcept>
namespace farmar::recovered {
namespace { FileIndex* files=nullptr; }
FileIndex*& fontFiles() noexcept { return files; }
cFont::cFont(const String& name) : cResource(name) {
    auto file=files->open(name); std::vector<unsigned char> bytes(file->size()); file->read(bytes.data(),bytes.size()); file.reset();
    const auto read=[&](std::size_t offset,void* output,std::size_t count) {
        if(offset>bytes.size() || count>bytes.size()-offset) throw std::invalid_argument("Truncated font data");
        std::memcpy(output,bytes.data()+offset,count);
    };
    std::uint32_t magic=0,count=0; read(0,&magic,4);
    if(magic!=0x666f6e74U) throw std::runtime_error("Specified resourse ("+std::string(name.view())+") is not font.");
    read(4,&count,4); std::size_t offset=8;
    for(std::uint32_t i=0;i<count;++i) {
        FontRange range; read(offset,&range,6); offset+=6; ranges_[range.last]=range;
    }
    read(offset,&count,4); offset+=4;
    if(count>(bytes.size()-offset)/20) throw std::invalid_argument("Truncated font glyphs");
    glyphs_.resize(count); read(offset,glyphs_.data(),std::size_t(count)*20);
}
bool cFont::isCharAvailable(std::uint16_t character) const noexcept {
    const auto found=ranges_.lower_bound(character);
    return found!=ranges_.end() && found->second.first<=character && character<=found->second.last;
}
bool cFont::getGlyph(std::uint16_t character,FontGlyph& output) const noexcept {
    const auto found=ranges_.lower_bound(character);
    if(found==ranges_.end() || character<found->second.first || found->second.last<character) return false;
    const auto& range=found->second;
    output=glyphs_[static_cast<std::uint32_t>(range.index)-range.first+character]; return true;
}
}
