#pragma once
#include "Resource.h"
#include <map>
#include <set>
#include <string>
#include <memory>

namespace farmar::recovered {
class FileIndex;
class cTexture;
class cShader;
class cFont;
class cSound;
class XmlResource;
class XmlNode;
// Recovered manager cache/lifetime behavior. Modern containers do not reproduce
// the original 40-byte manager ABI. Remaining resource factories are pending.
class ResourceManager {
public:
    explicit ResourceManager(FileIndex& files); // 00548100, singleton call sites
    ~ResourceManager(); // 005455c0
    ResourceManager(const ResourceManager&)=delete;
    ResourceManager& operator=(const ResourceManager&)=delete;
    cTexture* getTexture(const String& name); // 00545e80 / 00544260
    cShader* getShader(const String& name); // 00545a50 / 00543da0
    cFont* getFont(const String& name); // 00546770 / 00544720
    cSound* getSound(const String& name); // 00546fb0 / 005450a0
    XmlResource* getXML(const String& name); // 005473d0
    std::shared_ptr<const XmlNode> loadSettingsXML(const String& name); // settings deletes iXML directly
    void releaseResource(cResource* resource); // 005478c0, removes cache entries only
    void releaseResource(const String& name); // 00547ce0, exact case query
    const String& getResourceName(cResource* resource) const; // 00540b90
    void preResetGraphics(); // 005403b0, pointer order
    void restoreGraphics(); // 005407a0, pointer order
    void refresh(); // 00540f90, name order, only type 0
    void clearUseless(); // 005426b0, signed reference count <1
    void clear(); // 00542b60, likewise only signed reference count <1
    void deleteAllSound(); // 00542fd0, type 4 regardless of reference count
    std::size_t cachedCount() const noexcept { return named_.size(); }
private:
    struct PointerLess {
        bool operator()(const cResource* a,const cResource* b) const noexcept {
            return reinterpret_cast<std::uintptr_t>(a)<reinterpret_cast<std::uintptr_t>(b);
        }
    };
    FileIndex& files_;
    std::map<std::string,cResource*> named_;
    std::map<cResource*,String,PointerLess> names_;
    std::set<cResource*,PointerLess> resources_;
    cResource* find(const String& normalized,std::uint32_t type);
    void insert(const String& normalized,cResource* resource);
};
}
