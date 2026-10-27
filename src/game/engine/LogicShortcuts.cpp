#include "LogicShortcuts.h"
#include "InputBindings.h"
#include "GameTimer.h"
#include "Settings.h"
#include <stdexcept>
#include <utility>
namespace farmar::recovered {
namespace {
LogicShortcuts* active=nullptr;
// PE bytes0065d368..0065d36b are each1. These flags belong to the
// process, survive cLogic lifetimes and are independent of actual OS pause.
bool pauseToggle=true,slowToggle=true,fastToggle=true,wireframeToggle=true;
}
LogicShortcuts::LogicShortcuts(InputBindings& input,Settings& settings,GameTimer& timer,LogicShortcutServices services)
    :input_(input),settings_(settings),timer_(timer),services_(std::move(services)) {
    if(active) throw "Object already exist.";
    if(!services_.pause || !services_.requestGraphicsReset || !services_.wireframe || !services_.refreshTextures || !services_.prepareScreenshot)
        throw std::invalid_argument("Logic shortcuts require actual runtime services");
    active=this;
    input_.bind(0x138,&enableModified,nullptr);input_.bind(0x1b8,&enableModified,nullptr);
    input_.bind(0x38,&disableModified,nullptr);input_.bind(0xb8,&disableModified,nullptr);
    input_.bind(0xb7,&screenshot,nullptr);
}
LogicShortcuts::~LogicShortcuts() {
    input_.unbind(0x138,&enableModified,nullptr);input_.unbind(0x1b8,&enableModified,nullptr);
    input_.unbind(0x38,&disableModified,nullptr);input_.unbind(0xb8,&disableModified,nullptr);
    input_.unbind(0xb7,&screenshot,nullptr);
    //00531a70 does not remove the six temporary bindings if Alt remains down.
    // Normal application shutdown stops input dispatch before this destruction.
    active=nullptr;
}
void LogicShortcuts::enableModified(void*) {
    auto& input=active->input_;
    input.bind(0x1c,&fullscreen,nullptr);input.bind(0x14,&triangles,nullptr);
    input.bind(0xc7,&slow,nullptr);input.bind(0xc5,&pause,nullptr);
    input.bind(0xd2,&fast,nullptr);input.bind(0x3f,&refresh,nullptr);
}
void LogicShortcuts::disableModified(void*) {
    auto& input=active->input_;
    input.unbind(0x1c,&fullscreen,nullptr);input.unbind(0x14,&triangles,nullptr);
    input.unbind(0xc7,&slow,nullptr);input.unbind(0xc5,&pause,nullptr);
    input.unbind(0xd2,&fast,nullptr);input.unbind(0x3f,&refresh,nullptr);
}
void LogicShortcuts::fullscreen(void*) {
    auto& self=*active;bool value=false;self.settings_.get(String("graphics"),String("fullscreen"),value);
    self.settings_.set(String("graphics"),String("fullscreen"),!value);self.services_.requestGraphicsReset(true);
}
void LogicShortcuts::triangles(void*) {active->services_.wireframe(wireframeToggle);wireframeToggle=!wireframeToggle;}
void LogicShortcuts::slow(void*) {active->timer_.setMultiplier(slowToggle ? 0.1f:1.f);slowToggle=!slowToggle;}
void LogicShortcuts::pause(void*) {active->services_.pause(pauseToggle);pauseToggle=!pauseToggle;}
void LogicShortcuts::fast(void*) {active->timer_.setMultiplier(fastToggle ? 15.f:1.f);fastToggle=!fastToggle;}
void LogicShortcuts::refresh(void*) {active->services_.refreshTextures();}
void LogicShortcuts::screenshot(void*) {active->services_.prepareScreenshot();}
}
