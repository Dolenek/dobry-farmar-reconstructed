#include "ResourceManager.h"
#include "FileIndex.h"
#include "Texture.h"
#include "Shader.h"
#include "Font.h"
#include "Sound.h"
#include "XmlResource.h"
#include "StringConversion.h"
#include <memory>
#include <vector>
#include <sstream>

namespace farmar::recovered {
namespace { ResourceManager* manager=nullptr; }
ResourceManager::ResourceManager(FileIndex& files) : files_(files) {
    if(manager) throw "Object already exist.";
    manager=this;
    resourceDestroyedCallback()=[this](cResource* resource) { releaseResource(resource); };
    files_.onResourceInvalidated([this](const String& name) { releaseResource(name); });
}
ResourceManager::~ResourceManager() {
    while(!named_.empty()) delete named_.begin()->second;
    files_.onResourceInvalidated({});
    resourceDestroyedCallback()={};
    manager=nullptr;
}
cResource* ResourceManager::find(const String& normalized,std::uint32_t type) {
    const auto found=named_.find(std::string(normalized.view()));
    if(found==named_.end()) return nullptr;
    if(found->second->resourceType()!=type) throw "Wrong resource type.";
    return found->second->addReference();
}
void ResourceManager::insert(const String& normalized,cResource* resource) {
    named_.emplace(std::string(normalized.view()),resource);
    names_.emplace(resource,normalized);
    resources_.insert(resource);
}
cTexture* ResourceManager::getTexture(const String& name) {
    String normalized(name); normalized.lowercase();
    if(auto* resource=find(normalized,0)) {
        resources_.insert(resource);
        return static_cast<cTexture*>(resource);
    }
    auto resource=std::make_unique<cTexture>(normalized);
    insert(normalized,resource.get());
    return resource.release();
}
XmlResource* ResourceManager::getXML(const String& name) {
    String normalized(name); normalized.lowercase();
    if(auto* resource=find(normalized,1)) {
        resources_.insert(resource);
        return static_cast<XmlResource*>(resource);
    }
    auto resource=std::make_unique<XmlResource>(files_,normalized);
    insert(normalized,resource.get());
    return resource.release();
}
cShader* ResourceManager::getShader(const String& name) {
    String normalized(name); normalized.lowercase();
    if(auto* resource=find(normalized,3)) {
        resources_.insert(resource);
        return static_cast<cShader*>(resource);
    }
    auto resource=std::make_unique<cShader>(normalized);
    insert(normalized,resource.get());
    return resource.release();
}
cFont* ResourceManager::getFont(const String& name) {
    String normalized(name); normalized.lowercase();
    // Unlike texture/shader public factories, original getFont does not insert
    // into the address-ordered set when reusing an existing resource.
    if(auto* resource=find(normalized,2)) return static_cast<cFont*>(resource);
    auto resource=std::make_unique<cFont>(normalized);
    insert(normalized,resource.get()); return resource.release();
}
cSound* ResourceManager::getSound(const String& name) {
    String normalized(name);normalized.lowercase();
    // Original public sound lookup, like font, does not reinsert a cache hit.
    if(auto* resource=find(normalized,4)) return static_cast<cSound*>(resource);
    auto resource=std::make_unique<cSound>(normalized);
    insert(normalized,resource.get());return resource.release();
}
std::shared_ptr<const XmlNode> ResourceManager::loadSettingsXML(const String& name) {
    auto* resource=getXML(name);
    // Original cSettings calls the iXML deleting destructor, irrespective of
    // reference count. This owned view reproduces that call at scope exit.
    return {&resource->document(),[resource](const XmlNode*) { delete resource; }};
}
void ResourceManager::releaseResource(cResource* resource) {
    // Original reverse-map operator[] inserts an empty name for unknown pointers.
    // This also occurs when a detached object is eventually destroyed.
    const auto& name=names_[resource];
    named_.erase(std::string(name.view()));
    names_.erase(resource);
    resources_.erase(resource);
}
void ResourceManager::releaseResource(const String& name) {
    const auto found=named_.find(std::string(name.view()));
    if(found==named_.end()) return;
    auto* resource=found->second;
    named_.erase(found); names_.erase(resource); resources_.erase(resource);
}
const String& ResourceManager::getResourceName(cResource* resource) const {
    const auto found=names_.find(resource);
    if(found==names_.end()) throw "Unknown resource.";
    return found->second;
}
void ResourceManager::preResetGraphics() { for(auto* resource:resources_) resource->preResetGraphics(); }
void ResourceManager::restoreGraphics() { for(auto* resource:resources_) resource->restoreGraphics(); }
void ResourceManager::refresh() {
    for(const auto& item:named_) if(item.second->resourceType()==0) {
        item.second->preResetGraphics(); item.second->restoreGraphics();
    }
}
void ResourceManager::clearUseless() {
    std::vector<cResource*> unused;
    unused.reserve(named_.size());
    for(const auto& item:named_) if(item.second->referenceCount()<1) unused.push_back(item.second);
    for(auto* resource:unused) delete resource;
}
void ResourceManager::clear() { // Original uses a list snapshot; deletion order is identical.
    std::vector<cResource*> unused;
    for(const auto& item:named_) if(item.second->referenceCount()<1) unused.push_back(item.second);
    for(auto* resource:unused) delete resource;
}
void ResourceManager::deleteAllSound() {
    std::vector<cResource*> sounds;
    for(auto* resource:resources_) if(resource->resourceType()==4) sounds.push_back(resource);
    for(auto* resource:sounds) delete resource;
}
void ResourceManager::loadFromLog(const XmlNode& log,unsigned first,unsigned end) {
    // The original reuses the first argument's stack slot for Type. Missing
    // attributes preserve the last parsed type, initially the starting index.
    unsigned type=first;
    for(unsigned index=first;index<end;++index) {
        XmlNode* entry=nullptr;if(!log.section(index,entry)) continue;
        const auto name=toNarrow(entry->attribute(WString(L"Path")));
        const auto value=entry->attribute(WString(L"Type"));
        if(!value.empty()) {std::wistringstream stream(value.c_str());stream>>type;}
        cResource* resource=nullptr;
        switch(type) {
        case 0:resource=getTexture(name);break;
        case 1:delete getXML(name);continue; // iXML deleting destructor, not releaseReference
        case 2:resource=getFont(name);break;
        case 3:resource=getShader(name);break;
        case 4:resource=getSound(name);break;
        default:continue; // original ignores all other resource types
        }
        resource->releaseReference();
    }
}
void ResourceManager::logResources(const String& filename) {
    std::wostringstream stream;
    stream<<"<?xml version=\"1.0\" encoding=\"UTF-16\"?>\n"<<L"<ResourceLog>\n";
    for(const auto& entry:named_)
        stream<<L"<Tag Path=\""<<entry.first.c_str()<<L"\" Type=\""
              <<static_cast<std::int32_t>(entry.second->resourceType())<<L"\" />\n";
    stream<<L"</ResourceLog>";
    // Create invalidates a cached resource with the output name. The original
    // completes serialization first, so that entry must remain in this output.
    const auto text=stream.str();
    auto file=files_.create(filename);
    file->write(text.data(),static_cast<unsigned>(text.size()*sizeof(wchar_t)));
}
}
