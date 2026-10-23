// FieldObject is the explicit drawing/input base adapter on both sides.
// Actual Person XML and idle/job/hotspot bodies execute; full actor is pending.
stage=800;cursorPosition={790,590};std::array<unsigned char,5> visualLevelEntry{};std::memcpy(visualLevelEntry.data(),image+0x17050,5);
write(0x50a30,personIdleEntry.data(),5);redirect(0x17050,reinterpret_cast<void*>(&actionAreaLevel));
const auto oldVisualLevelMap=actionAreaNativeLevels;const auto oldVisualLevels=actionAreaLevels;
std::array<void*,2> visualLevelGlobals{field<void*>(image,0x25f74c),field<void*>(image,0x25f754)};
for(unsigned i=0;i<2;++i) {auto* p=allocate(1340);std::memset(p,0,1340);putPointer(i ? 0x25f754:0x25f74c,p);actionAreaNativeLevels[static_cast<unsigned char*>(p)+0x508]=i+1;}
struct VisualWorld final:PersonVisualWorld {
    unsigned seedKind=1,seedCount=4;unsigned carriedSeedKind() override {return seedKind;}unsigned carriedSeedCount() override {return seedCount;}
    int digLevel() override {actionAreaSourceQueries.push_back(1);return actionAreaLevels[1];}
    int mowLevel() override {actionAreaSourceQueries.push_back(2);return actionAreaLevels[2];}
} visualWorld;
PersonVisualContext visualContext{manager,interaction,profile,configuration,visualWorld,views,controls};
auto actor=std::make_unique<FieldObject>(nullptr,manager,input,interaction,views,controls);auto* nativeActor=allocate(0x2b30);std::memset(nativeActor,0,0x2b30);function<void*(__thiscall*)(void*,void*)>(0xf1c30)(nativeActor,nullptr);
std::array<void*,88> visualTable{};std::copy_n(field<void**>(nativeActor,0),88,visualTable.begin());visualTable[0xc0/4]=image+0x192a0;visualTable[0xe8/4]=image+0x19a90;visualTable[0x14c/4]=image+0x17fe0;field<void*>(nativeActor,0)=visualTable.data();
field<unsigned>(nativeActor,0x288)=2;field<unsigned>(nativeActor,0x28c)=12;field<unsigned char>(nativeActor,0x290)=1;field<unsigned>(nativeActor,0x2b2c)=6;
for(auto pair:std::initializer_list<std::pair<unsigned,unsigned>>{{0x298,72},{0xdd8,66},{0x1828,1},{0x1958,72},{0x2498,1}}) for(unsigned i=0;i<pair.second;++i) call(0xd2080,static_cast<unsigned char*>(nativeActor)+pair.first+i*40);
auto visual=std::make_unique<PersonVisuals>(*actor,visualContext);
for(auto pair:std::initializer_list<std::pair<unsigned,unsigned>>{{0x2850,584},{0x2854,604},{0x2858,568},{0x285c,568}}) {
    auto* p=allocate(pair.second);std::memset(p,0,pair.second);auto* parent=pair.first==0x285c ? field<void*>(nativeActor,0x2858):nativeActor;
    function<void*(__thiscall*)(void*,void*)>(pair.first==0x2850 ? 0xf91e0:pair.first==0x2854 ? 0x8d3d0:0xf16f0)(p,parent);field<void*>(nativeActor,pair.first)=p;
}
std::vector<std::pair<cResource*,cResource*>> visualSoundPairs;
const auto compareVisualSounds=[&] {for(const auto& p:visualSoundPairs) require(p.first->resourceName().view()==p.second->resourceName().view() && p.first->referenceCount()==p.second->referenceCount(),"Person retained sound references differ");};
const auto pairVisualSounds=[&](std::size_t before,std::size_t middle) {require(sounds.size()-middle==middle-before,"Person XML sound request count differs");for(std::size_t i=0;i<middle-before;++i) visualSoundPairs.emplace_back(sounds[before+i].get(),sounds[middle+i].get());compareVisualSounds();};
XmlDocument visualXML(files,String("Person/Person.xml"));const auto beforeInitialSounds=sounds.size();visual->loadXML(visualXML);const auto middleInitialSounds=sounds.size();function<void(__thiscall*)(void*,void*)>(0x4b600)(nativeActor,wrap(&visualXML));pairVisualSounds(beforeInitialSounds,middleInitialSounds);
const auto compareVisualAppearance=[&](const AnimableAppearance& a,unsigned offset) {
    auto* p=static_cast<unsigned char*>(nativeActor)+offset;require(a.texture==field<cTexture*>(p,4) && a.frames==field<unsigned>(p,8) && a.columns==field<unsigned>(p,20) && a.rows==field<unsigned>(p,24),"Person appearance identity/frame grid differs");
    compareFloat(a.length,field<float>(p,12),"Person appearance length");require(a.loop==bool(field<unsigned char>(p,16)) && a.playing==bool(field<unsigned char>(p,28)),"Person appearance loop/play differs");
};
const auto checkVisual=[&] {
    stage=801;compareControl(*actor,nativeActor);mainMenuCompareView(*actor,nativeActor);compareFloat(actor->heightOffset,field<float>(nativeActor,0x27c),"Person HeightOffset");compareFloat(visual->speed,field<float>(nativeActor,0x294),"Person speed");
    require(visual->direction==field<unsigned>(nativeActor,0x288) && visual->activity==field<unsigned>(nativeActor,0x28c) && visual->hoseAnimation==bool(field<unsigned char>(nativeActor,0x290)) && visual->hoseFrame==field<unsigned>(nativeActor,0x2b2c),"Person direction/activity/hose state differs");
    for(auto pair:std::initializer_list<std::pair<Control*,unsigned>>{{&visual->waterUse(),0x2850},{&visual->discreteUse(),0x2854},{&visual->sign(),0x2858},{&visual->substrate(),0x285c}}) {compareControl(*pair.first,field<void*>(nativeActor,pair.second));mainMenuCompareView(*pair.first,field<void*>(nativeActor,pair.second));}
    auto* waterUI=field<void*>(nativeActor,0x2850);compareFloat(visual->waterUse().maximum,field<float>(waterUI,0x238),"Person water UI maximum");compareFloat(visual->waterUse().current,field<float>(waterUI,0x23c),"Person water UI current");require(visual->waterUse().side==field<unsigned>(waterUI,0x240),"Person water UI side differs");
    auto& d=visual->discreteUse();auto* nd=field<void*>(nativeActor,0x2854);require(d.current==field<unsigned>(nd,0x22c),"Person discrete current differs");compareFloat(d.maxLength,field<float>(nd,0x23c),"Person discrete length");
    auto** first=field<void**>(nd,0x250);auto** last=field<void**>(nd,0x254);require(d.pictures().size()==(first ? std::size_t(last-first):0),"Person discrete count differs");for(unsigned i=0;i<d.pictures().size();++i) {compareControl(*d.pictures()[i],first[i]);mainMenuCompareView(*d.pictures()[i],first[i]);}
    require(actionAreaSourceQueries==actionAreaNativeQueries,"Person visual upgrade queries differ");actionAreaSourceQueries.clear();actionAreaNativeQueries.clear();++personVisualStates;
};
actionAreaSourceQueries.clear();actionAreaNativeQueries.clear();checkVisual();
const auto checkVisualData=[&] {
for(unsigned i=0;i<72;++i) {compareVisualAppearance(visual->moving[i],0x298+i*40);compareVisualAppearance(visual->idle[i],0x1958+i*40);}
for(unsigned i=0;i<66;++i) compareVisualAppearance(visual->working[i],0xdd8+i*40);compareVisualAppearance(visual->hose,0x1828);compareVisualAppearance(visual->markerAppearance,0x2498);
require(!std::memcmp(visual->markerSize.data(),static_cast<unsigned char*>(nativeActor)+0x24c0,8),"Person marker size differs");
require(!std::memcmp(visual->waterEmitterOffsets.data(),static_cast<unsigned char*>(nativeActor)+0x24c8,sizeof(visual->waterEmitterOffsets)),"Person water emitter offsets differ");
require(!std::memcmp(visual->productTips.data(),static_cast<unsigned char*>(nativeActor)+0x27c8,sizeof(visual->productTips)) && !std::memcmp(visual->seedTips.data(),static_cast<unsigned char*>(nativeActor)+0x27f8,sizeof(visual->seedTips)),"Person held tips offsets differ");
require(!std::memcmp(visual->hotSpots.data(),static_cast<unsigned char*>(nativeActor)+0x28ec,sizeof(visual->hotSpots)),"Person hotspot offsets differ");
for(unsigned i=0;i<10;++i) compareFloat(visual->actionTimes[i],field<float>(nativeActor,0x2860+i*4),"Person action time");
for(unsigned i=0;i<12;++i) {compareFloat(visual->carryHeight[i],field<float>(nativeActor,0x28bc+i*4),"Person carry height");compareFloat(visual->waterAngles[i],field<float>(nativeActor,0x26c8+i*4),"Person water angle");}
for(unsigned row=0;row<11;++row) for(unsigned col=0;col<5;++col) compareFloat(visual->jobHeight[row*6+col],field<float>(nativeActor,0x1850+(row*6+col)*4),"Person job height");
for(unsigned i=0;i<8;++i) require(visual->sounds[i] && field<cResource*>(nativeActor,0x2888+i*4) && visual->sounds[i]->resourceName().view()==field<cResource*>(nativeActor,0x2888+i*4)->resourceName().view(),"Person sound resource name differs");
};checkVisualData();
auto* savedVisualPerson=field<void*>(image,0x25ff5c);putPointer(0x25ff5c,nativeActor);
const Point2 actorPosition{430.125f,310.375f};actor->setPosition(actorPosition);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(nativeActor,&actorPosition);
for(unsigned carry=0;carry<12;++carry) for(unsigned level=0;level<4;++level) for(unsigned facing=0;facing<5;++facing) for(bool mirror:{false,true}) {
    stage=802;trial=carry;action=level*10+facing*2+mirror;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=carry;actionAreaLevels[1]=actionAreaLevels[2]=int(level);visual->direction=field<unsigned>(nativeActor,0x288)=facing;actor->setInvertX(mirror);function<void(__thiscall*)(void*,const bool*)>(0x13bf0)(nativeActor,&mirror);
    visual->playIdle();call(0x50a30,nativeActor);++personVisualActions;checkVisual();
    Point2 nativePoint{};const auto point=visual->hotSpot();function<Point2*(__thiscall*)(void*,Point2*)>(0x4f1a0)(nativeActor,&nativePoint);require(!std::memcmp(point.data(),nativePoint.data(),8),"Person world hotspot differs");
    const auto offset=visual->hotSpotOffset(facing);function<Point2*(__thiscall*)(void*,Point2*,unsigned)>(0x4f250)(nativeActor,&nativePoint,facing);require(!std::memcmp(offset.data(),nativePoint.data(),8),"Person mirrored hotspot differs");
    const auto raw=visual->rawHotSpot(facing);function<Point2*(__thiscall*)(void*,Point2*,unsigned)>(0x4f2e0)(nativeActor,&nativePoint,facing);require(!std::memcmp(raw.data(),nativePoint.data(),8),"Person raw hotspot differs");++personVisualActions;checkVisual();
}
for(unsigned level=0;level<4;++level) for(unsigned job=0;job<11;++job) for(unsigned facing=0;facing<5;++facing) for(bool mirror:{false,true}) {
    stage=803;trial=job;actionAreaLevels[1]=actionAreaLevels[2]=int(level);visual->direction=field<unsigned>(nativeActor,0x288)=facing;actor->setInvertX(mirror);function<void(__thiscall*)(void*,const bool*)>(0x13bf0)(nativeActor,&mirror);
    visual->playJob(job);function<void(__thiscall*)(void*,unsigned)>(0x49ea0)(nativeActor,job);++personVisualActions;checkVisual();
}
// The already-compared configuration is adapted as data; its actual native
// product getters and native seed identity/count getters execute unchanged.
for(unsigned i=0;i<19;++i) {const auto& a=configuration.products[i].icon;auto* p=static_cast<unsigned char*>(nativeGame)+0x30+i*100+16;field<cTexture*>(p,4)=a.texture;field<unsigned>(p,8)=a.frames;field<float>(p,12)=a.length;field<unsigned char>(p,16)=a.loop;field<unsigned>(p,20)=a.columns;field<unsigned>(p,24)=a.rows;field<unsigned char>(p,28)=a.playing;}
auto* nativeCarriedSeed=allocate(1684);std::memset(nativeCarriedSeed,0,1684);field<void*>(nativeActor,0x2848)=nativeCarriedSeed;
const auto priorVisualFloorFeatures=field<unsigned>(image,0x263b80);field<unsigned>(image,0x263b80)=1;
for(unsigned carry:{0U,6U,10U}) for(unsigned level:{0U,2U,3U}) for(unsigned maximum:{0U,1U,4U,6U,9U,16U}) for(unsigned current:{0U,1U,4U,0xffffffffU}) for(unsigned char visible:{0,1,2}) {
    stage=804;trial=carry;action=maximum;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=carry;actionAreaLevels[2]=int(level);
    for(unsigned i=0;i<8;++i) profile.uints["PCC"+std::to_string(i)]=nativeProfile.uints["PCC"+std::to_string(i)]=0;
    if(current==1) profile.uints["PCC3"]=nativeProfile.uints["PCC3"]=4;
    else if(current==4) {profile.uints["PCC0"]=nativeProfile.uints["PCC0"]=0xffffffffU;profile.uints["PCC1"]=nativeProfile.uints["PCC1"]=1;}
    visualWorld.seedKind=field<unsigned>(nativeCarriedSeed,0x64c)=1+level;visualWorld.seedCount=field<unsigned>(nativeCarriedSeed,0x650)=current;
    require(visual->carriedProducts()==function<unsigned(__thiscall*)(void*)>(0x4ed10)(nativeActor),"Person carried product sum differs");
    visual->showDiscrete(visible,current,maximum);function<void(__thiscall*)(void*,unsigned char,unsigned,unsigned)>(0x4ee70)(nativeActor,visible,current,maximum);++personVisualActions;checkVisual();
}
field<unsigned>(image,0x263b80)=priorVisualFloorFeatures;
// Execute the original CRT's real normal acos path, with its documented mode
// flag bypassing uninitialized old error callbacks in this mapped diagnostic.
const auto priorPersonMathMode=field<unsigned>(image,0x261618);field<unsigned>(image,0x261618)=1;
std::vector<Point2> moveDirections{{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,0},{1e-38f,1e-38f},{1e20f,-1e20f}};
constexpr float personPi=3.1415927f;
for(float part:{.125f,.375f,.625f,.875f}) for(float sign:{-1.f,1.f}) {
    const float x=sign*std::sin(personPi*part),y=-std::cos(personPi*part);
    for(float edge:{std::nextafter(x,-INFINITY),x,std::nextafter(x,INFINITY)}) moveDirections.push_back({edge,y});
}
std::mt19937 personDirectionRandom(0x44fe20);for(unsigned i=0;i<64;++i) moveDirections.push_back({float(int(personDirectionRandom()%20001)-10000)*.03125f,float(int(personDirectionRandom()%20001)-10000)*.03125f});
unsigned seenDirections=0;
for(unsigned carry:{0U,1U,2U,6U,10U,11U}) for(unsigned level=0;level<4;++level) for(const auto& delta:moveDirections) {
    stage=805;trial=carry;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=carry;actionAreaLevels[1]=actionAreaLevels[2]=int(level);
    visual->playWalk(delta);function<void(__thiscall*)(void*,const Point2*)>(0x4fe20)(nativeActor,&delta);++personVisualActions;checkVisual();seenDirections|=1U<<visual->direction;
}
require(seenDirections==31,"Person walking did not exercise all original directions");
for(float x:{-80.f,-20.f,-1.f,0.f,1.f,20.f,80.f}) for(float y:{-80.f,-1.f,0.f,1.f,80.f}) {
    stage=806;const Point2 target{actorPosition[0]+x-15,actorPosition[1]+y};visual->playHose(target);function<void(__thiscall*)(void*,const Point2*)>(0x4a330)(nativeActor,&target);++personVisualActions;checkVisual();
}
field<unsigned>(image,0x261618)=priorPersonMathMode;
// Render the recovered actor appearance/UI with an actual Field and actual
// task markers made from this Person XML, retaining the explicit base adapter.
const std::array<void*,2> visualLevelAdapters{field<void*>(image,0x25f74c),field<void*>(image,0x25f754)};
const auto useVisualAdapters=[&] {redirect(0x17050,reinterpret_cast<void*>(&actionAreaLevel));putPointer(0x25f74c,visualLevelAdapters[0]);putPointer(0x25f754,visualLevelAdapters[1]);};
const auto useActualScene=[&] {write(0x17050,visualLevelEntry.data(),5);putPointer(0x25f74c,visualLevelGlobals[0]);putPointer(0x25f754,visualLevelGlobals[1]);};
useActualScene();fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
field<void*>(nativeScenes[0],0)=fieldFarmVtable.data();fieldWorld.held=false;field<unsigned char>(nativeScenes[0],0x108)=0;
profile.ints["fcu"]=nativeProfile.ints["fcu"]=0;profile.ints["fua"]=nativeProfile.ints["fua"]=0;profile.uints["FFC0"]=nativeProfile.uints["FFC0"]=0;profile.uints["FFS0"]=nativeProfile.uints["FFS0"]=3;profile.floats["FFP0"]=nativeProfile.floats["FFP0"]=100;profile.floats["FFW0"]=nativeProfile.floats["FFW0"]=0;profile.floats["FFF0"]=nativeProfile.floats["FFF0"]=100;
auto visualPlot=std::make_unique<FarmField>(nullptr,0,0,0,fieldContext);auto* nvf=allocate(2384);std::memset(nvf,0,2384);fieldNativeIds[nvf]=0;function<void*(__thiscall*)(void*,void*,unsigned,unsigned,int)>(0x3c120)(nvf,nullptr,0,0,0);
const Point2 visualPlotPosition{540,330};visualPlot->setPosition(visualPlotPosition);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(nvf,&visualPlotPosition);
PersonTasksContext visualTasksContext{manager,views,controls,visual->markerAppearance,visual->markerSize};PersonTasks visualTasks(visualTasksContext);fieldWorld.tasks=&visualTasks;
write(0x4d2f0,personTaskAddEntry.data(),5);redirect(0x32c20,reinterpret_cast<void*>(&nativeDiscard));
std::map<View*,void*> visualMarkers;
const auto checkVisualTask=[&](const PersonTask& t,void* n) {require((!t.area && !field<void*>(n,0)) || (t.area==visualPlot.get() && field<void*>(n,0)==nvf),"Person XML scene task area differs");require(!std::memcmp(t.target.data(),static_cast<unsigned char*>(n)+8,8),"Person XML scene target differs");
    auto* marker=field<void*>(n,4);require(bool(t.marker)==bool(marker),"Person XML scene marker missing");if(t.marker) {visualMarkers[t.marker]=marker;compareControl(*t.marker,marker);mainMenuCompareView(*t.marker,marker);}};
