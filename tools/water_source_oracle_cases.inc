stage=600;hudUseActualWater=true;write(0x69400,waterInvestmentEntry.data(),5);redirect(0x50a30,reinterpret_cast<void*>(&waterNativePersonAppearance));redirect(0x32c20,reinterpret_cast<void*>(&waterNativeDiscardTask));redirect(0x33980,reinterpret_cast<void*>(&waterNativeCreateSprinkle));
redirect(0x2b7a0,reinterpret_cast<void*>(&digNativeAnimation));
struct WaterFixture final:WaterSourceWorld {
    WarehouseFixture& stock;bool cancel=false,hasTask=true;unsigned pending=0;Point2 actor{300,400};ButtonPrototype* created=nullptr;
    explicit WaterFixture(WarehouseFixture& s):stock(s) {}
    TutorialManager& tutorial() override {return stock.tutorial();}StateView& farmScene() override {return stock.farmScene();}
    void registerArea(ActionArea& a) override {stock.registerArea(a);}void queueAreaAction(ActionArea& a,const Point2& p) override {stock.queueAreaAction(a,p);}
    int warehouseLevel() override {return stock.warehouseLevel();}int digBuildingLevel() override {return stock.digBuildingLevel();}
    int mowBuildingLevel() override {return stock.mowBuildingLevel();}int waterSourceLevel() override {return WaterSource::instance()->level();}
    bool cursorOverAreaType(unsigned type,const ActionArea* excluded) override {return stock.cursorOverAreaType(type,excluded);}
    int heldType() override {return stock.profile.uints["PCarry"];}void setHeldType(int value) override {stock.profile.uints["PCarry"]=value;}
    void updatePersonAppearance() override {waterSourceEffects.push_back(1);}
    void cancelPersonAction(const String& origin) override {cancel=true;pending=0;if(hasTask) waterSourceOrigins.emplace_back(origin.view());}
    int money() override {return stock.money();}void setMoney(int value) override {stock.setMoney(value);}
    unsigned sprinkleCount() override {return sprinkleCountValue;}bool seedMoveActive() override {return seedMove;}bool sprinkleMoveActive() override {return sprinkleMove;}
    unsigned sprinkleCountValue=0;bool seedMove=false,sprinkleMove=false;
    std::function<ButtonPrototype*(unsigned)> actualSprinkleFactory;
    ButtonPrototype* createSprinkle(unsigned cost) override {if(actualSprinkleFactory) return actualSprinkleFactory(cost);waterSourceEffects.push_back(2);require(cost==WaterSource::instance()->sprinkleCost,"WaterSource Sprinkle cost differs");return created;}
    Point2 characterPosition() override {return actor;}
    void animateHeldUpgrade(const Point2& from,const Point2& to,int kind,bool flag) override {digSourceEffects.push_back({0,floatBits(from[0]),floatBits(from[1]),floatBits(to[0]),floatBits(to[1]),unsigned(kind),unsigned(flag)});}
} waterWorld(warehouseWorld);
auto* oldNativeWater=field<void*>(image,0x25f750);auto* oldWaterPerson=field<void*>(image,0x25ff5c);const bool oldWaterCharacter=hudInputs.character;hudInputs.character=true;
std::array<unsigned char,0x2850> waterPerson{};field<float>(waterPerson.data(),0x5c)=waterWorld.actor[0];field<float>(waterPerson.data(),0x60)=waterWorld.actor[1];field<unsigned>(waterPerson.data(),0x283c)=1;putPointer(0x25ff5c,waterPerson.data());
profile.ints["Upgrade33"]=nativeProfile.ints["Upgrade33"]=0;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;profile.uints["UpAvail13"]=nativeProfile.uints["UpAvail13"]=3;profile.ints["Money"]=nativeProfile.ints["Money"]=10000;
field<unsigned>(image,0x25ff98)=0;field<unsigned char>(image,0x25f720)=field<unsigned char>(image,0x25f721)=0;
WaterSourceContext waterContext{{manager,settings,strings,input,interaction,waterWorld,views,controls},productionUpgrades,configuration,waterWorld};
putPointer(0x25f750,nullptr);auto water=std::make_unique<WaterSource>(nullptr,waterContext);auto* ns=allocate(2360);std::memset(ns,0,2360);function<void*(__thiscall*)(void*,void*)>(0x6b330)(ns,nullptr);
const auto checkWater=[&] {
    stage=601;require(WaterSource::instance()==water.get() && field<void*>(image,0x25f750)==ns,"WaterSource singleton differs");compareControl(*water,ns);mainMenuCompareView(*water,ns);
    require(water->itemType==field<unsigned>(ns,0x50c) && water->propertyId==field<unsigned>(ns,0x52c) && water->levelCount==field<int>(ns,0x530),"WaterSource Upgrade identity differs");require(water->sprinkleCost==field<unsigned>(ns,0x534),"WaterSource Sprinkle cost differs");
    if(profile.ints!=nativeProfile.ints) {for(const auto& p:profile.ints) if(!nativeProfile.ints.count(p.first) || nativeProfile.ints[p.first]!=p.second) std::cerr<<"source int "<<p.first<<' '<<p.second<<'\n';for(const auto& p:nativeProfile.ints) if(!profile.ints.count(p.first) || profile.ints[p.first]!=p.second) std::cerr<<"native int "<<p.first<<' '<<p.second<<'\n';}
    require(profile.ints==nativeProfile.ints && profile.uints==nativeProfile.uints && profile.booleans==nativeProfile.booleans,"WaterSource profile stores differ");
    for(auto pair:std::initializer_list<std::pair<Control*,unsigned>>{{&water->upgradeButton(),0x8a8},{&water->sprinkleButton(),0x8b4},{&water->upgradePrice(),0x8ac},{&water->sprinklePrice(),0x8b0}}) {
        auto* n=field<void*>(ns,pair.second);compareControl(*pair.first,n);if(auto* text=dynamic_cast<TextLine*>(pair.first)) {if(text->textDirty()) text->makeLetters();compare(*text,n);}if(auto* button=dynamic_cast<TextAnimationButton*>(pair.first)) {auto& text=button->text();if(text.textDirty()) text.makeLetters();compare(text,field<void*>(n,0x27c));}
    }
    for(unsigned i=0;i<4;++i) {const auto& a=water->levels[i];auto* n=static_cast<unsigned char*>(ns)+0x538+i*220;if(i<3) compareFloat(a.capacity,field<float>(n,0),"WaterSource WaterCapacity");
        for(auto pair:std::initializer_list<std::pair<const AnimableAppearance*,unsigned>>{{&a.appearance,4},{&a.hovered,44},{&a.noInstrument,112},{&a.noInstrumentHovered,152}}) appearanceCheck(*pair.first,n+pair.second);
        require(a.maskName.view()==field<String>(n,84).view() && a.noInstrumentMask.view()==field<String>(n,192).view(),"WaterSource level masks differ");
    }
    for(unsigned i=0;i<3;++i) appearanceCheck(water->sprinkleStates[i],static_cast<unsigned char*>(ns)+0x8b8+i*40);
    const auto* maskBegin=field<NativeMaskColumn*>(ns,0x25c);const auto* maskEnd=field<NativeMaskColumn*>(ns,0x260);require(water->clickMask().size()==std::size_t(maskEnd-maskBegin),"WaterSource mask column count differs");
    for(unsigned i=0;i<water->clickMask().size();++i) {const auto& col=water->clickMask()[i];const auto& native=maskBegin[i];require(col.bitCount==native.count && col.words.size()==native.size && !std::memcmp(col.words.data(),native.words,col.words.size()*4),"WaterSource mask bits differ");}
    require(waterWorld.cancel==bool(field<unsigned char>(waterPerson.data(),0x284c)) && waterWorld.pending==field<unsigned>(waterPerson.data(),0x2838),"WaterSource Person cancellation fields differ");
    require(waterSourceEffects==waterNativeEffects && waterSourceOrigins==waterNativeOrigins && digSourceEffects==digNativeEffects,"WaterSource Person/Farm effects differ");waterSourceEffects.clear();waterNativeEffects.clear();waterSourceOrigins.clear();waterNativeOrigins.clear();digSourceEffects.clear();digNativeEffects.clear();
    require(warehouseSourceEvents==warehouseNativeEvents,"WaterSource Money/Farm gates differ");warehouseSourceEvents.clear();warehouseNativeEvents.clear();require(actionAreaSourceTasks==actionAreaNativeTasks,"WaterSource Person queue differs");actionAreaSourceTasks.clear();actionAreaNativeTasks.clear();
    require(actualSounds==expectedSounds,"WaterSource sound requests differ");actualSounds.clear();expectedSounds.clear();++waterSourceStates;
    require(hudSourceCalls==hudNativeCalls,"WaterSource HUD observations differ");hudSourceCalls.clear();hudNativeCalls.clear();
};checkWater();
for(int level=0;level<4;++level) for(int held:{0,1,2,6,-1}) {
    stage=602;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;water->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ns)+0x508,level,false);checkWater();
    compareFloat(float(water->capacity()),function<float(__thiscall*)(void*)>(0x683a0)(ns),"WaterSource capacity result");require(water->investment()==function<unsigned(__thiscall*)(void*)>(0x69400)(ns),"WaterSource investment differs");checkWater();
}
for(int level=0;level<3;++level) for(int held:{0,1,2}) {
    stage=604;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;profile.ints["Money"]=nativeProfile.ints["Money"]=10000;
    water->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ns)+0x508,level,false);checkWater();
    profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;water->upgradeButton().clickEvent(&water->upgradeButton());function<void(__cdecl*)(void*)>(0x68420)(field<void*>(ns,0x8a8));++waterSourceActions;checkWater();
}
for(int level=0;level<3;++level) for(unsigned available:{0U,1U,2U,3U,0xffffffffU,0x80000000U}) for(int money:{-100,-1,0,40,1000,0x7fffffff}) {
    stage=605;profile.ints["Money"]=nativeProfile.ints["Money"]=money;profile.uints["UpAvail13"]=nativeProfile.uints["UpAvail13"]=available;
    water->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ns)+0x508,level,false);checkWater();
    water->upgradeButton().clickEvent(&water->upgradeButton());function<void(__cdecl*)(void*)>(0x68420)(field<void*>(ns,0x8a8));++waterSourceActions;checkWater();
}
profile.ints["Money"]=nativeProfile.ints["Money"]=10000;profile.uints["UpAvail13"]=nativeProfile.uints["UpAvail13"]=3;
for(unsigned task:{0U,1U}) {
    waterWorld.hasTask=bool(task);field<unsigned>(waterPerson.data(),0x283c)=task;waterWorld.pending=field<unsigned>(waterPerson.data(),0x2838)=17;
    profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=1;water->setLevel(1,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ns)+0x508,1,false);checkWater();
}
profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;water->setLevel(2,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ns)+0x508,2,false);checkWater();
for(unsigned count:{0U,1U,2U,3U,4U,0xffffffffU}) for(unsigned move=0;move<4;++move) for(int money:{-1,0,49,50,10000,0x7fffffff}) {
    stage=606;waterWorld.sprinkleCountValue=field<unsigned>(image,0x25ff98)=count;waterWorld.sprinkleMove=bool(move&1);waterWorld.seedMove=bool(move&2);field<unsigned char>(image,0x25f720)=move&1;field<unsigned char>(image,0x25f721)=bool(move&2);
    profile.ints["Money"]=nativeProfile.ints["Money"]=money;water->refreshSprinkle();call(0x68f60,ns);checkWater();++waterSourceActions;
}
waterWorld.seedMove=waterWorld.sprinkleMove=false;field<unsigned char>(image,0x25f720)=field<unsigned char>(image,0x25f721)=0;waterWorld.sprinkleCountValue=field<unsigned>(image,0x25ff98)=0;
{
    stage=607;auto created=std::make_unique<ButtonPrototype>(nullptr,manager,input,interaction,views,controls);auto* nc=allocate(636);std::memset(nc,0,636);function<void*(__thiscall*)(void*,void*)>(0xf53b0)(nc,nullptr);
    for(bool success:{false,true}) {
        waterWorld.created=success ? created.get():nullptr;waterCreatedNative=success ? nc:nullptr;water->sprinkleButton().clickEvent(&water->sprinkleButton());function<void(__cdecl*)(void*)>(0x697f0)(field<void*>(ns,0x8b4));++waterSourceActions;checkWater();
        require(waterRequestedCost==water->sprinkleCost,"WaterSource native Sprinkle request cost differs");if(success) require(created->buttonState()==ButtonState::Pressed && field<unsigned>(nc,0x238)==3,"WaterSource newly created Sprinkle state differs");
    }
    created.reset();call(0xf5480,nc);waterWorld.created=nullptr;waterCreatedNative=nullptr;
}
for(unsigned gates=0;gates<4;++gates) {
    stage=608;warehouseCursorGates={bool(gates&1),bool(gates&2)};cursorPosition={790,590};water->clicked();call(0x6a010,ns);++waterSourceActions;checkWater();
    water->unclicked();call(0x684c0,ns);checkWater();require(water->tipsDirection==field<int>(ns,0x2b4),"WaterSource release tooltip direction differs");
}
warehouseCursorGates={};profile.ints["Money"]=nativeProfile.ints["Money"]=10000;
{
    const auto savedHover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
    menuMilliseconds=0;GameTimer waterTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor waterCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&waterCursor;
    std::array<unsigned char,24> waterCursorFrame{};field<void*>(waterCursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=waterCursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    water->enable();water->show();call(0xd3bb0,ns);call(0xd37d0,ns);
    for(int level=0;level<3;++level) for(int held:{0,2}) {
        profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;water->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ns)+0x508,level,false);checkWater();
        const Point2 at{water->geometry().x,float(originalSubtract(water->geometry().y,water->heightOffset))};
        for(unsigned frame=0;frame<6;++frame) {
            stage=609;action=frame;menuMilliseconds+=50;menuPoint=frame==5 ? Point2{790,590}:at;menuMouseDelta=frame==0 || frame==5 ? std::array<int,3>{1,0,0}:std::array<int,3>{};
            menuInputEvents=frame==2 ? std::vector<unsigned>{0x208}:frame==3 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
            waterSourcePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++waterSourceFrames;checkWater();
        }
    }
    water->setLevel(0,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ns)+0x508,0,false);checkWater();
    const Point2 at{water->upgradeButton().geometry().x,water->upgradeButton().geometry().y};
    for(unsigned frame=0;frame<4;++frame) {
        stage=610;action=frame;menuMilliseconds+=50;menuPoint=frame==3 ? Point2{790,590}:at;menuMouseDelta=frame==0 || frame==3 ? std::array<int,3>{1,0,0}:std::array<int,3>{};
        menuInputEvents=frame==1 ? std::vector<unsigned>{0x208}:frame==2 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
        waterSourcePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++waterSourceFrames;checkWater();
    }
    require(water->level()==1,"WaterSource real mouse did not buy upgrade");
    water->setLevel(2,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ns)+0x508,2,false);checkWater();
    for(unsigned count:{0U,1U,2U,3U,4U,0xffffffffU}) for(unsigned move=0;move<4;++move) {
        stage=613;waterWorld.sprinkleCountValue=field<unsigned>(image,0x25ff98)=count;waterWorld.sprinkleMove=bool(move&1);waterWorld.seedMove=bool(move&2);field<unsigned char>(image,0x25f720)=move&1;field<unsigned char>(image,0x25f721)=bool(move&2);
        water->refreshSprinkle();call(0x68f60,ns);menuMilliseconds+=50;menuPoint={790,590};menuMouseDelta={};menuInputEvents.clear();waterSourcePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++waterSourceFrames;checkWater();
    }
    waterWorld.seedMove=waterWorld.sprinkleMove=false;field<unsigned char>(image,0x25f720)=field<unsigned char>(image,0x25f721)=0;waterWorld.sprinkleCountValue=field<unsigned>(image,0x25ff98)=0;water->refreshSprinkle();call(0x68f60,ns);checkWater();
    interaction.playHover=savedHover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
}
stage=611;const auto waterFarmParent=waterWorld.farmScene().parent();const auto waterNativeFarmParent=field<void*>(nativeScenes[0],0x20);const bool waterFarmShown=waterWorld.farmScene().state().shown;
waterWorld.farmScene().setParent(nullptr);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nullptr);waterWorld.farmScene().show();call(0xd37d0,nativeScenes[0]);waterWorld.farmScene().movement=field<unsigned>(nativeScenes[0],0x4c)=0;
productionTutorial->tutorialEnabled=true;field<unsigned char>(nativeProductionTutorial,0xc09)=1;
for(unsigned step:{25U,46U,29U}) {
    profile.booleans["tip"+std::to_string(step)]=nativeProfile.booleans["tip"+std::to_string(step)]=false;productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
    const int level=step==25 ? 0:step==46 ? 1:2;water->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ns)+0x508,level,false);checkWater();
    if(step==29) {water->upgradeButton().hide();call(0xd3720,field<void*>(ns,0x8a8));}
    water->enabledStep();call(0x6a470,ns);checkWater();require(productionTutorial->currentStep==int(step) && field<int>(nativeProductionTutorial,0xbfc)==int(step),"WaterSource tutorial branch differs");
    auto& arrow=productionTutorial->arrow();auto* nativeArrow=field<void*>(nativeProductionTutorial,0xbec);mainMenuCompareView(arrow,nativeArrow);compareControl(arrow.image(),field<void*>(nativeArrow,0x40));compareControl(arrow.text(),field<void*>(nativeArrow,0x44));
    arrow.hide();call(0xd3720,nativeArrow);productionTutorial->finishTip(arrow);function<void(__thiscall*)(void*,void*)>(0xd0c30)(nativeProductionTutorial,nativeArrow);checkWater();
}
productionTutorial->tutorialEnabled=false;field<unsigned char>(nativeProductionTutorial,0xc09)=0;
if(waterFarmShown) {waterWorld.farmScene().show();call(0xd37d0,nativeScenes[0]);}else {waterWorld.farmScene().hide();call(0xd3720,nativeScenes[0]);}waterWorld.farmScene().setParent(waterFarmParent);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],waterNativeFarmParent);
const auto waterXmlDirectory=common/"water-reload";std::filesystem::create_directories(waterXmlDirectory);std::ofstream(waterXmlDirectory/"partial-water.xml")<<"<Water SprinkleCost='17'><Upgrades><Up0 WaterCapacity='7.25'/></Upgrades></Water>";files.mountCommon(String(waterXmlDirectory.generic_string()));
for(const char* path:{"Farm/ActionAreas/WaterSource.xml","partial-water.xml","Farm/ActionAreas/WaterSource.xml"}) {
    stage=612;XmlDocument xml(files,String(path));water->loadXML(xml);function<void(__thiscall*)(void*,void*)>(0x68750)(ns,wrap(&xml));checkWater();
    water->setLevel(0,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ns)+0x508,0,false);checkWater();
}
if(sprinkleMode) {
#include "sprinkle_oracle_cases.inc"
}
stage=603;const auto beforeWater=textureCounts();water.reset();const auto middleWater=textureCounts();call(0x68640,ns);compareReferenceDeltas(beforeWater,middleWater);
hudUseActualWater=false;putPointer(0x25f750,oldNativeWater);putPointer(0x25ff5c,oldWaterPerson);hudInputs.character=oldWaterCharacter;
