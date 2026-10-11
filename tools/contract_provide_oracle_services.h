namespace {
void* offerNativeLetter=nullptr;void* offerNativeReplacement=nullptr;void* offerNativeYes=nullptr;
bool offerTutorial=false;
int __cdecl offerNativeMoney() {contractNativeCalls.push_back({9,0,0,0,0});return contractNativeProfile->ints["Money"];}
void __fastcall offerNativeConsume(void*,void*,void* letter) {
    require(letter==offerNativeLetter,"Offer consumed a different native letter");contractNativeCalls.push_back({10,0,0,0,0});
    if(offerNativeReplacement) field<void*>(letter,0x37c)=offerNativeReplacement;
}
bool __fastcall offerNativeCanShow(void*,void*,unsigned step) {
    if(step==0x12) return false;require(step==0x2a,"Offer tutorial query step differs");contractNativeCalls.push_back({11,step,0,0,0});return offerTutorial;
}
void __fastcall offerNativeShow(void*,void*,unsigned step,void* anchor) {
    require(step==0x2a && anchor==offerNativeYes,"Offer tutorial anchor differs");contractNativeCalls.push_back({1,step,0,0,0});
}
struct OfferLetterFixture final:ContractOfferLetter {
    GameplayEvent* current=nullptr;
    GameplayEvent& contractEvent() const override {return *current;}
};
}
