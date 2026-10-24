// Actual carried-seed restoration, four sow uses, exhaustion and deferred cleanup.
if(personSeedFlowsMode) {
    stage=870;
    for(unsigned i=0;i<8;++i) {const auto& info=configuration.seeds[i];auto* row=static_cast<unsigned char*>(nativeGame)+0x79c+i*44;field<unsigned>(row,0)=info.price;auto* a=row+4;
        field<cTexture*>(a,4)=info.icon.texture;field<unsigned>(a,8)=info.icon.frames;field<float>(a,12)=info.icon.length;field<unsigned char>(a,16)=info.icon.loop;field<unsigned>(a,20)=info.icon.columns;field<unsigned>(a,24)=info.icon.rows;field<unsigned char>(a,28)=info.icon.playing;}
    auto seedBuy=std::make_unique<ExtendedAnimationButton>(&fieldWorld.farmScene().origin(),manager,settings,strings,input,interaction,[&]{return productionTutorial->active();},views,controls);
    auto* nativeBuy=allocate(864);std::memset(nativeBuy,0,864);function<void*(__thiscall*)(void*,void*)>(0x23550)(nativeBuy,field<void*>(nativeScenes[0],0x60));
    XmlDocument xml(files,String("UI/GameUI/Farm.xml"));XmlNode* buyXML=nullptr;require(xml.section(String("SeedsBuyButton"),buyXML),"Actor Seed buy XML missing");seedBuy->loadXML(*buyXML);reinterpret_cast<void(__thiscall*)(void*,void*)>(field<void**>(nativeBuy,0)[6])(nativeBuy,wrap(buyXML));
    auto* oldBuy=field<void*>(nativeScenes[0],0x9c);field<void*>(nativeScenes[0],0x9c)=nativeBuy;
    struct ActorSeedWorld final:SeedPackageWorld {
        ControllerWorld& scene;ExtendedAnimationButton& buy;ViewRegistry& registry;SeedPackage* selected=nullptr;
        ActorSeedWorld(ControllerWorld& s,ExtendedAnimationButton& b,ViewRegistry& r):scene(s),buy(b),registry(r) {}
        TutorialManager& tutorial() override {return scene.tutorial();}StateView& farmScene() override {return scene.farmScene();}
        void registerArea(ActionArea& a) override {scene.registerArea(a);}void queueAreaAction(ActionArea& a,const Point2& p) override {scene.queueAreaAction(a,p);}
        int warehouseLevel() override {return scene.warehouseLevel();}int digBuildingLevel() override {return scene.digBuildingLevel();}int mowBuildingLevel() override {return scene.mowBuildingLevel();}int waterSourceLevel() override {return scene.waterSourceLevel();}
        bool seedMoveActive() override {return scene.plots.water.seedMove;}void setSeedMoveActive(bool value) override {scene.plots.water.seedMove=value;}
        SeedPackage* selectedSeed() override {return selected;}void setSelectedSeed(SeedPackage* seed) override {selected=seed;}
        bool canPlaceSeed(SeedPackage& seed,const Point2& p) override {return scene.areas().canPlaceSeed(&seed,p);}ActionArea* topAt(const Point2& p,unsigned mask) override {return scene.areas().topAt(p,mask);}
        std::vector<ActionArea*> areasAt(const Point2& p,unsigned mask) override {std::vector<ActionArea*> result;scene.areas().appendAt(p,mask,result);return result;}
        void setFarmActionButtonEnabled(bool enabled) override {if(enabled) buy.enable();else buy.disable();}
        int money() override {return scene.money();}void setMoney(int value) override {scene.setMoney(value);}
        void removeSeed(SeedPackage& seed) override {scene.removeSeed(seed);}FarmField* fieldForSeedTutorial() override {return scene.areas().fieldForSeedTutorial();}
        Control& digBuilding() override {return *DigBuilding::instance();}
    } seedWorld(controllerWorld,*seedBuy,views);
    SeedPackageContext seedContext{{manager,settings,strings,input,interaction,seedWorld,views,controls},configuration,profile,seedWorld};
    profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;seedWorld.setSeedMoveActive(false);field<unsigned char>(image,0x25f721)=0;putPointer(0x25f724,nullptr);
    const Point2 seedPosition{320,340};auto seed=std::make_unique<SeedPackage>(&controllerWorld.farmScene().movingPicture(),1,4,seedPosition,3,false,seedContext);
    auto* nseed=allocate(1684);std::memset(nseed,0,1684);function<void*(__thiscall*)(void*,void*,unsigned,unsigned,const Point2*,unsigned,bool)>(0x5e130)(nseed,field<void*>(nativeScenes[0],0x38),1,4,&seedPosition,3,false);
    auto actor=std::make_unique<Person>(&controllerWorld.farmScene().movingPicture(),controllerContext);controllerWorld.actor=actor.get();auto* n=allocate(0x2b30);std::memset(n,0,0x2b30);function<void*(__thiscall*)(void*,void*)>(0x4f340)(n,field<void*>(nativeScenes[0],0x38));
    require(actor->carriedSeed==seed.get() && field<void*>(n,0x2848)==nseed && actor->heldType()==10,"Actual Person ctor did not restore carried seed");checkController(*actor,n);
    for(unsigned use=0;use<4;++use) {
        stage=871;trial=use;profile.ints["fcu"]=nativeProfile.ints["fcu"]=0;profile.ints["fua"]=nativeProfile.ints["fua"]=0;
        profile.uints["FFC0"]=nativeProfile.uints["FFC0"]=0;profile.uints["FFS0"]=nativeProfile.uints["FFS0"]=3;profile.floats["FFP0"]=nativeProfile.floats["FFP0"]=100;profile.floats["FFW0"]=nativeProfile.floats["FFW0"]=40;profile.floats["FFF0"]=nativeProfile.floats["FFF0"]=100;
        auto plot=std::make_unique<FarmField>(&controllerWorld.farmScene().movingPicture(),0,0,0,fieldContext);auto* np=allocate(2384);std::memset(np,0,2384);fieldNativeIds[np]=0;function<void*(__thiscall*)(void*,void*,unsigned,unsigned,int)>(0x3c120)(np,field<void*>(nativeScenes[0],0x38),0,0,0);
        fieldWorld.graph.fields={plot.get()};void* fields[]={np};field<void*>(nativeScenes[0],0x7c)=fields;field<void*>(nativeScenes[0],0x80)=field<void*>(nativeScenes[0],0x84)=fields+1;
        actor->installWork(PersonActionKind::Sow,*plot,seed.get());auto* task=allocate(24);std::memset(task,0,24);function<void*(__thiscall*)(void*,void*,void*)>(0x515c0)(task,np,nseed);function<void(__thiscall*)(void*,void*)>(0x15790)(static_cast<unsigned char*>(n)+0x284,task);++personControllerActions;checkController(*actor,n);
        for(float dt:{0.f,.013f,.1f,1.f,100.f}) {elapsed=dt;actor->absoluteStep();call(0x49ad0,n);++personControllerActions;checkController(*actor,n);}
        require(seed->count()==3-use && seed->count()==field<unsigned>(nseed,0x650) && plot->culture()==1 && plot->auxiliary==1,"Actual sow did not consume seed or plant Field");compareControl(*seed,nseed);checkField(*plot,np);
        fieldTip->hideFor(plot.get());function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,np);fieldWorld.graph.fields.clear();fieldWorld.graph.areas.erase(std::remove(fieldWorld.graph.areas.begin(),fieldWorld.graph.areas.end(),plot.get()),fieldWorld.graph.areas.end());
        const auto before=textureCounts();plot.reset();const auto middle=textureCounts();call(0x36340,np);compareReferenceDeltas(before,middle);for(unsigned offset:{0x78U,0x7cU,0x80U,0x84U}) field<unsigned>(nativeScenes[0],offset)=0;
    }
    require(actor->heldType()==0 && views.discarded().size()==1 && nativeDiscards.size()==1 && views.discarded().at(seed.get()).view()==nativeDiscards.at(nseed),"Actual exhausted seed removal differs");
    seed.release();const auto beforeSeed=textureCounts();views.deleteDiscarded();const auto middleSeed=textureCounts();call(0x5ce00,nseed);compareReferenceDeltas(beforeSeed,middleSeed);nativeDiscards.clear();
