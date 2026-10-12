#pragma once
#include "Button.h"
namespace farmar::recovered {
// Native cPictureButton, 640 bytes: actual Button with a solid-color hover
// overlay. BuyBuilding adds its four-byte building id to this common base.
class PictureButton:public Button {
public:
    PictureButton(View*,ResourceManager&,InputBindings&,ButtonInteraction&,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 005070d0
    ~PictureButton() override=default; // 00507160 -> Button/View ownership
    void loadXML(const XmlNode&) override; // 005071b0
    void onStateChanged(ButtonState) override; // 00507660
    void updateTransform() override; // 00507af0
    Picture& overlay() noexcept {return *overlay_;}
private:Picture* overlay_;
};
}
