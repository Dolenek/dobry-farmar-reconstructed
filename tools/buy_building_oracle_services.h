namespace {
std::vector<std::pair<unsigned,int>> buildingSourceChoices,buildingNativeChoices;
void __fastcall buildingNativeChoose(void*,void*,unsigned building,int slot) {buildingNativeChoices.emplace_back(building,slot);field<unsigned>(image,0x260150)=1;}
struct BuildingWorldFixture final:BuildingPurchaseWorld {
    GameMenu& menu;TutorialManager& tips;ProfileProperties& profile;
    BuildingWorldFixture(GameMenu& h,TutorialManager& t,ProfileProperties& p):menu(h),tips(t),profile(p) {}
    GameMenu& hud() override {return menu;}TutorialManager& tutorial() override {return tips;}
    int money() override {hudSourceCalls.push_back({10,0,0});return profile.ints["Money"];}
    void chooseBuilding(unsigned building,int slot) override {buildingSourceChoices.emplace_back(building,slot);tutorialInputs.wareSelection=true;}
};
}
