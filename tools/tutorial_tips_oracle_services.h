#include "TutorialTips.h"
namespace {
unsigned tipSourceFinishes=0,tipNativeFinishes=0,tipSourcePredicates=0,tipNativePredicates=0;
bool tipPredicateResult=false;
void* tipNativeAnchor=nullptr;Control* tipSourceAnchor=nullptr;
void __fastcall tipNativeFinish(void*,void*,void*) {++tipNativeFinishes;}
bool __cdecl tipNativePredicate(void* value) {require(value==tipNativeAnchor,"Tutorial native predicate anchor differs");++tipNativePredicates;return tipPredicateResult;}
bool tipSourcePredicate(Control* value) {require(value==tipSourceAnchor,"Tutorial source predicate anchor differs");++tipSourcePredicates;return tipPredicateResult;}
struct TipDiagnosticHost final:TutorialTipHost {
    GameMenu& hud;bool cancel=false;
    explicit TipDiagnosticHost(GameMenu& value):hud(value) {}
    void finishTip(TutorialCommonTip&) override {++tipSourceFinishes;}
    bool takeCancelRequest() override {const bool result=cancel;cancel=false;return result;}
    void setGameplayPaused(bool value) override {hud.gameplayPaused=value;}
};
}