const auto checkVisualTasks=[&] {auto* first=field<unsigned char*>(nativeActor,0x282c);auto* last=field<unsigned char*>(nativeActor,0x2830);require(visualTasks.queued().size()==(first ? std::size_t((last-first)/16):0),"Person XML scene queue size differs");for(unsigned i=0;i<visualTasks.queued().size();++i) checkVisualTask(visualTasks.queued()[i],first+i*16);checkVisualTask(visualTasks.current(),static_cast<unsigned char*>(nativeActor)+0x2838);require(visualPlot->clickIndex==field<unsigned>(nvf,0x4fc),"Person XML scene field click count differs");++personTaskStates;};
const auto deleteVisualMarks=[&] {require(views.discarded().size()==nativeDiscards.size(),"Person XML scene discard count differs");for(const auto& item:views.discarded()) require(item.second.view()==nativeDiscards.at(visualMarkers.at(item.first)),"Person XML scene discard origin differs");const auto before=textureCounts();views.deleteDiscarded();const auto middle=textureCounts();for(const auto& item:nativeDiscards) call(0xf1780,item.first);compareReferenceDeltas(before,middle);nativeDiscards.clear();};
for(unsigned i=0;i<3;++i) {visualPlot->enqueueClick();call(0x18110,nvf);++personTaskActions;checkVisualTasks();}
const auto sceneFloor=field<unsigned>(image,0x263b80),sceneMath=field<unsigned>(image,0x261618);field<unsigned>(image,0x263b80)=field<unsigned>(image,0x261618)=1;
{
    const auto hover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
    menuMilliseconds=0;GameTimer timer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor cursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&cursor;
    std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    for(unsigned frame=0;frame<16;++frame) {
        stage=807;action=frame;useVisualAdapters();const unsigned carries[]={0,1,2,6,10,11};const auto carry=carries[frame%6];profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=carry;actionAreaLevels[1]=actionAreaLevels[2]=int(frame%4);profile.uints["PCC3"]=nativeProfile.uints["PCC3"]=carry==6 ? 4:0;profile.uints["PCC0"]=nativeProfile.uints["PCC0"]=profile.uints["PCC1"]=nativeProfile.uints["PCC1"]=0;
        if(frame%4==0) {visual->playIdle();call(0x50a30,nativeActor);}
        else if(frame%4==1) {const Point2 delta{frame&4 ? -1.f:1.f,-.5f};visual->playWalk(delta);function<void(__thiscall*)(void*,const Point2*)>(0x4fe20)(nativeActor,&delta);}
        else if(frame%4==2) {visual->playJob(8);function<void(__thiscall*)(void*,unsigned)>(0x49ea0)(nativeActor,8);}
        else {const Point2 target{actorPosition[0]-40,actorPosition[1]+30};visual->playHose(target);function<void(__thiscall*)(void*,const Point2*)>(0x4a330)(nativeActor,&target);}
        visualWorld.seedCount=field<unsigned>(nativeCarriedSeed,0x650)=frame%5;visual->showDiscrete(carry==6 || carry==10,frame%5,4);function<void(__thiscall*)(void*,unsigned char,unsigned,unsigned)>(0x4ee70)(nativeActor,carry==6 || carry==10,frame%5,4);checkVisual();useActualScene();
        if(frame==4 || frame==8) {auto* t=visualTasks.next();auto* n=function<void*(__thiscall*)(void*)>(0x4e300)(nativeActor);require(bool(t)==bool(n),"Person XML scene FIFO differs");++personTaskActions;checkVisualTasks();deleteVisualMarks();}
        if(frame==12) {visualTasks.clear();call(0x4f870,nativeActor);++personTaskActions;checkVisualTasks();deleteVisualMarks();}
        menuMilliseconds+=50;menuPoint={790,590};menuMouseDelta={};menuInputEvents.clear();personVisualPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++personVisualFrames;checkVisual();checkVisualTasks();
    }
    interaction.playHover=hover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
}
field<unsigned>(image,0x263b80)=sceneFloor;field<unsigned>(image,0x261618)=sceneMath;visualTasks.clear();call(0x4f870,nativeActor);checkVisualTasks();deleteVisualMarks();fieldWorld.tasks=nullptr;
const auto beforeVisualPlot=textureCounts();visualPlot.reset();const auto middleVisualPlot=textureCounts();call(0x36340,nvf);compareReferenceDeltas(beforeVisualPlot,middleVisualPlot);fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;field<void*>(nativeScenes[0],0)=priorFieldFarmVtable;redirect(0x4d2f0,reinterpret_cast<void*>(&actionAreaQueue));redirect(0x32c20,reinterpret_cast<void*>(&waterNativeDiscardTask));
const auto visualReloadDirectory=common/"person-visual-reload";std::filesystem::create_directories(visualReloadDirectory);
std::ofstream(visualReloadDirectory/"missing.xml")<<"<Person/>";
std::ofstream(visualReloadDirectory/"partial.xml")<<"<Person Speed='137.25' HeightOffset='17.5'><AnimationConfig sowTime='0.125'/><States><A><A Picture='person/MoveWithNone/animation_down.jng' frames_count='1' anim_columns='1' anim_rows='1'/></A></States><IdleStates><A><A Picture='person/MoveWithNone/animation_up.jng' frames_count='1' anim_columns='1' anim_rows='1'/></A></IdleStates><HotSpots><A><A spotOffset='-2.25 5.75'/></A></HotSpots><WaterEmitterOfset><A offset='3.5,-7.25'/></WaterEmitterOfset><WaterAngleOfset><A angle='-19.5'/></WaterAngleOfset><CarryHeightOffsets><A height='9.25'/></CarryHeightOffsets><JobHeightOffsets><A><A height='12.5'/></A></JobHeightOffsets><DiscrUse ProgressMaxValue='4' ProgressCurrentValue='1' Step='4,3' MaxLength='17'><Lid Picture='person/mark.jng' size='4 4' Visible='1'/></DiscrUse></Person>";
files.mountCommon(String(visualReloadDirectory.generic_string()));field<unsigned>(image,0x263b80)=field<unsigned>(image,0x261618)=1;
for(const char* path:{"partial.xml","missing.xml","Person/Person.xml","partial.xml","Person/Person.xml"}) {
    stage=808;useVisualAdapters();XmlDocument xml(files,String(path));const auto before=textureCounts();const auto beforeSounds=sounds.size();visual->loadXML(xml);const auto middle=textureCounts();const auto middleSounds=sounds.size();function<void(__thiscall*)(void*,void*)>(0x4b600)(nativeActor,wrap(&xml));pairVisualSounds(beforeSounds,middleSounds);compareReferenceDeltas(before,middle);checkVisual();checkVisualData();++personVisualActions;
    actor->setPosition(actorPosition);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(nativeActor,&actorPosition);visual->playIdle();call(0x50a30,nativeActor);checkVisual();useActualScene();
}
useVisualAdapters();visual->showDiscrete(1,2,4);function<void(__thiscall*)(void*,unsigned char,unsigned,unsigned)>(0x4ee70)(nativeActor,1,2,4);checkVisual();useActualScene();
{
    menuMilliseconds=0;GameTimer timer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor cursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&cursor;
    std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    for(unsigned frame=0;frame<4;++frame) {stage=809;action=frame;menuMilliseconds+=50;menuPoint={790,590};menuMouseDelta={};menuInputEvents.clear();personVisualPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++personVisualFrames;checkVisual();}
    menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
}
field<unsigned>(image,0x263b80)=sceneFloor;field<unsigned>(image,0x261618)=sceneMath;
putPointer(0x25ff5c,savedVisualPerson);
// Actual appearance owners are released before the base View-owned controls.
const auto beforeVisual=textureCounts();visual.reset();const auto middleVisual=textureCounts();
for(unsigned i=0;i<8;++i) field<cResource*>(nativeActor,0x2888+i*4)->releaseReference();
compareVisualSounds();
for(auto pair:std::initializer_list<std::pair<unsigned,unsigned>>{{0x2498,1},{0x1958,72},{0x1828,1},{0xdd8,66},{0x298,72}}) for(unsigned i=pair.second;i;--i) call(0xd2130,static_cast<unsigned char*>(nativeActor)+pair.first+(i-1)*40);
compareReferenceDeltas(beforeVisual,middleVisual);
const auto beforeActor=textureCounts();actor.reset();const auto middleActor=textureCounts();call(0xf1c80,nativeActor);compareReferenceDeltas(beforeActor,middleActor);
write(0x17050,visualLevelEntry.data(),5);redirect(0x50a30,reinterpret_cast<void*>(&waterNativePersonAppearance));actionAreaNativeLevels=oldVisualLevelMap;actionAreaLevels=oldVisualLevels;putPointer(0x25f74c,visualLevelGlobals[0]);putPointer(0x25f754,visualLevelGlobals[1]);
