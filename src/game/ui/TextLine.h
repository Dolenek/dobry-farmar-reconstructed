#pragma once
#include "Control.h"
#include "TextLineLayout.h"
#include "GameWString.h"
namespace farmar::recovered {
class Settings;
class StringManager;
// 004dc1c0: reverse UTF-16 code units, retaining ASCII alphanumeric runs.
WString reverseTextDirection(const WString& text);
class Letter final : public Picture {
public:
    using Picture::Picture; // 0050c490, UV 0,0,1,1 and packed white
    void setUV(const std::array<float,4>& value); // 0050c400
    void setPackedColor(unsigned value); // 0050c3d0
    void setRenderFlags(unsigned value) noexcept { flags_=value; }
    unsigned packedColor() const noexcept { return packedColor_; }
    const std::array<float,4>& uv() const noexcept { return uv_; }
    void updateTransform() override; // 0050c500
private:
    std::array<float,4> uv_{0,0,1,1};
    unsigned packedColor_=0xffffffffU;
};
// Recovered lifecycle/resources of cTextLine. Modern layout, not original 696b.
class TextLine : public Control {
public:
    TextLine(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 004daca0
    ~TextLine() override; // 004da840
    void loadXML(const XmlNode& node) override; // 004dd980
    void onShown() override; // 004da2f0, the line itself is not drawable
    void onDisabled() override; // 004da2d0
    void enabledStep() override; // 004dae40
    void setDeep(float value) override; // 004dd350
    void setAlpha(float value) override; // 004dbdc0
    void setTexture(const String& name); // 004dab90
    void setString(const WString& text); // 004dd030, owns a reusable Letter pool
    const WString& string() const noexcept { return text_; }
    void setPackedColor(unsigned value); // 00415ca0
    void setMultiplier(float value); // 00415cc0
    void setAlignment(unsigned value); // 00415cf0
    void setLocalized(bool value) noexcept { localized_=value; } // 00415c70, no dirty flag
    virtual void makeLetters(); // 004dc800
    float textWidth(); // 004db210
    float textHeight(); // 004db5e0, deliberately retains previous max height
    const std::vector<Letter*>& letters() const noexcept { return letters_; }
    bool textDirty() const noexcept { return textDirty_; }
protected:
    StringManager& strings_;
    WString text_;
    std::vector<Letter*> letters_; // View owns these children
    String fontName_,textureName_;
    bool textDirty_=true,localized_=true,rightToLeft_=false;
    float spaceWidth_=10,multiplier_=1,width_=0,height_=0;
    unsigned packedColor_=0xffffffffU,alignment_=0;
    cFont* font_=nullptr;
    cTexture* textTexture_=nullptr;
};
}
