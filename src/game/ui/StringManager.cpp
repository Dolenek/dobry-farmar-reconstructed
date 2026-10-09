#include "StringManager.h"
#include "FileIndex.h"
#include "Settings.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
namespace farmar::recovered {
namespace { StringManager* singleton=nullptr; WString fallback; }
StringManager::StringManager(FileIndex& files,Settings& settings,ResourceManager& resources) : files_(files) {
    if(singleton) throw "Object already exist.";
    settings.get(String("game"),String("DumpStrings"),dump_);
    if(files.isExist(String("sys/strings.xml"))) {
        auto document=resources.loadSettingsXML(String("sys/strings.xml"));
        for(const auto& child:document->children()) {
            XmlNode *base=nullptr,*translation=nullptr;
            WString key,value;
            if(child.second->section(String("text"),base)) key=base->text();
            if(child.second->section(String("translation"),translation)) value=translation->text();
            translations_[std::wstring(key.view())]=value;
        }
    }
    singleton=this;
}
const WString& StringManager::getString(const WString& source) const {
    if(source.empty()) return source;
    const std::wstring key(source.view()); const auto found=translations_.find(key);
    if(found!=translations_.end()) return found->second;
    fallback=source;
    if(dump_) {
        fallback=WString(L"!!!!!!!"); fallback.append(source);
        translations_[key]=fallback; missing_.insert(key);
    }
    return fallback;
}
StringManager::~StringManager() noexcept(false) {
    if(dump_) {
        // Original writes UTF-16 characters without a BOM or escaping markup.
        std::wstring output=L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>\n<Strings>";
        for(const auto& text:missing_) output+=L"<s>\n<text>"+text+L"</text>\n<translation>"+text+L"</translation>\n</s>";
        output+=L"</Strings>";
        auto file=files_.create(String("strdubp.xml")); file->write(output.data(),static_cast<unsigned>(output.size()*2));
    }
    singleton=nullptr;
}
}
