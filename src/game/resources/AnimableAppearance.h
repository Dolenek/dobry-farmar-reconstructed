#pragma once
#include <cstdint>
#include <cstring>
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
    void copyAppearance(const AnimableAppearance&); // 004d2160, preserves playing and raw padding
    cTexture* texture=nullptr;
    std::uint32_t frames=1,columns=1,rows=1;
    float length=1;
    bool loop=true;
    // Native +11..+13 are untouched by the constructor/XML/copyAppearance.
    // ProductionBuilding's level -1 capacity read overlaps loop plus these
    // bytes. Keep them as raw, unspecified storage instead of inventing a
    // numerical capacity. The remainder of this class still has modern layout.
    unsigned char loopPadding[3];
    bool playing=false;
    std::uint32_t loopStorageWord() const noexcept {
        std::uint32_t value;std::memcpy(&value,&loop,sizeof(value));return value;
    }
};
}