#include "person_seed_scene_oracle_cases.inc"
    const auto beforeActor=textureCounts();actor.reset();controllerWorld.actor=nullptr;const auto middleActor=textureCounts();call(0x4b3a0,n);compareReferenceDeltas(beforeActor,middleActor);
    const auto beforeBuy=textureCounts();seedBuy.reset();const auto middleBuy=textureCounts();mainMenuDeleteNative(nativeBuy);compareReferenceDeltas(beforeBuy,middleBuy);field<void*>(nativeScenes[0],0x9c)=oldBuy;
}

// Separate diagnostic processes preserve pointer allocation order within the
// full scene while bounding the monotonic test heap. The production game has
// no such split or allocator.
if(personHoseFlowsMode) {
// Actual hose worker, real Field water setter and actual emitter configuration.
for(int wellLevel:{1,2}) {
    stage=872;profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;water->setLevel(wellLevel,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ns)+0x508,wellLevel,false);
    profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=11;auto actor=std::make_unique<Person>(&controllerWorld.farmScene().movingPicture(),controllerContext);controllerWorld.actor=actor.get();auto* n=allocate(0x2b30);std::memset(n,0,0x2b30);function<void*(__thiscall*)(void*,void*)>(0x4f340)(n,field<void*>(nativeScenes[0],0x38));checkController(*actor,n);
    for(float initialWater:{0.f,119.875f,120.f}) {
        stage=873;trial=wellLevel;profile.ints["fcu"]=nativeProfile.ints["fcu"]=0;profile.ints["fua"]=nativeProfile.ints["fua"]=0;profile.uints["FFC0"]=nativeProfile.uints["FFC0"]=1;profile.uints["FFS0"]=nativeProfile.uints["FFS0"]=0;
        profile.floats["FFP0"]=nativeProfile.floats["FFP0"]=configuration.products[1].creationTime;profile.floats["FFW0"]=nativeProfile.floats["FFW0"]=initialWater;profile.floats["FFF0"]=nativeProfile.floats["FFF0"]=100;
        auto plot=std::make_unique<FarmField>(&controllerWorld.farmScene().movingPicture(),0,0,0,fieldContext);auto* np=allocate(2384);std::memset(np,0,2384);fieldNativeIds[np]=0;function<void*(__thiscall*)(void*,void*,unsigned,unsigned,int)>(0x3c120)(np,field<void*>(nativeScenes[0],0x38),0,0,0);
        const Point2 at{540,330};plot->setPosition(at);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(np,&at);fieldWorld.graph.fields={plot.get()};void* fields[]={np};field<void*>(nativeScenes[0],0x7c)=fields;field<void*>(nativeScenes[0],0x80)=field<void*>(nativeScenes[0],0x84)=fields+1;
        actor->installWork(PersonActionKind::HoseWater,*plot);auto* task=allocate(20);std::memset(task,0,20);function<void(__thiscall*)(void*,void*,unsigned)>(0x15320)(task,nullptr,0);field<void*>(task,0)=image+0x1f3490;field<void*>(task,0xc)=np;function<void(__thiscall*)(void*,void*)>(0x15790)(static_cast<unsigned char*>(n)+0x284,task);++personControllerActions;checkController(*actor,n);
        const auto checkHose=[&] {compareControl(*controllerEmitter,nch);compareFloat(controllerEmitter->angle,field<float>(nch,0x264),"Actor hose angle");require(controllerEmitter->emission.generating==bool(field<unsigned char>(nch,0x250)),"Actor hose generation differs");for(unsigned i=0;i<2;++i) {compareFloat(controllerEmitter->startPoint[i],field<float>(nch,0x278+i*4),"Actor hose start");compareFloat(controllerEmitter->targetPoint[i],field<float>(nch,0x280+i*4),"Actor hose local target");}};checkHose();
        // Original hose work clears cancellation and continues watering.
        actor->requestCancel(true);function<void(__thiscall*)(void*,unsigned char)>(0x33040)(n,1);++personControllerActions;checkController(*actor,n);
        for(float dt:{0.f,-.01f,.013f,.1f,1.f,100.f}) {elapsed=dt;actor->absoluteStep();call(0x49ad0,n);++personControllerActions;checkController(*actor,n);checkHose();}
        require(plot->water()==plot->waterCapacity && actor->heldType()==11 && !controllerEmitter->emission.generating && field<void*>(field<void*>(n,0x284),0)==image+0x1f2c34,"Actual hose did not fill Field and return to hose idle");checkField(*plot,np);
        fieldTip->hideFor(plot.get());function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,np);fieldWorld.graph.fields.clear();fieldWorld.graph.areas.erase(std::remove(fieldWorld.graph.areas.begin(),fieldWorld.graph.areas.end(),plot.get()),fieldWorld.graph.areas.end());
        const auto before=textureCounts();plot.reset();const auto middle=textureCounts();call(0x36340,np);compareReferenceDeltas(before,middle);for(unsigned offset:{0x78U,0x7cU,0x80U,0x84U}) field<unsigned>(nativeScenes[0],offset)=0;
    }
    const auto before=textureCounts();actor.reset();controllerWorld.actor=nullptr;const auto middle=textureCounts();call(0x4b3a0,n);compareReferenceDeltas(before,middle);
}

