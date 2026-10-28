#pragma once
#include "GameWString.h"
#include <functional>
namespace farmar::recovered {
class InputBindings;
struct GameInputServices {
    std::function<bool()> gamePresent;
    std::function<void()> money,upgrades,seeds,products;
    std::function<int*()> requestedAward;
    std::function<void(unsigned,unsigned)> beep;
};
// cGameInput004030e0/00401de0 and character callback00402400.
// The default constructor binds actual Game/Award singletons and Win32 Beep.
class GameInput {
public:
    explicit GameInput(InputBindings&);
    GameInput(InputBindings&,GameInputServices);
    ~GameInput();
    GameInput(const GameInput&)=delete;
    GameInput& operator=(const GameInput&)=delete;
    static const WString& buffer() noexcept;
private:
    InputBindings& input_;GameInputServices services_;
    static void character(wchar_t,void*);
    static bool suffix(const wchar_t*); //00402240, clears buffer only on match
};
}
