#include "ProgressDiscrete.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <cmath>
#include <sstream>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* key,T& value) {
    const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value;}
}
void child(View& view,const XmlNode& node,const char* key) {XmlNode* part=nullptr;if(node.section(String(key),part)) view.loadXML(*part);}
}
ProgressDiscrete::ProgressDiscrete(View* parent,ResourceManager& resources,ViewRegistry& views,ControlRegistry& controls)
    :Control(parent,resources,views,controls),on_(std::make_unique<Picture>(nullptr,resources,views,controls)),off_(std::make_unique<Picture>(nullptr,resources,views,controls)) {}
void ProgressDiscrete::onShown() {Control::onShown();controls_.erase(this);}
void ProgressDiscrete::loadXML(const XmlNode& node) {
    Control::loadXML(node);unsigned count=0;attribute(node,L"ProgressMaxValue",count);attribute(node,L"ProgressCurrentValue",current);
    attribute(node,L"DeepStep",deepStep);attribute(node,L"MaxLength",maxLength);
    const auto text=node.attribute(WString(L"Step"));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>step[0];stream.ignore(1);stream>>step[1];}
    if(!count) throw "ProgressMaxValue == 0";
    child(*on_,node,"PictureOn");child(*off_,node,"PictureOff");XmlNode* section=nullptr;
    if(node.section(String("Lid"),section)) {lid_=new Picture(this,resources_,registry_,controls_);lid_->loadXML(*section);}
    setMaximum(count);
}
void ProgressDiscrete::setMaximum(unsigned count) {
    if(pictures_.size()==count) return;
    if(pictures_.size()<count) {pictures_.reserve(count);while(pictures_.size()<count) pictures_.push_back(new Picture(this,resources_,registry_,controls_));}
    else {for(unsigned i=count;i<pictures_.size();++i) delete pictures_[i];pictures_.resize(count);}
    refresh();
}
void ProgressDiscrete::refresh() {
    Point2 spacing=step,position{geometry_.x,geometry_.y};float depth=deep_;
    const auto squared=originalAdd(originalMultiply(step[1],step[1]),originalMultiply(step[0],step[0]));
    if(maxLength<originalMultiply(originalUnsigned(static_cast<unsigned>(pictures_.size())),originalSqrt(squared))) {
        const auto rounded=std::floor(originalDivide(maxLength,originalUnsigned(static_cast<unsigned>(pictures_.size()))));
        const float length=rounded>1 ? float(rounded):1.f;
        if(spacing[0]!=0 || spacing[1]!=0) {
            // 00421520 spills the shared normalization coefficient to float.
            const float scale=float(originalDivide(length,originalSqrt(originalAdd(originalMultiply(spacing[0],spacing[0]),originalMultiply(spacing[1],spacing[1])))));
            spacing={float(originalMultiply(scale,spacing[0])),float(originalMultiply(scale,spacing[1]))};
        }
    }
    unsigned index=1;for(auto* picture:pictures_) {
        picture->copyVisualProperties(index<=current ? *on_:*off_);picture->setPosition(position);picture->setDeep(depth);picture->show();
        position={float(originalAdd(spacing[0],position[0])),float(originalAdd(spacing[1],position[1]))};depth=float(originalAdd(depth,deepStep));++index;
    }
}
}
