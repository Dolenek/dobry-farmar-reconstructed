#include "PictureButton.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
namespace farmar::recovered {
PictureButton::PictureButton(View* parent,ResourceManager& resources,InputBindings& input,ButtonInteraction& interaction,ViewRegistry& views,ControlRegistry& controls)
    :Button(parent,resources,input,interaction,views,controls),overlay_(new Picture(this,resources,views,controls)) {}
void PictureButton::loadXML(const XmlNode& node) {
    Button::loadXML(node);overlay_->setSize({geometry().width,geometry().height});overlay_->setPosition({geometry().x,geometry().y});
    overlay_->setDeep(float(originalAdd(deep(),0.001f)));unsigned packed=0xbdbdbdff;
    const auto value=node.attribute(WString(L"OverlayColor"));if(!value.empty()) {std::wistringstream in(value.c_str());in>>std::hex>>packed;}
    overlay_->setColor(Color::fromARGB(packed));overlay_->setRenderFlags(9);overlay_->hide();
}
void PictureButton::onStateChanged(ButtonState state) {
    Button::onStateChanged(state);
    switch(state) {
    case ButtonState::Disabled:case ButtonState::Normal:overlay_->hide();setRenderFlags(renderFlags()&~4U);break;
    case ButtonState::Hovered:case ButtonState::Pressed:overlay_->show();setRenderFlags(renderFlags()|4U);break;
    default:break;
    }
}
void PictureButton::updateTransform() {overlay_->setPosition({geometry().x,geometry().y});Control::updateTransform();}
}
