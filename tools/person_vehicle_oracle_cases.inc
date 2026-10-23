// Native 004516e0 defines the motion fields of its 76-byte action. These
// isolated motion objects deliberately do not claim arrival/work dispatch.
auto* nativeVehicle=allocate(76);PersonVehicleWalkContext vehicleContext{walkContext,visualWorld};
const auto checkVehicle=[&](const PersonVehicleWalk& w) {
    checkVisual();for(auto pair:std::initializer_list<std::pair<const Point2*,unsigned>>{{&w.goal(),0xc},{&w.velocity(),0x14},{&w.relativeGoal(),0x28},{&w.xVelocity(),0x38},{&w.yVelocity(),0x40}}) for(unsigned i=0;i<2;++i) compareFloat(pair.first->at(i),field<float>(nativeVehicle,pair.second+i*4),"Vehicle motion vector");
    compareFloat(w.remaining(),field<float>(nativeVehicle,0x1c),"Vehicle direct remaining");compareFloat(w.yRemaining(),field<float>(nativeVehicle,0x30),"Vehicle Y remaining");compareFloat(w.xRemaining(),field<float>(nativeVehicle,0x34),"Vehicle X remaining");require(w.nearAtStart()==bool(field<unsigned char>(nativeVehicle,0x24)) && w.direct()==bool(field<unsigned char>(nativeVehicle,0x48)),"Vehicle near/direct flags differ");require(profile.floats==nativeProfile.floats,"Vehicle saved profile coordinates differ");++personVehicleStates;
};
// Axis selector leaves mirror/direction on zero and preserves unordered
// comparisons of the original x87 instructions without normalizing a vector.
unsigned axisNanBits=0x7fc00000;float axisNan;std::memcpy(&axisNan,&axisNanBits,4);
for(unsigned held=0;held<12;++held) for(unsigned level=0;level<4;++level) for(unsigned facing:{0U,2U,4U}) for(bool mirror:{false,true}) for(const Point2 delta:{Point2{0,0},Point2{-1,0},Point2{1,0},Point2{-1,-1},Point2{1,1},Point2{axisNan,0},Point2{0,axisNan}}) {
    stage=842;useVisualAdapters();profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;actionAreaLevels[1]=actionAreaLevels[2]=int(level);visual->direction=field<unsigned>(nativeActor,0x288)=facing;actor->setInvertX(mirror);function<void(__thiscall*)(void*,const bool*)>(0x13bf0)(nativeActor,&mirror);
    visual->playAxisWalk(delta);function<void(__thiscall*)(void*,const Point2*)>(0x504d0)(nativeActor,&delta);++personVehicleActions;checkVisual();++personVehicleStates;
}
for(unsigned held:{0U,1U,2U,6U,10U}) for(unsigned level=0;level<4;++level) for(unsigned start=0;start<12;++start) {
    stage=840;trial=held;action=level*12+start;useActualScene();walkFarm.movingPicture().setPosition(walkOrigin);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(field<void*>(nativeScenes[0],0x38),&walkOrigin);useVisualAdapters();
    for(const char* key:{"Upgrade31","Upgrade33","Upgrade34"}) profile.ints[key]=nativeProfile.ints[key]=int(level);actionAreaLevels[1]=actionAreaLevels[2]=int(level);profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=held;
    visual->direction=field<unsigned>(nativeActor,0x288)=2;visual->speed=field<float>(nativeActor,0x294)=200;const bool mirror=bool(start&1);actor->setInvertX(mirror);function<void(__thiscall*)(void*,const bool*)>(0x13bf0)(nativeActor,&mirror);visual->playIdle();call(0x50a30,nativeActor);
    const auto goal=walkPlot->goal(held);Point2 ng{};function<Point2*(__thiscall*)(void*,Point2*,unsigned)>(0x18800)(nwf,&ng,held);require(!std::memcmp(goal.data(),ng.data(),8),"Vehicle actual Field goal differs");const auto offset=visual->hotSpotOffset(visual->direction);Point2 no{};function<Point2*(__thiscall*)(void*,Point2*,unsigned)>(0x4f250)(nativeActor,&no,visual->direction);
    const Point2 deltas[]={{-200,-100},{200,-100},{-200,100},{200,100},{0,0},{2,2},{2.125f,0},{9.875f,60},{10,60},{60,9.875f},{60,10},{0,-120}};
    setWalkPosition({float(goal[0]-offset[0]+deltas[start][0]),float(goal[1]-offset[1]+deltas[start][1])});walkCancel=false;field<unsigned char>(nativeActor,0x284c)=0;
    PersonVehicleWalk w(*walkPlot,vehicleContext);std::memset(nativeVehicle,0,76);field<void*>(nativeVehicle,0x20)=nwf;w.begin();call(0x52bc0,nativeVehicle);++personVehicleActions;checkVehicle(w);
    bool done=false;for(unsigned tick=0;tick<160 && !done;++tick) {
        stage=841;action=tick;elapsed=tick==0 ? 0.f:tick==1 ? .001f:.125f;
        if(start<4 && tick==3) {const Point2 shifted{walkOrigin[0]+17.25f,walkOrigin[1]-8.5f};walkFarm.movingPicture().setPosition(shifted);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(field<void*>(nativeScenes[0],0x38),&shifted);}
        done=w.step();require(done==bool(function<unsigned char(__thiscall*)(void*)>(0x53380)(nativeVehicle)),"Vehicle completion differs");++personVehicleActions;checkVehicle(w);
    }require(done,"Vehicle motion failed to arrive at actual Field");
}

