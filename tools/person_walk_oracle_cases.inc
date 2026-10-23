stage=820;useActualScene();const auto priorExplicitWalkTime=explicitTipElapsed;explicitTipElapsed=true;
const auto walkMathMode=field<unsigned>(image,0x261618),walkFloorFeatures=field<unsigned>(image,0x263b80);field<unsigned>(image,0x261618)=field<unsigned>(image,0x263b80)=1;
auto& walkFarm=fieldWorld.farmScene();auto* walkFarmParent=walkFarm.parent();auto* nativeWalkFarmParent=field<void*>(nativeScenes[0],0x20);const bool walkFarmShown=walkFarm.state().shown,walkFarmEnabled=walkFarm.state().enabled;
auto walkRoot=std::make_unique<View>(nullptr,views);auto* nativeWalkRoot=allocate(44);std::memset(nativeWalkRoot,0,44);function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeWalkRoot,nullptr);
const auto savedWalkMovement=walkFarm.movement;walkFarm.setParent(walkRoot.get());function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nativeWalkRoot);walkRoot->show();call(0xd37d0,nativeWalkRoot);walkRoot->enable();call(0xd3bb0,nativeWalkRoot);walkRoot->absoluteEnable();call(0xd3fb0,nativeWalkRoot);walkFarm.show();call(0xd37d0,nativeScenes[0]);walkFarm.enable();call(0xd3bb0,nativeScenes[0]);walkFarm.movement=field<unsigned>(nativeScenes[0],0x4c)=0;
auto* actorOldParent=actor->parent();auto* actorNativeOldParent=field<void*>(nativeActor,0x20);actor->setParent(&walkFarm.movingPicture());function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeActor,field<void*>(nativeScenes[0],0x38));
const Point2 walkOrigin{walkFarm.movingPicture().geometry().x,walkFarm.movingPicture().geometry().y};
std::array<void*,94> walkActorTable{};std::copy(visualTable.begin(),visualTable.end(),walkActorTable.begin());const unsigned walkExtraSlots[]={0x15c30,0x4b590,0x4b5a0,0x4b5b0,0x4b5c0,0x4b5d0};for(unsigned i=0;i<6;++i) walkActorTable[88+i]=image+walkExtraSlots[i];field<void*>(nativeActor,0)=walkActorTable.data();
std::array<int,3> savedWalkUpgradeLevels{profile.ints["Upgrade31"],profile.ints["Upgrade33"],profile.ints["Upgrade34"]};
field<void*>(nativeScenes[0],0)=fieldFarmVtable.data();fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
profile.ints["fcu"]=nativeProfile.ints["fcu"]=0;profile.ints["fua"]=nativeProfile.ints["fua"]=0;profile.uints["FFC0"]=nativeProfile.uints["FFC0"]=0;profile.uints["FFS0"]=nativeProfile.uints["FFS0"]=3;profile.floats["FFP0"]=nativeProfile.floats["FFP0"]=100;profile.floats["FFW0"]=nativeProfile.floats["FFW0"]=0;profile.floats["FFF0"]=nativeProfile.floats["FFF0"]=100;
auto walkPlot=std::make_unique<FarmField>(&walkFarm.movingPicture(),0,0,0,fieldContext);auto* nwf=allocate(2384);std::memset(nwf,0,2384);fieldNativeIds[nwf]=0;function<void*(__thiscall*)(void*,void*,unsigned,unsigned,int)>(0x3c120)(nwf,field<void*>(nativeScenes[0],0x38),0,0,0);
const Point2 walkPlotPosition{540,330};walkPlot->setPosition(walkPlotPosition);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(nwf,&walkPlotPosition);
bool walkCancel=false;PersonWalkContext walkContext{*actor,*visual,profile,walkFarm,walkCancel};auto* nativeWalk=allocate(48);
const auto checkWalk=[&](const PersonWalk& w) {
    stage=821;checkVisual();const auto& goal=w.goal();const auto& velocity=w.velocity();const auto& relative=w.relativeGoal();
    for(unsigned i=0;i<2;++i) {compareFloat(goal[i],field<float>(nativeWalk,0xc+i*4),"Person walk goal");compareFloat(velocity[i],field<float>(nativeWalk,0x14+i*4),"Person walk velocity");compareFloat(relative[i],field<float>(nativeWalk,0x28+i*4),"Person walk relative goal");}
    compareFloat(w.remaining(),field<float>(nativeWalk,0x1c),"Person walk remaining");require(w.nearAtStart()==bool(field<unsigned char>(nativeWalk,0x24)),"Person walk near flag differs");
    require(profile.floats==nativeProfile.floats,"Person walk saved profile coordinates differ");++personWalkStates;
};
const auto setWalkPosition=[&](const Point2& p) {actor->setPosition(p);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(nativeActor,&p);};
fieldWorld.tracePersonGoalLevels=true;
for(unsigned held:{0U,1U,2U,6U,10U}) for(unsigned level=0;level<4;++level) for(unsigned start=0;start<4;++start) {
    stage=822;trial=held;action=level*4+start;useActualScene();walkFarm.movingPicture().setPosition(walkOrigin);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(field<void*>(nativeScenes[0],0x38),&walkOrigin);
    walkPlot->setPosition(walkPlotPosition);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(nwf,&walkPlotPosition);useVisualAdapters();
    for(const char* key:{"Upgrade31","Upgrade33","Upgrade34"}) profile.ints[key]=nativeProfile.ints[key]=int(level);actionAreaLevels[1]=actionAreaLevels[2]=int(level);profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;
    visual->direction=field<unsigned>(nativeActor,0x288)=2;visual->speed=field<float>(nativeActor,0x294)=200;const bool mirror=false;actor->setInvertX(mirror);function<void(__thiscall*)(void*,const bool*)>(0x13bf0)(nativeActor,&mirror);visual->playIdle();call(0x50a30,nativeActor);
    Point2 expectedGoal{};const auto goal=walkPlot->goal(held);function<Point2*(__thiscall*)(void*,Point2*,unsigned)>(0x18800)(nwf,&expectedGoal,held);require(!std::memcmp(goal.data(),expectedGoal.data(),8),"Person walk actual Field goal differs");
    const auto offset=visual->hotSpotOffset(visual->direction);Point2 nativeOffset{};function<Point2*(__thiscall*)(void*,Point2*,unsigned)>(0x4f250)(nativeActor,&nativeOffset,visual->direction);require(!std::memcmp(offset.data(),nativeOffset.data(),8),"Person walk initial offset differs");
    const float distance=start==1 ? 0:start==2 ? 2.f:2.125f;const Point2 initial=start==0 ? Point2{200,450}:Point2{float(goal[0]-offset[0]+distance),float(goal[1]-offset[1])};setWalkPosition(initial);
    actor->setTime(0);const float zero=0;function<void(__thiscall*)(void*,const float*)>(0x13bc0)(nativeActor,&zero);walkCancel=false;field<unsigned char>(nativeActor,0x284c)=0;
    PersonWalk w(*walkPlot,walkContext);std::memset(nativeWalk,0,48);field<void*>(nativeWalk,0x20)=nwf;w.begin();call(0x520a0,nativeWalk);++personWalkActions;checkWalk(w);
    bool done=false;for(unsigned tick=0;tick<100 && !done;++tick) {
        stage=823;action=tick;elapsed=tick==0 ? 0.f:tick==1 ? .001f:.125f;
        if(start==0 && tick==3) {const Point2 shifted{walkOrigin[0]+17.25f,walkOrigin[1]-8.5f};walkFarm.movingPicture().setPosition(shifted);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(field<void*>(nativeScenes[0],0x38),&shifted);}
        done=w.step();const bool nativeDone=bool(function<unsigned char(__thiscall*)(void*)>(0x52630)(nativeWalk));require(done==nativeDone,"Person walk completion differs");++personWalkActions;checkWalk(w);
    }require(done,"Person walk failed to arrive at actual Field");
}
if(personVehicleMode) {
#include "person_vehicle_oracle_cases.inc"
}
// The original C0/C3 comparison treats unordered coordinates as close.
for(unsigned axes:{1U,2U,3U}) {
    stage=834;useVisualAdapters();profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;walkCancel=false;field<unsigned char>(nativeActor,0x284c)=0;
    const auto goal=walkPlot->goal(0);const auto offset=visual->hotSpotOffset(visual->direction);Point2 initial{float(goal[0]-offset[0]),float(goal[1]-offset[1])};unsigned nanBits=0x7fc00000;float nan;std::memcpy(&nan,&nanBits,4);if(axes&1) initial[0]=nan;if(axes&2) initial[1]=nan;setWalkPosition(initial);
    PersonWalk w(*walkPlot,walkContext);std::memset(nativeWalk,0,48);field<void*>(nativeWalk,0x20)=nwf;w.begin();call(0x520a0,nativeWalk);++personWalkActions;checkWalk(w);require(w.nearAtStart(),"Person unordered coordinates should take original close branch");require(w.step()==bool(function<unsigned char(__thiscall*)(void*)>(0x52630)(nativeWalk)),"Person unordered near completion differs");++personWalkActions;checkWalk(w);
}
// Zero/negative/slow speeds and explicit cancellation use the same body.
for(float speed:{0.f,-10.f,7.25f,500.f}) {
    stage=824;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;visual->speed=field<float>(nativeActor,0x294)=speed;setWalkPosition({230,420});walkCancel=false;field<unsigned char>(nativeActor,0x284c)=0;
    PersonWalk w(*walkPlot,walkContext);std::memset(nativeWalk,0,48);field<void*>(nativeWalk,0x20)=nwf;w.begin();call(0x520a0,nativeWalk);++personWalkActions;checkWalk(w);
    for(unsigned tick=0;tick<4;++tick) {elapsed=.05f;if(tick==2) {walkCancel=true;field<unsigned char>(nativeActor,0x284c)=1;}const bool done=w.step(),nativeDone=bool(function<unsigned char(__thiscall*)(void*)>(0x52630)(nativeWalk));require(done==nativeDone,"Person walk cancel/speed result differs");++personWalkActions;checkWalk(w);if(done) break;}
}
for(bool move:{false,true}) for(const Point2 position:{Point2{100,200},Point2{-1.25f,.5f},Point2{100000.125f,-90000.25f}}) {
    stage=825;storePersonPosition(*actor,profile,position,move);function<void(__thiscall*)(void*,const Point2*,bool)>(0x4e780)(nativeActor,&position,move);++personWalkActions;checkVisual();require(profile.floats==nativeProfile.floats,"Person position setter profile differs");++personWalkStates;
}
// A moving actual SeedPackage ends this motion immediately; once its real
// status setter installs it, the same motion can execute its original body.
stage=826;useActualScene();for(const char* key:{"Upgrade31","Upgrade33","Upgrade34"}) profile.ints[key]=nativeProfile.ints[key]=0;actionAreaLevels[1]=actionAreaLevels[2]=0;
walkCancel=false;field<unsigned char>(nativeActor,0x284c)=0;visual->speed=field<float>(nativeActor,0x294)=200;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;setWalkPosition({200,450});
stage=827;auto seedGate=std::make_unique<ExtendedAnimationButton>(&walkFarm.origin(),manager,settings,strings,input,interaction,[&]{return productionTutorial->active();},views,controls);auto* nativeSeedGate=allocate(864);std::memset(nativeSeedGate,0,864);stage=828;function<void*(__thiscall*)(void*,void*)>(0x23550)(nativeSeedGate,field<void*>(nativeScenes[0],0x60));
XmlDocument walkFarmXML(files,String("UI/GameUI/Farm.xml"));XmlNode* walkSeedButtonXML=nullptr;require(walkFarmXML.section(String("SeedsBuyButton"),walkSeedButtonXML),"Person walk seed button XML missing");stage=829;seedGate->loadXML(*walkSeedButtonXML);stage=830;reinterpret_cast<void(__thiscall*)(void*,void*)>(field<void**>(nativeSeedGate,0)[6])(nativeSeedGate,wrap(walkSeedButtonXML));
auto* savedWalkSeedGate=field<void*>(nativeScenes[0],0x9c);field<void*>(nativeScenes[0],0x9c)=nativeSeedGate;
struct WalkSeedWorld final:SeedPackageWorld {
    FieldFixture& f;ExtendedAnimationButton& button;ViewRegistry& views;SeedPackage* selected=nullptr;
    WalkSeedWorld(FieldFixture& a,ExtendedAnimationButton& b,ViewRegistry& v):f(a),button(b),views(v) {}
    TutorialManager& tutorial() override {return f.tutorial();}StateView& farmScene() override {return f.farmScene();}
    void registerArea(ActionArea& a) override {f.registerArea(a);}void queueAreaAction(ActionArea& a,const Point2& p) override {f.queueAreaAction(a,p);}
    int warehouseLevel() override {return f.warehouseLevel();}int digBuildingLevel() override {return f.digBuildingLevel();}int mowBuildingLevel() override {return f.mowBuildingLevel();}int waterSourceLevel() override {return f.waterSourceLevel();}
    bool seedMoveActive() override {return f.water.seedMove;}void setSeedMoveActive(bool value) override {f.water.seedMove=value;}
    SeedPackage* selectedSeed() override {return selected;}void setSelectedSeed(SeedPackage* p) override {selected=p;}
    bool canPlaceSeed(SeedPackage& a,const Point2& p) override {return f.graph.canPlaceSeed(&a,p);}ActionArea* topAt(const Point2& p,unsigned mask) override {return f.graph.topAt(p,mask);}
    std::vector<ActionArea*> areasAt(const Point2& p,unsigned mask) override {std::vector<ActionArea*> out;f.graph.appendAt(p,mask,out);return out;}
    void setFarmActionButtonEnabled(bool enabled) override {if(enabled) button.enable();else button.disable();}
    int money() override {return f.water.money();}void setMoney(int value) override {f.water.setMoney(value);}
    void removeSeed(SeedPackage& a) override {f.graph.remove(a,views);}FarmField* fieldForSeedTutorial() override {return f.graph.fieldForSeedTutorial();}
    Control& digBuilding() override {throw std::logic_error("Person walk seed target does not exercise tutorial20/Dig anchor");}
} walkSeedWorld(fieldWorld,*seedGate,views);
SeedPackageContext walkSeedContext{{manager,settings,strings,input,interaction,walkSeedWorld,views,controls},configuration,profile,walkSeedWorld};
for(unsigned i=0;i<8;++i) {const auto& info=configuration.seeds[i];auto* p=static_cast<unsigned char*>(nativeGame)+0x79c+i*44;field<unsigned>(p,0)=info.price;p+=4;field<cTexture*>(p,4)=info.icon.texture;field<unsigned>(p,8)=info.icon.frames;field<float>(p,12)=info.icon.length;field<unsigned char>(p,16)=info.icon.loop;field<unsigned>(p,20)=info.icon.columns;field<unsigned>(p,24)=info.icon.rows;field<unsigned char>(p,28)=info.icon.playing;}
stage=831;const Point2 seedPosition{340,260};auto walkSeed=std::make_unique<SeedPackage>(&walkFarm.movingPicture(),1,4,seedPosition,2,false,walkSeedContext);auto* nws=allocate(1684);std::memset(nws,0,1684);stage=832;function<void*(__thiscall*)(void*,void*,unsigned,unsigned,const Point2*,unsigned,bool)>(0x5e130)(nws,field<void*>(nativeScenes[0],0x38),1,4,&seedPosition,2,false);
useVisualAdapters();PersonWalk seedWalk(*walkSeed,walkContext);std::memset(nativeWalk,0,48);field<void*>(nativeWalk,0x20)=nws;seedWalk.begin();call(0x520a0,nativeWalk);++personWalkActions;checkWalk(seedWalk);elapsed=.05f;
require(seedWalk.step() && bool(function<unsigned char(__thiscall*)(void*)>(0x52630)(nativeWalk)),"Person walk did not stop for moving actual seed");++personWalkActions;checkWalk(seedWalk);
useActualScene();walkSeed->setStatus(0);function<void(__thiscall*)(void*,unsigned)>(0x5cf50)(nws,0);useVisualAdapters();bool seedArrived=false;for(unsigned tick=0;tick<80 && !seedArrived;++tick) {elapsed=.05f;seedArrived=seedWalk.step();require(seedArrived==bool(function<unsigned char(__thiscall*)(void*)>(0x52630)(nativeWalk)),"Person walk installed seed completion differs");++personWalkActions;checkWalk(seedWalk);}require(seedArrived,"Person walk failed to reach installed actual seed");
useActualScene();walkSeedWorld.setSeedMoveActive(false);walkSeedWorld.selected=nullptr;field<unsigned char>(image,0x25f721)=0;putPointer(0x25f724,nullptr);redirect(0x32c20,reinterpret_cast<void*>(&nativeDiscard));walkSeedWorld.removeSeed(*walkSeed);function<void(__thiscall*)(void*,void*)>(0x33420)(nativeScenes[0],nws);
require(views.discarded().size()==1 && nativeDiscards.size()==1 && views.discarded().at(walkSeed.get()).view()==nativeDiscards.at(nws),"Person walk seed cleanup origin differs");walkSeed.release();const auto beforeWalkSeed=textureCounts();views.deleteDiscarded();const auto middleWalkSeed=textureCounts();call(0x5ce00,nws);compareReferenceDeltas(beforeWalkSeed,middleWalkSeed);nativeDiscards.clear();redirect(0x32c20,reinterpret_cast<void*>(&waterNativeDiscardTask));
const auto beforeSeedGate=textureCounts();seedGate.reset();const auto middleSeedGate=textureCounts();mainMenuDeleteNative(nativeSeedGate);compareReferenceDeltas(beforeSeedGate,middleSeedGate);field<void*>(nativeScenes[0],0x9c)=savedWalkSeedGate;
if(!personVehicleMode) {
// Actual mouse release creates the actual PersonTasks marker and FIFO entry.
// This consumes that entry with recovered motion; arrival work dispatch remains
// explicitly outside this comparison until its individual actions are restored.
useActualScene();walkFarm.movingPicture().setPosition(walkOrigin);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(field<void*>(nativeScenes[0],0x38),&walkOrigin);
profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;setWalkPosition({200,450});visual->speed=field<float>(nativeActor,0x294)=200;walkCancel=false;field<unsigned char>(nativeActor,0x284c)=0;
auto& walkTasks=visualTasks;fieldWorld.tasks=&walkTasks;write(0x4d2f0,personTaskAddEntry.data(),5);redirect(0x32c20,reinterpret_cast<void*>(&nativeDiscard));
std::map<View*,void*> walkMarkers;
const auto checkWalkTasks=[&] {
    const auto check=[&](const PersonTask& t,void* n) {require((!t.area && !field<void*>(n,0)) || (t.area==walkPlot.get() && field<void*>(n,0)==nwf),"Person walking task owner differs");require(!std::memcmp(t.target.data(),static_cast<unsigned char*>(n)+8,8),"Person walking task target differs");auto* m=field<void*>(n,4);require(bool(t.marker)==bool(m),"Person walking marker presence differs");if(t.marker) {walkMarkers[t.marker]=m;compareControl(*t.marker,m);mainMenuCompareView(*t.marker,m);}};
    auto* first=field<unsigned char*>(nativeActor,0x282c);auto* last=field<unsigned char*>(nativeActor,0x2830);require(walkTasks.queued().size()==(first ? std::size_t((last-first)/16):0),"Person walking FIFO differs");for(unsigned i=0;i<walkTasks.queued().size();++i) check(walkTasks.queued()[i],first+i*16);check(walkTasks.current(),static_cast<unsigned char*>(nativeActor)+0x2838);require(walkPlot->clickIndex==field<unsigned>(nwf,0x4fc),"Person walking Field click count differs");++personWalkStates;
};
const auto deleteWalkMarkers=[&] {require(views.discarded().size()==nativeDiscards.size(),"Person walking discard count differs");for(const auto& item:views.discarded()) require(item.second.view()==nativeDiscards.at(walkMarkers.at(item.first)),"Person walking marker discard origin differs");const auto before=textureCounts();views.deleteDiscarded();const auto middle=textureCounts();for(const auto& item:nativeDiscards) call(0xf1780,item.first);compareReferenceDeltas(before,middle);nativeDiscards.clear();};
{
    const auto hover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
    explicitTipElapsed=false;menuMilliseconds=0;GameTimer timer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor cursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&cursor;
    std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    std::unique_ptr<PersonWalk> motion;bool arrived=false;
    for(unsigned frame=0;frame<48;++frame) {
        stage=833;action=frame;menuMilliseconds+=frame<3 ? 50:250;menuPoint=frame<3 ? walkPlotPosition:Point2{790,590};menuMouseDelta={1,0,0};menuInputEvents=frame==1 ? std::vector<unsigned>{0x208}:frame==2 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
        if(frame==3) {require(walkTasks.queued().size()==1,"Actual Field mouse release did not queue walking task");auto* target=walkTasks.next();auto* nt=function<void*(__thiscall*)(void*)>(0x4e300)(nativeActor);require(target && target->area==walkPlot.get() && nt && field<void*>(nt,0)==nwf,"Person walk next task differs");++personWalkActions;checkWalkTasks();useVisualAdapters();motion=std::make_unique<PersonWalk>(*target->area,walkContext);std::memset(nativeWalk,0,48);field<void*>(nativeWalk,0x20)=nwf;motion->begin();call(0x520a0,nativeWalk);++personWalkActions;checkWalk(*motion);useActualScene();}
        if(motion && !arrived) {useVisualAdapters();arrived=motion->step();require(arrived==bool(function<unsigned char(__thiscall*)(void*)>(0x52630)(nativeWalk)),"Rendered Person walk completion differs");++personWalkActions;checkWalk(*motion);useActualScene();}
        personWalkPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++personWalkFrames;checkVisual();checkWalkTasks();
        if(frame==44) {require(arrived,"Rendered Person never reached the actual Field");walkTasks.finishCurrent();call(0x4d790,nativeActor);++personWalkActions;checkWalkTasks();deleteWalkMarkers();}
    }
    interaction.playHover=hover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);explicitTipElapsed=true;
}
walkTasks.clear();call(0x4f870,nativeActor);++personWalkActions;checkWalkTasks();deleteWalkMarkers();fieldWorld.tasks=nullptr;fieldWorld.held=false;field<unsigned char>(nativeScenes[0],0x108)=0;
fieldTip->hideFor(walkPlot.get());function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,nwf);redirect(0x4d2f0,reinterpret_cast<void*>(&actionAreaQueue));redirect(0x32c20,reinterpret_cast<void*>(&waterNativeDiscardTask));
}
useActualScene();fieldWorld.tracePersonGoalLevels=false;walkCancel=false;field<unsigned char>(nativeActor,0x284c)=0;
const auto beforeWalkPlot=textureCounts();walkPlot.reset();const auto middleWalkPlot=textureCounts();call(0x36340,nwf);compareReferenceDeltas(beforeWalkPlot,middleWalkPlot);
for(unsigned i=0;i<3;++i) {const char* keys[]={"Upgrade31","Upgrade33","Upgrade34"};profile.ints[keys[i]]=nativeProfile.ints[keys[i]]=savedWalkUpgradeLevels[i];}
walkFarm.movingPicture().setPosition(walkOrigin);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(field<void*>(nativeScenes[0],0x38),&walkOrigin);actor->setParent(actorOldParent);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeActor,actorNativeOldParent);field<void*>(nativeActor,0)=visualTable.data();
if(!walkFarmEnabled) {walkFarm.disable();call(0xd3750,nativeScenes[0]);}if(!walkFarmShown) {walkFarm.hide();call(0xd3720,nativeScenes[0]);}walkFarm.setParent(walkFarmParent);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nativeWalkFarmParent);walkFarm.movement=field<unsigned>(nativeScenes[0],0x4c)=savedWalkMovement;walkRoot.reset();call(0xd58a0,nativeWalkRoot);
fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;field<void*>(nativeScenes[0],0)=priorFieldFarmVtable;field<unsigned>(image,0x261618)=walkMathMode;field<unsigned>(image,0x263b80)=walkFloorFeatures;explicitTipElapsed=priorExplicitWalkTime;
