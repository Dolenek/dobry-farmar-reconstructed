#pragma once
#include <array>
#include <cstdint>
#include <set>
#include <vector>
namespace farmar::recovered {
// Binding/dispatch part of cInput (00526190/00526970/00527310).
// Device acquisition and character buffering are reconstructed separately.
class InputBindings {
public:
    using Handler=void(*)(void*);
    using MouseHandler=void(*)(unsigned,int,void*);
    using CharHandler=void(*)(wchar_t,void*);
    template<class Function> struct Binding {
        Function function; void* view;
        bool operator<(const Binding& other) const noexcept {
            const auto a=reinterpret_cast<std::uintptr_t>(function),b=reinterpret_cast<std::uintptr_t>(other.function);
            return a==b ? reinterpret_cast<std::uintptr_t>(view)<reinterpret_cast<std::uintptr_t>(other.view):a<b;
        }
    };
    static constexpr unsigned AllControls=531,ControlCount=532;
    void bind(unsigned control,Handler handler,void* view); // 00526190, ignores null handler
    void unbind(unsigned control,Handler handler,void* view); // 005280f0
    void bindMouse(MouseHandler handler,void* view); // 00526970
    void unbindMouse(MouseHandler handler,void* view); // 005288e0
    void bindKeyboard(CharHandler handler,void* view); // 00526580
    void unbindKeyboard(CharHandler handler,void* view); // 005284e0
    void unbindControl(unsigned control); // 00524640
    void unbindAll(); // 00524a30
    void unbindKeyboard(); // 00524e60
    const std::set<Binding<Handler>>& controls(unsigned control) const { return controls_.at(control); }
    const std::set<Binding<MouseHandler>>& mouse() const noexcept { return mouse_; }
    const std::set<Binding<CharHandler>>& keyboard() const noexcept { return keyboard_; }
    // 00527310 snapshots every control handler before any control callbacks;
    // mouse callbacks are snapshotted later and called X/Y/Z per handler.
    void dispatchControls(const std::vector<unsigned>& events);
    void dispatchMouse(const std::array<int,3>& delta);
    void dispatchCharacters(const std::vector<wchar_t>& text);
    void dispatchCharacter(wchar_t character);
private:
    std::array<std::set<Binding<Handler>>,ControlCount> controls_;
    std::set<Binding<MouseHandler>> mouse_;
    std::set<Binding<CharHandler>> keyboard_;
};
}
