stage=580;hudUseActualDig=true;write(0x26f50,digInvestmentEntry.data(),5);redirect(0x4ec00,reinterpret_cast<void*>(&digNativeHeld));redirect(0x2b7a0,reinterpret_cast<void*>(&digNativeAnimation));redirect(0x50f90,reinterpret_cast<void*>(&digNativeRefreshHeld));
struct DigFixture final:DigBuildingWorld {
    WarehouseFixture& stock;Point2 actor{300,400};
    explicit DigFixture(WarehouseFixture& s):stock(s) {}
    TutorialManager& tutorial() override {return stock.tutorial();}StateView& farmScene() override {return stock.farmScene();}
    void registerArea(ActionArea& a) override {stock.registerArea(a);}void queueAreaAction(ActionArea& a,const Point2& p) override {stock.queueAreaAction(a,p);}
    int warehouseLevel() override {return stock.warehouseLevel();}int digBuildingLevel() override {return DigBuilding::instance() ? DigBuilding::instance()->level():0;}
    int mowBuildingLevel() override {return stock.mowBuildingLevel();}int waterSourceLevel() override {return stock.waterSourceLevel();}
    bool cursorOverAreaType(unsigned type,const ActionArea* excluded) override {return stock.cursorOverAreaType(type,excluded);}
    int heldType() override {return digHeldType;}int money() override {return stock.money();}void setMoney(int value) override {stock.setMoney(value);}
    Point2 characterPosition() override {return actor;}
    void animateHeldDigUpgrade(const Point2& from,const Point2& to,int level,bool flag) override {digSourceEffects.push_back({0,floatBits(from[0]),floatBits(from[1]),floatBits(to[0]),floatBits(to[1]),unsigned(level),unsigned(flag)});}
    void refreshHeldAppearance() override {digSourceEffects.push_back({1,0,0,0,0,0,0});}
} digWorld(warehouseWorld);
auto* oldNativeDig=field<void*>(image,0x25f74c);auto* oldNativePerson=field<void*>(image,0x25ff5c);
const bool previousCharacterPresent=hudInputs.character;hudInputs.character=true;
std::array<unsigned char,256> person{};field<float>(person.data(),0x5c)=digWorld.actor[0];field<float>(person.data(),0x60)=digWorld.actor[1];putPointer(0x25ff5c,person.data());
profile.ints["Upgrade34"]=nativeProfile.ints["Upgrade34"]=0;profile.uints["UpAvail14"]=nativeProfile.uints["UpAvail14"]=3;profile.ints["Money"]=nativeProfile.ints["Money"]=10000;
DigBuildingContext digContext{{manager,settings,strings,input,interaction,digWorld,views,controls},productionUpgrades,configuration,digWorld};
putPointer(0x25f74c,nullptr);auto dig=std::make_unique<DigBuilding>(nullptr,digContext);auto* nd=allocate(2220);std::memset(nd,0,2220);function<void*(__thiscall*)(void*,void*)>(0x29fe0)(nd,nullptr);
const auto checkDig=[&] {
    stage=581;require(DigBuilding::instance()==dig.get() && field<void*>(image,0x25f74c)==nd,"DigBuilding singleton differs");compareControl(*dig,nd);mainMenuCompareView(*dig,nd);
    require(dig->itemType==field<unsigned>(nd,0x50c) && dig->propertyId==field<unsigned>(nd,0x52c) && dig->levelCount==field<int>(nd,0x530),"DigBuilding Upgrade identity differs");
    require(profile.ints==nativeProfile.ints && profile.uints==nativeProfile.uints && profile.booleans==nativeProfile.booleans,"DigBuilding profile stores differ");
    compareControl(dig->upgradeButton(),field<void*>(nd,0x8a4));compareControl(dig->upgradePrice(),field<void*>(nd,0x8a8));
    auto& buttonText=dig->upgradeButton().text();if(buttonText.textDirty()) buttonText.makeLetters();compare(buttonText,field<void*>(field<void*>(nd,0x8a4),0x27c));
    require(unsigned(dig->buttonState())==field<unsigned>(nd,0x238) && dig->clickIndex==field<unsigned>(nd,0x4fc),"DigBuilding input state differs");
    const auto* maskBegin=field<NativeMaskColumn*>(nd,0x25c);const auto* maskEnd=field<NativeMaskColumn*>(nd,0x260);
    require(dig->clickMask().size()==std::size_t(maskEnd-maskBegin),"DigBuilding mask column count differs");
    for(unsigned i=0;i<dig->clickMask().size();++i) {const auto& col=dig->clickMask()[i];const auto& native=maskBegin[i];require(col.bitCount==native.count && col.words.size()==native.size && !std::memcmp(col.words.data(),native.words,col.words.size()*4),"DigBuilding mask bits differ");}
    if(dig->upgradePrice().textDirty()) dig->upgradePrice().makeLetters();compare(dig->upgradePrice(),field<void*>(nd,0x8a8));
    require(digSourceEffects==digNativeEffects,"DigBuilding held-tool effects differ");require(actualSounds==expectedSounds,"DigBuilding sound requests differ");
    for(unsigned i=0;i<4;++i) {const auto& a=dig->levels[i];auto* n=static_cast<unsigned char*>(nd)+0x534+i*220;compareFloat(a.digTime,field<float>(n,0),"DigBuilding DigTime");
        for(auto pair:std::initializer_list<std::pair<const AnimableAppearance*,unsigned>>{{&a.appearance,4},{&a.hovered,44},{&a.noInstrument,112},{&a.noInstrumentHovered,152}}) appearanceCheck(*pair.first,n+pair.second);
        require(a.maskName.view()==field<String>(n,84).view() && a.noInstrumentMask.view()==field<String>(n,192).view(),"DigBuilding level mask names differ");
    }
    require(warehouseSourceEvents==warehouseNativeEvents,"DigBuilding world effects differ");warehouseSourceEvents.clear();warehouseNativeEvents.clear();
    require(actionAreaSourceTasks==actionAreaNativeTasks,"DigBuilding Person queue differs");actionAreaSourceTasks.clear();actionAreaNativeTasks.clear();
    if(hudSourceCalls!=hudNativeCalls) {for(const auto& c:hudSourceCalls) std::cerr<<"source HUD "<<c[0]<<' '<<c[1]<<' '<<c[2]<<'\n';for(const auto& c:hudNativeCalls) std::cerr<<"native HUD "<<c[0]<<' '<<c[1]<<' '<<c[2]<<'\n';}
    require(hudSourceCalls==hudNativeCalls,"DigBuilding HUD observations differ");hudSourceCalls.clear();hudNativeCalls.clear();
    actualSounds.clear();expectedSounds.clear();digSourceEffects.clear();digNativeEffects.clear();++digBuildingStates;
};checkDig();
for(int level=0;level<4;++level) for(int held:{0,1,2,6,-1}) {
    stage=582;digHeldType=held;dig->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nd)+0x508,level,false);checkDig();
    require(dig->investment()==function<unsigned(__thiscall*)(void*)>(0x26f50)(nd),"DigBuilding investment differs");checkDig();
}
for(int level=0;level<3;++level) for(int held:{0,2}) {
    stage=583;digHeldType=held;profile.ints["Money"]=nativeProfile.ints["Money"]=10000;dig->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nd)+0x508,level,false);
    dig->upgradeButton().clickEvent(&dig->upgradeButton());function<void(__cdecl*)(void*)>(0x26260)(field<void*>(nd,0x8a4));++digBuildingActions;checkDig();
}
for(int level=0;level<3;++level) for(unsigned available:{0U,1U,2U,3U,0xffffffffU,0x80000000U}) for(int money:{-100,-1,0,40,1000,0x7fffffff}) {
    stage=585;profile.ints["Money"]=nativeProfile.ints["Money"]=money;profile.uints["UpAvail14"]=nativeProfile.uints["UpAvail14"]=available;
    dig->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nd)+0x508,level,false);checkDig();
    dig->upgradeButton().clickEvent(&dig->upgradeButton());function<void(__cdecl*)(void*)>(0x26260)(field<void*>(nd,0x8a4));++digBuildingActions;checkDig();
}
profile.ints["Money"]=nativeProfile.ints["Money"]=10000;profile.uints["UpAvail14"]=nativeProfile.uints["UpAvail14"]=3;
for(unsigned gates=0;gates<4;++gates) {
    stage=586;warehouseCursorGates={bool(gates&1),bool(gates&2)};productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=44;
    productionTutorial->cancelRequested=false;field<unsigned char>(nativeProductionTutorial,0xc08)=0;dig->clicked();call(0x27340,nd);++digBuildingActions;checkDig();
    require(productionTutorial->cancelRequested==bool(field<unsigned char>(nativeProductionTutorial,0xc08)),"DigBuilding tutorial cancel differs");
}
warehouseCursorGates={};productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
{
    const auto savedHover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
    menuMilliseconds=0;GameTimer digTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor digCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&digCursor;
    std::array<unsigned char,24> digCursorFrame{};field<void*>(digCursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=digCursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    dig->enable();dig->show();call(0xd3bb0,nd);call(0xd37d0,nd);
    for(int level=0;level<4;++level) for(int held:{0,2}) {
        digHeldType=held;dig->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nd)+0x508,level,false);checkDig();
        const Point2 at{dig->geometry().x,float(originalSubtract(dig->geometry().y,dig->heightOffset))};
        for(unsigned frame=0;frame<6;++frame) {
            stage=587;action=frame;menuMilliseconds+=50;menuPoint=frame==5 ? Point2{790,590}:at;menuMouseDelta=frame==0 || frame==5 ? std::array<int,3>{1,0,0}:std::array<int,3>{};
            menuInputEvents=frame==2 ? std::vector<unsigned>{0x208}:frame==3 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
            digBuildingPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++digBuildingFrames;checkDig();
        }
    }
    digHeldType=2;dig->setLevel(0,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nd)+0x508,0,false);checkDig();
    const Point2 at{dig->upgradeButton().geometry().x,dig->upgradeButton().geometry().y};
    for(unsigned frame=0;frame<4;++frame) {
        stage=588;action=frame;menuMilliseconds+=50;menuPoint=frame==3 ? Point2{790,590}:at;menuMouseDelta=frame==0 || frame==3 ? std::array<int,3>{1,0,0}:std::array<int,3>{};
        menuInputEvents=frame==1 ? std::vector<unsigned>{0x208}:frame==2 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
        digBuildingPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++digBuildingFrames;checkDig();
    }
    require(dig->level()==1,"DigBuilding real mouse did not buy upgrade");
    interaction.playHover=savedHover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
}
stage=589;
const auto farmParent=digWorld.farmScene().parent();const auto nativeFarmParent=field<void*>(nativeScenes[0],0x20);const bool farmShown=digWorld.farmScene().state().shown;
digWorld.farmScene().setParent(nullptr);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nullptr);
productionTutorial->tutorialEnabled=true;field<unsigned char>(nativeProductionTutorial,0xc09)=1;
for(unsigned flags=0;flags<8;++flags) {
    productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;profile.booleans["tip28"]=nativeProfile.booleans["tip28"]=false;
    digWorld.farmScene().movement=field<unsigned>(nativeScenes[0],0x4c)=flags&1;
    if(flags&2) {digWorld.farmScene().show();call(0xd37d0,nativeScenes[0]);}else {digWorld.farmScene().hide();call(0xd3720,nativeScenes[0]);}
    if(flags&4) {dig->upgradeButton().show();call(0xd37d0,field<void*>(nd,0x8a4));}else {dig->upgradeButton().hide();call(0xd3720,field<void*>(nd,0x8a4));}
    dig->enabledStep();call(0x28310,nd);checkDig();
    const int expected=flags==6 ? 28:-1;require(productionTutorial->currentStep==expected && field<int>(nativeProductionTutorial,0xbfc)==expected,"DigBuilding tutorial visibility/movement guards differ");
    if(expected==28) {
        auto& arrow=productionTutorial->arrow();auto* nativeArrow=field<void*>(nativeProductionTutorial,0xbec);mainMenuCompareView(arrow,nativeArrow);compareControl(arrow.image(),field<void*>(nativeArrow,0x40));compareControl(arrow.text(),field<void*>(nativeArrow,0x44));
        arrow.hide();call(0xd3720,nativeArrow);productionTutorial->finishTip(arrow);function<void(__thiscall*)(void*,void*)>(0xd0c30)(nativeProductionTutorial,nativeArrow);checkDig();
    }
}
productionTutorial->tutorialEnabled=false;field<unsigned char>(nativeProductionTutorial,0xc09)=0;digWorld.farmScene().movement=field<unsigned>(nativeScenes[0],0x4c)=0;
if(farmShown) {digWorld.farmScene().show();call(0xd37d0,nativeScenes[0]);}else {digWorld.farmScene().hide();call(0xd3720,nativeScenes[0]);}
digWorld.farmScene().setParent(farmParent);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nativeFarmParent);
const auto digXmlDirectory=common/"dig-reload";std::filesystem::create_directories(digXmlDirectory);
std::ofstream(digXmlDirectory/"partial-dig.xml")<<"<Dig><Upgrades><Up0 DigTime='3.75'/></Upgrades></Dig>";
std::ofstream(digXmlDirectory/"bare-dig.xml")<<"<Dig><Upgrades><Up0><NoInstrument/></Up0></Upgrades></Dig>";files.mountCommon(String(digXmlDirectory.generic_string()));
for(const char* path:{"Farm/ActionAreas/DigBuilding.xml","partial-dig.xml","bare-dig.xml","Farm/ActionAreas/DigBuilding.xml"}) {
    stage=590;XmlDocument xml(files,String(path));dig->loadXML(xml);function<void(__thiscall*)(void*,void*)>(0x265e0)(nd,wrap(&xml));checkDig();
    for(int held:{0,2}) {digHeldType=held;dig->setLevel(0,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nd)+0x508,0,false);checkDig();}
}
stage=584;const auto beforeDig=textureCounts();dig.reset();const auto middleDig=textureCounts();call(0x26470,nd);compareReferenceDeltas(beforeDig,middleDig);
hudInputs.character=previousCharacterPresent;hudUseActualDig=false;putPointer(0x25f74c,oldNativeDig);putPointer(0x25ff5c,oldNativePerson);digSourceEffects.clear();digNativeEffects.clear();warehouseSourceEvents.clear();warehouseNativeEvents.clear();actualSounds.clear();expectedSounds.clear();
