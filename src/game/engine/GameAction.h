#pragma once
#include <memory>
namespace farmar::recovered {
class View;
// Original 12-byte cAction base. Its generic begin/finish are genuine RETs
// and its generic step returns true; farm/person derived actions supply their
// original timed behavior. The source object layout is kept separate from
// the diagnostic reference ABI.
class GameAction {
public:
    GameAction(View* owner,unsigned type=0); // 00415320, count 0065ff58
    virtual ~GameAction(); // 00415350
    virtual void begin() {} // 004bd820
    virtual bool step() {return true;} // 00415340, actual base body
    virtual void finish() {} // 004bd820
    GameAction(const GameAction&)=delete;GameAction& operator=(const GameAction&)=delete;
    View* owner;unsigned type;
    static unsigned count() noexcept;
};
// Original single current-action owner at Field+504 / Person+4. Completion
// detaches before finish, so finish may install another action in this owner.
class ActionProcessor {
public:
    ActionProcessor()=default; // 00415360
    ~ActionProcessor()=default; // 00415370, destroys current without finish
    ActionProcessor(const ActionProcessor&)=delete;ActionProcessor& operator=(const ActionProcessor&)=delete;
    void set(std::unique_ptr<GameAction>); // 00415790
    void step(); // 004153b0
    GameAction* current() const noexcept {return current_.get();}
private:
    std::unique_ptr<GameAction> current_;
};
}
