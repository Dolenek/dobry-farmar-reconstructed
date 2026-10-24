// Full native/source Person construction and real timed workers. Farm is the
// existing actual StateView/area graph with explicit remaining scene services.
stage=850;const auto controllerMath=field<unsigned>(image,0x261618),controllerFloor=field<unsigned>(image,0x263b80);field<unsigned>(image,0x261618)=field<unsigned>(image,0x263b80)=1;
const bool priorControllerElapsed=explicitTipElapsed;explicitTipElapsed=true;
std::map<unsigned,std::array<unsigned char,5>> controllerPriorEntries;
for(const auto& entry:personControllerOriginalEntries) {std::memcpy(controllerPriorEntries[entry.first].data(),image+entry.first,5);write(entry.first,entry.second.data(),5);}
redirect(0x32c20,reinterpret_cast<void*>(&nativeDiscard));
void* controllerOldPerson=field<void*>(image,0x25ff5c);void* controllerOldDig=field<void*>(image,0x25f74c);void* controllerOldMow=field<void*>(image,0x25f754);void* controllerOldFlight=field<void*>(image,0x25ff90);void* controllerOldEmitter=field<void*>(nativeScenes[0],0xc8);
const bool controllerOldCharacter=hudInputs.character;hudInputs.character=false;putPointer(0x25ff5c,nullptr);putPointer(0x25f74c,nullptr);putPointer(0x25f754,nullptr);putPointer(0x25ff90,nullptr);
fieldWorld.graph.areas.clear();fieldWorld.graph.fields.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U,0x78U,0x7cU,0x80U,0x84U}) field<unsigned>(nativeScenes[0],offset)=0;
struct ControllerWorld final:PersonWorld,DigBuildingWorld,MowBuildingWorld {
    FieldFixture& plots;ViewRegistry& registry;Person* actor=nullptr;HoseEmitter* emitter=nullptr;
    ControllerWorld(FieldFixture& p,ViewRegistry& r):plots(p),registry(r) {}
    FarmAreas& areas() override {return plots.graph;}HoseEmitter& hoseEmitter() override {require(emitter,"Person test needs actual HoseEmitter");return *emitter;}
    void removeSeed(SeedPackage& seed) override {plots.graph.remove(seed,registry);}
    int money() override {return plots.water.money();}void setMoney(int value) override {plots.water.setMoney(value);}
    TutorialManager& tutorial() override {return plots.tutorial();}StateView& farmScene() override {return plots.farmScene();}
    void registerArea(ActionArea& a) override {plots.registerArea(a);}void queueAreaAction(ActionArea& a,const Point2& at) override {require(actor,"Person area queue needs actor");actor->tasks().add(a,at);}
    int warehouseLevel() override {return plots.warehouseLevel();}int digBuildingLevel() override {return DigBuilding::instance()->level();}int mowBuildingLevel() override {return MowBuilding::instance()->level();}int waterSourceLevel() override {return WaterSource::instance()->level();}
    bool cursorOverAreaType(unsigned type,const ActionArea* excluded) override {return plots.water.cursorOverAreaType(type,excluded);}
    int heldType() override {return int(plots.water.stock.profile.uints["PCarry"]);}bool personPresent() override {return actor!=nullptr;}
    unsigned heldProductCount() override {unsigned result=0;for(unsigned i=0;i<8;++i) result+=plots.water.stock.profile.uints["PCC"+std::to_string(i)];return result;}void hidePersonHeldIndicator() override {actor->visual().waterUse().hide();}
    void updatePersonHeldIndicator(bool enabled,unsigned current,unsigned maximum) override {actor->visual().showDiscrete(enabled,current,maximum);}
    Point2 characterPosition() override {return {actor->geometry().x,actor->geometry().y};}
    void animateHeldDigUpgrade(const Point2& from,const Point2& to,int kind,bool flag) override {require(!flag,"Actual Person Dig flight reveal differs");ElasticAnimation::instance()->begin(from,to,kind,nullptr);}
    void animateHeldUpgrade(const Point2& from,const Point2& to,int kind,bool flag) override {require(!flag,"Actual Person Mow flight reveal differs");ElasticAnimation::instance()->begin(from,to,kind,nullptr);}
    void refreshHeldAppearance() override {actor->refreshHeldAppearance();}
} controllerWorld(fieldWorld,views);
profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;for(const char* key:{"Upgrade31","Upgrade34"}) profile.ints[key]=nativeProfile.ints[key]=0;
DigBuildingContext controllerDigContext{{manager,settings,strings,input,interaction,static_cast<DigBuildingWorld&>(controllerWorld),views,controls},productionUpgrades,configuration,controllerWorld};
MowBuildingContext controllerMowContext{{manager,settings,strings,input,interaction,static_cast<MowBuildingWorld&>(controllerWorld),views,controls},productionUpgrades,configuration,profile,controllerWorld};
stage=853;auto controllerDig=std::make_unique<DigBuilding>(&controllerWorld.farmScene().movingPicture(),controllerDigContext);auto* ncd=allocate(2220);std::memset(ncd,0,2220);function<void*(__thiscall*)(void*,void*)>(0x29fe0)(ncd,field<void*>(nativeScenes[0],0x38));
for(unsigned i=0;i<19;++i) {const auto& a=configuration.products[i].icon;auto* out=static_cast<unsigned char*>(nativeGame)+0x30+i*100+16;field<cTexture*>(out,4)=a.texture;field<unsigned>(out,8)=a.frames;field<float>(out,12)=a.length;field<unsigned char>(out,16)=a.loop;field<unsigned>(out,20)=a.columns;field<unsigned>(out,24)=a.rows;}
stage=854;auto controllerMow=std::make_unique<MowBuilding>(&controllerWorld.farmScene().movingPicture(),controllerMowContext);auto* ncm=allocate(2244);std::memset(ncm,0,2244);function<void*(__thiscall*)(void*,void*)>(0x49560)(ncm,field<void*>(nativeScenes[0],0x38));
stage=855;auto controllerFlight=std::make_unique<ElasticAnimation>(&controllerWorld.farmScene().movingPicture(),manager,audio,views,controls);auto* nce=allocate(1028);std::memset(nce,0,1028);function<void*(__thiscall*)(void*,void*)>(0x2c2d0)(nce,field<void*>(nativeScenes[0],0x38));
const String controllerHosePath("Effects/HoseEffect.xml");stage=856;auto controllerEmitter=std::make_unique<HoseEmitter>(&controllerWorld.farmScene().movingPicture(),manager,fieldRandom,String("Effects/HoseEffect.xml"),views,controls);controllerWorld.emitter=controllerEmitter.get();auto* nch=allocate(648);std::memset(nch,0,648);function<void*(__thiscall*)(void*,void*,const String*)>(0x45420)(nch,field<void*>(nativeScenes[0],0x38),&controllerHosePath);field<void*>(nativeScenes[0],0xc8)=nch;
PersonContext controllerContext{manager,input,interaction,configuration,profile,audio,controllerWorld.farmScene(),controllerWorld,views,controls};
const auto checkController=[&](Person& actor,void* n) {
    compareControl(actor,n);mainMenuCompareView(actor,n);compareFloat(actor.heightOffset,field<float>(n,0x27c),"Actual Person height");auto& v=actor.visual();
    require(v.direction==field<unsigned>(n,0x288) && v.activity==field<unsigned>(n,0x28c) && v.hoseAnimation==bool(field<unsigned char>(n,0x290)) && v.hoseFrame==field<unsigned>(n,0x2b2c),"Actual Person pose differs");
    require(actor.cancelRequested==bool(field<unsigned char>(n,0x284c)),"Actual Person cancel flag differs");
    for(auto pair:std::initializer_list<std::pair<Control*,unsigned>>{{&v.waterUse(),0x2850},{&v.discreteUse(),0x2854},{&v.sign(),0x2858},{&v.substrate(),0x285c}}) {compareControl(*pair.first,field<void*>(n,pair.second));mainMenuCompareView(*pair.first,field<void*>(n,pair.second));}
    compareFloat(v.waterUse().current,field<float>(field<void*>(n,0x2850),0x23c),"Actual Person water indicator");require(v.discreteUse().current==field<unsigned>(field<void*>(n,0x2854),0x22c),"Actual Person discrete value differs");
    if(profile.ints!=nativeProfile.ints || profile.uints!=nativeProfile.uints || profile.floats!=nativeProfile.floats || profile.booleans!=nativeProfile.booleans) {
        const auto differences=[](const auto& a,const auto& b,const char* type) {for(const auto& x:a) {const auto y=b.find(x.first);if(y==b.end() || y->second!=x.second) std::cerr<<"source "<<type<<' '<<x.first<<' '<<x.second<<'\n';}for(const auto& x:b) {const auto y=a.find(x.first);if(y==a.end() || y->second!=x.second) std::cerr<<"native "<<type<<' '<<x.first<<' '<<x.second<<'\n';}};
        differences(profile.ints,nativeProfile.ints,"int");differences(profile.uints,nativeProfile.uints,"uint");differences(profile.floats,nativeProfile.floats,"float");differences(profile.booleans,nativeProfile.booleans,"bool");
    }
    require(profile.ints==nativeProfile.ints && profile.uints==nativeProfile.uints && profile.floats==nativeProfile.floats && profile.booleans==nativeProfile.booleans,"Actual Person profiles differ");
    require(GameAction::count()==field<unsigned>(image,0x25ff58),"Actual Person action ownership differs");
    require(actualSounds==expectedSounds,"Actual Person sound requests differ");actualSounds.clear();expectedSounds.clear();
    require(hudSourceCalls==hudNativeCalls,"Actual Person HUD observations differ");hudSourceCalls.clear();hudNativeCalls.clear();
    require(warehouseSourceEvents==warehouseNativeEvents,"Actual Person Money/world observations differ");warehouseSourceEvents.clear();warehouseNativeEvents.clear();
    require(fieldSourceEvents==fieldNativeEvents,"Actual Person Field indication differs");fieldSourceEvents.clear();fieldNativeEvents.clear();
    ++personControllerStates;
};
for(int carry:{0,1,2,6,11}) for(int level=0;level<4;++level) {
    stage=851;trial=carry*4+level;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=carry;profile.floats["WaterLevel"]=nativeProfile.floats["WaterLevel"]=7.25f;
    profile.floats["XPersPos"]=nativeProfile.floats["XPersPos"]=300;profile.floats["YPersPos"]=nativeProfile.floats["YPersPos"]=350;
    profile.ints["Upgrade31"]=nativeProfile.ints["Upgrade31"]=level;profile.ints["Upgrade34"]=nativeProfile.ints["Upgrade34"]=level;
    for(unsigned i=0;i<8;++i) profile.uints["PCC"+std::to_string(i)]=nativeProfile.uints["PCC"+std::to_string(i)]=i==2 ? 2:0;
    auto actor=std::make_unique<Person>(&controllerWorld.farmScene().movingPicture(),controllerContext);controllerWorld.actor=actor.get();auto* n=allocate(0x2b30);std::memset(n,0,0x2b30);function<void*(__thiscall*)(void*,void*)>(0x4f340)(n,field<void*>(nativeScenes[0],0x38));checkController(*actor,n);
    for(float dt:{0.f,.05f,.25f}) {elapsed=dt;actor->absoluteStep();call(0x49ad0,n);++personControllerActions;checkController(*actor,n);}
    actor->requestCancel(true);function<void(__thiscall*)(void*,unsigned char)>(0x33040)(n,1);++personControllerActions;checkController(*actor,n);elapsed=.01f;actor->absoluteStep();call(0x49ad0,n);++personControllerActions;checkController(*actor,n);
    const auto before=textureCounts();actor.reset();controllerWorld.actor=nullptr;const auto middle=textureCounts();call(0x4b3a0,n);compareReferenceDeltas(before,middle);require(!Person::instance() && !field<void*>(image,0x25ff5c),"Actual Person destructor retains singleton");
}

