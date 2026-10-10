#pragma once
#include "GameString.h"
#include <map>
#include <set>
#include <functional>

namespace farmar::recovered {
class XmlNode;
class View;
// The recovered data and callbacks use modern containers; this is not the
// original 44-byte cView object ABI.
struct ViewState {
    float transparency=0, transparencySpeed=3.3f;
    unsigned transition=0;
    bool shown=false,parentShown=true,enabled=false,parentEnabled=true;
    bool absoluteEnabled=false,parentAbsoluteEnabled=true;
};
class ViewRegistry {
public:
    using Views=std::set<View*>;
    bool contains(const View* view) const;
    void registerEnabled(View* view); // 00506f90
    void unregisterEnabled(View* view); // 00507030
    void registerAbsolute(View* view); // 00506fe0
    void unregisterAbsolute(View* view); // 00507080
    void beginDispatch() noexcept { dispatching_=true; } // 005069d0
    void finishEnabledDispatch(); // 00506b90
    void finishAbsoluteDispatch(); // 00506d90
    void dispatchSteps(); // absolute then normal portion of 00531b20
    void updateTransitions(const std::function<float()>& elapsed); // transition portion of 00531b20
    const Views& enabled() const noexcept { return enabled_; }
    const Views& absoluteEnabled() const noexcept { return absolute_; }
    const Views& pendingEnabledAdds() const noexcept { return enabledAdds_; }
    const Views& pendingEnabledRemoves() const noexcept { return enabledRemoves_; }
    const Views& pendingAbsoluteAdds() const noexcept { return absoluteAdds_; }
    const Views& pendingAbsoluteRemoves() const noexcept { return absoluteRemoves_; }
    const Views& transitions() const noexcept { return transitions_; }
    const std::map<View*,String>& discarded() const noexcept { return discarded_; }
    void requestDiscard(View* view,const String& origin); // 0047d0b0
    void deleteDiscarded(); // 004fb180, detach all queued views then owned deletion
private:
    friend class View;
    Views all_,enabled_,absolute_,enabledAdds_,enabledRemoves_,absoluteAdds_,absoluteRemoves_,transitions_;
    std::map<View*,String> discarded_;
    bool dispatching_=false;
};
ViewRegistry& globalViewRegistry();
class View {
public:
    explicit View(View* parent=nullptr,ViewRegistry& registry=globalViewRegistry()); // 004d55b0
    virtual ~View(); // 004d58a0: owns and deletes children
    View(const View& source); // 004d56b0, same parent, fresh child set and transitions
    View& operator=(const View&)=delete;
    virtual unsigned type() const noexcept { return 0; } // 005585b0
    virtual void enabledStep() {} // base slots +8/+c: 004bd820
    virtual void absoluteStep() {}
    virtual void addChild(View* child); // 004d3700
    virtual void removeChild(View* child); // 004d55a0
    void setParent(View* parent); // 004d2e50; updates inherited visibility only
    virtual void loadXML(const XmlNode& node); // 004d43a0
    virtual void onShown(); // 004d2ec0
    virtual void onHidden(); // 004d2f50
    virtual void process(); // 004d2fc0
    virtual void onEnabled(); // 004d29a0
    virtual void onDisabled(); // 004d29b0
    virtual void onAbsoluteEnabled(); // 004d29c0
    virtual void onAbsoluteDisabled(); // 004d29d0
    void show(); // 004d37d0; repeated calls still emit onShown
    void hide(); // 004d3720
    void enable(); // 004d3bb0
    void disable(); // 004d3750, also inlined in 004d43a0 / 004d4d80
    void absoluteEnable(); // 004d3fb0
    void absoluteDisable(); // inlined in 004d4d80
    void showTransparently(); // 004d4590
    void hideTransparently(); // 004d4980
    void discardTransparently(); // 004d4d80
    void changeTransparency(float elapsed); // 004d59c0, root-only; elapsed is timer +8 result
    float rootTransparency() const noexcept; // 004ed4f0
    const ViewState& state() const noexcept { return state_; }
    View* parent() const noexcept { return parent_; }
    const ViewRegistry::Views& children() const noexcept { return children_; }
protected:
    ViewState state_;
    ViewRegistry& registry_;
private:
    View* parent_;
    ViewRegistry::Views children_;
    using Propagate=void(View::*)();
    void applyChildren(Propagate callback); // 004d2cb0
    void parentHide(); // 004d2cf0
    void parentDisable(); // 004d2d30
    void parentAbsoluteDisable(); // 004d2d70
    void parentShow(); // 004d2db0
    void parentEnable(); // 004d2df0
    void parentAbsoluteEnable(); // 004d2e20
};
}
