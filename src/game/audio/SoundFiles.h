#pragma once
#include "FileIndex.h"
#include <cstdint>
#include <map>
#include <memory>
namespace farmar::recovered {
// Native-verified file callback portion of cSoundManager, using real recovered
// file operations. Complete Squall binding is still a separate dependency.
class SoundFiles final {
public:
    explicit SoundFiles(FileIndex& files);
    ~SoundFiles();
    SoundFiles(const SoundFiles&)=delete;
    SoundFiles& operator=(const SoundFiles&)=delete;
    static unsigned __cdecl open(const char* name); // 00535900
    static int __cdecl seek(unsigned handle,int position,int mode); // 00533ea0
    static int __cdecl read(unsigned handle,void* buffer,int size); // 00533a90
    static void __cdecl close(unsigned handle); // 005342c0
    std::size_t openCount() const noexcept { return handles_.size(); }
private:
    FileIndex& files_;
    std::map<unsigned,std::unique_ptr<iFile>> handles_;
    iFile& at(unsigned handle);
    static SoundFiles& active();
};
}
