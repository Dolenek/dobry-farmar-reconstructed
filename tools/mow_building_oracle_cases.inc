stage=620;hudUseActualMow=true;write(0x47b40,mowInvestmentEntry.data(),5);redirect(0x4ee70,reinterpret_cast<void*>(&mowNativeHeldIndicator));redirect(0x2b7a0,reinterpret_cast<void*>(&digNativeAnimation));redirect(0x50f90,reinterpret_cast<void*>(&digNativeRefreshHeld));
View heldMowIndicator(nullptr,views);auto* nativeHeldMowIndicator=allocate(44);std::memset(nativeHeldMowIndicator,0,44);function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeHeldMowIndicator,nullptr);
struct MowFixture final:MowBuildingWorld {
    WarehouseFixture& stock;View& indicator;Point2 actor{300,400};bool person=true;
    MowFixture(WarehouseFixture& s,View& v):stock(s),indicator(v) {}
    TutorialManager& tutorial() override {return stock.tutorial();}StateView& farmScene() override {return stock.farmScene();}
    void registerArea(ActionArea& a) override {stock.registerArea(a);}void queueAreaAction(ActionArea& a,const Point2& p) override {stock.queueAreaAction(a,p);}
    int warehouseLevel() override {return stock.warehouseLevel();}int digBuildingLevel() override {return stock.digBuildingLevel();}int mowBuildingLevel() override {return MowBuilding::instance()->level();}int waterSourceLevel() override {return stock.waterSourceLevel();}
    bool cursorOverAreaType(unsigned type,const ActionArea* excluded) override {return stock.cursorOverAreaType(type,excluded);}
    int heldType() override {return int(stock.profile.uints["PCarry"]);}bool personPresent() override {return person;}
    unsigned heldProductCount() override {unsigned sum=0;for(unsigned i=0;i<8;++i) sum+=stock.profile.uints["PCC"+std::to_string(i)];return sum;}
    void hidePersonHeldIndicator() override {indicator.hide();}
    void updatePersonHeldIndicator(bool flag,unsigned count,unsigned capacity) override {mowSourceEffects.push_back({unsigned(flag),count,capacity});}
    int money() override {return stock.money();}void setMoney(int value) override {stock.setMoney(value);}
    Point2 characterPosition() override {return actor;}
    void animateHeldUpgrade(const Point2& from,const Point2& to,int kind,bool flag) override {digSourceEffects.push_back({0,floatBits(from[0]),floatBits(from[1]),floatBits(to[0]),floatBits(to[1]),unsigned(kind),unsigned(flag)});}
    void refreshHeldAppearance() override {digSourceEffects.push_back({1,0,0,0,0,0,0});}
} mowWorld(warehouseWorld,heldMowIndicator);
auto* oldNativeMow=field<void*>(image,0x25f754);auto* oldMowPerson=field<void*>(image,0x25ff5c);const bool oldMowCharacter=hudInputs.character;hudInputs.character=true;
std::array<unsigned char,0x2b30> mowPerson{};field<float>(mowPerson.data(),0x5c)=mowWorld.actor[0];field<float>(mowPerson.data(),0x60)=mowWorld.actor[1];field<void*>(mowPerson.data(),0x2850)=nativeHeldMowIndicator;putPointer(0x25ff5c,mowPerson.data());
profile.ints["Upgrade31"]=nativeProfile.ints["Upgrade31"]=0;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;profile.uints["UpAvail11"]=nativeProfile.uints["UpAvail11"]=3;profile.ints["Money"]=nativeProfile.ints["Money"]=10000;
for(unsigned i=0;i<8;++i) profile.uints["PCC"+std::to_string(i)]=nativeProfile.uints["PCC"+std::to_string(i)]=0;
for(unsigned i=0;i<19;++i) {const auto& a=configuration.products[i].icon;auto* out=static_cast<unsigned char*>(nativeGame)+0x30+i*100+16;field<cTexture*>(out,4)=a.texture;field<unsigned>(out,8)=a.frames;field<float>(out,12)=a.length;field<unsigned char>(out,16)=a.loop;field<unsigned>(out,20)=a.columns;field<unsigned>(out,24)=a.rows;}
MowBuildingContext mowContext{{manager,settings,strings,input,interaction,mowWorld,views,controls},productionUpgrades,configuration,profile,mowWorld};
putPointer(0x25f754,nullptr);auto mow=std::make_unique<MowBuilding>(nullptr,mowContext);auto* nm=allocate(2244);std::memset(nm,0,2244);function<void*(__thiscall*)(void*,void*)>(0x49560)(nm,nullptr);
const auto checkMow=[&] {
    stage=621;require(MowBuilding::instance()==mow.get() && field<void*>(image,0x25f754)==nm,"MowBuilding singleton differs");compareControl(*mow,nm);mainMenuCompareView(*mow,nm);
    require(mow->itemType==field<unsigned>(nm,0x50c) && mow->propertyId==field<unsigned>(nm,0x52c) && mow->levelCount==field<int>(nm,0x530),"MowBuilding Upgrade identity differs");
    require(profile.ints==nativeProfile.ints && profile.uints==nativeProfile.uints && profile.booleans==nativeProfile.booleans,"MowBuilding profile stores differ");
    for(auto pair:std::initializer_list<std::pair<Control*,unsigned>>{{&mow->upgradeButton(),0x8b4},{&mow->upgradePrice(),0x8b8},{&mow->heldProductSign(),0x8bc},{&mow->heldProductSubstrate(),0x8c0}}) {
        auto* n=field<void*>(nm,pair.second);compareControl(*pair.first,n);if(auto* text=dynamic_cast<TextLine*>(pair.first)) {if(text->textDirty()) text->makeLetters();compare(*text,n);}if(auto* button=dynamic_cast<TextAnimationButton*>(pair.first)) {auto& text=button->text();if(text.textDirty()) text.makeLetters();compare(text,field<void*>(n,0x27c));}
    }
    for(unsigned i=0;i<4;++i) {const auto& a=mow->levels[i];auto* n=static_cast<unsigned char*>(nm)+0x534+i*224;require(a.productionNum==field<unsigned>(n,0),"MowBuilding ProductionNum differs");compareFloat(a.mowTime,field<float>(n,4),"MowBuilding MowTime");
        for(auto pair:std::initializer_list<std::pair<const AnimableAppearance*,unsigned>>{{&a.appearance,8},{&a.hovered,48},{&a.noInstrument,116},{&a.noInstrumentHovered,156}}) appearanceCheck(*pair.first,n+pair.second);
        require(a.maskName.view()==field<String>(n,88).view() && a.noInstrumentMask.view()==field<String>(n,196).view(),"MowBuilding level masks differ");
    }
    mainMenuCompareView(heldMowIndicator,nativeHeldMowIndicator);require(mowSourceEffects==mowNativeEffects && digSourceEffects==digNativeEffects,"MowBuilding Person effects differ");mowSourceEffects.clear();mowNativeEffects.clear();digSourceEffects.clear();digNativeEffects.clear();
    const auto* maskBegin=field<NativeMaskColumn*>(nm,0x25c);const auto* maskEnd=field<NativeMaskColumn*>(nm,0x260);require(mow->clickMask().size()==std::size_t(maskEnd-maskBegin),"MowBuilding mask column count differs");
    for(unsigned i=0;i<mow->clickMask().size();++i) {const auto& col=mow->clickMask()[i];const auto& native=maskBegin[i];require(col.bitCount==native.count && col.words.size()==native.size && !std::memcmp(col.words.data(),native.words,col.words.size()*4),"MowBuilding mask bits differ");}
    require(warehouseSourceEvents==warehouseNativeEvents,"MowBuilding Money/Farm gates differ");warehouseSourceEvents.clear();warehouseNativeEvents.clear();require(actionAreaSourceTasks==actionAreaNativeTasks,"MowBuilding Person queue differs");actionAreaSourceTasks.clear();actionAreaNativeTasks.clear();
    require(actualSounds==expectedSounds,"MowBuilding sound requests differ");actualSounds.clear();expectedSounds.clear();++mowBuildingStates;
    require(hudSourceCalls==hudNativeCalls,"MowBuilding HUD observations differ");hudSourceCalls.clear();hudNativeCalls.clear();
};checkMow();
for(int level=0;level<4;++level) for(int held:{0,1,2,6,-1}) {
    stage=622;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;mow->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nm)+0x508,level,false);checkMow();
    require(mow->investment()==function<unsigned(__thiscall*)(void*)>(0x47b40)(nm),"MowBuilding investment differs");checkMow();
}
for(int level=0;level<3;++level) for(int held:{0,6}) {
    stage=623;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;profile.ints["Money"]=nativeProfile.ints["Money"]=10000;mow->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nm)+0x508,level,false);checkMow();
    mow->upgradeButton().clickEvent(&mow->upgradeButton());function<void(__cdecl*)(void*)>(0x471a0)(field<void*>(nm,0x8b4));++mowBuildingActions;checkMow();
}
std::mt19937 mowRandom(0x448b60);
for(int level=0;level<4;++level) for(int held:{0,6}) for(unsigned pattern=0;pattern<64;++pattern) {
    stage=625;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;
    for(unsigned i=0;i<8;++i) {unsigned amount=pattern==0 ? 0:pattern==1 ? 1:pattern==2 ? (i==0 ? 0xffffffffU:i==1 ? 1:0):mowRandom()%3 ? mowRandom():0;profile.uints["PCC"+std::to_string(i)]=nativeProfile.uints["PCC"+std::to_string(i)]=amount;}
    mow->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nm)+0x508,level,false);checkMow();
}
for(unsigned i=0;i<8;++i) profile.uints["PCC"+std::to_string(i)]=nativeProfile.uints["PCC"+std::to_string(i)]=0;
profile.uints["PCC7"]=nativeProfile.uints["PCC7"]=9;
for(bool present:{false,true}) {
    mowWorld.person=hudInputs.character=present;putPointer(0x25ff5c,present ? mowPerson.data():nullptr);
    for(int held:{0,6}) {profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;mow->refresh();call(0x49050,static_cast<unsigned char*>(nm)+0x508);checkMow();}
}
for(int level=0;level<3;++level) for(unsigned available:{0U,1U,2U,3U,0xffffffffU,0x80000000U}) for(int money:{-100,-1,0,40,1000,0x7fffffff}) {
    stage=626;profile.ints["Money"]=nativeProfile.ints["Money"]=money;profile.uints["UpAvail11"]=nativeProfile.uints["UpAvail11"]=available;
    mow->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nm)+0x508,level,false);checkMow();
    mow->upgradeButton().clickEvent(&mow->upgradeButton());function<void(__cdecl*)(void*)>(0x471a0)(field<void*>(nm,0x8b4));++mowBuildingActions;checkMow();
}
profile.ints["Money"]=nativeProfile.ints["Money"]=10000;profile.uints["UpAvail11"]=nativeProfile.uints["UpAvail11"]=3;
for(unsigned gates=0;gates<4;++gates) {stage=627;warehouseCursorGates={bool(gates&1),bool(gates&2)};mow->clicked();call(0x47f30,nm);++mowBuildingActions;checkMow();}warehouseCursorGates={};
{
    const auto savedHover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
    menuMilliseconds=0;GameTimer mowTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor mowCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&mowCursor;
    std::array<unsigned char,24> mowCursorFrame{};field<void*>(mowCursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=mowCursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    mow->enable();mow->show();call(0xd3bb0,nm);call(0xd37d0,nm);
    for(int level=0;level<4;++level) for(int held:{0,6}) {
        profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;mow->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nm)+0x508,level,false);checkMow();
        const Point2 at{mow->geometry().x,float(originalSubtract(mow->geometry().y,mow->heightOffset))};
        for(unsigned frame=0;frame<6;++frame) {stage=628;action=frame;menuMilliseconds+=50;menuPoint=frame==5 ? Point2{790,590}:at;menuMouseDelta=frame==0 || frame==5 ? std::array<int,3>{1,0,0}:std::array<int,3>{};menuInputEvents=frame==2 ? std::vector<unsigned>{0x208}:frame==3 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
            mowBuildingPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++mowBuildingFrames;checkMow();}
    }
    mow->setLevel(0,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nm)+0x508,0,false);checkMow();
    const Point2 at{mow->upgradeButton().geometry().x,mow->upgradeButton().geometry().y};
    for(unsigned frame=0;frame<4;++frame) {stage=629;action=frame;menuMilliseconds+=50;menuPoint=frame==3 ? Point2{790,590}:at;menuMouseDelta=frame==0 || frame==3 ? std::array<int,3>{1,0,0}:std::array<int,3>{};menuInputEvents=frame==1 ? std::vector<unsigned>{0x208}:frame==2 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
        mowBuildingPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++mowBuildingFrames;checkMow();}
    require(mow->level()==1,"MowBuilding real mouse did not buy upgrade");
    profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;mow->setLevel(0,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nm)+0x508,0,false);checkMow();
    for(unsigned product=0;product<8;++product) {
        for(unsigned i=0;i<8;++i) profile.uints["PCC"+std::to_string(i)]=nativeProfile.uints["PCC"+std::to_string(i)]=i==product ? product+1:0;
        mow->refresh();call(0x49050,static_cast<unsigned char*>(nm)+0x508);menuMilliseconds+=50;menuPoint={790,590};menuMouseDelta={};menuInputEvents.clear();stage=630;
        mowBuildingPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++mowBuildingFrames;checkMow();
    }
    interaction.playHover=savedHover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
}
stage=631;const auto mowFarmParent=mowWorld.farmScene().parent();const auto mowNativeFarmParent=field<void*>(nativeScenes[0],0x20);const bool mowFarmShown=mowWorld.farmScene().state().shown;
mowWorld.farmScene().setParent(nullptr);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nullptr);mowWorld.farmScene().show();call(0xd37d0,nativeScenes[0]);mowWorld.farmScene().movement=field<unsigned>(nativeScenes[0],0x4c)=0;
productionTutorial->tutorialEnabled=true;field<unsigned char>(nativeProductionTutorial,0xc09)=1;
for(unsigned step:{27U,47U}) {
    profile.booleans["tip"+std::to_string(step)]=nativeProfile.booleans["tip"+std::to_string(step)]=false;productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
    const int level=step==27 ? 0:2;mow->setLevel(level,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nm)+0x508,level,false);checkMow();
    mow->enabledStep();call(0x48330,nm);checkMow();require(productionTutorial->currentStep==int(step) && field<int>(nativeProductionTutorial,0xbfc)==int(step),"MowBuilding tutorial branch differs");
    auto& arrow=productionTutorial->arrow();auto* nativeArrow=field<void*>(nativeProductionTutorial,0xbec);mainMenuCompareView(arrow,nativeArrow);compareControl(arrow.image(),field<void*>(nativeArrow,0x40));compareControl(arrow.text(),field<void*>(nativeArrow,0x44));
    arrow.hide();call(0xd3720,nativeArrow);productionTutorial->finishTip(arrow);function<void(__thiscall*)(void*,void*)>(0xd0c30)(nativeProductionTutorial,nativeArrow);checkMow();
}
productionTutorial->tutorialEnabled=false;field<unsigned char>(nativeProductionTutorial,0xc09)=0;
if(mowFarmShown) {mowWorld.farmScene().show();call(0xd37d0,nativeScenes[0]);}else {mowWorld.farmScene().hide();call(0xd3720,nativeScenes[0]);}mowWorld.farmScene().setParent(mowFarmParent);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],mowNativeFarmParent);
const auto mowXmlDirectory=common/"mow-reload";std::filesystem::create_directories(mowXmlDirectory);std::ofstream(mowXmlDirectory/"partial-mow.xml")<<"<Mow><Upgrades><Up0 ProductionNum='7' MowTime='0.75'/></Upgrades></Mow>";files.mountCommon(String(mowXmlDirectory.generic_string()));
for(const char* path:{"Farm/ActionAreas/MowBuilding.xml","partial-mow.xml","Farm/ActionAreas/MowBuilding.xml"}) {
    stage=632;XmlDocument xml(files,String(path));mow->loadXML(xml);function<void(__thiscall*)(void*,void*)>(0x474b0)(nm,wrap(&xml));checkMow();
    for(int held:{0,6}) {profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;mow->setLevel(0,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nm)+0x508,0,false);checkMow();}
}
stage=624;const auto beforeMow=textureCounts();mow.reset();const auto middleMow=textureCounts();call(0x473b0,nm);compareReferenceDeltas(beforeMow,middleMow);
hudUseActualMow=false;call(0xd58a0,nativeHeldMowIndicator);putPointer(0x25f754,oldNativeMow);putPointer(0x25ff5c,oldMowPerson);hudInputs.character=oldMowCharacter;
