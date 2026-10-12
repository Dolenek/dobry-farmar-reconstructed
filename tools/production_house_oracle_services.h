// Diagnostic observations at unreconstructed Manufacture/warehouse boundaries.
// These providers never ship in a game target.
namespace {
using ProductionCalls=std::vector<std::array<unsigned,5>>;
ProductionCalls productionSourceCalls,productionNativeCalls;
std::array<unsigned,19> productionSourceGoods{},productionNativeGoods{};
unsigned productionSourceFree=100,productionNativeFree=100;
bool productionValid=true,productionBlocked=false;
std::vector<ProductionBuilding*> productionSourceBuildings;
std::vector<void*> productionNativeBuildings;
ProductionBuilding* productionMoving=nullptr;ProductionBuilding* productionFocused=nullptr;
unsigned __fastcall productionGoods(void*,void*,unsigned product) {productionNativeCalls.push_back({1,product,0,0,0});return productionNativeGoods.at(product);}
unsigned __fastcall productionFree(void*,void*) {productionNativeCalls.push_back({2,0,0,0,0});return productionNativeFree;}
void __fastcall productionTake(void*,void*,unsigned product,unsigned count,bool a,bool b) {
    productionNativeCalls.push_back({3,product,count,unsigned(a),unsigned(b)});productionNativeGoods.at(product)-=count;productionNativeFree+=count;
}
void __fastcall productionGive(void*,void*,unsigned product,unsigned count,bool a,bool b) {
    productionNativeCalls.push_back({4,product,count,unsigned(a),unsigned(b)});productionNativeGoods.at(product)+=count;productionNativeFree-=count;
}
bool __fastcall productionPlacement(void*,void*,void*) {productionNativeCalls.push_back({5,0,0,0,0});return productionValid;}
bool __fastcall productionManipulationBlocked(void*,void*) {productionNativeCalls.push_back({6,0,0,0,0});return productionBlocked;}
void __fastcall productionBegin(void*,void*) {productionNativeCalls.push_back({7,0,0,0,0});}
void __fastcall productionEnd(void*,void*) {productionNativeCalls.push_back({8,0,0,0,0});}
void __fastcall productionList(void*,void*,TutorialNativeVector* out) {
    productionNativeCalls.push_back({9,0,0,0,0});if(productionNativeBuildings.empty()) return;
    out->first=static_cast<void**>(allocate(productionNativeBuildings.size()*4));std::copy(productionNativeBuildings.begin(),productionNativeBuildings.end(),out->first);out->end=out->capacity=out->first+productionNativeBuildings.size();
}
void __fastcall productionFocus(void* self,void*,void* building,bool flag) {productionNativeCalls.push_back({10,0,unsigned(flag),0,0});field<void*>(self,0xf8)=building;}
void __fastcall productionRemove(void*,void*,void* building) {productionNativeCalls.push_back({11,0,0,0,0});productionNativeBuildings.erase(std::find(productionNativeBuildings.begin(),productionNativeBuildings.end(),building));}
int __cdecl productionMoney() {productionNativeCalls.push_back({12,0,0,0,0});return contractNativeProfile->ints["Money"];}
void __cdecl productionSetMoney(int value) {productionNativeCalls.push_back({13,unsigned(value),0,0,0});contractNativeProfile->ints["Money"]=value;}
struct ProductionWorldFixture final:ProductionBuildingWorld {
    HudWorld& hudWorld;TutorialManager& tips;ProfileProperties& profile;
    ProductionWorldFixture(HudWorld& w,TutorialManager& t,ProfileProperties& p):hudWorld(w),tips(t),profile(p) {}
    TutorialManager& tutorial() override {return tips;}GameMenu& hud() override {return *hudWorld.menu;}
    ProductionBuilding*& movingBuilding() override {return productionMoving;}
    ProductionBuilding* focusedBuilding() override {return productionFocused;}
    std::vector<ProductionBuilding*> buildings() override {productionSourceCalls.push_back({9,0,0,0,0});return productionSourceBuildings;}
    bool manipulationBlocked() override {productionSourceCalls.push_back({6,0,0,0,0});return productionBlocked;}
    bool validPlacement(ProductionBuilding&) override {productionSourceCalls.push_back({5,0,0,0,0});return productionValid;}
    void beginManipulation() override {productionSourceCalls.push_back({7,0,0,0,0});}
    void endManipulation() override {productionSourceCalls.push_back({8,0,0,0,0});}
    void focus(ProductionBuilding* building,bool flag) override {productionSourceCalls.push_back({10,0,unsigned(flag),0,0});productionFocused=building;}
    void remove(ProductionBuilding& building) override {productionSourceCalls.push_back({11,0,0,0,0});productionSourceBuildings.erase(std::find(productionSourceBuildings.begin(),productionSourceBuildings.end(),&building));}
    bool scenesMoving() override {return hudWorld.scenes[0]->movement || hudWorld.scenes[1]->movement || hudWorld.scenes[2]->movement;}
    unsigned warehouseGoods(unsigned product) override {productionSourceCalls.push_back({1,product,0,0,0});return productionSourceGoods.at(product);}
    unsigned warehouseFree() override {productionSourceCalls.push_back({2,0,0,0,0});return productionSourceFree;}
    void giveWarehouse(unsigned product,unsigned count,bool a,bool b) override {productionSourceCalls.push_back({4,product,count,unsigned(a),unsigned(b)});productionSourceGoods.at(product)+=count;productionSourceFree-=count;}
    void takeWarehouse(unsigned product,unsigned count,bool a,bool b) override {productionSourceCalls.push_back({3,product,count,unsigned(a),unsigned(b)});productionSourceGoods.at(product)-=count;productionSourceFree+=count;}
    int money() override {productionSourceCalls.push_back({12,0,0,0,0});return profile.ints["Money"];}
    void setMoney(int value) override {productionSourceCalls.push_back({13,unsigned(value),0,0,0});profile.ints["Money"]=value;}
};
}
