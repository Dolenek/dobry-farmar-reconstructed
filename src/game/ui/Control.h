#pragma once
#include "View.h"
#include "ControlAnimation.h"
#include <array>
#include <functional>
#include <list>
#include <set>
namespace farmar::recovered {
class ResourceManager;
class cTexture;
class SpriteBatches;
class Control;
using Point2=std::array<float,2>;
struct ControlMaterial {
    std::array<float,3> scale{1,1,1}; float yaw=0,pitch=0;
    bool lighting=false; unsigned lightTable=0;
    Color ambient{1,1,1,1},specular{1,1,1,1},emissive{1,1,1,1};
    float power=0;
};
class ControlRegistry {
public:
    struct Less { bool operator()(const Control* a,const Control* b) const noexcept; };
    using Controls=std::set<Control*,Less>;
    void insert(Control* control);
    void erase(Control* control);
    bool contains(Control* control) const;
    const Controls& visible() const noexcept { return visible_; }
    void forEachVisible(const std::function<void(Control&)>& callback); // 00506a10
private:
    Controls visible_;
};
ControlRegistry& globalControlRegistry();
std::function<float()>& uiElapsedTime(); // original timer singleton virtual +8
// Recovered cControl data, hierarchy and 2D resource path. Modern object/virtual
// layout differs from the original 556-byte class. Mesh/Animation resources and
// their 3D render branch are still explicit unsupported paths, never substitutes.
class Control : public View {
public:
    using Callback=bool(*)(Control*);
    Control(View* parent,ResourceManager& resources,ViewRegistry& views=globalViewRegistry(),
            ControlRegistry& controls=globalControlRegistry()); // 004edc80
    Control(const Control& source); // 004ed720, fresh sprites/callbacks, copied resources and properties
    ~Control() override; // 004f1560
    void loadXML(const XmlNode& node) override; // 004eded0 + 004d22a0
    void onShown() override; // 004f14e0
    void onHidden() override; // 004f1510
    void process() override; // 004ed590
    virtual void setPosition(const Point2& value); // 004f0110
    virtual void setDeep(float value); // 004f1080
    bool hitTest(const Point2& point) const noexcept; // 00413a50, axis-aligned inclusive bounds
    void setFrame(unsigned frame); // 004eebb0
    unsigned frame() const noexcept { return controlAnimationFrame(animation_.time,animation_.length,animation_.frames); }
    void setAngle(float value); // 00413b20
    virtual void setAlpha(float value); // 00413b50
    void setSize(const Point2& value); // 00413b80
    void setTime(float value); // 00413bc0
    void setAlphaSource(float* value) noexcept { alpha_=value; dirty_=true; }
    float alpha() const noexcept { return *alpha_; }
    void setTextureName(const String& name); // cAnimable::setPicture, 004d2260
    float deep() const noexcept { return deep_; }
    const ControlSpriteProperties& geometry() const noexcept { return geometry_; }
    const ControlAnimationState& animation() const noexcept { return animation_; }
    const ControlMaterial& material() const noexcept { return material_; }
    bool playing() const noexcept { return playing_; }
    unsigned userData=0; // original +1b4, copied by 004ed720; ListBox row index
    bool dirty() const noexcept { return dirty_; }
    void markTransformDirty() noexcept { dirty_=true; }
    cTexture* texture() const noexcept { return texture_; }
    unsigned renderFlags() const noexcept { return flags_; }
    void addCallback(Callback callback); // 004f1680, queued until enabledStep processing
    std::size_t callbackCount() const noexcept { return callbacks_.size(); }
    virtual void onAnimationEnded() {} // original cControl slot +dc, 004bd820
    virtual void updateTransform(); // 004f0b70, 2D branch
    const SpriteDefinition& sprite(); // 004ee7e0
    const SpriteDefinition& secondSprite() const noexcept { return second_; }
    void submitSprites(SpriteBatches& batches,bool fullscreen,const DesktopImagePlacement& placement); // 0052fe50, 2D branch
protected:
    ResourceManager& resources_;
    ControlRegistry& controls_;
    ControlSpriteProperties geometry_;
    ControlAnimationState animation_;
    ControlMaterial material_;
    float* alpha_=&geometry_.color.a;
    float deep_=0;
    bool dirty_=true,playing_=false;
    unsigned flags_=0;
    cTexture* texture_=nullptr;
    SpriteDefinition first_{},second_{};
    std::list<Callback> callbacks_;
    void processCallbacks(); // 004ed670
};
class Picture : public Control {
public:
    using Control::Control; // original 004f16f0 adds three null callbacks
    Picture(const Picture& source):Control(source),hiddenCallback(source.hiddenCallback),shownCallback(source.shownCallback),updateCallback(source.updateCallback) {} // 004f1730
    unsigned type() const noexcept override { return 2; } // 0050bd60
    void enabledStep() override; // 004f1810
    void onShown() override; // 004f17a0
    void onHidden() override; // 004f17c0
    Callback hiddenCallback=nullptr,shownCallback=nullptr,updateCallback=nullptr;
};
}
