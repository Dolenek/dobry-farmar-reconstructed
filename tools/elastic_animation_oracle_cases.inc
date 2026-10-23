// Actual instrument transfer plus the actual three tool-building controllers.
// Person position/appearance is an explicit boundary until full Person exists.
stage=840;const bool savedElasticElapsed=explicitTipElapsed;explicitTipElapsed=true;
const auto elasticMath=field<unsigned>(image,0x261618),elasticFloor=field<unsigned>(image,0x263b80);field<unsigned>(image,0x261618)=field<unsigned>(image,0x263b80)=1;
std::array<unsigned char,5> elasticPersonAppearance{};std::memcpy(elasticPersonAppearance.data(),image+0x50a30,5);redirect(0x50a30,reinterpret_cast<void*>(&waterNativePersonAppearance));
write(0x2b7a0,elasticOriginalBegin.data(),5);
struct ElasticToolWorld final:DigBuildingWorld,WaterSourceWorld {
    WarehouseFixture& stock;Point2 actor{300,400};bool cancel=false;
    explicit ElasticToolWorld(WarehouseFixture& value):stock(value) {}
    TutorialManager& tutorial() override {return stock.tutorial();}StateView& farmScene() override {return stock.farmScene();}
    void registerArea(ActionArea& a) override {stock.registerArea(a);}void queueAreaAction(ActionArea& a,const Point2& p) override {stock.queueAreaAction(a,p);}
    int warehouseLevel() override {return stock.warehouseLevel();}int digBuildingLevel() override {return DigBuilding::instance() ? DigBuilding::instance()->level():0;}
    int mowBuildingLevel() override {return MowBuilding::instance()->level();}int waterSourceLevel() override {return WaterSource::instance() ? WaterSource::instance()->level():0;}
    bool cursorOverAreaType(unsigned type,const ActionArea* excluded) override {return stock.cursorOverAreaType(type,excluded);}
    int heldType() override {return int(stock.profile.uints["PCarry"]);}void setHeldType(int value) override {stock.profile.uints["PCarry"]=value;}
    int money() override {return stock.money();}void setMoney(int value) override {stock.setMoney(value);}
    void updatePersonAppearance() override {waterSourceEffects.push_back(1);}
    void cancelPersonAction(const String&) override {cancel=true;}
    unsigned sprinkleCount() override {return 0;}bool seedMoveActive() override {return false;}bool sprinkleMoveActive() override {return false;}
    ButtonPrototype* createSprinkle(unsigned) override {throw std::runtime_error("Elastic comparison must not create Sprinkle");}
    Point2 characterPosition() override {return actor;}
    void animateHeldDigUpgrade(const Point2& from,const Point2& to,int kind,bool flag) override {require(!flag,"Elastic Dig reveal differs");ElasticAnimation::instance()->begin(from,to,kind,nullptr);}
    void animateHeldUpgrade(const Point2& from,const Point2& to,int kind,bool flag) override {require(!flag,"Elastic Water reveal differs");ElasticAnimation::instance()->begin(from,to,kind,nullptr);}
    void refreshHeldAppearance() override {digSourceEffects.push_back({1,0,0,0,0,0,0});}
} elasticWorld(warehouseWorld);
void* oldElasticDig=field<void*>(image,0x25f74c);void* oldElasticWater=field<void*>(image,0x25f750);void* oldElastic=field<void*>(image,0x25ff90);
for(const char* key:{"Upgrade33","Upgrade34"}) profile.ints[key]=nativeProfile.ints[key]=0;
for(const char* key:{"UpAvail13","UpAvail14"}) profile.uints[key]=nativeProfile.uints[key]=3;
profile.ints["Money"]=nativeProfile.ints["Money"]=100000;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;
DigBuildingContext elasticDigContext{{manager,settings,strings,input,interaction,static_cast<DigBuildingWorld&>(elasticWorld),views,controls},productionUpgrades,configuration,elasticWorld};
WaterSourceContext elasticWaterContext{{manager,settings,strings,input,interaction,static_cast<WaterSourceWorld&>(elasticWorld),views,controls},productionUpgrades,configuration,elasticWorld};
putPointer(0x25f74c,nullptr);auto elasticDig=std::make_unique<DigBuilding>(nullptr,elasticDigContext);auto* ned=allocate(2220);std::memset(ned,0,2220);function<void*(__thiscall*)(void*,void*)>(0x29fe0)(ned,nullptr);
putPointer(0x25f750,nullptr);auto elasticWater=std::make_unique<WaterSource>(nullptr,elasticWaterContext);auto* newater=allocate(2360);std::memset(newater,0,2360);function<void*(__thiscall*)(void*,void*)>(0x6b330)(newater,nullptr);
putPointer(0x25ff90,nullptr);auto flight=std::make_unique<ElasticAnimation>(nullptr,manager,audio,views,controls);auto* ne=allocate(1028);std::memset(ne,0,1028);function<void*(__thiscall*)(void*,void*)>(0x2c2d0)(ne,nullptr);
View reveal(nullptr,views);auto* nativeReveal=allocate(44);std::memset(nativeReveal,0,44);function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeReveal,nullptr);
mowWorld.actualFlight=flight.get();bool elasticHasKind=false;
const auto checkElastic=[&] {
    require(ElasticAnimation::instance()==flight.get() && field<void*>(image,0x25ff90)==ne,"Elastic singleton differs");compareControl(*flight,ne);mainMenuCompareView(*flight,ne);
    for(auto pair:std::initializer_list<std::pair<float,unsigned>>{{flight->rotationSpeed,0x23c},{flight->rotation,0x240},{flight->speed,0x3d4},{flight->acceleration,0x3d8},{flight->velocity[0],0x3e4},{flight->velocity[1],0x3e8},{flight->accelerationVector[0],0x3ec},{flight->accelerationVector[1],0x3f0},{flight->position[0],0x3f4},{flight->position[1],0x3f8},{flight->remaining,0x3fc}}) {trial=pair.second;compareFloat(pair.first,field<float>(ne,pair.second),"Elastic data");}
    require(flight->moveSound()==field<cResource*>(ne,0x3dc),"Elastic MoveSound differs");for(unsigned i=0;i<10;++i) appearanceCheck(flight->states[i],static_cast<unsigned char*>(ne)+0x244+i*40);
    if(elasticHasKind) {require(flight->kind==field<int>(ne,0x3e0),"Elastic kind differs");require((flight->revealView==&reveal && field<void*>(ne,0x400)==nativeReveal) || (!flight->revealView && !field<void*>(ne,0x400)),"Elastic reveal differs");}
    require(elasticWorld.cancel==bool(field<unsigned char>(mowPerson.data(),0x284c)),"Elastic Water actor cancel differs");
    mainMenuCompareView(reveal,nativeReveal);compareControl(*elasticDig,ned);compareControl(*mow,nm);compareControl(*elasticWater,newater);
    require(profile.ints==nativeProfile.ints && profile.uints==nativeProfile.uints && profile.booleans==nativeProfile.booleans,"Elastic tool profile differs");
    require(actualSounds==expectedSounds,"Elastic sound requests differ");actualSounds.clear();expectedSounds.clear();
    require(waterSourceEffects==waterNativeEffects && mowSourceEffects==mowNativeEffects,"Elastic Person observations differ");waterSourceEffects.clear();waterNativeEffects.clear();mowSourceEffects.clear();mowNativeEffects.clear();
    // MowFixture records begin as an observation in its separate module. Here
    // both source and native call the actual Elastic body, compared above.
    if(!digSourceEffects.empty() && digSourceEffects.front()[0]==0) digSourceEffects.erase(digSourceEffects.begin());
    require(digSourceEffects==digNativeEffects,"Elastic held appearance differs");digSourceEffects.clear();digNativeEffects.clear();
    require(warehouseSourceEvents==warehouseNativeEvents,"Elastic Money effects differ");warehouseSourceEvents.clear();warehouseNativeEvents.clear();
    require(hudSourceCalls==hudNativeCalls,"Elastic HUD effects differ");hudSourceCalls.clear();hudNativeCalls.clear();++elasticStates;
};checkElastic();
const auto beginElastic=[&](Point2 from,Point2 to,int kind) {reveal.hide();call(0xd3720,nativeReveal);flight->begin(from,to,kind,&reveal);function<void(__thiscall*)(void*,const Point2*,const Point2*,int,void*)>(0x2b7a0)(ne,&from,&to,kind,nativeReveal);elasticHasKind=elasticHasKind || from!=to;++elasticActions;checkElastic();};
const auto stepElastic=[&](float dt) {elapsed=dt;flight->absoluteStep();call(0x2b280,ne);++elasticActions;checkElastic();};
const std::array<std::pair<Point2,Point2>,9> elasticPaths{{{{300,400},{450,500}},{{450,500},{300,400}},{{300,500},{450,400}},{{450,400},{300,500}},{{300,400},{300,500}},{{300,400},{450,400}},{{300,400},{300.125f,399.875f}},{{300,400},{300,400}},{{0,0},{-0.0f,0}}}};
for(int kind=0;kind<10;++kind) for(unsigned path=0;path<elasticPaths.size();++path) {stage=841;action=kind*10+path;
    beginElastic(elasticPaths[path].first,elasticPaths[path].second,kind);stepElastic(0);stepElastic(0.013f);stepElastic(0.1f);stepElastic(0.33f);stepElastic(10);stepElastic(0.1f);
}
// Restart in flight, equal-coordinate no-op and completion at exact duration.
stage=842;beginElastic({120,220},{360,410},9);stepElastic(.017f);beginElastic({400,230},{430,170},3);beginElastic({4,5},{4,5},8);stepElastic(flight->remaining);stepElastic(0);
// Native Euler motion does not snap to the requested destination.
stage=843;for(float speed:{0.0f,-20.0f,1.25f,250.0f}) {flight->speed=field<float>(ne,0x3d4)=speed;beginElastic({300,400},{441,208},5);stepElastic(.125f);stepElastic(20);}
// Zero acceleration produces the original unordered duration. Preserve its
// branch and data rather than substituting a guessed uniform-speed flight.
stage=843;const float elasticAcceleration=flight->acceleration;
flight->acceleration=field<float>(ne,0x3d8)=0;beginElastic({300,400},{401,501},9);stepElastic(.125f);stepElastic(10);
flight->acceleration=field<float>(ne,0x3d8)=elasticAcceleration;
flight->speed=field<float>(ne,0x3d4)=250;
// Actual three building upgrades initiate the same real instrument transfer.
const auto upgradeElastic=[&](int which) {
    profile.ints["Money"]=nativeProfile.ints["Money"]=100000;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=which==0 ? 1:which==1 ? 2:6;
    if(which==0) {elasticWater->setLevel(0,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(newater)+0x508,0,false);elasticWater->upgradeButton().clickEvent(&elasticWater->upgradeButton());function<void(__cdecl*)(void*)>(0x68420)(field<void*>(newater,0x8a8));require(elasticWater->level()==1,"Actual Water upgrade missing");}
    if(which==1) {elasticDig->setLevel(0,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ned)+0x508,0,false);elasticDig->upgradeButton().clickEvent(&elasticDig->upgradeButton());function<void(__cdecl*)(void*)>(0x26260)(field<void*>(ned,0x8a4));require(elasticDig->level()==1,"Actual Dig upgrade missing");}
    if(which==2) {mow->setLevel(0,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nm)+0x508,0,false);mow->upgradeButton().clickEvent(&mow->upgradeButton());function<void(__cdecl*)(void*)>(0x471a0)(field<void*>(nm,0x8b4));require(mow->level()==1,"Actual Mow upgrade missing");}
    elasticHasKind=true;++elasticActions;checkElastic();require(flight->remaining>0,"Actual tool upgrade did not launch flight");
};
stage=844;for(int which=0;which<3;++which) {upgradeElastic(which);stepElastic(.013f);stepElastic(10);}
{
    const auto hover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
    explicitTipElapsed=false;menuMilliseconds=0;GameTimer timer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor cursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&cursor;
    std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    for(int which=0;which<3;++which) {stage=845;upgradeElastic(which);for(unsigned frame=0;frame<16;++frame) {action=which*16+frame;menuMilliseconds+=250;menuPoint={790,590};menuMouseDelta={};menuInputEvents.clear();elasticPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++elasticFrames;checkElastic();}require(flight->remaining==0,"Rendered actual upgrade flight incomplete");}
    stage=848;beginElastic({300,400},{460,290},9);
    for(unsigned frame=0;frame<16;++frame) {action=frame;menuMilliseconds+=250;menuPoint={790,590};menuMouseDelta={};menuInputEvents.clear();elasticPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++elasticFrames;checkElastic();}
    require(reveal.state().shown && flight->remaining==0,"Rendered Elastic reveal did not complete");
    // Real mouse release invokes actual Mow upgrade callback, which launches
    // the actual source/native flight in their registered UI hierarchies.
    stage=849;profile.ints["Money"]=nativeProfile.ints["Money"]=100000;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=6;
    mow->setLevel(0,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nm)+0x508,0,false);checkElastic();
    const Point2 upgradeAt{mow->upgradeButton().geometry().x,mow->upgradeButton().geometry().y};
    for(unsigned frame=0;frame<20;++frame) {action=frame;menuMilliseconds+=250;menuPoint=frame<3 ? upgradeAt:Point2{790,590};menuMouseDelta=frame==0 || frame==3 ? std::array<int,3>{1,0,0}:std::array<int,3>{};menuInputEvents=frame==1 ? std::vector<unsigned>{0x208}:frame==2 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};elasticPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++elasticFrames;checkElastic();if(frame==2) {require(mow->level()==1 && flight->remaining>0,"Real Mow mouse release did not launch actual Elastic");++elasticActions;}}
    require(flight->remaining==0,"Real mouse Mow upgrade flight did not finish");
    interaction.playHover=hover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);explicitTipElapsed=true;
}
// XML reload deliberately preserves the original old sound-reference behavior.
const auto elasticDirectory=common/"elastic-reload";std::filesystem::create_directories(elasticDirectory);std::ofstream(elasticDirectory/"partial-elastic.xml")<<"<Elastic RotationSpeed='-0.25' Speed='15' Accelerate='80'/>";files.mountCommon(String(elasticDirectory.generic_string()));
for(const char* path:{"Farm/ElasticAnimation.xml","partial-elastic.xml","Farm/ElasticAnimation.xml"}) {stage=846;XmlDocument xml(files,String(path));auto* move=flight->moveSound();const auto soundBefore=move->referenceCount();flight->loadXML(xml);const auto soundMiddle=move->referenceCount();function<void(__thiscall*)(void*,void*)>(0x2bce0)(ne,wrap(&xml));require(soundMiddle-soundBefore==move->referenceCount()-soundMiddle,"Elastic reload sound references differ");++elasticActions;checkElastic();beginElastic({100,200},{300,400},9);stepElastic(.05f);stepElastic(10);}
stage=847;mowWorld.actualFlight=nullptr;auto* elasticSound=flight->moveSound();const auto elasticSoundBefore=elasticSound->referenceCount();const auto elasticBefore=textureCounts();flight.reset();const auto elasticSoundMiddle=elasticSound->referenceCount();const auto elasticMiddle=textureCounts();call(0x2b1a0,ne);require(elasticSoundMiddle-elasticSoundBefore==elasticSound->referenceCount()-elasticSoundMiddle,"Elastic destructor sound reference differs");compareReferenceDeltas(elasticBefore,elasticMiddle);
call(0xd58a0,nativeReveal);const auto toolBefore=textureCounts();elasticWater.reset();elasticDig.reset();const auto toolMiddle=textureCounts();call(0x68640,newater);call(0x26470,ned);compareReferenceDeltas(toolBefore,toolMiddle);
putPointer(0x25f74c,oldElasticDig);putPointer(0x25f750,oldElasticWater);putPointer(0x25ff90,oldElastic);write(0x50a30,elasticPersonAppearance.data(),5);redirect(0x2b7a0,reinterpret_cast<void*>(&digNativeAnimation));
field<unsigned>(image,0x261618)=elasticMath;field<unsigned>(image,0x263b80)=elasticFloor;explicitTipElapsed=savedElasticElapsed;
