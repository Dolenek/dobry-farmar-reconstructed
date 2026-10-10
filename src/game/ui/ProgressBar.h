#pragma once
#include "Control.h"
namespace farmar::recovered {
class ProgressBar final : public Picture {
public:
    using Picture::Picture; // 004f91e0: maximum=100, current=0, side=2, lid=null
    void loadXML(const XmlNode&) override; // 004f9f80
    void updateTransform() override; // 004f9290
    void updateLid(); // 004f9b20, always marks dirty
    float maximum=100,current=0;
    unsigned side=2; // native mutable accessor 004f9260; setter copies a word
    Picture* lid() const noexcept {return lid_;}
private:
    Picture* lid_=nullptr; // owned by View children; reload retains old child
};
}
