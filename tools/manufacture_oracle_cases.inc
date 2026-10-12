// Restore the original Manufacture methods that the separate house oracle
// observes as boundaries. The real controller now owns all twenty slots.
stage=440;for(const auto& entry:manufactureEntries) write(entry.first,entry.second.data(),5);
// Money is a profile query in the original (00410ce0). Upgrade reads the same
// source profile directly; compare its values/effects, not diagnostic logging.
redirect(0x10ce0,reinterpret_cast<void*>(&manufactureMoney));redirect(0x32b20,reinterpret_cast<void*>(&deferredString));
// The reference is mapped without its CRT startup; retain registrations as
// diagnostic observations instead of entering its uninitialized atexit table.
redirect(0x180aed,reinterpret_cast<void*>(&registerOracleExit));
audio.tracePlay=true;soundTable[1]=reinterpret_cast<void*>(&contractPlaySound);field<cResource*>(nativeGame,0xa08)=configuration.upgradeSound();field<void*>(image,0x25f74c)=nullptr;
struct ManufactureFixture final:ManufactureWorld {
    HudWorld& world;TutorialManager& tips;ProfileProperties& profile;BuyBuilding* panel=nullptr;
    ManufactureFixture(HudWorld& w,TutorialManager& t,ProfileProperties& p):world(w),tips(t),profile(p) {}
    GameMenu& hud() override {return *world.menu;}TutorialManager& tutorial() override {return tips;}
    BuyBuilding& purchasePanel() override {require(panel,"Manufacture purchase panel not connected");return *panel;}
    StateView& farmScene() override {return *world.scenes[0];}StateView& tradeScene() override {return *world.scenes[1];}
    unsigned warehouseGoods(unsigned n) override {productionSourceCalls.push_back({1,n,0,0,0});return productionSourceGoods.at(n);}
    unsigned warehouseFree() override {productionSourceCalls.push_back({2,0,0,0,0});return productionSourceFree;}
    void giveWarehouse(unsigned n,unsigned amount,bool a,bool b) override {productionSourceCalls.push_back({4,n,amount,unsigned(a),unsigned(b)});productionSourceGoods.at(n)+=amount;productionSourceFree-=amount;}
    void takeWarehouse(unsigned n,unsigned amount,bool a,bool b) override {productionSourceCalls.push_back({3,n,amount,unsigned(a),unsigned(b)});productionSourceGoods.at(n)-=amount;productionSourceFree+=amount;}
    int money() override {return profile.ints["Money"];}
    void setMoney(int value) override {productionSourceCalls.push_back({13,unsigned(value),0,0,0});profile.ints["Money"]=value;}
};
ManufactureFixture manufactureWorld(world,*productionTutorial,profile);
ManufactureContext manufactureContext{manager,settings,strings,input,interaction,profile,configuration,audio,productionUpgrades,manufactureWorld,views,controls};
// Manufacture runs in its own process rather than repeating the separately
// required 55-house oracle first. This keeps the fixed shared 512 MiB test heap
// within scope while preserving every house case in production-houses modes.
productionSourceGoods.fill(100);productionNativeGoods=productionSourceGoods;productionSourceFree=productionNativeFree=100;
for(unsigned slot=0;slot<20;++slot) {
    const auto id=std::to_string(slot);profile.ints["Upgrade"+id]=nativeProfile.ints["Upgrade"+id]=slot<11 ? 3:-1;
    profile.uints["ProdType"+id]=nativeProfile.uints["ProdType"+id]=slot<11 ? slot:0;
    profile.uints["ProdCount"+id]=nativeProfile.uints["ProdCount"+id]=slot<11 ? 1:0;profile.uints["ProdStop"+id]=nativeProfile.uints["ProdStop"+id]=0;
    profile.floats["ProdPct"+id]=nativeProfile.floats["ProdPct"+id]=0;profile.floats["XProdPos"+id]=nativeProfile.floats["XProdPos"+id]=400;profile.floats["YProdPos"+id]=nativeProfile.floats["YProdPos"+id]=300;
}
putPointer(0x25f758,nullptr);auto scene=std::make_unique<Manufacture>(&game,manufactureContext);auto* nativeScene=allocate(256);std::memset(nativeScene,0,256);function<void*(__thiscall*)(void*,void*)>(0x99730)(nativeScene,nativeGame);
const auto sceneCheck=[&] {
    stage=441;require(Manufacture::instance()==scene.get() && field<void*>(image,0x25f758)==nativeScene,"Manufacture singleton differs");mainMenuCompareView(*scene,nativeScene);
    compareControl(scene->movingPicture(),field<void*>(nativeScene,0x38));compareControl(scene->origin(),field<void*>(nativeScene,0x60));
    require(scene->count==field<unsigned>(nativeScene,0xf4),"Manufacture count differs");require(!std::memcmp(scene->allowed.data(),static_cast<unsigned char*>(nativeScene)+0x68,16),"Manufacture allowed rectangle differs");
    auto* rectangles=field<ProductionRectangle*>(nativeScene,0x7c);require(scene->forbidden.size()==std::size_t(field<ProductionRectangle*>(nativeScene,0x80)-rectangles),"Manufacture forbidden rectangle count differs");
    for(unsigned i=0;i<scene->forbidden.size();++i) require(!std::memcmp(scene->forbidden[i].data(),rectangles[i].data(),16),"Manufacture forbidden rectangle differs");
    for(auto pair:std::initializer_list<std::pair<Control*,unsigned>>{{&scene->buildButton(),0x88},{&scene->currentText(),0x8c},{&scene->currentPicture(),0x90},{&scene->destroyButton(),0x94},{&scene->upgradeButton(),0x98},{&scene->upgradePrice(),0x9c}}) {
        action=pair.second;auto* n=field<void*>(nativeScene,pair.second);compareControl(*pair.first,n);if(auto* text=dynamic_cast<TextLine*>(pair.first)) {if(text->textDirty()) text->makeLetters();compare(*text,n);}
        if(auto* button=dynamic_cast<TextAnimationButton*>(pair.first)) {auto& line=button->text();if(line.textDirty()) line.makeLetters();compare(line,field<void*>(n,0x27c));}
    }
    unsigned occupied=0;for(unsigned slot=0;slot<20;++slot) {
        auto* building=scene->slots()[slot];auto* n=field<void*>(nativeScene,0xa4+slot*4);require(bool(building)==bool(n),"Manufacture slot occupancy differs");if(!building) continue;++occupied;
        compareControl(*building,n);require(building->buildingType==field<unsigned>(n,0x390) && building->propertyId==field<unsigned>(n,0x388) && building->flags==field<unsigned>(n,0x574),"Manufacture slot building differs");
        compareFloat(building->productionSpeed,field<float>(n,0x394),"Manufacture house speed");
        require(building->built==bool(field<unsigned char>(n,0x578)),"Manufacture built flag differs");
        for(auto pair:std::initializer_list<std::pair<Control*,unsigned>>{{&building->warehouseQuantity(),0x58c},{&building->loadedQuantity(),0x590},{&building->loadedSlider(),0x598},{&building->lighter(),0x620}}) {auto* child=field<void*>(n,pair.second);compareControl(*pair.first,child);if(auto* line=dynamic_cast<TextLine*>(pair.first)) {if(line->textDirty()) line->makeLetters();compare(*line,child);}}
        if(scene->focusedBuilding()==building) require(field<void*>(nativeScene,0xf8)==n,"Manufacture focused slot differs");
        if(scene->movingBuilding()==building) require(field<void*>(image,0x260150)==n,"Manufacture moving slot differs");
    }
    require(bool(scene->focusedBuilding())==bool(field<void*>(nativeScene,0xf8)),"Manufacture focus presence differs");require(bool(scene->movingBuilding())==bool(field<void*>(image,0x260150)),"Manufacture moving presence differs");
    require(occupied==scene->buildings().size(),"Manufacture slot listing differs");
    require(profile.ints==nativeProfile.ints && profile.uints==nativeProfile.uints && profile.floats==nativeProfile.floats && profile.booleans==nativeProfile.booleans,"Manufacture profile effects differ");
    require(productionSourceGoods==productionNativeGoods && productionSourceFree==productionNativeFree,"Manufacture warehouse observations differ");
    if(productionSourceCalls!=productionNativeCalls) {for(auto c:productionSourceCalls) std::cerr<<"MS "<<c[0]<<' '<<c[1]<<'\n';for(auto c:productionNativeCalls) std::cerr<<"MN "<<c[0]<<' '<<c[1]<<'\n';}
    require(productionSourceCalls==productionNativeCalls,"Manufacture primitive call order differs");productionSourceCalls.clear();productionNativeCalls.clear();require(actualSounds==expectedSounds,"Manufacture sound requests differ");actualSounds.clear();expectedSounds.clear();++manufactureStates;
};sceneCheck();++manufactureObjects;
for(unsigned type=0;type<11;++type) {
    stage=442;action=type;
    auto* building=scene->create(type,-1);auto* n=function<void*(__thiscall*)(void*,unsigned,int)>(0x94390)(nativeScene,type,-1);require(bool(building)==bool(n),"Manufacture factory capacity differs");sceneCheck();++manufactureFactories;
}
// Existing saved + newly created houses reach twenty; additional creates must
// return null at the exact native count gate. Their real profile/UI are compared.
require(scene->count==20 && !scene->create(0,-1) && !function<void*(__thiscall*)(void*,unsigned,int)>(0x94390)(nativeScene,0,-1),"Manufacture full factory gate differs");sceneCheck();
// Select all eleven genuine saved house kinds through every upgrade level,
// availability rejection and signed-money branch, including repeated selection.
for(unsigned slot=0;slot<11;++slot) for(int level=0;level<4;++level) for(unsigned available:{0xffffffffU,0U,3U}) for(int cash:{-1,0,100000}) {
    stage=444;trial=slot;action=level;auto* house=scene->slots()[slot];auto* n=field<void*>(nativeScene,0xa4+slot*4);
    profile.uints["UpAvail"+std::to_string(house->itemType)]=nativeProfile.uints["UpAvail"+std::to_string(house->itemType)]=available;profile.ints["Money"]=nativeProfile.ints["Money"]=cash;
    stage=4440;house->setLevel(level,false);stage=4441;function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(n)+0x364,level,false);
    stage=4442;scene->focus(house,true);stage=4443;function<void(__thiscall*)(void*,void*,bool)>(0x94e70)(nativeScene,n,true);sceneCheck();
    scene->focus(house,true);function<void(__thiscall*)(void*,void*,bool)>(0x94e70)(nativeScene,n,true);sceneCheck();
    scene->focus(nullptr,true);function<void(__thiscall*)(void*,void*,bool)>(0x94e70)(nativeScene,nullptr,true);sceneCheck();
}
for(unsigned slot=0;slot<11;++slot) for(int level=0;level<3;++level) for(int step:{-1,5,34}) {
    stage=445;trial=slot;action=step;auto* house=scene->slots()[slot];auto* n=field<void*>(nativeScene,0xa4+slot*4);
    productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=step;
    profile.uints["UpAvail"+std::to_string(house->itemType)]=nativeProfile.uints["UpAvail"+std::to_string(house->itemType)]=3;profile.ints["Money"]=nativeProfile.ints["Money"]=100000;
    house->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(n)+0x364,level,false);
    scene->focus(house,false);function<void(__thiscall*)(void*,void*,bool)>(0x94e70)(nativeScene,n,false);sceneCheck();
    scene->upgradeButton().clickEvent(&scene->upgradeButton());function<void(__cdecl*)(void*)>(0x95fd0)(field<void*>(nativeScene,0x98));sceneCheck();
}
productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
scene->focus(nullptr,false);function<void(__thiscall*)(void*,void*,bool)>(0x94e70)(nativeScene,nullptr,false);sceneCheck();
// The scene owns real house masks: compare placement with all other slots,
// borders and forbidden regions, independently of the isolated geometry matrix.
std::mt19937 manufactureRandom(0x494a20);
for(unsigned slot:{0U,3U,7U,10U}) for(unsigned sample=0;sample<512;++sample) {
    stage=446;trial=slot;action=sample;auto* house=scene->slots()[slot];auto* n=field<void*>(nativeScene,0xa4+slot*4);
    const Point2 point{float(int(manufactureRandom()%15000)-1000)*0.0625f,float(int(manufactureRandom()%12000)-1000)*0.0625f};
    house->positionProduction(point,false);function<void(__thiscall*)(void*,const Point2*,bool)>(0x9f3c0)(n,&point,false);
    require(scene->validPlacement(*house)==function<bool(__thiscall*)(void*,void*)>(0x94a20)(nativeScene,n),"Manufacture real mask placement differs");++manufacturePlacements;
    cursorPosition=point;require(scene->manipulationBlocked()==function<bool(__thiscall*)(void*)>(0x93b70)(nativeScene),"Manufacture real control guard differs");++manufacturePlacements;
}
sceneCheck();
// Actual deferred removal of the nine unfinished factory houses. The map
// helper is an ABI adapter; both original removal wrappers still execute.
const auto deleteDeferredHouse=[&](ProductionBuilding* house,void* n) {
    require(views.discarded().size()==1 && nativeDeferredStrings.size()==1 && views.discarded().at(house).view()==nativeDeferredStrings.at(n).view(),"Manufacture deferred house origin differs");
    const auto before=textureCounts();views.deleteDiscarded();const auto middle=textureCounts();call(0x9b080,n);compareReferenceDeltas(before,middle);nativeDeferredStrings.clear();++manufactureRemovals;
};
for(unsigned slot=11;slot<20;++slot) {
    stage=447;trial=slot;auto* house=scene->slots()[slot];auto* n=field<void*>(nativeScene,0xa4+slot*4);require(house && !house->built,"Manufacture factory restoration fixture differs");
    scene->remove(*house);function<void(__thiscall*)(void*,void*)>(0x98c60)(nativeScene,n);sceneCheck();deleteDeferredHouse(house,n);sceneCheck();
}
// Replace the HUD's Manufacture service with the actual restored scene. Its
// other scene providers remain the declared Farm/WareDetail diagnostic bases.
auto* previousSourceManufacture=world.scenes[2];world.scenes[2]=scene.get();scene->movingBuilding()=nullptr;putPointer(0x260150,nullptr);
for(unsigned i=0;i<3;++i) {scenes[i]->hide();scenes[i]->disable();scenes[i]->absoluteDisable();call(0xd3720,nativeScenes[i]);call(0xd3750,nativeScenes[i]);call(0xd3790,nativeScenes[i]);}
game.enable();call(0xd3bb0,nativeGame);scene->enable();call(0xd3bb0,nativeScene);sceneCheck();
for(unsigned mask=0;mask<32;++mask) {
    stage=448;trial=mask;
    unsigned index=0;for(auto pair:std::initializer_list<std::pair<View*,void*>>{{&scene->buildButton(),field<void*>(nativeScene,0x88)},{&scene->destroyButton(),field<void*>(nativeScene,0x94)},{&scene->upgradeButton(),field<void*>(nativeScene,0x98)},{&hud->farmButton(),field<void*>(original,0x44)},{&hud->marketButton(),field<void*>(original,0x40)}}) {
        if(mask&(1U<<index++)) {pair.first->enable();call(0xd3bb0,pair.second);}else {pair.first->disable();call(0xd3750,pair.second);}
    }
    scene->beginManipulation();call(0x957c0,nativeScene);sceneCheck();scene->endManipulation();call(0x95bd0,nativeScene);sceneCheck();
    for(auto pair:std::initializer_list<std::pair<Control*,unsigned>>{{&hud->mainButton(),0x3c},{&hud->marketButton(),0x40},{&hud->farmButton(),0x44},{&hud->wareButton(),0x48},{&hud->journalButton(),0x68}}) compareControl(*pair.first,field<void*>(original,pair.second));
}
// Restore the actual scene through enable/show/hide; these invoke real stock
// labels, hover logic and mouse bindings, including parent-state propagation.
game.enable();call(0xd3bb0,nativeGame);game.show();call(0xd37d0,nativeGame);hud->hide();call(0xd3720,original);
scene->show();call(0xd37d0,nativeScene);scene->enable();call(0xd3bb0,nativeScene);sceneCheck();
scene->disable();call(0xd3750,nativeScene);sceneCheck();scene->enable();call(0xd3bb0,nativeScene);sceneCheck();
scene->hide();call(0xd3720,nativeScene);sceneCheck();scene->show();call(0xd37d0,nativeScene);sceneCheck();
// The genuine BuyBuilding dispatch now creates genuine Manufacture-owned
// houses. No factory observation substitutes for this integration.
BuyBuildingContext scenePurchaseContext{manager,settings,strings,input,interaction,profile,configuration,audio,*scene,views,controls};
putPointer(0x260010,nullptr);auto scenePanel=std::make_unique<BuyBuilding>(nullptr,scenePurchaseContext);manufactureWorld.panel=scenePanel.get();world.overlays[0]=scenePanel.get();
auto* nativeScenePanel=allocate(1700);std::memset(nativeScenePanel,0,1700);function<void*(__thiscall*)(void*,void*)>(0x91690)(nativeScenePanel,nullptr);
const auto panelSceneCheck=[&] {compareControl(*scenePanel,nativeScenePanel);require(scenePanel->movement==field<unsigned>(nativeScenePanel,0x248),"Manufacture purchase panel movement differs");sceneCheck();};
for(int step:{5,-1,32}) {
    stage=449;trial=step;productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=step;
    scene->buildButton().clickEvent(&scene->buildButton());function<void(__cdecl*)(void*)>(0x93c60)(field<void*>(nativeScene,0x88));panelSceneCheck();
    if(step!=5) {scenePanel->cancel();call(0xd3bb0,original);call(0xd3fb0,original);call(0x8e130,nativeScenePanel);panelSceneCheck();}
}
for(unsigned type:{0U,10U}) {
    stage=450;trial=type;productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
    profile.ints["Money"]=nativeProfile.ints["Money"]=100000;
    scene->buildButton().clickEvent(&scene->buildButton());function<void(__cdecl*)(void*)>(0x93c60)(field<void*>(nativeScene,0x88));panelSceneCheck();
    auto& button=*scenePanel->cells()[type].button;require(button.building==type,"Manufacture panel availability fixture differs");
    scenePanel->choose(button);function<void(__cdecl*)(void*)>(0x8e980)(field<void*>(nativeScenePanel,0x5c8+type*20));panelSceneCheck();++manufacturePurchases;
    auto* house=scene->movingBuilding();auto* n=field<void*>(image,0x260150);require(house && !house->built && house->buildingType==type,"Manufacture purchase did not create moving house");
    bool placed=false;for(unsigned y=100;y<450 && !placed;y+=35) for(unsigned x=80;x<650 && !placed;x+=35) {
        const Point2 point{float(x),float(y)};house->positionProduction(point,false);function<void(__thiscall*)(void*,const Point2*,bool)>(0x9f3c0)(n,&point,false);cursorPosition=point;
        const bool valid=scene->validPlacement(*house);require(valid==function<bool(__thiscall*)(void*,void*)>(0x94a20)(nativeScene,n),"Manufacture purchase placement differs");
        if(!valid || scene->manipulationBlocked()) continue;
        house->clicked();call(0x9f8c0,n);sceneCheck();require(house->built && !scene->movingBuilding(),"Manufacture valid purchase did not build");placed=true;++manufactureBuilds;
    }require(placed,"Manufacture purchase has no valid placement in original data");
}
manufactureWorld.panel=nullptr;world.overlays[0]=&overlay;
const auto beforePanel=textureCounts();scenePanel.reset();const auto middlePanel=textureCounts();call(0x8edf0,nativeScenePanel);compareReferenceDeltas(beforePanel,middlePanel);putPointer(0x260010,nativeOverlay);
// The whole reconstructed scene enters through actual StateView movement and
// executes original/source input dispatch, hierarchy steps and D3D8 rendering.
hud->disable();hud->absoluteDisable();call(0xd3750,original);call(0xd3790,original);
game.absoluteEnable();call(0xd3fb0,nativeGame);scene->absoluteEnable();call(0xd3fb0,nativeScene);
menuMilliseconds=0;GameTimer sceneTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor sceneCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&sceneCursor;
std::array<unsigned char,24> sceneCursorFrame{};field<void*>(sceneCursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=sceneCursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
const auto manufactureFrame=[&](Point2 point=Point2{790,590},std::vector<unsigned> events={},std::array<int,3> delta={}) {
    stage=451;action=manufactureFrames;menuMilliseconds+=50;menuPoint=point;menuInputEvents=std::move(events);menuMouseDelta=delta;
    manufacturePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++manufactureFrames;sceneCheck();
};
const Point2 sceneStart{-800,0},sceneEnd{0,0};scene->move(2,sceneStart,sceneEnd);function<void(__thiscall*)(void*,unsigned,const Point2*,const Point2*)>(0xc5630)(nativeScene,2,&sceneStart,&sceneEnd);sceneCheck();
for(unsigned i=0;i<24;++i) manufactureFrame();
require(!scene->movement,"Manufacture entrance remains in movement");
for(unsigned slot:{11U,12U}) {
    auto* house=scene->slots()[slot];auto* n=field<void*>(nativeScene,0xa4+slot*4);require(house && house->built,"Manufacture input fixture house missing");
    const Point2 at{house->geometry().x,house->geometry().y};manufactureFrame(at,{}, {1,0,0});manufactureFrame(at);manufactureFrame(at,{0x208});manufactureFrame(at,{0x200});manufactureFrame({790,590},{},{1,0,0});
    require(scene->focusedBuilding()==house && field<void*>(nativeScene,0xf8)==n,"Manufacture actual input did not focus purchased house");
}
// Built-house drag crosses the exact squared-distance gate. A real right
// press restores the saved position through Manufacture's original binding.
{
    auto* house=scene->slots()[12];const Point2 saved{house->geometry().x,house->geometry().y};
    manufactureFrame(saved,{}, {1,0,0});manufactureFrame(saved,{0x208});manufactureFrame({saved[0]+6,saved[1]},{},{1,0,0});manufactureFrame({saved[0]+7,saved[1]},{},{1,0,0});
    require(scene->movingBuilding()==house,"Manufacture real drag did not enter moving state");manufactureFrame({saved[0]+7,saved[1]},{0x209});manufactureFrame(saved,{0x200});
    require(!scene->movingBuilding() && house->geometry().x==saved[0] && house->geometry().y==saved[1],"Manufacture right press did not restore house position");
}
// Outside press clears selection, including edge coordinates outside the
// native fixed 10/10/790/500 rectangle and tutorial-protected selections.
for(int step:{-1,33,34,38,48}) {
    productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=step;scene->focus(scene->slots()[11],false);function<void(__thiscall*)(void*,void*,bool)>(0x94e70)(nativeScene,field<void*>(nativeScene,0xd0),false);sceneCheck();
    for(Point2 at:{Point2{0,0},Point2{799,599},Point2{770,480}}) {manufactureFrame(at,{}, {1,0,0});manufactureFrame(at,{0x208});manufactureFrame(at,{0x200});}
}
productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
// Destroy protection and actual completed-house demolition, followed by the
// real effect animation callback and deferred teardown of both owned objects.
scene->focus(scene->slots()[11],false);function<void(__thiscall*)(void*,void*,bool)>(0x94e70)(nativeScene,field<void*>(nativeScene,0xd0),false);sceneCheck();
for(int step:{38,34,48}) {
    productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=step;
    scene->destroyButton().clickEvent(&scene->destroyButton());function<void(__cdecl*)(void*)>(0x99630)(field<void*>(nativeScene,0x94));sceneCheck();require(scene->slots()[11],"Manufacture protected tutorial destroyed house");
}
productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
auto* demolished=scene->slots()[11];auto* nativeDemolished=field<void*>(nativeScene,0xd0);
std::set<View*> sceneChildren=scene->children();std::vector<void*> nativeChildren;collectControls(field<Node*>(nativeScene,0x18)->parent,nativeChildren);
scene->destroyButton().clickEvent(&scene->destroyButton());function<void(__cdecl*)(void*)>(0x99630)(field<void*>(nativeScene,0x94));sceneCheck();deleteDeferredHouse(demolished,nativeDemolished);sceneCheck();
Picture* effect=nullptr;for(auto* child:scene->children()) if(!sceneChildren.count(child)) {require(!effect,"Manufacture created multiple demolition effects");effect=dynamic_cast<Picture*>(child);}
std::vector<void*> newNativeChildren;collectControls(field<Node*>(nativeScene,0x18)->parent,newNativeChildren);void* nativeEffect=nullptr;for(auto* child:newNativeChildren) if(std::find(nativeChildren.begin(),nativeChildren.end(),child)==nativeChildren.end()) {require(!nativeEffect,"Native Manufacture created multiple demolition effects");nativeEffect=child;}
require(effect && nativeEffect,"Manufacture demolition effect missing");compareControl(*effect,nativeEffect);
// Animation completion is exercised by the actual Picture step, with its
// original duration rather than a direct synthetic discard callback.
explicitTipElapsed=true;elapsed=10;effect->enabledStep();call(0xf1810,nativeEffect);explicitTipElapsed=false;
effect->updateTransform();call(0xf0b70,nativeEffect);
require(views.discarded().size()==1 && nativeDeferredStrings.size()==1 && views.discarded().at(effect).view()==nativeDeferredStrings.at(nativeEffect).view(),"Manufacture demolition completion origin differs");
const auto beforeEffect=textureCounts();views.deleteDiscarded();const auto middleEffect=textureCounts();call(0xf1560,nativeEffect);compareReferenceDeltas(beforeEffect,middleEffect);nativeDeferredStrings.clear();++manufactureEffects;
menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
// Increment-without-allocation on invalid requested slots is a real native
// quirk. It must remain distinct from the exact count==20 capacity guard.
for(int requested:{-2,20,21}) {
    stage=452;action=requested;require(!scene->create(0,requested) && !function<void*(__thiscall*)(void*,unsigned,int)>(0x94390)(nativeScene,0,requested),"Manufacture invalid slot allocated a house");sceneCheck();++manufactureInvalidSlots;
}
// Repeated XML resizing and comma-separated four-float attributes, then the
// missing-section case. Actual scene child controls remain owned throughout.
const auto manufactureXmlDirectory=common/"manufacture-reload";std::filesystem::create_directories(manufactureXmlDirectory);
for(unsigned rows:{0U,1U,3U,2U}) {
    std::ofstream xml(manufactureXmlDirectory/("reload-"+std::to_string(rows)+".xml"));xml<<"<Manufacture DepotArea=\"1.25,2.5,700.75,510.125\"><AntiDepotAreaList>";
    for(unsigned i=0;i<rows;++i) xml<<"<Area Rect=\""<<i<<",2,3,4\"/>";xml<<"</AntiDepotAreaList></Manufacture>";xml.close();
}
std::ofstream(manufactureXmlDirectory/"missing.xml")<<"<Manufacture/>";
files.mountCommon(String(manufactureXmlDirectory.generic_string()));
for(unsigned rows:{0U,1U,3U,2U}) {XmlDocument definition(files,String("reload-"+std::to_string(rows)+".xml"));scene->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0x979a0)(nativeScene,wrap(&definition));sceneCheck();++manufactureReloads;}
{XmlDocument definition(files,String("missing.xml"));scene->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0x979a0)(nativeScene,wrap(&definition));sceneCheck();++manufactureReloads;}
world.scenes[2]=previousSourceManufacture;
audio.tracePlay=false;soundTable[1]=reinterpret_cast<void*>(&nativeHoverSound);
stage=443;const auto beforeScene=textureCounts();scene.reset();const auto middleScene=textureCounts();call(0x96fa0,nativeScene);compareReferenceDeltas(beforeScene,middleScene);require(!Manufacture::instance() && !field<void*>(image,0x25f758),"Manufacture destructor keeps singleton");
putPointer(0x25f758,nativeScenes[1]);