// Full hose input path and actual live emitted particles in the registered UI.
{
    stage=874;auto& farm=controllerWorld.farmScene();auto* oldParent=farm.parent();auto* nativeOldParent=field<void*>(nativeScenes[0],0x20);const auto oldMovement=farm.movement;const bool oldShown=farm.state().shown,oldEnabled=farm.state().enabled;
    auto root=std::make_unique<View>(nullptr,views);auto* nativeRoot=allocate(44);std::memset(nativeRoot,0,44);function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeRoot,nullptr);
    farm.setParent(root.get());function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nativeRoot);root->show();call(0xd37d0,nativeRoot);root->enable();call(0xd3bb0,nativeRoot);root->absoluteEnable();call(0xd3fb0,nativeRoot);farm.show();call(0xd37d0,nativeScenes[0]);farm.enable();call(0xd3bb0,nativeScenes[0]);farm.movement=field<unsigned>(nativeScenes[0],0x4c)=0;
    std::array<unsigned char,5> oldHoseDiscard{};std::memcpy(oldHoseDiscard.data(),image+0x43ca0,5);const auto discard=+[](void* particle,const String* origin) {nativeDiscards[particle]=std::string(origin->view());};redirect(0x43ca0,reinterpret_cast<void*>(discard));
    field<void*>(nativeScenes[0],0)=fieldFarmVtable.data();fieldWorld.actualPersonGoalLevels=true;fieldWorld.held=false;field<unsigned char>(nativeScenes[0],0x108)=0;
    fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
    for(auto pair:std::initializer_list<std::pair<ActionArea*,void*>>{{controllerDig.get(),ncd},{controllerMow.get(),ncm},{water.get(),ns}}) {fieldWorld.graph.registerArea(*pair.first);function<void(__thiscall*)(void*,void*)>(0x31530)(nativeScenes[0],pair.second);}
    profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=0;water->setLevel(1,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(ns)+0x508,1,false);profile.uints["PCarry"]=nativeProfile.uints["PCarry"]=11;
    profile.ints["fcu"]=nativeProfile.ints["fcu"]=0;profile.ints["fua"]=nativeProfile.ints["fua"]=0;profile.uints["FFC0"]=nativeProfile.uints["FFC0"]=1;profile.uints["FFS0"]=nativeProfile.uints["FFS0"]=0;
    profile.floats["FFP0"]=nativeProfile.floats["FFP0"]=configuration.products[1].creationTime;profile.floats["FFW0"]=nativeProfile.floats["FFW0"]=0;profile.floats["FFF0"]=nativeProfile.floats["FFF0"]=100;
    auto plot=std::make_unique<FarmField>(&farm.movingPicture(),0,0,0,fieldContext);auto* np=allocate(2384);std::memset(np,0,2384);fieldNativeIds[np]=0;function<void*(__thiscall*)(void*,void*,unsigned,unsigned,int)>(0x3c120)(np,field<void*>(nativeScenes[0],0x38),0,0,0);
    const Point2 plotPosition{540,330};plot->setPosition(plotPosition);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(np,&plotPosition);fieldWorld.graph.fields={plot.get()};void* fields[]={np};field<void*>(nativeScenes[0],0x7c)=fields;field<void*>(nativeScenes[0],0x80)=field<void*>(nativeScenes[0],0x84)=fields+1;
    auto actor=std::make_unique<Person>(&farm.movingPicture(),controllerContext);controllerWorld.actor=actor.get();auto* n=allocate(0x2b30);std::memset(n,0,0x2b30);function<void*(__thiscall*)(void*,void*)>(0x4f340)(n,field<void*>(nativeScenes[0],0x38));fieldWorld.tasks=&actor->tasks();checkController(*actor,n);
    struct Link {Link* next;Link* previous;void* value;};std::map<View*,void*> pairs;
    const auto checkParticles=[&] {
        auto& e=*controllerEmitter;compareControl(e,nch);mainMenuCompareView(e,nch);require(e.emission.generating==bool(field<unsigned char>(nch,0x250)),"Rendered actor hose flag differs");
        compareFloat(e.angle,field<float>(nch,0x264),"Rendered actor hose angle");for(unsigned i=0;i<2;++i) {compareFloat(e.startPoint[i],field<float>(nch,0x278+i*4),"Rendered actor hose start");compareFloat(e.targetPoint[i],field<float>(nch,0x280+i*4),"Rendered actor hose target");}
        require(e.particles().size()==field<unsigned>(nch,0x240),"Rendered actor hose particle count differs");require(fieldRandom.state()==field<unsigned>(gameplayThreadData.data(),0x14),"Rendered actor hose RNG differs");
        auto* head=field<Link*>(nch,0x23c);auto* item=head->next;
        for(auto* picture:e.particles()) {require(item!=head,"Rendered actor hose missing particle");pairs[picture]=item->value;auto& p=static_cast<HoseParticle&>(*picture);auto* native=item->value;compareControl(p,native);mainMenuCompareView(p,native);
            for(auto value:std::initializer_list<std::pair<float,unsigned>>{{p.motion.velocity[0],0x238},{p.motion.velocity[1],0x23c},{p.gravity,0x240},{p.motion.maximumScale,0x244},{p.motion.originalSize[0],0x24c},{p.motion.originalSize[1],0x250},{p.motion.lifetime,0x254},{p.motion.elapsed,0x258}}) compareFloat(value.first,field<float>(native,value.second),"Rendered actor hose particle motion");require(p.motion.finished==bool(field<unsigned char>(native,0x248)),"Rendered actor hose particle finished differs");item=item->next;
        }require(item==head,"Rendered actor hose extra native particle");
        // Hose idle consumes and completes a fresh Field marker in one step.
        // Pair that short-lived real marker by its actual parent and origin.
        for(const auto& pending:views.discarded()) if(!pairs.count(pending.first)) {require(pending.first->parent()==plot.get(),"Unknown actor hose discard owner");void* found=nullptr;for(const auto& native:nativeDiscards) if(field<void*>(native.first,0x20)==np && pending.second.view()==native.second) {require(!found,"Ambiguous actor hose marker");found=native.first;}require(found,"Missing actor hose native marker");pairs[pending.first]=found;}
    };
    const auto deletePending=[&] {require(views.discarded().size()==nativeDiscards.size(),"Rendered actor hose discard count differs");for(const auto& pending:views.discarded()) require(pending.second.view()==nativeDiscards.at(pairs.at(pending.first)),"Rendered actor hose discard origin differs");const auto before=textureCounts();views.deleteDiscarded();const auto middle=textureCounts();std::vector<void*> pending;for(const auto& item:nativeDiscards) pending.push_back(item.first);for(auto* item:pending) mainMenuDeleteNative(item);compareReferenceDeltas(before,middle);};
    const auto hover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
    explicitTipElapsed=false;menuMilliseconds=0;GameTimer timer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor cursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&cursor;
    std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    bool emitted=false,filled=false;unsigned maximumParticles=0;
    for(unsigned frame=0;frame<64;++frame) {stage=875;action=frame;menuMilliseconds+=250;menuPoint=frame<3 ? plotPosition:Point2{790,590};menuMouseDelta=frame==0 || frame==3 ? std::array<int,3>{1,0,0}:std::array<int,3>{};menuInputEvents=frame==1 ? std::vector<unsigned>{0x208}:frame==2 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
        personControllerPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++personControllerFrames;checkController(*actor,n);checkField(*plot,np);checkParticles();emitted=emitted || controllerEmitter->emission.generating;filled=filled || plot->water()==plot->waterCapacity;maximumParticles=(std::max)(maximumParticles,unsigned(controllerEmitter->particles().size()));deletePending();
    }
    require(emitted && filled && maximumParticles>0 && !controllerEmitter->emission.generating && controllerEmitter->particles().empty() && actor->tasks().empty() && !actor->tasks().current().area && !plot->clickIndex,"Actual Field mouse hose path did not fill and drain");
    actor->tasks().clear();call(0x4f870,n);checkParticles();deletePending();fieldWorld.tasks=nullptr;fieldWorld.held=false;field<unsigned char>(nativeScenes[0],0x108)=0;
    interaction.playHover=hover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);explicitTipElapsed=true;
    const auto beforeActor=textureCounts();actor.reset();controllerWorld.actor=nullptr;const auto middleActor=textureCounts();call(0x4b3a0,n);compareReferenceDeltas(beforeActor,middleActor);
    fieldTip->hideFor(plot.get());function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,np);const auto beforePlot=textureCounts();plot.reset();const auto middlePlot=textureCounts();call(0x36340,np);compareReferenceDeltas(beforePlot,middlePlot);fieldWorld.graph.fields.clear();
    fieldWorld.actualPersonGoalLevels=false;field<void*>(nativeScenes[0],0)=priorFieldFarmVtable;for(unsigned offset:{0x78U,0x7cU,0x80U,0x84U}) field<unsigned>(nativeScenes[0],offset)=0;
    if(!oldEnabled) {farm.disable();call(0xd3750,nativeScenes[0]);}if(!oldShown) {farm.hide();call(0xd3720,nativeScenes[0]);}farm.setParent(oldParent);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nativeOldParent);farm.movement=field<unsigned>(nativeScenes[0],0x4c)=oldMovement;root.reset();call(0xd58a0,nativeRoot);write(0x43ca0,oldHoseDiscard.data(),5);
}

}
