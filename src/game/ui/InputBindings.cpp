#include "InputBindings.h"
namespace farmar::recovered {
void InputBindings::bind(unsigned control,Handler handler,void* view) { if(handler) controls_.at(control).insert({handler,view}); }
void InputBindings::unbind(unsigned control,Handler handler,void* view) { if(handler) controls_.at(control).erase({handler,view}); }
void InputBindings::bindMouse(MouseHandler handler,void* view) { mouse_.insert({handler,view}); }
void InputBindings::unbindMouse(MouseHandler handler,void* view) { mouse_.erase({handler,view}); }
void InputBindings::bindKeyboard(CharHandler handler,void* view) { keyboard_.insert({handler,view}); }
void InputBindings::unbindKeyboard(CharHandler handler,void* view) { keyboard_.erase({handler,view}); }
void InputBindings::unbindControl(unsigned control) { controls_.at(control).clear(); }
void InputBindings::unbindAll() { for(auto& bindings:controls_) bindings.clear(); }
void InputBindings::unbindKeyboard() { keyboard_.clear(); }
void InputBindings::dispatchControls(const std::vector<unsigned>& events) {
    std::vector<Binding<Handler>> calls;
    for(const auto event:events) {
        const auto& direct=controls_.at(event); calls.insert(calls.end(),direct.begin(),direct.end());
        const auto& all=controls_[AllControls]; calls.insert(calls.end(),all.begin(),all.end());
    }
    for(const auto& binding:calls) binding.function(binding.view);
}
void InputBindings::dispatchMouse(const std::array<int,3>& delta) {
    const std::vector<Binding<MouseHandler>> calls(mouse_.begin(),mouse_.end());
    for(const auto& binding:calls) for(unsigned axis=0;axis<3;++axis) if(delta[axis]) binding.function(0x210+axis,delta[axis],binding.view);
}
void InputBindings::dispatchCharacters(const std::vector<wchar_t>& text) {
    for(const auto unit:text) dispatchCharacter(unit);
}
void InputBindings::dispatchCharacter(wchar_t unit) { for(auto it=keyboard_.begin();it!=keyboard_.end();++it) it->function(unit,it->view); }
}
