stage=670;loadingFiles=&files;managerTable[6]=reinterpret_cast<void*>(&loadingXML);xmlTable[0]=reinterpret_cast<void*>(&loadingDeleteXML);xmlTable[34]=reinterpret_cast<void*>(&sectionCount);
redirect(0x185a13,reinterpret_cast<void*>(&gameplayGetThreadData));redirect(0x2fe80,reinterpret_cast<void*>(&decimalString));redirect(0x1cc0,reinterpret_cast<void*>(&concatenateString));
redirect(0x326c0,reinterpret_cast<void*>(&sprinkleCollect));redirect(0x2fa90,reinterpret_cast<void*>(&sprinklePreviewClear));redirect(0x2c7e0,reinterpret_cast<void*>(&sprinkleOverlay<true>));redirect(0x2c7f0,reinterpret_cast<void*>(&sprinkleOverlay<false>));
redirect(0x2df60,reinterpret_cast<void*>(&sprinkleAreaAt));redirect(0x2e3a0,reinterpret_cast<void*>(&sprinkleBuildingAt));redirect(0x2d290,reinterpret_cast<void*>(&sprinklePlaceAt));redirect(0x33de0,reinterpret_cast<void*>(&sprinkleRemove));redirect(0x36880,reinterpret_cast<void*>(&sprinklePreview));redirect(0x51680,reinterpret_cast<void*>(&sprinkleWater));redirect(0x31950,reinterpret_cast<void*>(&sprinkleAreasAt));
const auto discardSprinkleParticle=+[](void* p,const String* origin) {nativeDiscards[p]=std::string(origin->view());};redirect(0x62af0,reinterpret_cast<void*>(discardSprinkleParticle));
struct SprinkleFixture final:SprinkleWorld {
    WaterFixture& water;Sprinkle* selected=nullptr;std::vector<ActionArea*> selectedFields;
    explicit SprinkleFixture(WaterFixture& w):water(w) {}
    TutorialManager& tutorial() override {return water.tutorial();}
    void registerArea(ActionArea& a) override {water.registerArea(a);}void queueAreaAction(ActionArea& a,const Point2& p) override {water.queueAreaAction(a,p);}
    int warehouseLevel() override {return water.warehouseLevel();}int digBuildingLevel() override {return water.digBuildingLevel();}int mowBuildingLevel() override {return water.mowBuildingLevel();}int waterSourceLevel() override {return water.waterSourceLevel();}
    bool sprinkleMoveActive() override {return water.sprinkleMove;}void setSprinkleMoveActive(bool v) override {water.sprinkleMove=v;}
    ActionArea* areaAt(const Point2& p,unsigned mask) override {sprinkleSourceEvents.push_back({4,wordOf(p[0]),wordOf(p[1]),mask,0});return selected;}
    bool buildingAt(const Point2&) override {return sprinkleBuilding;}
    std::vector<ActionArea*> areasAt(const Point2&,unsigned) override {return selectedFields;}
    bool canPlaceSprinkle(Sprinkle&,const Point2& p) override {sprinkleSourceEvents.push_back({5,wordOf(p[0]),wordOf(p[1]),0,0});return sprinklePlace;}
    std::vector<ActionArea*> wateredFields(const Point2& p,float r) override {sprinkleSourceEvents.push_back({1,wordOf(p[0]),wordOf(p[1]),wordOf(r),0});return selectedFields;}
    void clearWaterPreview() override {sprinkleSourceEvents.push_back({2,0,0,0,0});}
    void previewWater(ActionArea& a,bool value) override {sprinkleSourceEvents.push_back({7,unsigned(&a==selectedFields.at(0) ? 0:1),value,0,0});}
    double addWater(ActionArea& a,float amount) override {sprinkleSourceEvents.push_back({8,unsigned(&a==selectedFields.at(0) ? 0:1),wordOf(amount),0,0});return 0;}
    void setFarmActionButtonEnabled(bool v) override {sprinkleSourceEvents.push_back({3,!v,0,0,0});}
    int money() override {return water.money();}void setMoney(int m) override {water.setMoney(m);}
    void removeSprinkle(Sprinkle&) override {sprinkleSourceEvents.push_back({6,0,0,0,0});}
} sprinkleWorld(waterWorld);
GameRandom sprinkleRandom(0x12345678);field<unsigned>(gameplayThreadData.data(),0x14)=sprinkleRandom.state();
SprinkleContext sprinkleContext{{manager,settings,strings,input,interaction,sprinkleWorld,views,controls},profile,sprinkleRandom,sprinkleWorld};
// Field geometry is actual ActionArea; cField watering/preview is an explicit
// remaining boundary. Future full Farm tests must replace these observations.
std::array<std::unique_ptr<ActionArea>,2> sprinklerFields;std::array<void*,2> nf{};
for(unsigned i=0;i<2;++i) {sprinklerFields[i]=std::make_unique<ActionArea>(1,nullptr,sprinkleContext.area);nf[i]=allocate(1284);std::memset(nf[i],0,1284);function<void*(__thiscall*)(void*,void*,unsigned)>(0x1c010)(nf[i],nullptr,1);sprinkleNativeIds[nf[i]]=i;}
const auto checkSprinkle=[&](Sprinkle& s,void* n) {
    stage=671;mainMenuCompareView(s,n);compareControl(s,n);require(s.id==field<unsigned>(n,0x504) && s.installed==bool(field<unsigned char>(n,0x50c)),"Sprinkle identity/install flag differs");compareFloat(s.radius,field<float>(n,0x508),"Sprinkle radius");
    for(unsigned i=0;i<2;++i) {compareFloat(s.pressPosition[i],field<float>(n,0x510+4*i),"Sprinkle press point");compareFloat(s.dragOffset[i],field<float>(n,0x518+4*i),"Sprinkle drag offset");}
    require(!std::memcmp(&s.normalColor,static_cast<unsigned char*>(n)+0x540,16) && !std::memcmp(&s.unfitColor,static_cast<unsigned char*>(n)+0x550,16),"Sprinkle colors differ");
    auto* ne=field<void*>(n,0x520);compareControl(s.effect(),ne);mainMenuCompareView(s.effect(),ne);if(s.effect().emission.generating!=bool(field<unsigned char>(ne,0x250))) std::cerr<<"sprinkle status "<<s.status()<<" native "<<nativeProfile.uints["SPStatus"+std::to_string(s.id)]<<" fields "<<s.fields().size()<<" generation "<<s.effect().emission.generating<<' '<<unsigned(field<unsigned char>(ne,0x250))<<'\n';require(s.effect().emission.generating==bool(field<unsigned char>(ne,0x250)),"Sprinkle emitter generation differs");
    require(sprinkleWorld.water.sprinkleMove==bool(field<unsigned char>(image,0x25f720)),"Sprinkle moving flag differs");
    require(profile.floats==nativeProfile.floats && profile.uints==nativeProfile.uints && profile.booleans==nativeProfile.booleans && profile.ints==nativeProfile.ints,"Sprinkle actual profile stores differ");
    require(s.fields().size()==std::size_t((field<unsigned>(n,0x52c)-field<unsigned>(n,0x528))/4),"Sprinkle affected fields count differs");
    require(s.tipsDirection==field<int>(n,0x2b4) && s.tipsDisabled==bool(field<unsigned char>(n,0x280)),"Sprinkle tooltip state differs");
    auto** tips=field<void**>(n,0x288);require(s.tips().size()==std::size_t(field<void**>(n,0x28c)-tips),"Sprinkle tooltip count differs");
    for(unsigned i=0;i<s.tips().size();++i) {auto* control=s.tips()[i];mainMenuCompareView(*control,tips[i]);compareControl(*control,tips[i]);if(auto* line=dynamic_cast<TextLine*>(control)) {if(line->textDirty()) line->makeLetters();compare(*line,tips[i]);}}
    require(sprinkleRandom.state()==field<unsigned>(gameplayThreadData.data(),0x14),"Sprinkle particle RNG differs");
    struct Link {Link *next,*previous;void* value;};auto* head=field<Link*>(ne,0x23c);auto* link=head->next;
    require(s.effect().particles().size()==field<unsigned>(ne,0x240),"Sprinkle effect particle count differs");
    for(auto* p:s.effect().particles()) {require(link!=head && link->next->previous==link,"Sprinkle particle order/links differ");auto& particle=static_cast<SprinkleParticle&>(*p);mainMenuCompareView(particle,link->value);compareControl(particle,link->value);
        require(particle.parent()==&s.effect() && field<void*>(link->value,0x20)==ne,"Sprinkle particle parent differs");
        for(auto pair:std::initializer_list<std::pair<float,unsigned>>{{particle.motion.velocity[0],0x238},{particle.motion.velocity[1],0x23c},{particle.motion.lifetime,0x250},{particle.motion.elapsed,0x254}}) compareFloat(pair.first,field<float>(link->value,pair.second),"Sprinkle live particle motion");link=link->next;
    }require(link==head,"Sprinkle extra native particle");
    if(sprinkleSourceEvents!=sprinkleNativeEvents) {for(const auto& e:sprinkleSourceEvents) {std::cerr<<"source sprinkle event";for(auto v:e) std::cerr<<' '<<v;std::cerr<<'\n';}for(const auto& e:sprinkleNativeEvents) {std::cerr<<"native sprinkle event";for(auto v:e) std::cerr<<' '<<v;std::cerr<<'\n';}}
    require(sprinkleSourceEvents==sprinkleNativeEvents,"Sprinkle Farm/field actions differ");sprinkleSourceEvents.clear();sprinkleNativeEvents.clear();
    require(actualSounds==expectedSounds,"Sprinkle sounds differ");actualSounds.clear();expectedSounds.clear();require(warehouseSourceEvents==warehouseNativeEvents,"Sprinkle Money effects differ");warehouseSourceEvents.clear();warehouseNativeEvents.clear();++sprinkleStates;
};
for(unsigned id=0;id<3;++id) for(unsigned mode:{0U,2U}) {
    stage=672;trial=sprinkleObjects;cursorPosition={300,200};sprinkleWorld.water.sprinkleMove=false;field<unsigned char>(image,0x25f720)=0;sprinkleWorld.selectedFields.clear();sprinkleNativeFields.clear();
    profile.floats["SPXPos"+std::to_string(id)]=nativeProfile.floats["SPXPos"+std::to_string(id)]=300;profile.floats["SPYPos"+std::to_string(id)]=nativeProfile.floats["SPYPos"+std::to_string(id)]=200;profile.booleans["SPExist"+std::to_string(id)]=nativeProfile.booleans["SPExist"+std::to_string(id)]=bool(mode);
    auto s=std::make_unique<Sprinkle>(nullptr,mode,id,sprinkleContext);auto* n=allocate(1380);std::memset(n,0,1380);function<void*(__thiscall*)(void*,void*,unsigned,unsigned)>(0x60180)(n,nullptr,mode,id);checkSprinkle(*s,n);require(s->status()==mode,"Sprinkle constructor's original mode differs");
    sprinkleWorld.selected=s.get();sprinkleNativeSelected=n;
    for(unsigned fieldCount=0;fieldCount<3;++fieldCount) {
        sprinkleWorld.selectedFields.clear();sprinkleNativeFields.clear();for(unsigned i=0;i<fieldCount;++i) {sprinkleWorld.selectedFields.push_back(sprinklerFields[i].get());sprinkleNativeFields.push_back(nf[i]);}
        for(unsigned state:{0U,1U,2U,0xffffffffU}) {stage=673;s->setStatus(state);function<void(__thiscall*)(void*,unsigned)>(0x5f3d0)(n,state);++sprinkleActions;checkSprinkle(*s,n);}
        s->setStatus(0);function<void(__thiscall*)(void*,unsigned)>(0x5f3d0)(n,0);checkSprinkle(*s,n);
        for(float dt:{0.f,.017f,.05f,.25f}) {elapsed=dt;s->enabledStep();call(0x60450,n);++sprinkleActions;checkSprinkle(*s,n);}
    }
    sprinkleWorld.selectedFields.clear();sprinkleNativeFields.clear();s->setStatus(0);function<void(__thiscall*)(void*,unsigned)>(0x5f3d0)(n,0);checkSprinkle(*s,n);
    for(bool building:{false,true}) for(bool selected:{false,true}) {
        stage=679;sprinkleBuilding=building;sprinkleWorld.selected=selected ? s.get():nullptr;sprinkleNativeSelected=selected ? n:nullptr;
        s->hovered();call(0x5f230,n);++sprinkleActions;checkSprinkle(*s,n);s->unhovered();call(0x5f210,n);++sprinkleActions;checkSprinkle(*s,n);
    }sprinkleBuilding=false;sprinkleWorld.selected=s.get();sprinkleNativeSelected=n;
    for(Point2 delta:{Point2{0,0},Point2{4,0},Point2{4.001f,0},Point2{0,5}}) {
        stage=674;cursorPosition={300,200};s->setStoredPosition(cursorPosition);function<void(__thiscall*)(void*,const Point2*)>(0x5f340)(n,&cursorPosition);s->clicked();call(0x5f580,n);++sprinkleActions;checkSprinkle(*s,n);
        cursorPosition={300+delta[0],200+delta[1]};s->enabledStep();call(0x60450,n);++sprinkleActions;checkSprinkle(*s,n);
        for(bool fit:{false,true}) {sprinklePlace=fit;s->enabledStep();call(0x60450,n);++sprinkleActions;checkSprinkle(*s,n);}
        s->unclicked();call(0x5fb40,n);++sprinkleActions;checkSprinkle(*s,n);
    }
    // Explicit placed/new install branches, including the original release's
    // absence of Money deduction and rejected insufficient-Money click removal.
    for(bool installed:{false,true}) for(bool fit:{false,true}) for(bool release:{false,true}) for(int money:{0,1000}) {
        stage=675;s->installed=installed;field<unsigned char>(n,0x50c)=installed;profile.booleans["SPExist"+std::to_string(id)]=nativeProfile.booleans["SPExist"+std::to_string(id)]=installed;profile.ints["Money"]=nativeProfile.ints["Money"]=money;
        sprinklePlace=fit;s->setStatus(2);function<void(__thiscall*)(void*,unsigned)>(0x5f3d0)(n,2);checkSprinkle(*s,n);if(release) {s->unclicked();call(0x5fb40,n);}else {s->clicked();call(0x5f580,n);}++sprinkleActions;checkSprinkle(*s,n);
    }
    // Actual root + tooltip + owned emitter/particle GPU traversal.
    sprinkleWorld.selectedFields={sprinklerFields[0].get(),sprinklerFields[1].get()};sprinkleNativeFields={nf[0],nf[1]};s->setStatus(0);function<void(__thiscall*)(void*,unsigned)>(0x5f3d0)(n,0);checkSprinkle(*s,n);
    {
        const auto savedHover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
        menuMilliseconds=0;GameTimer sprinklerTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor sprinklerCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&sprinklerCursor;
        std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        const Point2 at{s->geometry().x,s->geometry().y};
        for(unsigned frame=0;frame<12;++frame) {stage=677;action=frame;menuMilliseconds+=50;sprinklePlace=frame!=3;
            menuPoint=frame<5 ? Point2{at[0]+(frame>=2 ? 5.f:0.f),at[1]}:Point2{790,590};menuMouseDelta=frame==0 || frame==2 || frame==5 ? std::array<int,3>{1,0,0}:std::array<int,3>{};
            menuInputEvents=frame==1 ? std::vector<unsigned>{0x208}:frame==4 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
            sprinklePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++sprinkleFrames;checkSprinkle(*s,n);
        }sprinklePlace=true;
        interaction.playHover=savedHover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }
    const auto reloadDirectory=common/"sprinkle-reload";std::filesystem::create_directories(reloadDirectory);std::ofstream(reloadDirectory/"partial-sprinkle.xml")<<"<Sprinkle Radius='125.5' UnfitColor='FF804020'/>";files.mountCommon(String(reloadDirectory.generic_string()));
    for(unsigned reload=0;reload<2;++reload) {stage=680;XmlDocument xml(files,String("partial-sprinkle.xml"));s->loadXML(xml);function<void(__thiscall*)(void*,void*)>(0x5eb40)(n,wrap(&xml));checkSprinkle(*s,n);++sprinkleActions;}
    stage=676;const auto before=textureCounts();s.reset();views.deleteDiscarded();const auto middle=textureCounts();call(0x5e9d0,n);compareReferenceDeltas(before,middle);nativeDiscards.clear();++sprinkleObjects;
}
sprinkleWorld.selectedFields.clear();sprinkleNativeFields.clear();sprinkleWorld.selected=nullptr;sprinkleNativeSelected=nullptr;
// Actual WaterSource callback -> actual Farm factory -> actual Sprinkle object.
write(0x33980,sprinkleFactoryEntry.data(),5);
std::unique_ptr<Sprinkle> createdSprinkle;
waterWorld.actualSprinkleFactory=[&](unsigned cost)->ButtonPrototype* {createdSprinkle.reset(createFarmSprinkle(&waterWorld.farmScene().movingPicture(),cost,waterWorld.sprinkleCountValue,sprinkleContext));return createdSprinkle.get();};
for(unsigned available=0;available<4;++available) for(int money:{0,1000}) {
    stage=678;trial=available;waterWorld.sprinkleCountValue=field<unsigned>(image,0x25ff98)=available;waterWorld.sprinkleMove=false;field<unsigned char>(image,0x25f720)=0;profile.ints["Money"]=nativeProfile.ints["Money"]=money;
    for(unsigned i=0;i<3;++i) profile.booleans["SPExist"+std::to_string(i)]=nativeProfile.booleans["SPExist"+std::to_string(i)]=i<available;
    cursorPosition={420,310};water->sprinkleButton().clickEvent(&water->sprinkleButton());function<void(__cdecl*)(void*)>(0x697f0)(field<void*>(ns,0x8b4));++sprinkleActions;
    require(waterWorld.sprinkleCountValue==field<unsigned>(image,0x25ff98),"Actual Farm sprinkle count differs");
    if(available<3 && money>=int(water->sprinkleCost)) {
        require(createdSprinkle && createdSprinkle->buttonState()==ButtonState::Pressed,"Actual WaterSource did not create pressed Sprinkle");
        auto** end=field<void**>(nativeScenes[0],0x70);auto* n=end[-1];require(field<unsigned>(n,0x238)==3,"Native WaterSource Sprinkle was not pressed");checkSprinkle(*createdSprinkle,n);
        const auto before=textureCounts();createdSprinkle.reset();views.deleteDiscarded();const auto middle=textureCounts();call(0x5e9d0,n);compareReferenceDeltas(before,middle);nativeDiscards.clear();++sprinkleObjects;
    }else {require(!createdSprinkle,"Actual Farm admitted unavailable Sprinkle");require(sprinkleSourceEvents==sprinkleNativeEvents,"Rejected Farm sprinkle emitted side effects");}
}
waterWorld.actualSprinkleFactory={};redirect(0x33980,reinterpret_cast<void*>(&waterNativeCreateSprinkle));
for(unsigned i=0;i<2;++i) {const auto before=textureCounts();sprinklerFields[i].reset();const auto middle=textureCounts();call(0x187a0,nf[i]);compareReferenceDeltas(before,middle);}sprinkleNativeFields.clear();sprinkleNativeIds.clear();sprinkleNativeSelected=nullptr;
