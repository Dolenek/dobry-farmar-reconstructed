stage=660;loadingFiles=&files;managerTable[6]=reinterpret_cast<void*>(&loadingXML);xmlTable[0]=reinterpret_cast<void*>(&loadingDeleteXML);xmlTable[34]=reinterpret_cast<void*>(&sectionCount);redirect(0x185a13,reinterpret_cast<void*>(&gameplayGetThreadData));
const auto discard=+[](void* particle,const String* origin) {nativeDiscards[particle]=std::string(origin->view());};redirect(0x43ca0,reinterpret_cast<void*>(discard));redirect(0x62af0,reinterpret_cast<void*>(discard));
redirect(0x2fe80,reinterpret_cast<void*>(&decimalString));redirect(0x1cc0,reinterpret_cast<void*>(&concatenateString));
for(bool hose:{true,false}) {
    ViewRegistry views;ControlRegistry controls;GameRandom random(0x12345678);field<unsigned>(gameplayThreadData.data(),0x14)=random.state();
    std::unique_ptr<HoseEmitter> he;std::unique_ptr<SprinkleEmitter> se;
    const String path(hose ? "Effects/HoseEffect.xml":"Effects/SprinkleEffect.xml");
    if(hose) he=std::make_unique<HoseEmitter>(nullptr,manager,random,path,views,controls);else se=std::make_unique<SprinkleEmitter>(nullptr,manager,random,path,views,controls);
    WaterEmitter& emitter=hose ? static_cast<WaterEmitter&>(*he):static_cast<WaterEmitter&>(*se);
    auto* n=allocate(hose ? 648:628);std::memset(n,0,hose ? 648:628);function<void*(__thiscall*)(void*,void*,const String*)>(hose ? 0x45420:0x64270)(n,nullptr,&path);
    std::map<Picture*,void*> particlePairs;
    struct ParticleLink {ParticleLink *next,*previous;void* value;};
    const auto checkEmitter=[&] {
        stage=661;mainMenuCompareView(emitter,n);compareControl(emitter,n);const auto& s=emitter.emission;
        compareFloat(s.accumulated,field<float>(n,0x22c),"WaterEmitter accumulated");compareFloat(s.interval,field<float>(n,0x244),"WaterEmitter interval");compareFloat(s.timeLimit,field<float>(n,0x248),"WaterEmitter time limit");compareFloat(s.timeGenerated,field<float>(n,0x24c),"WaterEmitter generation time");
        require(s.maximumCount==field<unsigned>(n,0x230) && s.createdCount==field<unsigned>(n,0x234) && s.mode==field<unsigned>(n,0x25c),"WaterEmitter generation counters differ");require(s.generating==bool(field<unsigned char>(n,0x250)) && s.infinite==bool(field<unsigned char>(n,0x251)),"WaterEmitter generation flags differ");
        auto* proto=field<void*>(n,0x260);if(hose) {checkParticle(he->prototype(),he->prototype().motion,proto,true);compareFloat(he->speed[0],field<float>(n,0x268),"HoseEmitter speed0");compareFloat(he->speed[1],field<float>(n,0x26c),"HoseEmitter speed1");compareFloat(he->distortionRadius,field<float>(n,0x270),"HoseEmitter radius");}
        else {checkParticle(se->prototype(),se->prototype().motion,proto,false);compareFloat(se->lifetimeRange[0],field<float>(n,0x264),"SprinkleEmitter life0");compareFloat(se->lifetimeRange[1],field<float>(n,0x268),"SprinkleEmitter life1");compareFloat(se->randomAngle,field<float>(n,0x270),"SprinkleEmitter angle");}
        require(random.state()==field<unsigned>(gameplayThreadData.data(),0x14),"WaterEmitter random sequence differs");
        require(emitter.particles().size()==field<unsigned>(n,0x240),"WaterEmitter particle list count differs");auto* head=field<ParticleLink*>(n,0x23c);auto* link=head->next;
        for(auto* p:emitter.particles()) {require(link!=head && link->next->previous==link,"WaterEmitter particle links differ");require(p->parent()==&emitter && field<void*>(link->value,0x20)==n,"WaterEmitter particle parent differs");auto found=particlePairs.find(p);if(found==particlePairs.end()) particlePairs[p]=link->value;else require(found->second==link->value,"WaterEmitter particle order differs");
            if(hose) {auto& part=static_cast<HoseParticle&>(*p);checkParticle(part,part.motion,link->value,true);compareFloat(part.gravity,field<float>(link->value,0x240),"HoseEmitter live gravity");}else {auto& part=static_cast<SprinkleParticle&>(*p);checkParticle(part,part.motion,link->value,false);}link=link->next;
        }require(link==head,"WaterEmitter extra native particle");
        require(views.discarded().size()==nativeDiscards.size(),"WaterEmitter deferred discard count differs");for(const auto& pair:particlePairs) {require(views.discarded().count(pair.first)==nativeDiscards.count(pair.second),"WaterEmitter deferred discard membership differs");if(views.discarded().count(pair.first)) require(views.discarded().at(pair.first).view()==nativeDiscards.at(pair.second),"WaterEmitter discard origin differs");}
        ++waterEmitterStates;
    };checkEmitter();
    if(hose) {he->angle=field<float>(n,0x264)=0;he->startPoint={10,20};he->targetPoint={200,150};std::memcpy(static_cast<unsigned char*>(n)+0x278,he->startPoint.data(),8);std::memcpy(static_cast<unsigned char*>(n)+0x280,he->targetPoint.data(),8);}
    emitter.setPosition({300,200});Point2 pos{300,200};function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(n,&pos);
    for(unsigned i=0;i<128;++i) {
        stage=662;std::unique_ptr<HoseParticle> hp;std::unique_ptr<SprinkleParticle> sp;if(hose) {he->angle=field<float>(n,0x264)=float(i%7)*15;hp.reset(he->createParticle());}else sp.reset(se->createParticle());
        auto* np=function<void*(__thiscall*)(void*)>(hose ? 0x42820:0x61820)(n);require(np,"WaterEmitter unexpectedly returned null particle");Picture& p=hose ? static_cast<Picture&>(*hp):static_cast<Picture&>(*sp);checkParticle(p,hose ? hp->motion:sp->motion,np,hose);
        if(hose) compareFloat(hp->gravity,field<float>(np,0x240),"HoseEmitter created gravity");checkEmitter();++waterEmitterCreations;
        const auto before=textureCounts();hp.reset();sp.reset();const auto middle=textureCounts();function<void*(__thiscall*)(void*,unsigned)>(0x41770)(np,0);compareReferenceDeltas(before,middle);
    }
    emitter.start(0.3f);function<void(__thiscall*)(void*,float)>(0x2ccc0)(n,0.3f);checkEmitter();
    {
        menuMilliseconds=0;GameTimer emitterTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor emitterCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&emitterCursor;
        std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned frame=0;frame<40;++frame) {stage=663;action=frame;menuMilliseconds+=50;menuPoint={790,590};menuMouseDelta={};menuInputEvents.clear();waterEmitterPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++waterEmitterFrames;checkEmitter();}
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }
    require(!emitter.emission.generating && emitter.particles().empty(),"WaterEmitter timed stream did not drain");
    const auto deletePending=[&] {const auto before=textureCounts();views.deleteDiscarded();const auto middle=textureCounts();std::vector<void*> pending;for(const auto& entry:nativeDiscards) pending.push_back(entry.first);for(auto* p:pending) mainMenuDeleteNative(p);compareReferenceDeltas(before,middle);checkEmitter();};deletePending();
    for(unsigned mode=0;mode<2;++mode) {
        stage=664;if(mode==0) {emitter.emission.accumulated=field<float>(n,0x22c)=emitter.emission.interval;emitter.emission.maximumCount=field<unsigned>(n,0x230)=4;emitter.emission.createdCount=field<unsigned>(n,0x234)=0;emitter.emission.mode=field<unsigned>(n,0x25c)=0;emitter.emission.infinite=false;field<unsigned char>(n,0x251)=0;emitter.emission.generating=true;field<unsigned char>(n,0x250)=1;}
        else {emitter.start(0);function<void(__thiscall*)(void*,float)>(0x2ccc0)(n,0);}checkEmitter();
        menuMilliseconds=0;GameTimer emitterTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor emitterCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&emitterCursor;
        std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned frame=0;frame<24;++frame) {stage=665;action=frame;if(mode==1 && frame==4) {emitter.emission.generating=false;field<unsigned char>(n,0x250)=0;}menuMilliseconds+=50;menuPoint={790,590};menuMouseDelta={};menuInputEvents.clear();waterEmitterPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++waterEmitterFrames;checkEmitter();deletePending();}
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
        require(!emitter.emission.generating,"WaterEmitter count/infinite generation did not stop");if(mode==0) require(emitter.emission.createdCount==4,"WaterEmitter count stream did not create four particles");
    }
    const auto before=textureCounts();he.reset();se.reset();views.deleteDiscarded();const auto middle=textureCounts();function<void*(__thiscall*)(void*,unsigned)>(hose ? 0x45540:0x64380)(n,0);compareReferenceDeltas(before,middle);nativeDiscards.clear();++waterEmitterObjects;
}
