stage=460;redirect(0x17050,reinterpret_cast<void*>(&actionAreaLevel));redirect(0x4d2f0,reinterpret_cast<void*>(&actionAreaQueue));
actionAreaNativeLevels.clear();
const unsigned areaGlobals[]={0x25f748,0x25f74c,0x25f754,0x25f750};
std::array<void*,4> previousAreaGlobals{};
for(unsigned i=0;i<4;++i) {previousAreaGlobals[i]=field<void*>(image,areaGlobals[i]);auto* observation=allocate(1340);std::memset(observation,0,1340);putPointer(areaGlobals[i],observation);actionAreaNativeLevels[static_cast<unsigned char*>(observation)+0x508]=i;}
// The 256-byte Farm diagnostic base reserves room for its real area vector.
for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
AreaWorldFixture areaWorld(*productionTutorial);
ActionAreaContext areaContext{manager,settings,strings,input,interaction,areaWorld,views,controls};
const char* areaDefinitions[]={"Farm/ActionAreas/Warehouse.xml","Farm/ActionAreas/WaterSource.xml","Farm/ActionAreas/DigBuilding.xml","Farm/ActionAreas/MowBuilding.xml","Farm/ActionAreas/FarmField.xml","Farm/ActionAreas/SeedPackage.xml","Farm/ActionAreas/Sprinkle.xml"};
for(const char* path:areaDefinitions) for(unsigned kind:{1U,2U,4U,8U,16U,32U}) {
    stage=461;trial=actionAreaObjects;cursorPosition={790,590};actualSounds.clear();expectedSounds.clear();
    auto area=std::make_unique<ActionArea>(kind,nullptr,areaContext);auto* n=allocate(1284);std::memset(n,0,1284);function<void*(__thiscall*)(void*,void*,unsigned)>(0x1c010)(n,nullptr,kind);
    auto** begin=field<void**>(nativeScenes[0],0x6c);auto** end=field<void**>(nativeScenes[0],0x70);
    require(areaWorld.registered.size()==std::size_t(end-begin) && end[-1]==n,"ActionArea Farm registration differs");
    XmlDocument definition(files,String(path));area->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0x1cd10)(n,wrap(&definition));
    const auto areaCheck=[&] {
        stage=462;mainMenuCompareView(*area,n);compareControl(*area,n);compareControl(area->overlay(),field<void*>(n,0x500));
        require(area->areaType==field<unsigned>(n,0x364) && area->clickIndex==field<unsigned>(n,0x4fc),"ActionArea type/index differs");
        require(unsigned(area->buttonState())==field<unsigned>(n,0x238),"ActionArea button state differs");compareFloat(area->heightOffset,field<float>(n,0x27c),"ActionArea HeightOffset");
        require(!std::memcmp(area->parkingOffsets.data(),static_cast<unsigned char*>(n)+0x36c,sizeof(area->parkingOffsets)),"ActionArea parking offsets differ");
        auto* offsets=field<Point2*>(n,0x4f0);require(area->clickOffsets.size()==std::size_t(field<Point2*>(n,0x4f4)-offsets),"ActionArea click offset count differs");
        for(unsigned i=0;i<area->clickOffsets.size();++i) require(!std::memcmp(area->clickOffsets[i].data(),offsets[i].data(),8),"ActionArea click offset differs");
        require(area->tipsDirection==field<int>(n,0x2b4) && area->tipsDisabled==bool(field<unsigned char>(n,0x280)),"ActionArea tooltip state differs");
        auto** tips=field<void**>(n,0x288);require(area->tips().size()==std::size_t(field<void**>(n,0x28c)-tips),"ActionArea tooltip count differs");
        for(unsigned i=0;i<area->tips().size();++i) {auto* control=area->tips()[i];compareControl(*control,tips[i]);if(auto* line=dynamic_cast<TextLine*>(control)) {if(line->textDirty()) line->makeLetters();compare(*line,tips[i]);}}
        require(actionAreaSourceTasks==actionAreaNativeTasks,"ActionArea Person task observations differ");actionAreaSourceTasks.clear();actionAreaNativeTasks.clear();
        require(actionAreaSourceQueries==actionAreaNativeQueries,"ActionArea upgrade query order differs");actionAreaSourceQueries.clear();actionAreaNativeQueries.clear();
        require(actualSounds==expectedSounds,"ActionArea sound requests differ");actualSounds.clear();expectedSounds.clear();++actionAreaStates;
    };areaCheck();
    for(unsigned i=0;i<3;++i) {area->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0x1cd10)(n,wrap(&definition));areaCheck();}
    for(unsigned level=0;level<4;++level) for(unsigned carry=0;carry<12;++carry) {
        stage=463;action=carry;actionAreaLevels={int(level),int(level),int(level),int(level)};
        Point2 nativeGoal{};const auto goal=area->goal(carry);function<void(__thiscall*)(void*,Point2*,unsigned)>(0x18800)(n,&nativeGoal,carry);
        require(!std::memcmp(goal.data(),nativeGoal.data(),8),"ActionArea parking goal differs");areaCheck();++actionAreaGoals;
    }
    std::mt19937 areaRandom(0x418800+trial);
    for(unsigned sample=0;sample<128;++sample) {
        for(auto& level:actionAreaLevels) level=int(areaRandom()%4);const unsigned carry=areaRandom()%12;
        Point2 nativeGoal{};const auto goal=area->goal(carry);function<void(__thiscall*)(void*,Point2*,unsigned)>(0x18800)(n,&nativeGoal,carry);
        require(!std::memcmp(goal.data(),nativeGoal.data(),8),"ActionArea independent upgrade goal differs");areaCheck();++actionAreaGoals;
    }
    for(unsigned state=0;state<4;++state) {area->setState(static_cast<ButtonState>(state));function<void(__thiscall*)(void*,unsigned)>(0xf30e0)(n,state);areaCheck();}
    const Point2 shifted{area->geometry().x+0.375f,area->geometry().y-0.625f};area->setPosition(shifted);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(n,&shifted);areaCheck();
    if(!area->clickOffsets.empty()) for(unsigned i=0;i<area->clickOffsets.size()+4;++i) {
        stage=464;action=i;productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=i%3==0 ? 44:-1;
        productionTutorial->cancelRequested=bool(i%2);field<unsigned char>(nativeProductionTutorial,0xc08)=i%2;
        area->clicked();call(0x18c60,n);areaCheck();require(productionTutorial->cancelRequested==bool(field<unsigned char>(nativeProductionTutorial,0xc08)),"ActionArea guide cancellation request differs");++actionAreaClicks;
    }
    productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
    if(!area->clickOffsets.empty()) for(unsigned index:{0U,1U,unsigned(area->clickOffsets.size()-1),unsigned(area->clickOffsets.size()),unsigned(area->clickOffsets.size()+1),0xffffffffU}) {
        area->clickIndex=field<unsigned>(n,0x4fc)=index;area->clicked();call(0x18c60,n);areaCheck();++actionAreaClicks;
    }
    for(float storage:{0.f,23.75f,-12.f}) {area->areaExtentHeight=field<float>(n,0x368)=storage;Point2 extent{};const auto actual=area->areaExtent();function<void(__thiscall*)(void*,Point2*)>(0x17f60)(n,&extent);require(!std::memcmp(actual.data(),extent.data(),8),"ActionArea stored extent differs");}
    if(kind==2) {
        menuMilliseconds=0;GameTimer areaTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor areaCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&areaCursor;
        std::array<unsigned char,24> areaCursorFrame{};field<void*>(areaCursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=areaCursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        area->enable();area->show();call(0xd3bb0,n);call(0xd37d0,n);
        const Point2 at{area->geometry().x,float(originalSubtract(area->geometry().y,area->heightOffset))};
        for(unsigned frame=0;frame<6;++frame) {
            stage=465;action=frame;menuMilliseconds+=50;menuPoint=frame==5 ? Point2{790,590}:at;menuMouseDelta=frame==0 || frame==5 ? std::array<int,3>{1,0,0}:std::array<int,3>{};
            menuInputEvents=!area->clickOffsets.empty() && frame==2 ? std::vector<unsigned>{0x208}:!area->clickOffsets.empty() && frame==3 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
            actionAreaPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++actionAreaFrames;areaCheck();
        }
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }
    const auto before=textureCounts();area.reset();const auto middle=textureCounts();call(0x187a0,n);compareReferenceDeltas(before,middle);++actionAreaObjects;
}
// Missing sections preserve click offsets but clear all parking offsets. The
// row/column limits and case-sensitive Offset/offset names use actual loaders.
const auto areaXmlDirectory=common/"action-area-reload";std::filesystem::create_directories(areaXmlDirectory);
std::ofstream(areaXmlDirectory/"missing.xml")<<"<Area/>";
{std::ofstream xml(areaXmlDirectory/"limits.xml");xml<<"<Area OverlayColor=\"80402010\"><ClickOffsetList><P Offset=\"1.25,-3.5\"/></ClickOffsetList><ParkingOffsets>";
 for(unsigned row=0;row<6;++row) {xml<<"<Row>";for(unsigned column=0;column<14;++column) xml<<"<P offset=\""<<row<<','<<column<<"\"/>";xml<<"</Row>";}xml<<"</ParkingOffsets></Area>";}
