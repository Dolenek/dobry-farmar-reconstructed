#include "Profile.h"
#include "FileIndex.h"
#include "Settings.h"
#include "XmlDocument.h"
#include <sstream>
namespace farmar::recovered {
namespace {
Profile* currentProfile=nullptr;
String join(const String& prefix,const String& suffix) { return String(std::string(prefix.view())+std::string(suffix.view())); }
}
Profile* Profile::instance() noexcept { return currentProfile; }
Profile::Profile(FileIndex& files,Settings& settings,const WString& selectedName,Initializer initialize,XmlLoader loader,Invalidator invalidate)
    :files_(files),settings_(settings),headers_(selectedName),initialize_(std::move(initialize)),xmlLoader_(std::move(loader)),invalidate_(std::move(invalidate)) {
    if(currentProfile) throw "Object already exist.";
    currentProfile=this;
    if(initialize_) initialize_();
    load();
}
Profile::~Profile() noexcept(false) {
    if(!suppressSave_) {
        headers_.nowPlaying_=false;headers_.dirty_=true;saveHeaders();saveProperties();
    }
    properties().clear();currentProfile=nullptr;
}
String Profile::directory() {
    String path;settings_.get(String("game"),String("profiles"),path);
    std::string text(path.view());if(!text.empty() && text.back()!='/') text+='/';return String(text);
}
std::shared_ptr<const XmlNode> Profile::loadXML(const String& filename) {
    return xmlLoader_ ? xmlLoader_(filename):std::make_shared<XmlDocument>(files_,filename);
}
void Profile::write(const String& filename,const WString& text) {
    auto file=files_.create(filename);const wchar_t bom=0xfeff;file->write(&bom,2);file->write(text.data(),text.size()*2);
}
void Profile::create() {
    const unsigned id=headers_.nextID_++;headers_.names_.emplace(headers_.currentName_,id);
    properties().clear();if(initialize_) initialize_();headers_.dirty_=true;
}
void Profile::load() {
    if(headers_.currentName_.empty()) return;
    const String path=directory();const String filename=join(path,String("profiles.xml"));
    if(!files_.isExist(filename)) create();
    else {
        { const auto xml=loadXML(filename);headers_.loadXML(*xml); }
        if(headers_.currentName_.view()!=headers_.storedCurrentName_.view()) {
            // 004ec44a places stored name (+8) in ECX and selected name (+24)
            // on the stack: the caller's selection replaces the stored name.
            headers_.storedCurrentName_=headers_.currentName_;headers_.dirty_=true;
        }
        unsigned id=0;if(!headers_.findID(headers_.currentName_,id)) create();
        const String propertyFile=join(join(path,headers_.currentID()),String(".xml"));
        if(files_.isExist(propertyFile)) { const auto xml=loadXML(propertyFile);properties().loadXML(*xml); }
    }
    headers_.nowPlaying_=true;headers_.dirty_=true;saveHeaders();
}
void Profile::saveHeaders() {
    if(!headers_.dirty_) return;
    headers_.dirty_=false;const String filename=join(directory(),String("profiles.xml"));
    if(headers_.names_.empty()) { if(invalidate_) invalidate_(filename);files_.removeCommon(filename);return; }
    std::wostringstream stream;
    stream<<L"<?xml version=\"1.0\" encoding=\"UTF-16\"?>\n<Profiles NextID=\""<<headers_.nextID_
        <<L"\" CurrentUser=\"";stream.write(headers_.currentName_.data(),headers_.currentName_.size());
    stream<<L"\" NowPlaying=\""<<headers_.nowPlaying_<<L"\" >\n";
    for(const auto& item:headers_.names_) {
        stream<<L"<Profile Name=\"";stream.write(item.first.data(),item.first.size());stream<<L"\" ID=\""<<item.second<<L"\" />\n";
    }
    stream<<L"</Profiles>";write(filename,WString(stream.str()));
}
void Profile::saveProperties() {
    if(headers_.currentName_.empty()) return;
    write(join(join(directory(),headers_.currentID()),String(".xml")),properties().dumpXML());
}
}