// Timed actions own the same actual actor and actual tool/warehouse controllers.
stage=857;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;profile.ints["Upgrade31"]=nativeProfile.ints["Upgrade31"]=0;profile.ints["Upgrade34"]=nativeProfile.ints["Upgrade34"]=0;
profile.floats["XPersPos"]=nativeProfile.floats["XPersPos"]=300;profile.floats["YPersPos"]=nativeProfile.floats["YPersPos"]=350;
{
    auto actor=std::make_unique<Person>(&controllerWorld.farmScene().movingPicture(),controllerContext);controllerWorld.actor=actor.get();auto* n=allocate(0x2b30);std::memset(n,0,0x2b30);function<void*(__thiscall*)(void*,void*)>(0x4f340)(n,field<void*>(nativeScenes[0],0x38));checkController(*actor,n);
    const auto installWork=[&](PersonActionKind kind,ActionArea& area,void* nativeArea,unsigned ctor) {actor->installWork(kind,area);auto* task=allocate(20);std::memset(task,0,20);function<void*(__thiscall*)(void*,void*)>(ctor)(task,nativeArea);function<void(__thiscall*)(void*,void*)>(0x15790)(static_cast<unsigned char*>(n)+0x284,task);++personControllerActions;checkController(*actor,n);};
    const auto runWork=[&] {for(float dt:{0.f,.013f,.1f,1.f,100.f}) {elapsed=dt;actor->absoluteStep();call(0x49ad0,n);++personControllerActions;checkController(*actor,n);}require(field<void*>(field<void*>(n,0x284),0)==image+0x1f2c44 && actor->visual().activity==12,"Actual timed worker did not install idle");};
    installWork(PersonActionKind::TakeSpade,*controllerDig,ncd,0x51530);runWork();
    installWork(PersonActionKind::TakeScythe,*controllerMow,ncm,0x51560);runWork();
    installWork(PersonActionKind::TakeBucket,*water,ns,0x51590);runWork();
    // Actual warehouse deposit, with all PCC kinds and exact partial capacity.
    profile.ints["Upgrade32"]=nativeProfile.ints["Upgrade32"]=0;for(unsigned i=0;i<19;++i) profile.uints["WGoodCount"+std::to_string(i)]=nativeProfile.uints["WGoodCount"+std::to_string(i)]=0;
    warehouse->occupied=field<unsigned>(nw,0x534)=0;warehouse->refresh();call(0x66530,static_cast<unsigned char*>(nw)+0x508);
    for(unsigned i=0;i<8;++i) profile.uints["PCC"+std::to_string(i)]=nativeProfile.uints["PCC"+std::to_string(i)]=i<3 ? i+1:0;
    actor->setHeldType(6);function<void(__thiscall*)(void*,int)>(0x4ec80)(n,6);installWork(PersonActionKind::Store,*warehouse,nw,0x51620);runWork();
    for(PersonActionKind kind:{PersonActionKind::Dig,PersonActionKind::Harvest,PersonActionKind::Water}) {
        stage=858;trial=unsigned(kind);profile.ints["fcu"]=nativeProfile.ints["fcu"]=2;profile.ints["fua"]=nativeProfile.ints["fua"]=3;
        profile.uints["FFC0"]=nativeProfile.uints["FFC0"]=kind==PersonActionKind::Dig ? 0:1;profile.uints["FFS0"]=nativeProfile.uints["FFS0"]=kind==PersonActionKind::Harvest ? 1:0;
        profile.floats["FFP0"]=nativeProfile.floats["FFP0"]=configuration.products[kind==PersonActionKind::Dig ? 0:1].creationTime;profile.floats["FFW0"]=nativeProfile.floats["FFW0"]=0;profile.floats["FFF0"]=nativeProfile.floats["FFF0"]=20;
        auto plot=std::make_unique<FarmField>(&controllerWorld.farmScene().movingPicture(),0,0,0,fieldContext);auto* np=allocate(2384);std::memset(np,0,2384);fieldNativeIds[np]=0;function<void*(__thiscall*)(void*,void*,unsigned,unsigned,int)>(0x3c120)(np,field<void*>(nativeScenes[0],0x38),0,0,0);
        fieldWorld.graph.fields={plot.get()};void* nativeFields[]={np};field<void*>(nativeScenes[0],0x7c)=nativeFields;field<void*>(nativeScenes[0],0x80)=nativeFields+1;
        const int carry=kind==PersonActionKind::Dig ? 2:kind==PersonActionKind::Harvest ? 6:1;actor->setHeldType(carry);function<void(__thiscall*)(void*,int)>(0x4ec80)(n,carry);
        actor->setHeldWater(47);function<void(__thiscall*)(void*,float)>(0x4f0a0)(n,47);checkController(*actor,n);
        installWork(kind,*plot,np,kind==PersonActionKind::Dig ? 0x51500:kind==PersonActionKind::Harvest ? 0x515f0:0x51650);runWork();checkField(*plot,np);
        fieldTip->hideFor(plot.get());function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,np);
        fieldWorld.graph.fields.clear();fieldWorld.graph.areas.erase(std::remove(fieldWorld.graph.areas.begin(),fieldWorld.graph.areas.end(),plot.get()),fieldWorld.graph.areas.end());
        const auto before=textureCounts();plot.reset();const auto middle=textureCounts();call(0x36340,np);compareReferenceDeltas(before,middle);for(unsigned offset:{0x78U,0x7cU,0x80U,0x84U}) field<unsigned>(nativeScenes[0],offset)=0;
    }
    const auto before=textureCounts();actor.reset();controllerWorld.actor=nullptr;const auto middle=textureCounts();call(0x4b3a0,n);compareReferenceDeltas(before,middle);
}

