#pragma once
#include "Control.h"
#include <memory>
#include <vector>
namespace farmar::recovered {
// cProgressDiscrete, native604 bytes. SeedPackage uses its actual four
// on/off pictures to display remaining uses; it is not a text counter.
class ProgressDiscrete final:public Control {
public:
    ProgressDiscrete(View*,ResourceManager&,ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 0048d3d0
    ~ProgressDiscrete() override=default; // 0048cbc0
    void loadXML(const XmlNode&) override; // 0048d640
    void onShown() override; // 0048dda0
    void setMaximum(unsigned); // 0048d4e0; same count does not refresh
    void refresh(); // 0048c9e0
    Picture& pictureOn() noexcept {return *on_;}Picture& pictureOff() noexcept {return *off_;}
    Picture* lid() noexcept {return lid_;}
    const std::vector<Picture*>& pictures() const noexcept {return pictures_;}
    unsigned current=0;float deepStep=0;Point2 step{};float maxLength=0;
private:
    // The two prototypes have no View parent. Display pictures and optional
    // lid are owned by the ordinary View tree; resizing deletes only its tail.
    std::vector<Picture*> pictures_;Picture* lid_=nullptr;
    std::unique_ptr<Picture> on_,off_;
};
}
