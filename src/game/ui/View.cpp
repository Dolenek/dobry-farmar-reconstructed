#include "View.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <vector>
namespace farmar::recovered {
ViewRegistry& globalViewRegistry() { static ViewRegistry registry; return registry; }
bool ViewRegistry::contains(const View* view) const { return all_.count(const_cast<View*>(view))!=0; }
void ViewRegistry::registerEnabled(View* view) {
    if(dispatching_) { enabledRemoves_.erase(view); enabledAdds_.insert(view); }
    else enabled_.insert(view);
}
void ViewRegistry::unregisterEnabled(View* view) {
    if(dispatching_) { enabledAdds_.erase(view); enabledRemoves_.insert(view); }
    else enabled_.erase(view);
}
void ViewRegistry::registerAbsolute(View* view) {
    if(dispatching_) { absoluteRemoves_.erase(view); absoluteAdds_.insert(view); }
    else absolute_.insert(view);
}
void ViewRegistry::unregisterAbsolute(View* view) {
    if(dispatching_) { absoluteAdds_.erase(view); absoluteRemoves_.insert(view); }
    else absolute_.erase(view);
}
void ViewRegistry::finishEnabledDispatch() {
    enabled_.insert(enabledAdds_.begin(),enabledAdds_.end());
    for(auto* view:enabledRemoves_) enabled_.erase(view);
    enabledAdds_.clear(); enabledRemoves_.clear(); dispatching_=false;
}
void ViewRegistry::finishAbsoluteDispatch() {
    absolute_.insert(absoluteAdds_.begin(),absoluteAdds_.end());
    for(auto* view:absoluteRemoves_) absolute_.erase(view);
    absoluteAdds_.clear(); absoluteRemoves_.clear(); dispatching_=false;
}
void ViewRegistry::dispatchSteps() {
    // 00660528 is the size word of the transition set at 00660520, not
    // a separate suspend flag. Native 00531b20 skips BOTH step passes while
    // any transition is registered; transition processing still follows.
    if(!transitions_.empty()) return;
    beginDispatch();
    // Both native traversals test NORMAL pending removals (00660538), including
    // the absolute pass. This preserves cross-pass disabling by callbacks.
    for(auto* view:absolute_) if(!enabledRemoves_.count(view)) view->absoluteStep();
    finishAbsoluteDispatch();
    beginDispatch();
    for(auto* view:enabled_) if(!enabledRemoves_.count(view)) view->enabledStep();
    finishEnabledDispatch();
}
void ViewRegistry::updateTransitions(const std::function<float()>& elapsed) {
    std::vector<View*> finished;
    for(auto* view:transitions_) {
        const auto mode=view->state().transition;
        view->changeTransparency(!view->parent() && mode>=1 && mode<=3 ? elapsed():0);
        if(view->state().transition==0) finished.push_back(view);
    }
    // 00531b20 collects completed transitions, then erases them after traversal.
    for(auto* view:finished) transitions_.erase(view);
}
void ViewRegistry::requestDiscard(View* view,const String& origin) {
    if(!view) throw "discard";
    discarded_[view]=origin;
}
void ViewRegistry::deleteDiscarded() {
    // 004fb180 detaches every queued view before deleting the snapshot. This
    // prevents a queued ancestor from also deleting a separately queued child.
    std::vector<View*> queued;
    for(const auto& entry:discarded_) { entry.first->setParent(nullptr); queued.push_back(entry.first); }
    for(auto* view:queued) { discarded_[view]=String(); delete view; }
}
View::View(View* parent,ViewRegistry& registry) : registry_(registry),parent_(parent) {
    registry_.all_.insert(this);
    if(parent_) {
        parent_->addChild(this);
        const auto& p=parent_->state_;
        state_.parentEnabled=p.enabled && p.parentEnabled;
        state_.parentShown=p.shown && p.parentShown;
        state_.parentAbsoluteEnabled=p.absoluteEnabled && p.parentAbsoluteEnabled;
    }
}
View::View(const View& source):registry_(source.registry_),parent_(source.parent_) {
    state_.transparency=0;state_.transparencySpeed=0;state_.transition=0;
    state_.shown=source.state_.shown;state_.enabled=source.state_.enabled;state_.absoluteEnabled=source.state_.absoluteEnabled;
    registry_.all_.insert(this);
    if(parent_) {
        parent_->addChild(this);const auto& p=parent_->state_;
        state_.parentShown=p.shown && p.parentShown;state_.parentEnabled=p.enabled && p.parentEnabled;
        state_.parentAbsoluteEnabled=p.absoluteEnabled && p.parentAbsoluteEnabled;
    }
    if(state_.enabled && state_.parentEnabled) registry_.registerEnabled(this);
    if(state_.absoluteEnabled && state_.parentAbsoluteEnabled) registry_.registerAbsolute(this);
}
View::~View() {
    registry_.transitions_.erase(this);
    while(!children_.empty()) delete *children_.begin();
    if(parent_) parent_->removeChild(this);
    registry_.unregisterEnabled(this); registry_.unregisterAbsolute(this);
    registry_.all_.erase(this); registry_.discarded_.erase(this);
}
void View::addChild(View* child) { children_.insert(child); }
void View::removeChild(View* child) { children_.erase(child); }
void View::setParent(View* parent) {
    if(parent_==parent) return;
    if(parent_) parent_->removeChild(this);
    parent_=parent;
    if(parent_) {
        parent_->addChild(this);
        const bool inherited=parent_->state_.shown && parent_->state_.parentShown;
        if(state_.parentShown==inherited) return;
        if(state_.parentShown) { parentHide(); return; }
    }
    parentShow();
}
void View::applyChildren(Propagate callback) { for(auto* child:children_) (child->*callback)(); }
void View::parentHide() {
    state_.parentShown=false; applyChildren(&View::parentHide); onHidden();
}
void View::parentDisable() {
    state_.parentEnabled=false; applyChildren(&View::parentDisable); onDisabled();
}
void View::parentAbsoluteDisable() {
    state_.parentAbsoluteEnabled=false; applyChildren(&View::parentAbsoluteDisable); onAbsoluteDisabled();
}
void View::parentShow() {
    state_.parentShown=true;
    if(state_.shown) {
        if(state_.enabled && state_.parentEnabled) onEnabled();
        applyChildren(&View::parentShow); onShown();
    }
}
void View::parentEnable() {
    state_.parentEnabled=true;
    if(state_.enabled && state_.shown && state_.parentShown) {
        applyChildren(&View::parentEnable); onEnabled();
    }
}
void View::parentAbsoluteEnable() {
    state_.parentAbsoluteEnabled=true;
    if(state_.absoluteEnabled) { applyChildren(&View::parentAbsoluteEnable); onAbsoluteEnabled(); }
}
void View::onShown() {
    if(state_.enabled && state_.parentEnabled) { onEnabled(); applyChildren(&View::parentEnable); }
    for(auto* child:children_) {
        child->state_.parentShown=true;
        if(child->state_.shown) {
            if(child->state_.enabled && child->state_.parentEnabled) child->onEnabled();
            child->applyChildren(&View::parentShow); child->onShown();
        }
    }
    state_.transition=0;
}
void View::onHidden() {
    if(state_.enabled && state_.parentEnabled) { onDisabled(); applyChildren(&View::parentDisable); }
    for(auto* child:children_) {
        child->state_.parentShown=false; child->applyChildren(&View::parentHide); child->onHidden();
    }
    state_.transition=0;
}
void View::process() { for(auto* child:children_) child->process(); }
void View::onEnabled() { registry_.registerEnabled(this); }
void View::onDisabled() { registry_.unregisterEnabled(this); }
void View::onAbsoluteEnabled() { registry_.registerAbsolute(this); }
void View::onAbsoluteDisabled() { registry_.unregisterAbsolute(this); }
void View::show() {
    if(registry_.contains(this)) { state_.shown=true; state_.transparency=1; if(state_.parentShown) onShown(); }
}
void View::hide() {
    if(registry_.contains(this)) {
        state_.shown=false; state_.transparency=0; onHidden(); applyChildren(&View::parentHide);
    }
}
void View::enable() {
    if(registry_.contains(this) && !state_.enabled) {
        state_.enabled=true;
        if(state_.parentEnabled && state_.shown && state_.parentShown) { applyChildren(&View::parentEnable); onEnabled(); }
    }
}
void View::disable() {
    if(registry_.contains(this) && state_.enabled) {
        state_.enabled=false; applyChildren(&View::parentDisable); onDisabled();
    }
}
void View::absoluteEnable() {
    if(registry_.contains(this) && !state_.absoluteEnabled) {
        state_.absoluteEnabled=true;
        if(state_.parentAbsoluteEnabled) { applyChildren(&View::parentAbsoluteEnable); onAbsoluteEnabled(); }
    }
}
void View::absoluteDisable() {
    if(registry_.contains(this) && state_.absoluteEnabled) {
        state_.absoluteEnabled=false; applyChildren(&View::parentAbsoluteDisable); onAbsoluteDisabled();
    }
}
void View::showTransparently() {
    state_.shown=true; if(state_.parentShown) onShown();
    registry_.transitions_.insert(this); state_.transition=1;
}
void View::hideTransparently() {
    registry_.transitions_.insert(this); state_.transition=2;
    if(state_.enabled) { onDisabled(); applyChildren(&View::parentDisable); }
}
void View::discardTransparently() {
    registry_.transitions_.insert(this); state_.transition=3; disable(); absoluteDisable();
}
void View::changeTransparency(float elapsed) {
    if(parent_) return;
    if(state_.transition==1) {
        const double value=originalAdd(originalMultiply(elapsed,state_.transparencySpeed),state_.transparency);
        state_.transparency=static_cast<float>(value);
        if(!(value<1.0)) state_.transition=0; // original retains an overshoot
    } else if(state_.transition==2 || state_.transition==3) {
        const double value=originalSubtract(state_.transparency,originalMultiply(elapsed,state_.transparencySpeed));
        state_.transparency=static_cast<float>(value);
        if(value<=0) {
            const bool discard=state_.transition==3;
            if(discard) state_.transition=0;
            hide(); state_.transition=0;
            if(discard) registry_.requestDiscard(this,String(".\\Ui\\View.cpp432"));
        }
    }
    process();
}
float View::rootTransparency() const noexcept {
    const View* root=this; while(root->parent_) root=root->parent_; return root->state_.transparency;
}
void View::loadXML(const XmlNode& node) {
    bool visible=false,enableValue=false,absolute=false;
    auto extract=[&](const wchar_t* key,auto& value) {
        const WString text=node.attribute(WString(key));
        if(!text.empty()) { std::wistringstream stream(text.c_str()); stream>>value; }
    };
    extract(L"Visible",visible); extract(L"Enable",enableValue);
    extract(L"TranparencySpeed",state_.transparencySpeed); extract(L"AbsoluteEnable",absolute);
    if(visible) show(); else hide();
    if(enableValue) enable(); else disable();
    if(absolute) absoluteEnable(); // false does not disable a previously enabled view
}
}
