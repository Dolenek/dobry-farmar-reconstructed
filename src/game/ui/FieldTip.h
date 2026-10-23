#pragma once
#include "MovableUnion.h"
#include "TextLine.h"
#include "ProgressBar.h"
#include "AnimableAppearance.h"
#include <array>
namespace farmar::recovered {
class FarmField;class GameConfiguration;class FieldTip;
class FieldTipLifetime {protected:explicit FieldTipLifetime(FieldTip*);~FieldTipLifetime();};
struct FieldTipContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;GameConfiguration& configuration;
    ViewRegistry& views;ControlRegistry& controls;
};
// cFarmFieldTip, native1012 bytes, primary65 slots; singleton0065f774.
class FieldTip final:public MovableUnion,private FieldTipLifetime {
public:
    FieldTip(View*,FieldTipContext&); // 00441360
    ~FieldTip() override=default; // 004415e0
    void loadXML(const XmlNode&) override; // 004408d0
    void absoluteStep() override; // 004405e0
    void select(FarmField*); // 004402a0
    void hideFor(FarmField*); // 004402d0, only matching owner
    void refresh(); // 00440380
    static FieldTip* instance() noexcept;
    ProgressBar& waterBar() noexcept {return *water_;}ProgressBar& fertilBar() noexcept {return *fertil_;}
    ProgressBar& readyBar() noexcept {return *ready_;}TextLine& cultureText() noexcept {return *cultureText_;}
    Picture& cultureIcon() noexcept {return *cultureIcon_;}
    const std::vector<TextLine*>& labels() const noexcept {return labels_;}
    FarmField* selected=nullptr;float hideRemaining=0;bool showPending=false;float showRemaining;
    std::array<AnimableAppearance,8> icons;
private:
    FieldTipContext& tipContext_;ProgressBar *water_,*fertil_,*ready_;TextLine* cultureText_;Picture* cultureIcon_;
    std::vector<TextLine*> labels_;
};
}
