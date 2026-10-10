#include "Control.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "Shader.h"
#include "SpriteBatches.h"
#include "XmlDocument.h"
#include "StringConversion.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
#include <vector>
namespace farmar::recovered {
namespace {
template<class Value> void attribute(const XmlNode& node,const wchar_t* key,Value& value) {
    const WString text=node.attribute(WString(key));
    if(!text.empty()) { std::wistringstream stream(text.c_str()); stream>>value; }
}
template<std::size_t N> void vectorAttribute(const XmlNode& node,const wchar_t* key,std::array<float,N>& value) {
    const WString text=node.attribute(WString(key));
    if(text.empty()) return;
    std::wistringstream stream(text.c_str());
    for(unsigned index=0;index<N;++index) { if(index) stream.ignore(1); stream>>value[index]; }
}
void colorAttribute(const XmlNode& node,const wchar_t* key,Color& value) {
    const WString text=node.attribute(WString(key)); if(text.empty()) return;
    unsigned packed=0; std::wistringstream stream(text.c_str()); stream>>std::hex>>packed; value=Color::fromARGB(packed);
}
float elapsed() {
    if(!uiElapsedTime()) throw std::logic_error("UI timer is not bound");
    return uiElapsedTime()();
}
}
ControlRegistry& globalControlRegistry() { static ControlRegistry controls; return controls; }
std::function<float()>& uiElapsedTime() { static std::function<float()> source; return source; }
bool ControlRegistry::Less::operator()(const Control* a,const Control* b) const noexcept {
    if(a->deep()==b->deep()) return reinterpret_cast<std::uintptr_t>(a)<reinterpret_cast<std::uintptr_t>(b);
    return a->deep()<b->deep();
}
void ControlRegistry::insert(Control* control) { visible_.insert(control); }
void ControlRegistry::erase(Control* control) { visible_.erase(control); }
bool ControlRegistry::contains(Control* control) const { return visible_.count(control)!=0; }
void ControlRegistry::forEachVisible(const std::function<void(Control&)>& callback) {
    std::vector<Control*> hidden;
    for(auto* control:visible_) {
        if(control->state().shown && control->state().parentShown) callback(*control);
        else hidden.push_back(control);
    }
    for(auto* control:hidden) erase(control);
}
Control::Control(View* parent,ResourceManager& resources,ViewRegistry& views,ControlRegistry& controls)
    : View(parent,views),resources_(resources),controls_(controls) {
    animation_.loop=true; state_.transparency=1;
    for(auto* sprite:{&first_,&second_}) {
        for(auto& corner:sprite->corners) corner.color=0xffffffffU;
        sprite->subdivisions=1; sprite->u1=sprite->v1=1;
    }
}
Control::Control(const Control& source):View(source),resources_(source.resources_),controls_(source.controls_),geometry_(source.geometry_),animation_(source.animation_),material_(source.material_),deep_(source.deep_),playing_(source.playing_),flags_(source.flags_),texture_(source.texture_) {
    userData=source.userData;
    if(texture_) texture_->addReference();
    for(auto* sprite:{&first_,&second_}) {
        for(auto& corner:sprite->corners) corner.color=0xffffffffU;
        sprite->subdivisions=1;sprite->u1=sprite->v1=1;
    }
}
Control::~Control() {
    controls_.erase(this); View::onHidden(); controls_.erase(this);
    callbacks_.clear();
    if(second_.shader) second_.shader->releaseReference();
    if(first_.shader) first_.shader->releaseReference();
    if(texture_) texture_->releaseReference();
}
void Control::onShown() { controls_.insert(this); View::onShown(); }
void Control::onHidden() { controls_.erase(this); View::onHidden(); }
void Control::process() { dirty_=true; View::process(); }
void Control::setPosition(const Point2& value) {
    const Point2 delta{static_cast<float>(double(value[0])-geometry_.x),static_cast<float>(double(value[1])-geometry_.y)};
    for(auto* child:children()) {
        auto* control=static_cast<Control*>(child);
        const Point2 moved{static_cast<float>(double(control->geometry_.x)+delta[0]),static_cast<float>(double(control->geometry_.y)+delta[1])};
        control->setPosition(moved); control->dirty_=true;
    }
    geometry_.x=value[0]; geometry_.y=value[1]; dirty_=true;
}
void Control::setDeep(float value) {
    const float delta=static_cast<float>(double(value)-deep_);
    for(auto* child:children()) {
        auto* control=static_cast<Control*>(child);
        control->setDeep(static_cast<float>(double(delta)+control->deep_)); control->dirty_=true;
    }
    if(state_.shown && state_.parentShown && controls_.contains(this)) {
        controls_.erase(this); deep_=value; controls_.insert(this);
    }
    deep_=value; dirty_=true;
}
bool Control::hitTest(const Point2& point) const noexcept {
    return double(geometry_.x)-double(geometry_.width)*0.5<=point[0]
        && point[0]<=double(geometry_.x)+double(geometry_.width)*0.5
        && double(geometry_.y)-double(geometry_.height)*0.5<=point[1]
        && point[1]<=double(geometry_.y)+double(geometry_.height)*0.5;
}
void Control::setFrame(unsigned frameValue) {
    if(frame()==frameValue) return;
    animation_.time=static_cast<float>(originalDivide(originalMultiply(originalUnsigned(frameValue),animation_.length),originalUnsigned(animation_.frames)));
    if(animation_.time<0) animation_.time=0;
    if(animation_.length<animation_.time) animation_.time=animation_.length;
    dirty_=true;
}
void Control::setAngle(float value) { if(geometry_.angle!=value) { geometry_.angle=value; dirty_=true; } }
void Control::setAlpha(float value) { if(*alpha_!=value) { *alpha_=value; dirty_=true; } }
void Control::setTextureName(const String& name) {
    if(texture_) texture_->releaseReference();
    texture_=name.empty() ? nullptr:resources_.getTexture(name);
}
void Control::setSize(const Point2& value) { if(geometry_.width!=value[0] || geometry_.height!=value[1]) { geometry_.width=value[0]; geometry_.height=value[1]; dirty_=true; } }
void Control::setTime(float value) { if(animation_.time!=value) { animation_.time=value; dirty_=true; } }
void Control::addCallback(Callback callback) { if(callback) callbacks_.push_back(callback); }
void Control::processCallbacks() {
    // Capture the successor before invoking the callback, as the original list
    // traversal does. A callback may append more callbacks without visiting a
    // new last item during the same pass.
    for(auto it=callbacks_.begin();it!=callbacks_.end();) {
        auto current=it++;
        if((*current)(this)) callbacks_.erase(current);
    }
}
void Control::updateTransform() {
    if(!dirty_) return;
    if(animation_.time>animation_.length && !animation_.loop) onAnimationEnded();
    // onAnimationEnded may change animation properties, so resolve afterward.
    const auto frameState=resolveControlAnimation(animation_);
    auto geometry=geometry_; geometry.alpha=*alpha_; geometry.rootAlpha=rootTransparency();
    updateAnimatedControlSprites(first_,second_,geometry,frameState); dirty_=false;
}
const SpriteDefinition& Control::sprite() { updateTransform(); return first_; }
void Control::submitSprites(SpriteBatches& batches,bool fullscreen,const DesktopImagePlacement& placement) {
    batches.submit(sprite(),texture_,flags_,fullscreen,placement);
    if(animation_.doubleRender) batches.submit(second_,texture_,flags_,fullscreen,placement);
}
void Control::loadXML(const XmlNode& node) {
    View::loadXML(node);
    float depth=0; attribute(node,L"deep",depth); setDeep(depth);
    Point2 position{geometry_.x,geometry_.y},size{geometry_.width,geometry_.height};
    vectorAttribute(node,L"position",position); geometry_.x=position[0]; geometry_.y=position[1];
    vectorAttribute(node,L"size",size); geometry_.width=size[0]; geometry_.height=size[1];
    attribute(node,L"angle",geometry_.angle); colorAttribute(node,L"color",geometry_.color);
    attribute(node,L"alpha",*alpha_); attribute(node,L"RenderType",flags_);
    attribute(node,L"InverceX",geometry_.invertX); attribute(node,L"InverceY",geometry_.invertY);
    attribute(node,L"DoubleAnimRender",animation_.doubleRender);
    attribute(node,L"pitch",material_.pitch); material_.pitch=static_cast<float>(double(material_.pitch)*0.01745329238474369F);
    attribute(node,L"yaw",material_.yaw); material_.yaw=static_cast<float>(double(material_.yaw)*0.01745329238474369F);
    vectorAttribute(node,L"scale",material_.scale); attribute(node,L"LightingEnable",material_.lighting);
    attribute(node,L"LightTable",material_.lightTable); colorAttribute(node,L"AmbientColor",material_.ambient);
    colorAttribute(node,L"SpecularColor",material_.specular); colorAttribute(node,L"EmissiveColor",material_.emissive);
    attribute(node,L"Power",material_.power);
    const String picture=toNarrow(node.attribute(WString(L"Picture")));
    const String mesh=toNarrow(node.attribute(WString(L"Mesh")));
    const String animation=toNarrow(node.attribute(WString(L"Animation")));
    if(!mesh.empty() || !animation.empty()) throw std::runtime_error("Control Mesh/Animation resource factories are not reconstructed yet");
    setTextureName(picture);
    attribute(node,L"anim_length",animation_.length); attribute(node,L"PlayAnimation",playing_);
    attribute(node,L"frames_count",animation_.frames); attribute(node,L"anim_loop",animation_.loop);
    attribute(node,L"anim_columns",geometry_.columns); attribute(node,L"anim_rows",geometry_.rows);
    const unsigned capacity=geometry_.columns*geometry_.rows;
    if(capacity<animation_.frames) animation_.frames=capacity;
    dirty_=true;
}
void Picture::enabledStep() {
    if(updateCallback) updateCallback(this);
    if(playing_) { animation_.time=static_cast<float>(double(elapsed())+animation_.time); dirty_=true; }
    processCallbacks(); View::enabledStep();
}
void Picture::onShown() { Control::onShown(); addCallback(shownCallback); }
void Picture::onHidden() { Control::onHidden(); addCallback(hiddenCallback); }
}
