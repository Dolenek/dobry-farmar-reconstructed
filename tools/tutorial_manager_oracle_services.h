#include "TutorialManager.h"
namespace {
// Controlled world observations for the manager oracle. These are test inputs,
// never providers shipped as a production replacement for the actual world.
struct TutorialInputs {
    int wellUpgrade=0,mowerUpgrade=0,heldType=0,activity=0,plant=0,itemUpgrade=0,anchorState=0,plotKind=0;
    unsigned itemFlags=0,parentProducts=0,warehouseProducts=0,manufactureProducts=0,free=0;
    double capacity=0,progress=0;float water=0,previous=0;
    bool tool=false,seed=false,wareSelection=false,selection=false,sceneSelection=false;
    unsigned plots=1,plants=1,items=1;
    std::array<int,4> activities{};bool varyingActivity=false;
};
TutorialInputs tutorialInputs;
std::vector<unsigned> tutorialSourceQueries,tutorialNativeQueries;
unsigned tutorialSourceActivity=0,tutorialNativeActivity=0;int tutorialNativeActivityValue=0;
void* tutorialNativeControl=nullptr;void* tutorialNativeMower=nullptr;void* tutorialNativeItemUpgrade=nullptr;
int tutorialActivity(bool native) {auto& count=native ? tutorialNativeActivity:tutorialSourceActivity;(native ? tutorialNativeQueries:tutorialSourceQueries).push_back(5);return tutorialInputs.varyingActivity ? tutorialInputs.activities[(count++)%4]:(++count,tutorialInputs.activity);}
int* __fastcall tutorialCharacterActivity(void*,void*) {tutorialNativeActivityValue=tutorialActivity(true);return &tutorialNativeActivityValue;}
int __fastcall tutorialHeldType(void*,void*) {tutorialNativeQueries.push_back(2);return tutorialInputs.heldType;}
float __fastcall tutorialHeldWater(void*,void*) {tutorialNativeQueries.push_back(3);return tutorialInputs.water;}
double __fastcall tutorialCapacity(void*,void*) {tutorialNativeQueries.push_back(4);return tutorialInputs.capacity;}
int __fastcall tutorialUpgrade(void* self,void*) {
    if(self==static_cast<unsigned char*>(tutorialNativeControl)+0x508) {tutorialNativeQueries.push_back(1);return tutorialInputs.wellUpgrade;}
    if(self==static_cast<unsigned char*>(tutorialNativeMower)+0x508) {tutorialNativeQueries.push_back(18);return tutorialInputs.mowerUpgrade;}
    if(self==static_cast<unsigned char*>(tutorialNativeControl)+0x364) {tutorialNativeQueries.push_back(13);return tutorialInputs.itemUpgrade;}
    return hudLevel();
}
struct TutorialNativeVector {unsigned allocator=0;void **first=nullptr,**end=nullptr,**capacity=nullptr;};
template<unsigned Id> void __fastcall tutorialList(void*,void*,TutorialNativeVector* output) {
    const unsigned count=Id==9 ? tutorialInputs.plots:Id==7 ? tutorialInputs.plants:tutorialInputs.items;
    tutorialNativeQueries.push_back(Id);if(!count) return;output->first=static_cast<void**>(allocate(count*4));for(unsigned i=0;i<count;++i) output->first[i]=tutorialNativeControl;output->end=output->capacity=output->first+count;
}
double __fastcall tutorialProgress(void*,void*) {tutorialNativeQueries.push_back(10);return tutorialInputs.progress;}
int __fastcall tutorialPlant(void*,void*) {tutorialNativeQueries.push_back(8);return tutorialInputs.plant;}
int __fastcall tutorialPlotKind(void*,void*) {tutorialNativeQueries.push_back(11);return tutorialInputs.plotKind;}
unsigned __fastcall tutorialParentProducts(void*,void*) {tutorialNativeQueries.push_back(14);return tutorialInputs.parentProducts;}
bool __fastcall tutorialTool(void*,void*) {tutorialNativeQueries.push_back(15);return tutorialInputs.tool;}
unsigned __fastcall tutorialWarehouseProducts(void*,void*,unsigned n) {require(n==0,"Tutorial warehouse product differs");tutorialNativeQueries.push_back(6);return tutorialInputs.warehouseProducts;}
unsigned __fastcall tutorialManufactureProducts(void*,void*,unsigned n) {require(n==0,"Tutorial manufacture product differs");tutorialNativeQueries.push_back(16);return tutorialInputs.manufactureProducts;}
unsigned __fastcall tutorialWarehouseFree(void*,void*) {tutorialNativeQueries.push_back(17);return tutorialInputs.free;}
bool __cdecl tutorialWareSelection() {return tutorialInputs.wareSelection;}
void __stdcall tutorialVectorDestroy(void* data,unsigned size,int count,void(__thiscall* destroy)(void*)) {
    // Explicit old CRT normal-path adapter, calling every native record dtor
    // in the original reverse order. Exception cleanup is a separate scope.
    while(count>0) destroy(static_cast<unsigned char*>(data)+size*unsigned(--count));
}
struct TutorialWorldFixture final:TutorialWorld {
    HudWorld& world;Control& anchor;
    std::array<View*,4> dialogs{};
    TutorialWorldFixture(HudWorld& w,Control& a):world(w),anchor(a) {}
    GameMenu& hud() override {return *world.menu;}
    StateView& farm() override {return world.farm();}StateView& wareDetails() override {return world.wareDetails();}StateView& manufacture() override {return world.manufacture();}
    View& selectionOverlay() override {return world.selectionOverlay();}View& sceneOverlay() override {return world.sceneOverlay();}
    Control& well() override {return anchor;}Control& warehouse() override {return anchor;}
    std::vector<Control*> farmPlots() override {tutorialSourceQueries.push_back(9);return std::vector<Control*>(tutorialInputs.plots,&anchor);}
    std::vector<Control*> farmPlants() override {tutorialSourceQueries.push_back(7);return std::vector<Control*>(tutorialInputs.plants,&anchor);}
    std::vector<Control*> wareItems() override {tutorialSourceQueries.push_back(12);return std::vector<Control*>(tutorialInputs.items,&anchor);}
    double plotWater(Control&) override {tutorialSourceQueries.push_back(10);return tutorialInputs.progress;}float plotWaterLimit(Control&) const override {return tutorialInputs.previous;}
    int plotKind(Control&) override {tutorialSourceQueries.push_back(11);return tutorialInputs.plotKind;}int plantState(Control&) override {tutorialSourceQueries.push_back(8);return tutorialInputs.plant;}
    int wareItemUpgrade(Control&) override {tutorialSourceQueries.push_back(13);return tutorialInputs.itemUpgrade;}unsigned wareItemFlags(Control&) const override {return tutorialInputs.itemFlags;}
    int anchorState(Control&) const override {return tutorialInputs.anchorState;}unsigned parentProductCount(Control&) override {tutorialSourceQueries.push_back(14);return tutorialInputs.parentProducts;}
    int wellUpgradeLevel() override {tutorialSourceQueries.push_back(1);return tutorialInputs.wellUpgrade;}int mowerUpgradeLevel() override {tutorialSourceQueries.push_back(18);return tutorialInputs.mowerUpgrade;}
    double wellCapacity() override {tutorialSourceQueries.push_back(4);return tutorialInputs.capacity;}int characterHeldType() override {tutorialSourceQueries.push_back(2);return tutorialInputs.heldType;}
    float characterHeldWater() override {tutorialSourceQueries.push_back(3);return tutorialInputs.water;}int characterActivity() override {return tutorialActivity(false);}
    bool characterToolFlag() override {tutorialSourceQueries.push_back(15);return tutorialInputs.tool;}
    unsigned warehouseProductCount(unsigned) override {tutorialSourceQueries.push_back(6);return tutorialInputs.warehouseProducts;}unsigned manufactureProductCount(unsigned) override {tutorialSourceQueries.push_back(16);return tutorialInputs.manufactureProducts;}
    unsigned warehouseFree() override {tutorialSourceQueries.push_back(17);return tutorialInputs.free;}bool seedMoveActive() const override {return tutorialInputs.seed;}
    bool wareSelectionPresent() const override {return tutorialInputs.wareSelection;}bool selectionPresent() const override {return tutorialInputs.selection;}
    bool sceneSelectionPresent() const override {return tutorialInputs.sceneSelection;}
    View& contractConfirm() override {return *dialogs[0];}View& contractAim() override {return *dialogs[1];}
    View& contractDialog764() override {return *dialogs[2];}View& contractProvide() override {return *dialogs[3];}
};
}
