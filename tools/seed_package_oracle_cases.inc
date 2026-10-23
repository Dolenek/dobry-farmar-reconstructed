// First restore the actual SeedPackage Use control. The full SeedPackage
// controller is the next integration step; this does not substitute for it.
stage=740;fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
// The host supports the original CRT's SSE2 floor fast path. Its real body
// and data operands execute; its uninitialized old CRT fallback is not used.
const auto priorFloorFeatures=field<unsigned>(image,0x263b80);field<unsigned>(image,0x263b80)=1;
ActionAreaContext seedAreaContext{manager,settings,strings,input,interaction,fieldWorld,views,controls};
auto seedArea=std::make_unique<ActionArea>(32,nullptr,seedAreaContext);auto* nativeSeedArea=allocate(1284);std::memset(nativeSeedArea,0,1284);function<void*(__thiscall*)(void*,void*,unsigned)>(0x1c010)(nativeSeedArea,nullptr,32);
XmlDocument seedDefinition(files,String("Farm/ActionAreas/SeedPackage.xml"));seedArea->loadXML(seedDefinition);function<void(__thiscall*)(void*,void*)>(0x1cd10)(nativeSeedArea,wrap(&seedDefinition));
auto discrete=std::make_unique<ProgressDiscrete>(seedArea.get(),manager,views,controls);auto* nativeDiscrete=allocate(604);std::memset(nativeDiscrete,0,604);function<void*(__thiscall*)(void*,void*)>(0x8d3d0)(nativeDiscrete,nativeSeedArea);
XmlNode* use=nullptr;require(seedDefinition.section(String("Use"),use),"Actual SeedPackage Use XML missing");discrete->loadXML(*use);function<void(__thiscall*)(void*,void*)>(0x8d640)(nativeDiscrete,wrap(use));
const Point2 seedPosition{350,260};seedArea->setPosition(seedPosition);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(nativeSeedArea,&seedPosition);discrete->show();call(0xd37d0,nativeDiscrete);
const auto checkUse=[&](ProgressDiscrete& d,void* n) {
    stage=741;mainMenuCompareView(d,n);compareControl(d,n);
    require(d.current==field<unsigned>(n,0x22c),"ProgressDiscrete current differs");compareFloat(d.deepStep,field<float>(n,0x230),"ProgressDiscrete depth step");compareFloat(d.maxLength,field<float>(n,0x23c),"ProgressDiscrete max length");
    for(unsigned i=0;i<2;++i) compareFloat(d.step[i],field<float>(n,0x234+i*4),"ProgressDiscrete step");
    compareControl(d.pictureOn(),field<void*>(n,0x244));compareControl(d.pictureOff(),field<void*>(n,0x248));
    auto** first=field<void**>(n,0x250);auto** last=field<void**>(n,0x254);require(d.pictures().size()==std::size_t(last-first),"ProgressDiscrete picture count differs");
    for(unsigned i=0;i<d.pictures().size();++i) {auto& p=*d.pictures()[i];auto* native=first[i];mainMenuCompareView(p,native);compareControl(p,native);
        require(p.renderFlags()==field<unsigned>(native,0x6c) && p.playing()==bool(field<unsigned char>(native,0x48)) && p.animation().doubleRender==bool(field<unsigned char>(native,0x1c6)),"Visual copy preserved properties differ");
        compareFloat(p.geometry().color.r,field<float>(native,0x218),"Visual copy red");compareFloat(p.geometry().color.g,field<float>(native,0x21c),"Visual copy green");compareFloat(p.geometry().color.b,field<float>(native,0x220),"Visual copy blue");
    }
    require(!controls.contains(&d),"ProgressDiscrete root was registered for drawing");++progressDiscreteStates;
};const auto checkDiscrete=[&] {checkUse(*discrete,nativeDiscrete);};checkDiscrete();
for(unsigned count:{0U,1U,2U,4U,7U,16U,4U}) {
    discrete->setMaximum(count);function<void(__thiscall*)(void*,unsigned)>(0x8d4e0)(nativeDiscrete,count);checkDiscrete();
    for(unsigned current:{0U,1U,count,count+1,0xffffffffU}) {discrete->current=field<unsigned>(nativeDiscrete,0x22c)=current;discrete->refresh();call(0x8c9e0,nativeDiscrete);checkDiscrete();}
}
for(const Point2 spacing:{Point2{10,0},Point2{0,10},Point2{-10,0},Point2{3,4},Point2{-2.5f,-7.5f},Point2{0,0}}) for(float length:{0.f,1.f,3.5f,32.f,100.f}) {
    discrete->step=spacing;std::memcpy(static_cast<unsigned char*>(nativeDiscrete)+0x234,spacing.data(),8);discrete->maxLength=field<float>(nativeDiscrete,0x23c)=length;
    for(unsigned current=0;current<6;++current) {discrete->current=field<unsigned>(nativeDiscrete,0x22c)=current;discrete->refresh();call(0x8c9e0,nativeDiscrete);checkDiscrete();}
}
// Actual reload with the same count intentionally leaves the existing child
// appearances/coordinates until the next explicit refresh; XML appends a Lid.
const auto discreteDirectory=common/"seed-use-reload";std::filesystem::create_directories(discreteDirectory);
std::ofstream(discreteDirectory/"colored.xml")<<"<Use Visible='1' ProgressMaxValue='4' ProgressCurrentValue='2' Step='7 3' MaxLength='19' DeepStep='0.2'><PictureOn size='11 9' Picture='Farm/ActionAreas/SeedPackage/shar_on.jng' color='80402080' DoubleAnimRender='1'/><PictureOff size='8 10' Picture='Farm/ActionAreas/SeedPackage/shar_back.jng' color='C0208040'/><Lid Visible='1' size='4 4' Picture='Farm/ActionAreas/SeedPackage/shar_on.jng'/></Use>";
files.mountCommon(String(discreteDirectory.generic_string()));
for(unsigned reload=0;reload<2;++reload) {XmlDocument xml(files,String("colored.xml"));discrete->loadXML(xml);function<void(__thiscall*)(void*,void*)>(0x8d640)(nativeDiscrete,wrap(&xml));checkDiscrete();discrete->refresh();call(0x8c9e0,nativeDiscrete);checkDiscrete();}
discrete->loadXML(*use);function<void(__thiscall*)(void*,void*)>(0x8d640)(nativeDiscrete,wrap(use));discrete->refresh();call(0x8c9e0,nativeDiscrete);discrete->show();call(0xd37d0,nativeDiscrete);checkDiscrete();
{
    menuMilliseconds=0;GameTimer timer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor cursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&cursor;
    std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    for(unsigned frame=0;frame<16;++frame) {stage=742;discrete->current=field<unsigned>(nativeDiscrete,0x22c)=frame%6;discrete->refresh();call(0x8c9e0,nativeDiscrete);menuMilliseconds+=50;menuPoint={790,590};menuMouseDelta={};menuInputEvents.clear();progressDiscretePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++progressDiscreteFrames;checkDiscrete();}
    menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
}
// Both prototypes are manually owned, while all displayed/retained children
// (including reload lids) follow the real View tree through destruction.
const auto beforeDiscrete=textureCounts();discrete.reset();const auto middleDiscrete=textureCounts();call(0x8cbc0,nativeDiscrete);compareReferenceDeltas(beforeDiscrete,middleDiscrete);
const auto beforeSeedArea=textureCounts();seedArea.reset();const auto middleSeedArea=textureCounts();call(0x187a0,nativeSeedArea);compareReferenceDeltas(beforeSeedArea,middleSeedArea);
fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
#include "seed_package_controller_oracle_cases.inc"
field<unsigned>(image,0x263b80)=priorFloorFeatures;