files.mountCommon(String(areaXmlDirectory.generic_string()));
{
    auto area=std::make_unique<ActionArea>(2,nullptr,areaContext);auto* n=allocate(1284);std::memset(n,0,1284);function<void*(__thiscall*)(void*,void*,unsigned)>(0x1c010)(n,nullptr,2);
    for(const char* path:{"limits.xml","missing.xml","limits.xml","limits.xml","missing.xml"}) {
        XmlDocument definition(files,String(path));area->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0x1cd10)(n,wrap(&definition));
        compareControl(*area,n);compareControl(area->overlay(),field<void*>(n,0x500));require(!std::memcmp(area->parkingOffsets.data(),static_cast<unsigned char*>(n)+0x36c,sizeof(area->parkingOffsets)),"ActionArea partial/clamped parking reload differs");
        auto* first=field<Point2*>(n,0x4f0);require(area->clickOffsets.size()==std::size_t(field<Point2*>(n,0x4f4)-first),"ActionArea partial click reload differs");for(unsigned i=0;i<area->clickOffsets.size();++i) require(!std::memcmp(area->clickOffsets[i].data(),first[i].data(),8),"ActionArea comma offset differs");++actionAreaStates;++actionAreaReloads;
    }
    const auto before=textureCounts();area.reset();const auto middle=textureCounts();call(0x187a0,n);compareReferenceDeltas(before,middle);++actionAreaObjects;
}
for(unsigned i=0;i<4;++i) putPointer(areaGlobals[i],previousAreaGlobals[i]);
actionAreaNativeLevels.clear();
