#pragma once
#include "GameString.h"
#include <memory>
#include <windows.h>
namespace farmar::recovered {
struct GameApplicationOptions {String dataDirectory{"Data"},commonDirectory;};
// Source application composition for004d1850/0050a2d0/005096a0/00531b20.
// Original global dependencies are concrete restored objects; original PE code
// is never loaded by this application.
class GameApplication final {
public:
    GameApplication(HINSTANCE,const GameApplicationOptions&);
    ~GameApplication();
    GameApplication(const GameApplication&)=delete;GameApplication& operator=(const GameApplication&)=delete;
    void run();
    static bool activateExisting(HINSTANCE);
private:struct Runtime;std::unique_ptr<Runtime> runtime_;
};
}
