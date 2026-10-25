#pragma once
#include "GameString.h"
namespace farmar::recovered {
class FileIndex;class Profile;class Farm;class GameMenu;class Gameplay;class SalesHistory;
// Separate binary .save owned by cGame, alongside the typed XML profile.
// Uses the actual world objects; no parallel replacement game state.
class GamePersistence final {
public:
    GamePersistence(FileIndex& files,Profile& profile,Farm& farm,GameMenu& hud,Gameplay& gameplay,SalesHistory& sales)
        :files_(files),profile_(profile),farm_(farm),hud_(hud),gameplay_(gameplay),sales_(sales) {}
    void save(); //0040fbb0
    void restore(); //00412850; before Person is constructed
    String filename() const;
private:
    FileIndex& files_;Profile& profile_;Farm& farm_;GameMenu& hud_;Gameplay& gameplay_;SalesHistory& sales_;
};
}
