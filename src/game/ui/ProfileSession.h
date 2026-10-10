#pragma once
#include "Profile.h"
namespace farmar::recovered {
class ResourceManager;
// Owns the Profile singleton across menu selection. Native controllers delete
// the current profile and construct 004ecb20 for the requested name; this owner
// executes the recovered persistence/default/resource pipeline for that action.
class ProfileSession final {
public:
    ProfileSession(FileIndex& files,Settings& settings,ResourceManager& resources);
    ~ProfileSession() noexcept(false);
    ProfileSession(const ProfileSession&)=delete;
    ProfileSession& operator=(const ProfileSession&)=delete;
    Profile* current() const noexcept { return profile_; }
    void close();
    void open(const WString& name);
private:
    FileIndex& files_;Settings& settings_;ResourceManager& resources_;Profile* profile_=nullptr;
};
}
