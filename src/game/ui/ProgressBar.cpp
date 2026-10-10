#include "ProgressBar.h"
#include "ProgressSprite.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
namespace farmar::recovered {
namespace {
template<class T> void attribute(const XmlNode& node,const wchar_t* key,T& value) {
    const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value;}
}
}
void ProgressBar::loadXML(const XmlNode& node) {
    Picture::loadXML(node);
    attribute(node,L"ProgressMaxValue",maximum);attribute(node,L"ProgressCurrentValue",current);
    unsigned nextSide=0;attribute(node,L"Side",nextSide);side=nextSide;
    XmlNode* definition=nullptr;
    if(node.section(String("Lid"),definition)) {
        lid_=new Picture(this,resources_,registry_,controls_);lid_->loadXML(*definition);
    }
    updateLid();
}
void ProgressBar::updateTransform() {
    if(!dirty_) return;
    animation_.time=normalizeControlAnimationTime(animation_.time,animation_.length,animation_.loop);
    auto geometry=geometry_;geometry.alpha=*alpha_;geometry.rootAlpha=rootTransparency();
    updateProgressSprite(first_,geometry,frame(),maximum,current,side);dirty_=false;
}
void ProgressBar::updateLid() {
    dirty_=true;if(!lid_) return;
    updateTransform();if(!lid_) return;
    const auto& corners=first_.corners;Point2 position;
    switch(side) {
    case 0:position={corners[0].x,static_cast<float>(originalMultiply(originalAdd(corners[2].y,corners[0].y),.5))};break;
    case 1:position={corners[1].x,static_cast<float>(originalMultiply(originalAdd(corners[3].y,corners[1].y),.5))};break;
    case 2:position={static_cast<float>(originalMultiply(originalAdd(corners[1].x,corners[0].x),.5)),corners[0].y};break;
    case 3:position={static_cast<float>(originalMultiply(originalAdd(corners[3].x,corners[2].x),.5)),corners[2].y};break;
    default:return;
    }
    lid_->setPosition(position);
}
}