for(float speed:{0.f,-10.f,7.25f,500.f}) {
    stage=843;useVisualAdapters();profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;visual->speed=field<float>(nativeActor,0x294)=speed;setWalkPosition({230,420});walkCancel=false;field<unsigned char>(nativeActor,0x284c)=0;
    PersonVehicleWalk w(*walkPlot,vehicleContext);std::memset(nativeVehicle,0,76);field<void*>(nativeVehicle,0x20)=nwf;w.begin();call(0x52bc0,nativeVehicle);++personVehicleActions;checkVehicle(w);
    for(unsigned tick=0;tick<4;++tick) {elapsed=.05f;if(tick==2) {walkCancel=true;field<unsigned char>(nativeActor,0x284c)=1;}const bool done=w.step();require(done==bool(function<unsigned char(__thiscall*)(void*)>(0x53380)(nativeVehicle)),"Vehicle cancel/speed completion differs");++personVehicleActions;checkVehicle(w);if(done) break;}
}
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
useVisualAdapters();PersonVehicleWalk seedWalk(*walkSeed,vehicleContext);std::memset(nativeVehicle,0,76);field<void*>(nativeVehicle,0x20)=nws;seedWalk.begin();call(0x52bc0,nativeVehicle);++personVehicleActions;checkVehicle(seedWalk);elapsed=.05f;
require(seedWalk.step() && bool(function<unsigned char(__thiscall*)(void*)>(0x53380)(nativeVehicle)),"Person walk did not stop for moving actual seed");++personVehicleActions;checkVehicle(seedWalk);
useActualScene();walkSeed->setStatus(0);function<void(__thiscall*)(void*,unsigned)>(0x5cf50)(nws,0);useVisualAdapters();bool seedArrived=false;for(unsigned tick=0;tick<80 && !seedArrived;++tick) {elapsed=.05f;seedArrived=seedWalk.step();require(seedArrived==bool(function<unsigned char(__thiscall*)(void*)>(0x53380)(nativeVehicle)),"Person walk installed seed completion differs");++personVehicleActions;checkVehicle(seedWalk);}require(seedArrived,"Person walk failed to reach installed actual seed");
useActualScene();walkSeedWorld.setSeedMoveActive(false);walkSeedWorld.selected=nullptr;field<unsigned char>(image,0x25f721)=0;putPointer(0x25f724,nullptr);redirect(0x32c20,reinterpret_cast<void*>(&nativeDiscard));walkSeedWorld.removeSeed(*walkSeed);function<void(__thiscall*)(void*,void*)>(0x33420)(nativeScenes[0],nws);
require(views.discarded().size()==1 && nativeDiscards.size()==1 && views.discarded().at(walkSeed.get()).view()==nativeDiscards.at(nws),"Person walk seed cleanup origin differs");walkSeed.release();const auto beforeWalkSeed=textureCounts();views.deleteDiscarded();const auto middleWalkSeed=textureCounts();call(0x5ce00,nws);compareReferenceDeltas(beforeWalkSeed,middleWalkSeed);nativeDiscards.clear();redirect(0x32c20,reinterpret_cast<void*>(&waterNativeDiscardTask));
const auto beforeSeedGate=textureCounts();seedGate.reset();const auto middleSeedGate=textureCounts();mainMenuDeleteNative(nativeSeedGate);compareReferenceDeltas(beforeSeedGate,middleSeedGate);field<void*>(nativeScenes[0],0x9c)=savedWalkSeedGate;

