#pragma once
#include "MultilineText.h"
#include <functional>
namespace farmar::recovered {
// cAwardTip, native 744 bytes. Original cAnimable corner definitions are
// independent resource owners, not extra Views or rendered corner sprites.
class TooltipFrame final : public Picture {
public:
    using ScreenSize=std::function<std::array<unsigned,2>()>;
    TooltipFrame(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,
        ScreenSize screenSize,ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 004c8d10
    ~TooltipFrame() override; // 004c8e00
    void loadXML(const XmlNode& node) override; // 004c8e80
    void enabledStep() override; // 004c9a80; timer virtual +c, no Picture tick
    void placeBeside(const Point2& point); // 004c95c0, selects and assigns cAnimable
    MultilineText& text() noexcept { return *text_; }
    const Point2& offset() const noexcept { return offset_; }
    float fadeDirection=0; // native +244, assigned by AwardButton hover callbacks
private:
    struct Appearance {
        cTexture* texture=nullptr;
        unsigned frames=1,columns=1,rows=1;
        float length=1;
        bool loop=true,playing=false;
        ~Appearance(); // 004d2130, selected texture also independently retained by Control
        void loadXML(const XmlNode& node,ResourceManager& resources); // 004d22a0, shipped 2D path
    };
    MultilineText* text_;
    Point2 offset_{};
    std::array<Appearance,4> appearances_;
    ScreenSize screenSize_;
    void assignAppearance(const Appearance& appearance); // 004d2160: preserves time/playing
};
}
