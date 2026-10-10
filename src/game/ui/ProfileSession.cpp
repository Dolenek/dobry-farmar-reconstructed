#include "ProfileSession.h"
#include "ResourceManager.h"
#include "GameProfileDefaults.h"
namespace farmar::recovered {
ProfileSession::ProfileSession(FileIndex& files,Settings& settings,ResourceManager& resources)
    :files_(files),settings_(settings),resources_(resources) {}
ProfileSession::~ProfileSession() noexcept(false) { close(); }
void ProfileSession::close() { auto* profile=profile_;profile_=nullptr;delete profile; }
void ProfileSession::open(const WString& name) {
    close();profile_=new Profile(files_,settings_,name,
        [] { initializeGameProfile(profileProperties()); },
        [this](const String& path) { return resources_.loadSettingsXML(path); },
        [this](const String& path) { resources_.releaseResource(path); });
}
}