for(unsigned renderCarry:{2U,6U}) {
// Actual mouse release creates the actual PersonTasks marker and FIFO entry.
// This consumes that entry with recovered motion; arrival work dispatch remains
// explicitly outside this comparison until its individual actions are restored.
useActualScene();walkFarm.movingPicture().setPosition(walkOrigin);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(field<void*>(nativeScenes[0],0x38),&walkOrigin);
profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=renderCarry;setWalkPosition(renderCarry==2 ? Point2{200,450}:Point2{760,200});visual->speed=field<float>(nativeActor,0x294)=600;walkCancel=false;field<unsigned char>(nativeActor,0x284c)=0;
for(const char* key:{"Upgrade31","Upgrade33","Upgrade34"}) profile.ints[key]=nativeProfile.ints[key]=1;actionAreaLevels[1]=actionAreaLevels[2]=1;useVisualAdapters();visual->playIdle();call(0x50a30,nativeActor);checkVisual();useActualScene();
auto& walkTasks=visualTasks;fieldWorld.tasks=&walkTasks;write(0x4d2f0,personTaskAddEntry.data(),5);redirect(0x32c20,reinterpret_cast<void*>(&nativeDiscard));
std::map<View*,void*> walkMarkers;
const auto checkWalkTasks=[&] {
    const auto check=[&](const PersonTask& t,void* n) {require((!t.area && !field<void*>(n,0)) || (t.area==walkPlot.get() && field<void*>(n,0)==nwf),"Person walking task owner differs");require(!std::memcmp(t.target.data(),static_cast<unsigned char*>(n)+8,8),"Person walking task target differs");auto* m=field<void*>(n,4);require(bool(t.marker)==bool(m),"Person walking marker presence differs");if(t.marker) {walkMarkers[t.marker]=m;compareControl(*t.marker,m);mainMenuCompareView(*t.marker,m);}};
    auto* first=field<unsigned char*>(nativeActor,0x282c);auto* last=field<unsigned char*>(nativeActor,0x2830);require(walkTasks.queued().size()==(first ? std::size_t((last-first)/16):0),"Person walking FIFO differs");for(unsigned i=0;i<walkTasks.queued().size();++i) check(walkTasks.queued()[i],first+i*16);check(walkTasks.current(),static_cast<unsigned char*>(nativeActor)+0x2838);require(walkPlot->clickIndex==field<unsigned>(nwf,0x4fc),"Person walking Field click count differs");++personVehicleStates;
};
const auto deleteWalkMarkers=[&] {require(views.discarded().size()==nativeDiscards.size(),"Person walking discard count differs");for(const auto& item:views.discarded()) require(item.second.view()==nativeDiscards.at(walkMarkers.at(item.first)),"Person walking marker discard origin differs");const auto before=textureCounts();views.deleteDiscarded();const auto middle=textureCounts();for(const auto& item:nativeDiscards) call(0xf1780,item.first);compareReferenceDeltas(before,middle);nativeDiscards.clear();};
{
    const auto hover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
    explicitTipElapsed=false;menuMilliseconds=0;GameTimer timer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor cursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&cursor;
    std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    std::unique_ptr<PersonVehicleWalk> motion;bool arrived=false;
    for(unsigned frame=0;frame<32;++frame) {
        stage=833;action=frame;menuMilliseconds+=frame<3 ? 50:250;menuPoint=frame<3 ? walkPlotPosition:Point2{790,590};menuMouseDelta={1,0,0};menuInputEvents=frame==1 ? std::vector<unsigned>{0x208}:frame==2 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
        if(frame==3) {require(walkTasks.queued().size()==1,"Actual Field mouse release did not queue walking task");auto* target=walkTasks.next();auto* nt=function<void*(__thiscall*)(void*)>(0x4e300)(nativeActor);require(target && target->area==walkPlot.get() && nt && field<void*>(nt,0)==nwf,"Person walk next task differs");++personVehicleActions;checkWalkTasks();useVisualAdapters();motion=std::make_unique<PersonVehicleWalk>(*target->area,vehicleContext);std::memset(nativeVehicle,0,76);field<void*>(nativeVehicle,0x20)=nwf;motion->begin();call(0x52bc0,nativeVehicle);++personVehicleActions;checkVehicle(*motion);useActualScene();}
        if(motion && !arrived) {useVisualAdapters();arrived=motion->step();require(arrived==bool(function<unsigned char(__thiscall*)(void*)>(0x53380)(nativeVehicle)),"Rendered Person walk completion differs");++personVehicleActions;checkVehicle(*motion);useActualScene();}
        personVehiclePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++personVehicleFrames;checkVisual();checkWalkTasks();
        if(frame==28) {require(arrived,"Rendered Person never reached the actual Field");walkTasks.finishCurrent();call(0x4d790,nativeActor);++personVehicleActions;checkWalkTasks();deleteWalkMarkers();}
    }
    interaction.playHover=hover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);explicitTipElapsed=true;
}
walkTasks.clear();call(0x4f870,nativeActor);++personVehicleActions;checkWalkTasks();deleteWalkMarkers();fieldWorld.tasks=nullptr;fieldWorld.held=false;field<unsigned char>(nativeScenes[0],0x108)=0;
fieldTip->hideFor(walkPlot.get());function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,nwf);redirect(0x4d2f0,reinterpret_cast<void*>(&actionAreaQueue));redirect(0x32c20,reinterpret_cast<void*>(&waterNativeDiscardTask));
}
