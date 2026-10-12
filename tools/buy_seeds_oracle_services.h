namespace {
std::vector<unsigned> seedSourceChoices,seedNativeChoices;
void __fastcall seedNativeChoose(void*,void*,unsigned seed) {seedNativeChoices.push_back(seed);field<unsigned char>(image,0x25f721)=1;}
struct SeedWorldFixture final:SeedPurchaseWorld {
    GameMenu& menu;TutorialManager& tips;ProfileProperties& profile;
    SeedWorldFixture(GameMenu& h,TutorialManager& t,ProfileProperties& p):menu(h),tips(t),profile(p) {}
    GameMenu& hud() override {return menu;}TutorialManager& tutorial() override {return tips;}
    int money() override {hudSourceCalls.push_back({10,0,0});return profile.ints["Money"];}
    void chooseSeed(unsigned seed) override {seedSourceChoices.push_back(seed);tutorialInputs.seed=true;}
};
}
