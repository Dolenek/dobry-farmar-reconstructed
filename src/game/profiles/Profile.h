#pragma once
#include "ProfileHeaders.h"
#include "ProfileProperties.h"
#include <functional>
#include <memory>
namespace farmar::recovered {
class FileIndex;class Settings;class XmlNode;
// Recovered profile persistence lifecycle. The engine's initialization callback
// must be supplied by the game; its game-specific defaults are separate work.
class Profile {
public:
    using XmlLoader=std::function<std::shared_ptr<const XmlNode>(const String&)>;
    using Initializer=std::function<void()>;
    using Invalidator=std::function<void(const String&)>;
    Profile(FileIndex& files,Settings& settings,const WString& selectedName,Initializer initialize={},XmlLoader loader={},Invalidator invalidate={}); // 004ecb20
    ~Profile() noexcept(false); // 004ead60
    Profile(const Profile&)=delete;Profile& operator=(const Profile&)=delete;
    void load(); // 004ec310
    void create(); // 004ea5d0, current selected name, even an existing name consumes an ID
    void saveHeaders(); // 004e3a70
    void saveProperties(); // 004e6cf0
    void remove(const WString& name); // 004e8d20, selects first remaining name and suppresses destructor writes
    const ProfileHeaders& headers() const noexcept { return headers_; }
    ProfileProperties& properties() noexcept { return profileProperties(); }
    String directory(); // 004e1010
    static Profile* instance() noexcept;
    static bool headersExist(FileIndex& files,Settings& settings); // 004e14e0
    static WString storedName(FileIndex& files,Settings& settings,XmlLoader loader={}); // 004e15e0, startup selection
private:
    FileIndex& files_;Settings& settings_;ProfileHeaders headers_;
    Initializer initialize_;XmlLoader xmlLoader_;Invalidator invalidate_;bool suppressSave_=false;
    std::shared_ptr<const XmlNode> loadXML(const String& filename);
    void write(const String& filename,const WString& text);
};
}
