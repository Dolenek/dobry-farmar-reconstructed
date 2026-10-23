// Actual queue bodies execute on a data adapter for the Person fields they use.
// Both Area clicks and SeedPackage releases now create actual owned Pictures.
stage=770;auto savedPersonBytes=waterPerson;std::memset(waterPerson.data()+0x2828,0,0x20);
write(0x4d2f0,personTaskAddEntry.data(),5);redirect(0x32c20,reinterpret_cast<void*>(&nativeDiscard));
XmlDocument personXML(files,String("Person/Person.xml"));XmlNode* markerXML=nullptr;require(personXML.section(String("moveMarkState"),markerXML),"Actual Person moveMarkState missing");
AnimableAppearance markerAppearance;markerAppearance.loadXML(*markerXML,manager);Point2 markerSize{};
{const auto text=personXML.attribute(WString(L"MarkSize"));std::wistringstream stream(text.c_str());stream>>markerSize[0]>>markerSize[1];require(bool(stream),"Actual Person MarkSize missing");}
auto* nativeAppearance=waterPerson.data()+0x2498;call(0xd2080,nativeAppearance);function<void(__thiscall*)(void*,void*)>(0xd22a0)(nativeAppearance,wrap(markerXML));std::memcpy(waterPerson.data()+0x24c0,markerSize.data(),8);
PersonTasksContext tasksContext{manager,views,controls,markerAppearance,markerSize};PersonTasks tasks(tasksContext);fieldWorld.tasks=&tasks;
fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
field<void*>(nativeScenes[0],0)=fieldFarmVtable.data();fieldWorld.held=false;field<unsigned char>(nativeScenes[0],0x108)=0;
profile.ints["fcu"]=nativeProfile.ints["fcu"]=0;profile.ints["fua"]=nativeProfile.ints["fua"]=0;profile.uints["FFC0"]=nativeProfile.uints["FFC0"]=0;profile.uints["FFS0"]=nativeProfile.uints["FFS0"]=3;
profile.floats["FFP0"]=nativeProfile.floats["FFP0"]=100;profile.floats["FFW0"]=nativeProfile.floats["FFW0"]=0;profile.floats["FFF0"]=nativeProfile.floats["FFF0"]=100;
auto plot=std::make_unique<FarmField>(nullptr,0,0,0,fieldContext);auto* nf=allocate(2384);std::memset(nf,0,2384);fieldNativeIds[nf]=0;function<void*(__thiscall*)(void*,void*,unsigned,unsigned,int)>(0x3c120)(nf,nullptr,0,0,0);
const Point2 fieldPosition{540,320};plot->setPosition(fieldPosition);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(nf,&fieldPosition);
cursorPosition={330,250};auto package=std::make_unique<SeedPackage>(nullptr,1,4,cursorPosition,2,false,seedContext);auto* np=allocate(1684);std::memset(np,0,1684);function<void*(__thiscall*)(void*,void*,unsigned,unsigned,const Point2*,unsigned,bool)>(0x5e130)(np,nullptr,1,4,&cursorPosition,2,false);package->clicked();call(0x5d660,np);checkSeed(*package,np);
std::map<ActionArea*,void*> taskAreas{{plot.get(),nf},{package.get(),np}};std::map<View*,void*> taskMarkers;
const auto compareTask=[&](const PersonTask& t,void* n) {
    const auto area=field<void*>(n,0),marker=field<void*>(n,4);require((!t.area && !area) || (t.area && taskAreas.at(t.area)==area),"Actual Person task area differs");
    require(!std::memcmp(t.target.data(),static_cast<unsigned char*>(n)+8,8),"Actual Person stored task target differs");require(bool(t.marker)==bool(marker),"Actual Person task marker presence differs");
    if(t.marker) {auto found=taskMarkers.find(t.marker);if(found==taskMarkers.end()) taskMarkers.emplace(t.marker,marker);else require(found->second==marker,"Actual Person marker ownership differs");compareControl(*t.marker,marker);mainMenuCompareView(*t.marker,marker);}
};
const auto checkTasks=[&] {
    stage=771;auto* begin=field<unsigned char*>(waterPerson.data(),0x282c);auto* end=field<unsigned char*>(waterPerson.data(),0x2830);
    require(tasks.queued().size()==(begin ? std::size_t((end-begin)/16):0),"Actual Person queue length differs");for(unsigned i=0;i<tasks.queued().size();++i) compareTask(tasks.queued()[i],begin+i*16);compareTask(tasks.current(),waterPerson.data()+0x2838);
    require(tasks.empty()==bool(function<unsigned char(__thiscall*)(void*)>(0x49930)(waterPerson.data())),"Actual Person empty excludes-current behavior differs");
    for(const auto& area:taskAreas) require(area.first->clickIndex==field<unsigned>(area.second,0x4fc),"Actual Person Area clickIndex differs");
    require(views.discarded().size()==nativeDiscards.size(),"Actual Person deferred discard count differs");for(const auto& item:views.discarded()) require(item.second.view()==nativeDiscards.at(taskMarkers.at(item.first)),"Actual Person discard origin differs");++personTaskStates;
};
const auto discardTaskMarkers=[&] {
    const auto before=textureCounts();views.deleteDiscarded();const auto middle=textureCounts();for(const auto& item:nativeDiscards) call(0xf1780,item.first);compareReferenceDeltas(before,middle);nativeDiscards.clear();
};
const auto nextTask=[&] {auto* t=tasks.next();auto* n=function<void*(__thiscall*)(void*)>(0x4e300)(waterPerson.data());require(bool(t)==bool(n),"Actual Person next result differs");++personTaskActions;checkTasks();discardTaskMarkers();};
const auto clickPlot=[&] {plot->enqueueClick();call(0x18110,nf);++personTaskActions;checkTasks();};
const auto clickPackage=[&] {cursorPosition={330,250};package->clicked();call(0x5d660,np);package->unclicked();call(0x5dbe0,np);++personTaskActions;checkSeed(*package,np);checkTasks();};
checkTasks();nextTask();
clickPlot();clickPackage();clickPlot();clickPackage();clickPlot();clickPackage();clickPlot();
// FIFO keeps stored targets while completion repositions only marker Pictures.
nextTask();nextTask();nextTask();
tasks.removeQueued(*package);function<void(__thiscall*)(void*,void*)>(0x4dd80)(waterPerson.data(),np);++personTaskActions;checkTasks();discardTaskMarkers();
clickPlot();nextTask();
// Cancellation discards the current marker first, then queued marks in order.
tasks.clear();call(0x4f870,waterPerson.data());++personTaskActions;checkTasks();discardTaskMarkers();nextTask();
// Unsigned wrap and original unspilled depth arithmetic, including high counts.
for(unsigned count:{0U,1U,0x7fffffffU,0x80000000U,0xffffffffU}) {
    plot->clickIndex=field<unsigned>(nf,0x4fc)=count;const Point2 target{401.25f,299.75f};tasks.add(*plot,target);function<void(__thiscall*)(void*,void*,const Point2*)>(0x4d2f0)(waterPerson.data(),nf,&target);++personTaskActions;checkTasks();nextTask();tasks.clear();call(0x4f870,waterPerson.data());++personTaskActions;checkTasks();discardTaskMarkers();
}
plot->clickIndex=field<unsigned>(nf,0x4fc)=0;package->clickIndex=field<unsigned>(np,0x4fc)=0;clickPlot();clickPackage();clickPlot();
nextTask();
const Point2 translated{552.125f,311.875f};plot->setPosition(translated);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(nf,&translated);
tasks.finishCurrent();call(0x4d790,waterPerson.data());++personTaskActions;checkTasks();discardTaskMarkers();
// Removing an area from the waiting vector never removes its current task.
clickPlot();nextTask();nextTask();tasks.removeQueued(*plot);function<void(__thiscall*)(void*,void*)>(0x4dd80)(waterPerson.data(),nf);++personTaskActions;checkTasks();discardTaskMarkers();
require(tasks.current().area==plot.get(),"Removing queued area unexpectedly cancelled current task");
tasks.clear();call(0x4f870,waterPerson.data());++personTaskActions;checkTasks();discardTaskMarkers();
plot->clickIndex=field<unsigned>(nf,0x4fc)=0;package->clickIndex=field<unsigned>(np,0x4fc)=0;
const auto beforeInputTasks=tasks.queued().size();
{
    const auto hover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
    menuMilliseconds=0;GameTimer timer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor cursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&cursor;
    std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    for(unsigned frame=0;frame<8;++frame) {stage=772;action=frame;menuMilliseconds+=50;menuPoint=frame<3 ? translated:frame<6 ? Point2{330,250}:Point2{790,590};menuMouseDelta={1,0,0};menuInputEvents=frame==1 || frame==4 ? std::vector<unsigned>{0x208}:frame==2 || frame==5 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};personTaskPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++personTaskFrames;checkTasks();}
    interaction.playHover=hover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
}
require(tasks.queued().size()>=beforeInputTasks+2,"Actual Field and Seed mouse input did not enqueue Person tasks");
fieldWorld.held=false;field<unsigned char>(nativeScenes[0],0x108)=0;fieldTip->hideFor(plot.get());function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,nf);
tasks.clear();call(0x4f870,waterPerson.data());++personTaskActions;checkTasks();discardTaskMarkers();
fieldWorld.tasks=nullptr;field<void*>(nativeScenes[0],0)=priorFieldFarmVtable;
seedWorld.setSeedMoveActive(false);seedWorld.selected=nullptr;field<unsigned char>(image,0x25f721)=0;putPointer(0x25f724,nullptr);
const auto beforePackage=textureCounts();package.reset();const auto middlePackage=textureCounts();call(0x5ce00,np);compareReferenceDeltas(beforePackage,middlePackage);
const auto beforePlot=textureCounts();plot.reset();const auto middlePlot=textureCounts();call(0x36340,nf);compareReferenceDeltas(beforePlot,middlePlot);
call(0xd2130,nativeAppearance);waterPerson=savedPersonBytes;
fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
redirect(0x4d2f0,reinterpret_cast<void*>(&actionAreaQueue));redirect(0x32c20,reinterpret_cast<void*>(&waterNativeDiscardTask));