// Complete registered UI -> Field click -> PersonTasks -> automatic motion ->
// arrival dispatch -> timed Dig -> real Field change -> automatic idle.
{
    stage=860;auto& farm=controllerWorld.farmScene();auto* oldParent=farm.parent();auto* nativeOldParent=field<void*>(nativeScenes[0],0x20);const auto oldMovement=farm.movement;const bool oldShown=farm.state().shown,oldEnabled=farm.state().enabled;
    auto root=std::make_unique<View>(nullptr,views);auto* nativeRoot=allocate(44);std::memset(nativeRoot,0,44);function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeRoot,nullptr);
    farm.setParent(root.get());function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nativeRoot);root->show();call(0xd37d0,nativeRoot);root->enable();call(0xd3bb0,nativeRoot);root->absoluteEnable();call(0xd3fb0,nativeRoot);farm.show();call(0xd37d0,nativeScenes[0]);farm.enable();call(0xd3bb0,nativeScenes[0]);farm.movement=field<unsigned>(nativeScenes[0],0x4c)=0;
    field<void*>(nativeScenes[0],0)=fieldFarmVtable.data();fieldWorld.actualPersonGoalLevels=true;fieldWorld.held=false;field<unsigned char>(nativeScenes[0],0x108)=0;
    fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
    fieldWorld.graph.registerArea(*controllerDig);fieldWorld.graph.registerArea(*controllerMow);function<void(__thiscall*)(void*,void*)>(0x31530)(nativeScenes[0],ncd);function<void(__thiscall*)(void*,void*)>(0x31530)(nativeScenes[0],ncm);
    profile.ints["Money"]=nativeProfile.ints["Money"]=100000;profile.ints["Upgrade31"]=nativeProfile.ints["Upgrade31"]=3;profile.ints["Upgrade34"]=nativeProfile.ints["Upgrade34"]=3;
    profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=2;profile.ints["fcu"]=nativeProfile.ints["fcu"]=0;profile.ints["fua"]=nativeProfile.ints["fua"]=0;profile.uints["FFC0"]=nativeProfile.uints["FFC0"]=0;profile.uints["FFS0"]=nativeProfile.uints["FFS0"]=0;
    profile.floats["FFP0"]=nativeProfile.floats["FFP0"]=configuration.products[0].creationTime;profile.floats["FFW0"]=nativeProfile.floats["FFW0"]=40;profile.floats["FFF0"]=nativeProfile.floats["FFF0"]=20;
    auto plot=std::make_unique<FarmField>(&farm.movingPicture(),0,0,0,fieldContext);auto* np=allocate(2384);std::memset(np,0,2384);fieldNativeIds[np]=0;function<void*(__thiscall*)(void*,void*,unsigned,unsigned,int)>(0x3c120)(np,field<void*>(nativeScenes[0],0x38),0,0,0);
    const Point2 plotPosition{540,330};plot->setPosition(plotPosition);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(np,&plotPosition);fieldWorld.graph.fields={plot.get()};void* nativeFields[]={np};field<void*>(nativeScenes[0],0x7c)=nativeFields;field<void*>(nativeScenes[0],0x80)=field<void*>(nativeScenes[0],0x84)=nativeFields+1;
    profile.floats["XPersPos"]=nativeProfile.floats["XPersPos"]=480;profile.floats["YPersPos"]=nativeProfile.floats["YPersPos"]=370;
    auto actor=std::make_unique<Person>(&farm.movingPicture(),controllerContext);controllerWorld.actor=actor.get();auto* n=allocate(0x2b30);std::memset(n,0,0x2b30);function<void*(__thiscall*)(void*,void*)>(0x4f340)(n,field<void*>(nativeScenes[0],0x38));fieldWorld.tasks=&actor->tasks();
    const auto goal=plot->goal(2),offset=actor->visual().hotSpotOffset(actor->visual().direction);const Point2 at{float(originalSubtract(originalSubtract(goal[0],offset[0]),60)),float(originalAdd(originalSubtract(goal[1],offset[1]),35))};actor->storePosition(at,true);function<void(__thiscall*)(void*,const Point2*,bool)>(0x4e780)(n,&at,true);checkController(*actor,n);
    std::map<View*,void*> markers;const auto checkTasks=[&] {const auto check=[&](const PersonTask& t,void* nt) {require((!t.area && !field<void*>(nt,0)) || (t.area==plot.get() && field<void*>(nt,0)==np),"Full Person target differs");require(!std::memcmp(t.target.data(),static_cast<unsigned char*>(nt)+8,8),"Full Person target coordinates differ");auto* m=field<void*>(nt,4);require(bool(t.marker)==bool(m),"Full Person marker presence differs");if(t.marker) {markers[t.marker]=m;compareControl(*t.marker,m);mainMenuCompareView(*t.marker,m);}};auto* first=field<unsigned char*>(n,0x282c);auto* last=field<unsigned char*>(n,0x2830);require(actor->tasks().queued().size()==(first ? std::size_t((last-first)/16):0),"Full Person FIFO differs");for(unsigned i=0;i<actor->tasks().queued().size();++i) check(actor->tasks().queued()[i],first+i*16);check(actor->tasks().current(),static_cast<unsigned char*>(n)+0x2838);require(plot->clickIndex==field<unsigned>(np,0x4fc),"Full Person Field click count differs");};
    const auto deleteMarks=[&] {require(views.discarded().size()==nativeDiscards.size(),"Full Person deferred discard count differs");for(const auto& item:views.discarded()) require(item.second.view()==nativeDiscards.at(markers.at(item.first)),"Full Person deferred discard origin differs");const auto before=textureCounts();views.deleteDiscarded();const auto middle=textureCounts();for(const auto& item:nativeDiscards) call(0xf1780,item.first);compareReferenceDeltas(before,middle);nativeDiscards.clear();};
    const auto hover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
    explicitTipElapsed=false;menuMilliseconds=0;GameTimer timer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor cursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&cursor;
    std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    bool hadWork=false;
    for(unsigned frame=0;frame<64;++frame) {stage=861;action=frame;menuMilliseconds+=250;menuPoint=frame<3 ? plotPosition:Point2{790,590};menuMouseDelta=frame==0 || frame==3 ? std::array<int,3>{1,0,0}:std::array<int,3>{};menuInputEvents=frame==1 ? std::vector<unsigned>{0x208}:frame==2 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
        personControllerPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++personControllerFrames;checkController(*actor,n);checkTasks();checkField(*plot,np);if(actor->visual().activity!=12) hadWork=true;if(!views.discarded().empty()) deleteMarks();
    }
    require(hadWork && plot->actionState()==3 && actor->visual().activity==12 && actor->tasks().empty() && !actor->tasks().current().area && !plot->clickIndex,"Full registered Person never completed actual Field Dig");
    actor->tasks().clear();call(0x4f870,n);checkTasks();deleteMarks();fieldWorld.tasks=nullptr;fieldWorld.held=false;field<unsigned char>(nativeScenes[0],0x108)=0;
    interaction.playHover=hover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);explicitTipElapsed=true;
    const auto beforeActor=textureCounts();actor.reset();controllerWorld.actor=nullptr;const auto middleActor=textureCounts();call(0x4b3a0,n);compareReferenceDeltas(beforeActor,middleActor);
    fieldTip->hideFor(plot.get());function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,np);const auto beforePlot=textureCounts();plot.reset();const auto middlePlot=textureCounts();call(0x36340,np);compareReferenceDeltas(beforePlot,middlePlot);fieldWorld.graph.fields.clear();
    fieldWorld.actualPersonGoalLevels=false;field<void*>(nativeScenes[0],0)=priorFieldFarmVtable;for(unsigned offset:{0x78U,0x7cU,0x80U,0x84U}) field<unsigned>(nativeScenes[0],offset)=0;
    if(!oldEnabled) {farm.disable();call(0xd3750,nativeScenes[0]);}if(!oldShown) {farm.hide();call(0xd3720,nativeScenes[0]);}farm.setParent(oldParent);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nativeOldParent);farm.movement=field<unsigned>(nativeScenes[0],0x4c)=oldMovement;root.reset();call(0xd58a0,nativeRoot);
}
stage=852;const auto beforeControllerTools=textureCounts();controllerFlight.reset();controllerEmitter.reset();controllerMow.reset();controllerDig.reset();const auto middleControllerTools=textureCounts();call(0x2b1a0,nce);call(0x45540,nch);call(0x473b0,ncm);call(0x26470,ncd);compareReferenceDeltas(beforeControllerTools,middleControllerTools);
fieldWorld.graph.areas.clear();fieldWorld.graph.fields.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U,0x78U,0x7cU,0x80U,0x84U}) field<unsigned>(nativeScenes[0],offset)=0;
putPointer(0x25ff5c,controllerOldPerson);putPointer(0x25f74c,controllerOldDig);putPointer(0x25f754,controllerOldMow);putPointer(0x25ff90,controllerOldFlight);field<void*>(nativeScenes[0],0xc8)=controllerOldEmitter;hudInputs.character=controllerOldCharacter;
for(const auto& entry:controllerPriorEntries) write(entry.first,entry.second.data(),5);redirect(0x32c20,reinterpret_cast<void*>(&waterNativeDiscardTask));field<unsigned>(image,0x261618)=controllerMath;field<unsigned>(image,0x263b80)=controllerFloor;explicitTipElapsed=priorControllerElapsed;
