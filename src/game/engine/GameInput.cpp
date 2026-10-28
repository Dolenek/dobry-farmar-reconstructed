#include "GameInput.h"
#include "InputBindings.h"
#include "Game.h"
#include "GameAwardManager.h"
#include <windows.h>
#include <stdexcept>
#include <utility>
#include <cwchar>
namespace farmar::recovered {
namespace {
GameInput* active=nullptr;
WString characters; //0065f704, shared across object lifetimes
GameInputServices actualServices() {
    return {[]{return Game::instance()!=nullptr;},
        []{Game::instance()->cheatMoney();},[]{Game::instance()->cheatUpgrades();},
        []{Game::instance()->cheatSeeds();},[]{Game::instance()->cheatProducts();},
        []()->int*{auto* awards=GameAwardManager::instance();return awards ? &awards->requested:nullptr;},
        [](unsigned frequency,unsigned duration){Beep(frequency,duration);}};
}
}
GameInput::GameInput(InputBindings& input):GameInput(input,actualServices()) {}
GameInput::GameInput(InputBindings& input,GameInputServices services):input_(input),services_(std::move(services)) {
    if(active) throw "Object already exist.";
    if(!services_.gamePresent || !services_.money || !services_.upgrades || !services_.seeds || !services_.products || !services_.requestedAward || !services_.beep)
        throw std::invalid_argument("GameInput requires actual game/award/audio services");
    active=this;input_.bindKeyboard(&character,this);
}
GameInput::~GameInput() {
    // Original Input vtable005ffc48+18 is00524e60: clear ALL char handlers.
    input_.unbindKeyboard();characters.erase();active=nullptr;
}
const WString& GameInput::buffer() noexcept {return characters;}
bool GameInput::suffix(const wchar_t* text) {
    const auto count=static_cast<WString::size_type>(std::wcslen(text));
    if(count>characters.size()) return false;
    //00402240 lowercases a temporary suffix via00401140. The game keeps
    // the CRT C locale:00580415 folds ASCII A..Z only; the buffer is unchanged.
    for(unsigned i=0,start=characters.size()-count;i<count;++i) {
        wchar_t unit=characters.data()[start+i];if(unit>=L'A' && unit<=L'Z') unit+=0x20;
        if(unit!=text[i]) return false;
    }
    characters.erase();return true;
}
void GameInput::character(wchar_t code,void* object) {
    auto& self=*static_cast<GameInput*>(object);characters.append(&code,1);
    if(characters.size()>20) characters.erase(0,1);
    auto& s=self.services_;
    if(s.gamePresent()) {
        if(suffix(L"cheatmoney")) s.money();
        else if(suffix(L"cheatupgrades")) s.upgrades();
        else if(suffix(L"cheatseeds")) s.seeds();
        else if(suffix(L"cheatproduct")) s.products();
        else if(suffix(L"cheatall")) {s.beep(1000,200);s.money();s.upgrades();s.seeds();s.products();}
    }
    if(auto* requested=s.requestedAward()) {
        for(unsigned index=1;index<=21;++index) {
            const wchar_t name[]={L'a',wchar_t(L'0'+index/10),wchar_t(L'0'+index%10),0};
            if(suffix(name)) {*requested=int(index-1);break;}
        }
    }
}
}
