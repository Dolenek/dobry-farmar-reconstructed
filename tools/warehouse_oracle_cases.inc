stage=480;for(const auto& entry:warehouseEntries) write(entry.first,entry.second.data(),5);
for(const auto& entry:manufactureEntries) write(entry.first,entry.second.data(),5);
redirect(0x10ce0,reinterpret_cast<void*>(&manufactureMoney));redirect(0x10d60,reinterpret_cast<void*>(&warehouseSetMoney));
redirect(0x6f9e0,reinterpret_cast<void*>(&warehouseMarket));redirect(0x64b40,reinterpret_cast<void*>(&warehouseTip));
redirect(0x2d700,reinterpret_cast<void*>(&warehouseGate<0>));redirect(0x2db30,reinterpret_cast<void*>(&warehouseGate<1>));
redirect(0x4d2f0,reinterpret_cast<void*>(&actionAreaQueue));redirect(0x180aed,reinterpret_cast<void*>(&registerOracleExit));
audio.tracePlay=true;soundTable[1]=reinterpret_cast<void*>(&contractPlaySound);field<cResource*>(nativeGame,0xa08)=configuration.upgradeSound();
struct WarehouseFixture final:WarehouseWorld,ManufactureWorld {
    HudWorld& hudWorld;TutorialManager& tips;ProfileProperties& profile;
    Manufacture* scene=nullptr;BuyBuilding* panel=nullptr;std::vector<ActionArea*> registered;
    WarehouseFixture(HudWorld& h,TutorialManager& t,ProfileProperties& p):hudWorld(h),tips(t),profile(p) {}
    GameMenu& hud() override {return *hudWorld.menu;}TutorialManager& tutorial() override {return tips;}
    Manufacture& manufactureScene() override {require(scene,"Warehouse Manufacture scene not bound");return *scene;}
    StateView& farmScene() override {return *hudWorld.scenes[0];}StateView& tradeScene() override {return *hudWorld.scenes[1];}
    BuyBuilding& purchasePanel() override {require(panel,"Warehouse test did not open the purchase panel");return *panel;}
    void registerArea(ActionArea& area) override {registered.push_back(&area);}
    void queueAreaAction(ActionArea& area,const Point2& point) override {actionAreaSourceTasks.push_back(areaTask(point,area.clickIndex));}
    int warehouseLevel() override {return Warehouse::instance()->level();}
    int digBuildingLevel() override {return profile.ints["Upgrade33"];}
    int mowBuildingLevel() override {return profile.ints["Upgrade34"];}
    int waterSourceLevel() override {return profile.ints["Upgrade35"];}
    bool cursorOverAreaType(unsigned type,const ActionArea* excluded) override {
        require(!excluded && (type==64 || type==32),"Warehouse source Farm gate differs");warehouseSourceEvents.push_back(warehouseSourceSnapshot(3,type));return warehouseCursorGates[type==32];
    }
    void refreshMarketStocks() override {if(hudUseActualMarket) {Market::instance()->refresh();return;}warehouseSourceEvents.push_back(warehouseSourceSnapshot(1));}
    void refreshWarehouseTip() override {if(hudUseActualWarehouseTip) {WarehouseTip::instance()->refresh();return;}warehouseSourceEvents.push_back(warehouseSourceSnapshot(2));}
    unsigned warehouseGoods(unsigned product) override {return Warehouse::instance()->goods(product);}
    unsigned warehouseFree() override {return Warehouse::instance()->freeSpace();}
    void giveWarehouse(unsigned product,unsigned count,bool a,bool b) override {Warehouse::instance()->give(product,count,a,b);}
    void takeWarehouse(unsigned product,unsigned count,bool a,bool b) override {Warehouse::instance()->take(product,count,a,b);}
    int money() override {return profile.ints["Money"];}
    void setMoney(int value) override {warehouseSourceEvents.push_back(warehouseSourceSnapshot(4,unsigned(value)));profile.ints["Money"]=(std::max)(value,0);}
};
WarehouseFixture warehouseWorld(world,*productionTutorial,profile);warehouseSourceHud=hud;
ManufactureContext warehouseManufactureContext{manager,settings,strings,input,interaction,profile,configuration,audio,productionUpgrades,warehouseWorld,views,controls};
for(unsigned slot=0;slot<20;++slot) profile.ints["Upgrade"+std::to_string(slot)]=nativeProfile.ints["Upgrade"+std::to_string(slot)]=-1;
putPointer(0x25f758,nullptr);auto stockScene=std::make_unique<Manufacture>(&game,warehouseManufactureContext);warehouseWorld.scene=stockScene.get();
auto* nativeStockScene=allocate(256);std::memset(nativeStockScene,0,256);function<void*(__thiscall*)(void*,void*)>(0x99730)(nativeStockScene,nativeGame);
const auto priorSourceManufacture=world.scenes[2];world.scenes[2]=stockScene.get();
for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
WarehouseContext warehouseContext{{manager,settings,strings,input,interaction,warehouseWorld,views,controls},productionUpgrades,profile,configuration,warehouseWorld};
hudUseActualWarehouse=true;profile.ints["Upgrade32"]=nativeProfile.ints["Upgrade32"]=0;
for(unsigned i=0;i<19;++i) profile.uints["WGoodCount"+std::to_string(i)]=nativeProfile.uints["WGoodCount"+std::to_string(i)]=i%3;
cursorPosition={790,590};actualSounds.clear();expectedSounds.clear();hudSourceCalls.clear();hudNativeCalls.clear();
putPointer(0x25f748,nullptr);stage=481;auto warehouse=std::make_unique<Warehouse>(nullptr,warehouseContext);
auto* nw=allocate(1800);std::memset(nw,0,1800);function<void*(__thiscall*)(void*,void*)>(0x67fe0)(nw,nullptr);
profile.ints["Upgrade0"]=nativeProfile.ints["Upgrade0"]=0;profile.uints["ProdType0"]=nativeProfile.uints["ProdType0"]=0;
profile.uints["ProdCount0"]=nativeProfile.uints["ProdCount0"]=1;profile.uints["ProdStop0"]=nativeProfile.uints["ProdStop0"]=0;
profile.floats["ProdPct0"]=nativeProfile.floats["ProdPct0"]=0;
profile.floats["XProdPos0"]=nativeProfile.floats["XProdPos0"]=400;profile.floats["YProdPos0"]=nativeProfile.floats["YProdPos0"]=300;
stage=484;auto* stockBuilding=stockScene->create(0,0);auto* nativeStockBuilding=function<void*(__thiscall*)(void*,unsigned,int)>(0x94390)(nativeStockScene,0,0);
std::function<void()> warehouseTipCheck;std::unique_ptr<WarehouseTip> detail;void* nativeDetail=nullptr;
WarehouseTipContext detailContext{manager,settings,strings,configuration,views,controls};
if(warehouseTipMode) {
#include "warehouse_tip_oracle_setup.inc"
}
const auto warehouseCheck=[&] {
    stage=482;mainMenuCompareView(*warehouse,nw);compareControl(*warehouse,nw);compareControl(warehouse->overlay(),field<void*>(nw,0x500));
    require(unsigned(warehouse->buttonState())==field<unsigned>(nw,0x238) && warehouse->tipsDirection==field<int>(nw,0x2b4) && warehouse->clickIndex==field<unsigned>(nw,0x4fc),"Warehouse ActionArea state differs");
    require(productionTutorial->cancelRequested==bool(field<unsigned char>(nativeProductionTutorial,0xc08)),"Warehouse tutorial cancel differs");
    require(warehouse->occupied==field<unsigned>(nw,0x534),"Warehouse occupied differs");
    require(warehouse->itemType==field<unsigned>(nw,0x50c) && warehouse->propertyId==field<unsigned>(nw,0x52c) && warehouse->levelCount==field<int>(nw,0x530),"Warehouse Upgrade fields differ");
    for(unsigned i=0;i<4;++i) {const auto& level=warehouse->levels[i];auto* n=static_cast<unsigned char*>(nw)+0x538+i*112;
        require(level.capacity==field<unsigned>(n,0),"Warehouse capacity differs");appearanceCheck(level.appearance,n+4);appearanceCheck(level.hovered,n+44);require(level.maskName.view()==field<String>(n,84).view(),"Warehouse mask name differs");}
    for(auto pair:std::initializer_list<std::pair<Control*,unsigned>>{{&warehouse->loadingSubstrate(),0x6f8},{&warehouse->loadingProgress(),0x6fc},{&warehouse->upgradeButton(),0x700},{&warehouse->upgradePrice(),0x704}}) {
        action=pair.second;auto* n=field<void*>(nw,pair.second);compareControl(*pair.first,n);
        if(auto* line=dynamic_cast<TextLine*>(pair.first)) {if(line->textDirty()) line->makeLetters();compare(*line,n);}
        if(auto* button=dynamic_cast<TextAnimationButton*>(pair.first)) {auto& line=button->text();if(line.textDirty()) line.makeLetters();compare(line,field<void*>(n,0x27c));}
    }
    compareFloat(warehouse->loadingProgress().current,field<float>(field<void*>(nw,0x6fc),0x23c),"Warehouse loading percent");
    auto& stockLabel=stockBuilding->warehouseQuantity();if(stockLabel.textDirty()) stockLabel.makeLetters();compare(stockLabel,field<void*>(nativeStockBuilding,0x58c));
    auto& loadedLabel=stockBuilding->loadedQuantity();if(loadedLabel.textDirty()) loadedLabel.makeLetters();compare(loadedLabel,field<void*>(nativeStockBuilding,0x590));
    require(stockBuilding->productionCount()==function<unsigned(__thiscall*)(void*)>(0x9f010)(nativeStockBuilding),"Warehouse connected production count differs");
    require(stockScene->count==field<unsigned>(nativeStockScene,0xf4) && stockScene->count==1,"Warehouse actual Manufacture count differs");
    compareControl(hud->warehouseProgress(),field<void*>(original,0x8c));compareFloat(hud->warehouseProgress().current,field<float>(field<void*>(original,0x8c),0x23c),"Warehouse HUD percent");
    require(profile.ints==nativeProfile.ints && profile.uints==nativeProfile.uints && profile.floats==nativeProfile.floats && profile.booleans==nativeProfile.booleans,"Warehouse profile differs");
    require(warehouseSourceEvents==warehouseNativeEvents,"Warehouse remaining boundary effects differ");warehouseSourceEvents.clear();warehouseNativeEvents.clear();
    require(actionAreaSourceTasks==actionAreaNativeTasks,"Warehouse Person queue observations differ");actionAreaSourceTasks.clear();actionAreaNativeTasks.clear();
    require(hudSourceCalls==hudNativeCalls,"Warehouse actual HUD queries differ");hudSourceCalls.clear();hudNativeCalls.clear();
    require(actualSounds==expectedSounds,"Warehouse sound events differ");actualSounds.clear();expectedSounds.clear();++warehouseStates;if(warehouseTipCheck) warehouseTipCheck();
};warehouseCheck();++warehouseObjects;
for(unsigned product:{0U,1U,18U,19U,20U,0xffffffffU}) {
    require(warehouse->goods(product)==function<unsigned(__thiscall*)(void*,unsigned)>(0x67fa0)(nw,product),"Warehouse goods accessor differs");warehouseCheck();
}
require(warehouse->freeSpace()==function<unsigned(__thiscall*)(void*)>(0x65e70)(nw),"Warehouse free accessor differs");warehouseCheck();
for(int level=0;level<4;++level) {
    warehouse->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nw)+0x508,level,false);warehouseCheck();
    require(warehouse->investment()==function<unsigned(__thiscall*)(void*)>(0x66a40)(nw),"Warehouse investment differs");warehouseCheck();
    for(unsigned product:{0U,18U,19U,0xffffffffU}) for(unsigned amount:{0U,1U,50U,0xffffffffU}) for(unsigned flags=0;flags<4;++flags) {
        trial=warehouseTransfers;const bool market=bool(flags&1),preview=bool(flags&2);
        const auto given=warehouse->give(product,amount,market,preview);
        const auto expectedGiven=function<unsigned(__thiscall*)(void*,unsigned,unsigned,bool,bool)>(0x67ec0)(nw,product,amount,market,preview);
        require(given==expectedGiven,"Warehouse Give result differs");++warehouseTransfers;warehouseCheck();
        const auto taken=warehouse->take(product,amount,market,preview);
        const auto expectedTaken=function<unsigned(__thiscall*)(void*,unsigned,unsigned,bool,bool)>(0x682d0)(nw,product,amount,market,preview);
        require(taken==expectedTaken,"Warehouse Take result differs");++warehouseTransfers;warehouseCheck();
    }
}
// Actual Warehouse -> Manufacture -> ProductionBuilding -> Warehouse flow.
// Neither controller's stock services are replaced by observer arithmetic.
stage=485;for(unsigned product=0;product<19;++product) profile.uints["WGoodCount"+std::to_string(product)]=nativeProfile.uints["WGoodCount"+std::to_string(product)]=0;
warehouse->occupied=field<unsigned>(nw,0x534)=0;warehouse->refresh();call(0x66530,static_cast<unsigned char*>(nw)+0x508);
stockBuilding->setProductionCount(0);function<void(__thiscall*)(void*,unsigned)>(0x9fdf0)(nativeStockBuilding,0);warehouseCheck();
const auto stockInput=productionKind(0).inputProduct(),stockOutput=productionKind(0).outputProduct();
require(warehouse->give(stockInput,4,true,false)==function<unsigned(__thiscall*)(void*,unsigned,unsigned,bool,bool)>(0x67ec0)(nw,stockInput,4,true,false),"Connected stock supply differs");warehouseCheck();
stockBuilding->addDepotButton().clickEvent(&stockBuilding->addDepotButton());function<void(__cdecl*)(void*)>(0xa0230)(field<void*>(nativeStockBuilding,0x5a0));warehouseCheck();
require(stockBuilding->productionCount()==1 && warehouse->goods(stockInput)==3,"Connected production did not consume actual Warehouse input");
stockBuilding->addStockButton().clickEvent(&stockBuilding->addStockButton());function<void(__cdecl*)(void*)>(0xa02c0)(field<void*>(nativeStockBuilding,0x59c));warehouseCheck();
require(stockBuilding->productionCount()==0 && warehouse->goods(stockInput)==4,"Connected production did not return actual Warehouse input");
stockBuilding->addDepotButton().clickEvent(&stockBuilding->addDepotButton());function<void(__cdecl*)(void*)>(0xa0230)(field<void*>(nativeStockBuilding,0x5a0));warehouseCheck();
stockBuilding->setProductionStopped(false);function<void(__thiscall*)(void*,bool)>(0x9f210)(nativeStockBuilding,false);
stockBuilding->setProductionProgress(99);function<void(__thiscall*)(void*,float)>(0x9f300)(nativeStockBuilding,99);
elapsed=100;stockBuilding->absoluteStep();call(0xa1c90,nativeStockBuilding);warehouseCheck();
require(stockBuilding->productionCount()==0 && warehouse->goods(stockOutput)==1 && warehouse->occupied==4,"Actual production completion did not reach Warehouse output");
for(unsigned gates=0;gates<4;++gates) {
    warehouseCursorGates={bool(gates&1),bool(gates&2)};
    productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=44;
    productionTutorial->cancelRequested=false;field<unsigned char>(nativeProductionTutorial,0xc08)=0;
    warehouse->clicked();call(0x66e30,nw);warehouseCheck();
}
productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;warehouseCursorGates={};
stage=487;
for(int level=0;level<3;++level) for(unsigned available:{0U,1U,2U,3U,0xffffffffU,0x80000000U}) for(int money:{-100,-1,0,40,1000,0x7fffffff}) {
    profile.ints["Money"]=nativeProfile.ints["Money"]=money;profile.uints["UpAvail12"]=nativeProfile.uints["UpAvail12"]=available;
    warehouse->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nw)+0x508,level,false);warehouseCheck();
    warehouse->upgradeButton().clickEvent(&warehouse->upgradeButton());function<void(__cdecl*)(void*)>(0x65ee0)(field<void*>(nw,0x700));warehouseCheck();
}
profile.ints["Money"]=nativeProfile.ints["Money"]=1000;profile.uints["UpAvail12"]=nativeProfile.uints["UpAvail12"]=3;
{
    menuMilliseconds=0;GameTimer warehouseTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor warehouseCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&warehouseCursor;
    std::array<unsigned char,24> warehouseCursorFrame{};field<void*>(warehouseCursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=warehouseCursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    warehouse->enable();warehouse->show();call(0xd3bb0,nw);call(0xd37d0,nw);
    for(int level=0;level<4;++level) {
        warehouse->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nw)+0x508,level,false);warehouseCheck();
        const Point2 at{warehouse->geometry().x,float(originalSubtract(warehouse->geometry().y,warehouse->heightOffset))};
        for(unsigned frame=0;frame<6;++frame) {
            stage=486;action=frame;menuMilliseconds+=50;menuPoint=frame==5 ? Point2{790,590}:at;
            menuMouseDelta=frame==0 || frame==5 ? std::array<int,3>{1,0,0}:std::array<int,3>{};
            menuInputEvents=frame==2 ? std::vector<unsigned>{0x208}:frame==3 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
            warehousePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++warehouseFrames;warehouseCheck();
        }
    }
    menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
}
if(warehouseTipMode) {
#include "warehouse_tip_oracle_cases.inc"
}
if(digBuildingMode) {
#include "dig_building_oracle_cases.inc"
}
if(marketMode) {
#include "market_oracle_cases.inc"
}
stage=483;const auto beforeWarehouse=textureCounts();warehouse.reset();const auto middleWarehouse=textureCounts();call(0x66020,nw);compareReferenceDeltas(beforeWarehouse,middleWarehouse);
for(int savedLevel=0;savedLevel<4;++savedLevel) for(unsigned counts=0;counts<2;++counts) {
    stage=488;profile.ints["Upgrade32"]=nativeProfile.ints["Upgrade32"]=savedLevel;
    for(unsigned product=0;product<19;++product) profile.uints["WGoodCount"+std::to_string(product)]=nativeProfile.uints["WGoodCount"+std::to_string(product)]=counts ? product%3:0;
    warehouse=std::make_unique<Warehouse>(nullptr,warehouseContext);nw=allocate(1800);std::memset(nw,0,1800);function<void*(__thiscall*)(void*,void*)>(0x67fe0)(nw,nullptr);
    warehouseCheck();++warehouseObjects;
    require(warehouse->freeSpace()==function<unsigned(__thiscall*)(void*)>(0x65e70)(nw),"Saved Warehouse free differs");warehouseCheck();
    const auto before=textureCounts();warehouse.reset();const auto middle=textureCounts();call(0x66020,nw);compareReferenceDeltas(before,middle);
    require(!Warehouse::instance() && !field<void*>(image,0x25f748),"Warehouse destructor retains singleton");
}
const auto warehouseXmlDirectory=common/"warehouse-reload";std::filesystem::create_directories(warehouseXmlDirectory);
std::ofstream(warehouseXmlDirectory/"missing.xml")<<"<Warehouse/>";
std::ofstream(warehouseXmlDirectory/"partial.xml")<<"<Warehouse><Upgrades><Up0 ProductionNum=\"17\"/></Upgrades></Warehouse>";
std::ofstream(warehouseXmlDirectory/"level.xml")<<"<Level ProductionNum=\"77\" Picture=\"farm/ActionAreas/Warehouse/house3.jng\"/>";
files.mountCommon(String(warehouseXmlDirectory.generic_string()));
stage=489;warehouse=std::make_unique<Warehouse>(nullptr,warehouseContext);nw=allocate(1800);std::memset(nw,0,1800);function<void*(__thiscall*)(void*,void*)>(0x67fe0)(nw,nullptr);warehouseCheck();++warehouseObjects;
{
    XmlDocument level(files,String("level.xml"));warehouse->loadLevel(1,level);function<void(__thiscall*)(void*,unsigned,void*)>(0x67910)(static_cast<unsigned char*>(nw)+0x508,1,wrap(&level));warehouseCheck();
}
for(const char* path:{"missing.xml","partial.xml","Farm/ActionAreas/Warehouse.xml","missing.xml"}) {
    XmlDocument definition(files,String(path));warehouse->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0x67220)(nw,wrap(&definition));warehouseCheck();
}
const auto beforeReloadWarehouse=textureCounts();warehouse.reset();const auto middleReloadWarehouse=textureCounts();call(0x66020,nw);compareReferenceDeltas(beforeReloadWarehouse,middleReloadWarehouse);
if(detail) {hudUseActualWarehouseTip=false;const auto before=textureCounts();detail.reset();const auto middle=textureCounts();call(0x65a10,nativeDetail);compareReferenceDeltas(before,middle);putPointer(0x25ffc4,nativeTip);}
hudUseActualWarehouse=false;world.scenes[2]=priorSourceManufacture;warehouseSourceHud=nullptr;
const auto beforeStockScene=textureCounts();stockScene.reset();const auto middleStockScene=textureCounts();call(0x96fa0,nativeStockScene);compareReferenceDeltas(beforeStockScene,middleStockScene);
