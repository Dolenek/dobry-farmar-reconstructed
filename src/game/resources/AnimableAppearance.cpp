#include "AnimableAppearance.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* name,T& output) {
    const auto text=node.attribute(WString(name));
    if(!text.empty()) { std::wistringstream stream(text.c_str());stream>>output; }
}
}
AnimableAppearance::~AnimableAppearance() { if(texture) texture->releaseReference(); }
void AnimableAppearance::loadXML(const XmlNode& node,ResourceManager& resources) {
    const auto picture=toNarrow(node.attribute(WString(L"Picture")));
    if(!node.attribute(WString(L"Mesh")).empty() || !node.attribute(WString(L"Animation")).empty())
        throw std::runtime_error("cAnimable Mesh/Animation resource factories are not reconstructed yet");
    if(texture) texture->releaseReference();texture=picture.empty() ? nullptr:resources.getTexture(picture);
    attribute(node,L"anim_length",length);attribute(node,L"PlayAnimation",playing);
    attribute(node,L"frames_count",frames);attribute(node,L"anim_loop",loop);
    attribute(node,L"anim_columns",columns);attribute(node,L"anim_rows",rows);
    const std::uint32_t capacity=columns*rows;if(capacity<frames) frames=capacity;
}
}
