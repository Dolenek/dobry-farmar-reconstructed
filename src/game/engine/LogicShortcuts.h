#pragma once
#include <functional>
namespace farmar::recovered {
class Settings;
class GameTimer;
class InputBindings;
struct LogicShortcutServices {
    std::function<void(bool)> pause;
    std::function<void(bool)> requestGraphicsReset;
    std::function<void(bool)> wireframe;
    std::function<void()> refreshTextures;
    std::function<void()> prepareScreenshot;
};
// Binding/callback portion of original cLogic005315b0, not its24-byte ABI.
// Every service is required and supplied by the actual application runtime.
class LogicShortcuts {
public:
    LogicShortcuts(InputBindings& input,Settings& settings,GameTimer& timer,LogicShortcutServices services);
    ~LogicShortcuts();
    LogicShortcuts(const LogicShortcuts&)=delete;
    LogicShortcuts& operator=(const LogicShortcuts&)=delete;
private:
    InputBindings& input_;Settings& settings_;GameTimer& timer_;LogicShortcutServices services_;
    static void enableModified(void*); //005314b0
    static void disableModified(void*); //00531530
    static void fullscreen(void*); //00530700
    static void triangles(void*); //00531010
    static void slow(void*); //0052fa00
    static void pause(void*); //0052f9a0
    static void fast(void*); //0052fa40
    static void refresh(void*); //0052fa80
    static void screenshot(void*); //00530c50
};
}
