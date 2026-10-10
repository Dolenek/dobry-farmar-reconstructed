#include "SoundFiles.h"
#include <stdexcept>
namespace farmar::recovered {
namespace {
SoundFiles* currentFiles=nullptr;
// Original 0065d384 contains 1 in the PE and increments before insertion.
// It persists across SoundManager destruction/recreation; first success is 2.
unsigned nextHandle=1;
}
SoundFiles::SoundFiles(FileIndex& files):files_(files) {
    if(currentFiles) throw "Object already exist.";currentFiles=this;
}
SoundFiles::~SoundFiles() { handles_.clear();currentFiles=nullptr; }
SoundFiles& SoundFiles::active() { if(!currentFiles) throw std::logic_error("Sound file callbacks are not bound");return *currentFiles; }
iFile& SoundFiles::at(unsigned handle) {
    const auto entry=handles_.find(handle);if(entry==handles_.end()) throw L"Invalid handle.";return *entry->second;
}
unsigned __cdecl SoundFiles::open(const char* name) {
    auto& self=active();auto file=self.files_.open(String(name));if(!file) return 0xffffffffU;
    const unsigned handle=++nextHandle;self.handles_[handle]=std::move(file);return handle;
}
int __cdecl SoundFiles::seek(unsigned handle,int position,int mode) {
    auto& file=active().at(handle);
    // Original callback reports size only for SEEK_END, offset 0. All other
    // requests return zero without changing the file position.
    return mode==2 && position==0 ? static_cast<int>(file.size()):0;
}
int __cdecl SoundFiles::read(unsigned handle,void* buffer,int size) {
    return static_cast<int>(active().at(handle).read(buffer,static_cast<std::uint32_t>(size)));
}
void __cdecl SoundFiles::close(unsigned handle) {
    auto& self=active();self.at(handle).close();self.handles_.erase(handle);
}
}
