#pragma once
#include <cstdint>
namespace farmar::recovered {
class cTexture;
class ResourceManager;
class XmlNode;
// Independent cAnimable resource owner, 004d2080/004d2130. This is not a View.
// Modern layout differs from the original 40 bytes. Mesh/Animation remain
// explicit unsupported resource paths until their real factories are restored.
class AnimableAppearance final {
public:
    AnimableAppearance()=default;
    AnimableAppearance(const AnimableAppearance&)=delete;
    AnimableAppearance& operator=(const AnimableAppearance&)=delete;
    ~AnimableAppearance();
    void loadXML(const XmlNode& node,ResourceManager& resources); // 004d22a0
    cTexture* texture=nullptr;
    std::uint32_t frames=1,columns=1,rows=1;
    float length=1;
    bool loop=true,playing=false;
};
}
