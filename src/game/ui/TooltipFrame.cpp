#include "TooltipFrame.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include "GameTimer.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
#include <utility>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* key,T& value) {
    const auto text=node.attribute(WString(key));if(!text.empty()) { std::wistringstream stream(text.c_str());stream>>value; }
}
}
TooltipFrame::Appearance::~Appearance() { if(texture) texture->releaseReference(); }
void TooltipFrame::Appearance::loadXML(const XmlNode& node,ResourceManager& resources) {
    const auto picture=toNarrow(node.attribute(WString(L"Picture")));
    if(!node.attribute(WString(L"Mesh")).empty() || !node.attribute(WString(L"Animation")).empty())
        throw std::runtime_error("Tooltip cAnimable Mesh/Animation factories are not reconstructed yet");
    if(texture) texture->releaseReference();texture=picture.empty() ? nullptr:resources.getTexture(picture);
    attribute(node,L"anim_length",length);attribute(node,L"PlayAnimation",playing);
    attribute(node,L"frames_count",frames);attribute(node,L"anim_loop",loop);
    attribute(node,L"anim_columns",columns);attribute(node,L"anim_rows",rows);
    const unsigned capacity=columns*rows;if(capacity<frames) frames=capacity;
}
TooltipFrame::TooltipFrame(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,
    ScreenSize screenSize,ViewRegistry& views,ControlRegistry& controls)
    :Picture(parent,resources,views,controls),text_(new MultilineText(this,resources,settings,strings,views,controls)),screenSize_(std::move(screenSize)) {}
TooltipFrame::~TooltipFrame()=default; // corners released before Picture/View deletes owned text
void TooltipFrame::loadXML(const XmlNode& node) {
    XmlNode* child=nullptr;if(node.section(String("Text"),child)) text_->loadXML(*child);
    if(node.section(String("Podlogka"),child)) {
        Picture::loadXML(*child);
        const auto text=child->attribute(WString(L"Offset"));
        if(!text.empty()) { std::wistringstream stream(text.c_str());stream>>offset_[0];stream.ignore(1);stream>>offset_[1]; }
        constexpr const char* names[]={"UpLeftImage","UpRightImage","DownLeftImage","DownRightImage"};
        for(unsigned i=0;i<4;++i) { XmlNode* image=nullptr;if(child->section(String(names[i]),image)) appearances_[i].loadXML(*image,resources_); }
    }
    setAlpha(0); // does not reset text alpha, fadeDirection, or animation time
}
void TooltipFrame::assignAppearance(const Appearance& appearance) {
    if(texture_!=appearance.texture) {
        if(texture_) texture_->releaseReference();texture_=appearance.texture;if(texture_) texture_->addReference();
    }
    geometry_.columns=appearance.columns;geometry_.rows=appearance.rows;
    animation_.frames=appearance.frames;animation_.length=appearance.length;animation_.loop=appearance.loop;
}
void TooltipFrame::placeBeside(const Point2& point) {
    if(!screenSize_) throw std::logic_error("Tooltip graphics dimensions are not bound");
    const auto dimensions=screenSize_();const bool left=point[0]<=originalMultiply(originalUnsigned(dimensions[0]),0.5f);
    const bool top=point[1]<=originalMultiply(originalUnsigned(dimensions[1]),0.5f);
    Point2 position{};
    for(unsigned i=0;i<2;++i) {
        const auto half=originalMultiply(i ? geometry_.height:geometry_.width,0.5f);
        position[i]=static_cast<float>((i ? top:left) ? originalAdd(originalAdd(half,offset_[i]),point[i]):originalSubtract(originalSubtract(point[i],half),offset_[i]));
    }
    assignAppearance(appearances_[unsigned(left)+2*unsigned(top)]);setPosition(position);
}
void TooltipFrame::enabledStep() {
    if(!screenSize_) throw std::logic_error("Tooltip graphics dimensions are not bound");
    const auto dimensions=screenSize_();
    const auto halfWidth=originalMultiply(geometry_.width,0.5f),halfHeight=originalMultiply(geometry_.height,0.5f);
    if(!(originalSubtract(geometry_.x,halfWidth)>=0 && originalAdd(halfWidth,geometry_.x)<=originalUnsigned(dimensions[0])
        && originalSubtract(geometry_.y,halfHeight)>=0 && originalAdd(halfHeight,geometry_.y)<=originalUnsigned(dimensions[1])))
        placeBeside({static_cast<float>(originalAdd(offset_[0],geometry_.x)),static_cast<float>(originalAdd(offset_[1],geometry_.y))});
    if(!GameTimer::instance()) throw std::logic_error("Tooltip timer is not bound");
    const float elapsed=GameTimer::instance()->unscaledElapsed();
    const auto extended=originalAdd(originalMultiply(originalMultiply(elapsed,fadeDirection),3.3f),alpha());
    float value=static_cast<float>(extended);
    // Native first comparison uses the unspilled accumulator, second its float.
    if(extended<0) { value=0;hide(); }else if(value>=1) value=1;
    setAlpha(value);text_->setAlpha(value);
}
}
