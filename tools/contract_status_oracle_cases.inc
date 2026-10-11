stage=250;new(image+0x2601e8) String();field<unsigned char>(image,0x260204)=1;
redirect(0x8860,reinterpret_cast<void*>(&upgradeStreamConstruct));
redirect(0x10770,reinterpret_cast<void*>(&contractProperty));redirect(0x106a0,reinterpret_cast<void*>(&contractFloatProperty));redirect(0x105e0,reinterpret_cast<void*>(&contractBoolProperty));
redirect(0xa670,reinterpret_cast<void*>(&gameConfigurationLevel));
redirect(0x6b00,reinterpret_cast<void*>(&upgradeStreamConstruct));redirect(0x5060,reinterpret_cast<void*>(&listStreamNoop));
redirect(0x6700,reinterpret_cast<void*>(&upgradeWriteLiteral));redirect(0x6570,reinterpret_cast<void*>(&upgradeWriteIndex));
redirect(0xa450,reinterpret_cast<void*>(&upgradeStreamContents));redirect(0x59e0,reinterpret_cast<void*>(&listStreamNoop));
redirect(0x87d0,reinterpret_cast<void*>(&listStreamConstruct));redirect(0x3900,reinterpret_cast<void*>(&contractClearStream));
redirect(0x50d0,reinterpret_cast<void*>(&contractSetStream));redirect(0xa8be0,reinterpret_cast<void*>(&contractWriteSigned));field<unsigned>(image,0x2601e0)=1;
redirect(0xd0a20,reinterpret_cast<void*>(&contractNativeCanShow));redirect(0xcdef0,reinterpret_cast<void*>(&contractNativeStep));redirect(0xcf3c0,reinterpret_cast<void*>(&contractNativeShow));
redirect(0x10d60,reinterpret_cast<void*>(&contractNativeMoney));redirect(0x682d0,reinterpret_cast<void*>(&contractNativeStock));
redirect(0xc06b0,reinterpret_cast<void*>(&contractNativeRemove));redirect(0xc44a0,reinterpret_cast<void*>(&contractNativeNotify));
redirect(0xb7fe0,reinterpret_cast<void*>(&contractNativeReady));redirect(0xa94e0,reinterpret_cast<void*>(&contractNativeRequirements));redirect(0xb6b90,reinterpret_cast<void*>(&contractNativeResult));
putPointer(0x25f73c,contractTutorialObject.data());field<int>(contractTutorialObject.data(),0xbfc)=-1;
xmlTable[23]=reinterpret_cast<void*>(&gameConfigurationUnsigned);xmlTable[34]=reinterpret_cast<void*>(&sectionCount);
interaction.loadSound=[&](const String& name)->cResource* {return manager.getSound(name);};
LoadingDisabledAudio backend;ContractAudio audio(files,settings,backend,0);
loadingFiles=&files;managerTable[6]=reinterpret_cast<void*>(&loadingXML);managerTable[3]=reinterpret_cast<void*>(&loadingSoundResource);xmlTable[0]=reinterpret_cast<void*>(&loadingDeleteXML);
GameConfiguration configuration(manager);XmlDocument game(files,String("game.xml"));configuration.loadXML(game);
std::array<unsigned char,2716> nativeGame{};
// Borrowed appearances use the already independently verified GameConfiguration
// records. Native status still executes its original product lookup and copy.
for(unsigned n=0;n<19;++n) {
    const auto& appearance=configuration.products[n].icon;auto* out=nativeGame.data()+0x30+n*100+16;
    field<cTexture*>(out,4)=appearance.texture;field<unsigned>(out,8)=appearance.frames;field<float>(out,12)=appearance.length;
    field<unsigned char>(out,16)=appearance.loop;field<unsigned>(out,20)=appearance.columns;field<unsigned>(out,24)=appearance.rows;
}
putPointer(0x25f6ec,nativeGame.data());
std::array<std::array<unsigned char,80>,3> nativeTransitions{};
putPointer(0x25f758,nativeTransitions[0].data());putPointer(0x25f744,nativeTransitions[1].data());putPointer(0x25ffc8,nativeTransitions[2].data());
XmlDocument definition(files,String("UI/gameUI/ContractStatus.xml")),timeline(files,String("Gameplay/Gameplay.xml"));
GameplayEventDefinitions definitions;definitions.loadXML(timeline);std::vector<GameplayEvent*> contracts;
for(const auto& item:definitions.events()) if(item->type==GameplayEventType::Contract) contracts.push_back(item.get());
require(contracts.size()==131,"Shipped contract count differs");
std::mt19937 random(0x4bcc20);
for(unsigned n=0;n<4096;++n) {
    GameplayTime time{random(),random(),random()};if(n<100) time={n%12,n%61,n%63};
    for(bool omit:{false,true}) {
        WString original;function<void*(__thiscall*)(void*,WString*,bool)>(0xb1080)(&time,&original,omit);
        require(ContractStatus::formatTime(time,omit).view()==original.view(),"Contract native formatted time differs");++contractFormats;
    }
}
for(trial=0;trial<4;++trial) {
    stage=251;ViewRegistry views;ControlRegistry controls;View depot(nullptr,views),farm(nullptr,views),manufacture(nullptr,views);
    std::array<View*,3> transitionViews{&depot,&farm,&manufacture};
    ProfileProperties sourceProfile,nativeProfile;contractNativeProfile=&nativeProfile;ContractTrace world(sourceProfile,transitionViews);
    const unsigned identifier=trial<3 ? trial:0x80000001U;
    contractSourceCalls.clear();contractNativeCalls.clear();actualSounds.clear();expectedSounds.clear();contractTutorial=false;
    auto source=std::make_unique<ContractStatus>(nullptr,identifier,manager,settings,strings,input,interaction,sourceProfile,configuration,audio,world,views,controls);world.source=source.get();
    auto* original=allocate(136);std::memset(original,0,136);contractNativeStatus=original;
    const auto deleted=loadingNativeXMLDeletes;function<void*(__thiscall*)(void*,void*,unsigned)>(0xbb830)(original,nullptr,identifier);
    require(loadingNativeXMLDeletes==deleted+1,"Contract constructor XML ownership differs");
    std::array<unsigned char,256> nativeEvent{};new(nativeEvent.data()+0x68) WString();new(nativeEvent.data()+0x84) WString();
    const auto setEvent=[&](GameplayEvent& event) {
        const auto& contract=std::get<ContractEvent>(event.data);field<unsigned>(nativeEvent.data(),0x34)=event.index;
        field<GameplayTime>(nativeEvent.data(),0x38)=contract.winTime;field<int>(nativeEvent.data(),0x44)=contract.bonus;field<int>(nativeEvent.data(),0x48)=contract.penalty;
        std::memcpy(nativeEvent.data()+0x50,contract.requirements.data(),24);
        field<WString>(nativeEvent.data(),0x68)=contract.winNote;field<WString>(nativeEvent.data(),0x84)=contract.lossNote;
    };
    const auto check=[&] {
        stage=252;
        require(source->id==field<unsigned>(original,0x2c),"Contract id differs");
        require(source->active==bool(field<unsigned char>(original,0x70)) && source->dirty==bool(field<unsigned char>(original,0x71)) && source->expired==bool(field<unsigned char>(original,0x84)),"Contract runtime flags differ");
        compareFloat(source->accumulator,field<float>(original,0x58),"Contract accumulator");compareFloat(source->fullDuration,field<float>(original,0x68),"Contract full duration");
        if(!(source->remaining==field<GameplayTime>(original,0x5c)) || std::memcmp(source->initialStocks.data(),static_cast<unsigned char*>(original)+0x74,8)) { const auto t=field<GameplayTime>(original,0x5c);std::cerr<<"Time "<<source->remaining.hours<<":"<<source->remaining.minutes<<":"<<source->remaining.seconds<<" / "<<t.hours<<":"<<t.minutes<<":"<<t.seconds<<" stocks "<<source->initialStocks[0]<<","<<source->initialStocks[1]<<" / "<<field<unsigned>(original,0x74)<<","<<field<unsigned>(original,0x78)<<" assigned "<<contractAssignments<<"\n";}
        require(source->remaining==field<GameplayTime>(original,0x5c) && !std::memcmp(source->initialStocks.data(),static_cast<unsigned char*>(original)+0x74,8),"Contract remaining/initial stocks differ");
        require(sourceProfile.ints==nativeProfile.ints && sourceProfile.uints==nativeProfile.uints && sourceProfile.floats==nativeProfile.floats && sourceProfile.booleans==nativeProfile.booleans,"Contract profile maps differ");
        require(contractSourceCalls==contractNativeCalls,"Contract world callback order/arguments differ");require(actualSounds==expectedSounds,"Contract sound callback order differs");
        const auto& state=source->state();require(state.shown==bool(field<unsigned char>(original,0x24)) && state.enabled==bool(field<unsigned char>(original,0x26)) && state.absoluteEnabled==bool(field<unsigned char>(original,0x28)),"Contract root activation differs");
        compareControl(source->substrate(),field<void*>(original,0x30));compareControl(source->goalButton(),field<void*>(original,0x34));
        for(unsigned n=0;n<2;++n) {
            compareControl(source->productIcon(n),field<void*>(original,0x38+n*4));compareControl(source->requirementText(n),field<void*>(original,0x40+n*4));
            contractLetters+=compare(source->requirementText(n),field<void*>(original,0x40+n*4));
            compareControl(source->requirementCheck(n),field<void*>(original,0x48+n*4));
            require(source->requirementCheck(n).checked()==bool(field<unsigned char>(field<void*>(original,0x48+n*4),0x284)),"Contract tick state differs");
        }
        compareControl(source->timeText(),field<void*>(original,0x50));contractLetters+=compare(source->timeText(),field<void*>(original,0x50));
        compareControl(source->progress(),field<void*>(original,0x54));compareFloat(source->progress().current,field<float>(field<void*>(original,0x54),0x23c),"Contract progress current");
        compareFloat(source->progress().maximum,field<float>(field<void*>(original,0x54),0x238),"Contract progress maximum");
        require(source->lossSound()==field<cSound*>(original,0x7c) && source->winSound()==field<cSound*>(original,0x80),"Contract retained sounds differ");++contractStates;
    };check();
    for(unsigned product:{0U,18U,19U,0xffffffffU}) {require(source->stock(product)==function<unsigned(__thiscall*)(void*,unsigned)>(0x67fa0)(nativeTransitions[0].data(),product),"Contract stock getter differs");check();}
    source->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0xba730)(original,wrap(&definition));++contractReloads;check();
    for(auto* event:contracts) {
        stage=253;setEvent(*event);const auto& contract=std::get<ContractEvent>(event->data);
        for(unsigned n=0;n<19;++n) sourceProfile.uints["WGoodCount"+std::to_string(n)]=nativeProfile.uints["WGoodCount"+std::to_string(n)]=int(random()%71);
        contractTutorial=false;field<int>(contractTutorialObject.data(),0xbfc)=-1;interaction.active=nullptr;interaction.pending=0;putPointer(0x2604dc,nullptr);field<unsigned>(image,0x2604e0)=0;
        for(unsigned n=0;n<3;++n) {transitionViews[n]->hide();transitionViews[n]->disable();field<unsigned>(nativeTransitions[n].data(),0x4c)=0;}
        const bool restored=event->index%2;
        if(restored) {sourceProfile.floats["taskTime"+std::to_string(identifier)]=nativeProfile.floats["taskTime"+std::to_string(identifier)]=float(event->index%600);}
        source->assign(*event,restored);function<void(__thiscall*)(void*,void*,bool)>(0xbd2f0)(original,nativeEvent.data(),restored);++contractAssignments;check();
        contractCanShow=event->index%3==0;source->enabledStep();call(0xb98e0,original);check();
        for(action=0;action<8;++action) {
            stage=254;contractTutorial=action==2;field<int>(contractTutorialObject.data(),0xbfc)=contractTutorial ? 0x12:-1;contractTutorialStep=action==3;
            source->goalClicked();function<void(__cdecl*)(void*)>(0xb9350)(field<void*>(original,0x34));++contractClicks;check();
            if(action==4) {for(const auto& req:contract.requirements) if(req.reqType==0) sourceProfile.uints["WGoodCount"+std::to_string(req.productType)]=nativeProfile.uints["WGoodCount"+std::to_string(req.productType)]=int(req.count);source->dirty=true;field<unsigned char>(original,0x71)=1;source->refreshRequirements();call(0xba0b0,original);}
            elapsed=std::array<float,8>{0,.99f,.1f,1.f,2.75f,0,3.25f,-.5f}[action];
            if(action==3) {depot.showTransparently();field<unsigned>(nativeTransitions[0].data(),0x4c)=1;}else {depot.hide();field<unsigned>(nativeTransitions[0].data(),0x4c)=0;}
            source->absoluteStep();call(0xbcc20,original);++contractSteps;check();
        }
    }
    // Controlled numeric/profile edge states retain real owned UI and actual
    // original methods. They exercise expiry continuation and both completion branches.
    GameplayEvent edge(GameplayEventType::Contract);edge.index=0x80000001U;auto& contract=std::get<ContractEvent>(edge.data);
    contract.winTime={1,0,0};contract.bonus=-17;contract.penalty=73;contract.winNote=WString(L"win-note");contract.lossNote=WString(L"loss-note");
    contract.requirements[0]={};contract.requirements[1]={};
    for(action=0;action<512;++action) {
        stage=255;contract.requirements[0].reqType=action%3;contract.requirements[1].reqType=(action/3)%3;
        for(unsigned n=0;n<2;++n) {contract.requirements[n].productType=random()%19;contract.requirements[n].count=action%17==0 ? 0xffffffffU:random()%83;}
        setEvent(edge);contractTutorial=action%11==0;field<int>(contractTutorialObject.data(),0xbfc)=contractTutorial ? 0x12:-1;
        for(unsigned n=0;n<3;++n) {transitionViews[n]->hide();field<unsigned>(nativeTransitions[n].data(),0x4c)=0;}
        if(action%13==0) {farm.showTransparently();field<unsigned>(nativeTransitions[1].data(),0x4c)=1;}
        if(action%19==0) {manufacture.showTransparently();field<unsigned>(nativeTransitions[2].data(),0x4c)=2;}
        source->assign(edge,false);function<void(__thiscall*)(void*,void*,bool)>(0xbd2f0)(original,nativeEvent.data(),false);++contractAssignments;check();
        source->remaining=field<GameplayTime>(original,0x5c)=action%7==0 ? GameplayTime{1,0,0}:GameplayTime{0,action%61,action%63};
        source->accumulator=field<float>(original,0x58)=action%9==0 ? .99999994f:float(int(random()%100)-50)/13.f;
        source->expired=bool(field<unsigned char>(original,0x84)=action%5==0);source->active=bool(field<unsigned char>(original,0x70)=action%23!=0);
        source->setPassCancel(action%2);function<void(__thiscall*)(void*,bool)>(0xbc840)(original,action%2);
        sourceProfile.ints["Money"]=nativeProfile.ints["Money"]=action%2 ? INT_MAX:31;
        sourceProfile.uints["awQTask"]=nativeProfile.uints["awQTask"]=action%3 ? action:0xffffffffU;
        elapsed=action%9==0 ? 0.000000035f:float(int(random()%100)-20)/7.f;
        source->absoluteStep();call(0xbcc20,original);++contractSteps;check();
        contractTutorialStep=action%2;source->goalClicked();function<void(__cdecl*)(void*)>(0xb9350)(field<void*>(original,0x34));++contractClicks;check();
    }
    contractTutorial=false;field<int>(contractTutorialObject.data(),0xbfc)=-1;contractCanShow=false;
    for(unsigned n=0;n<3;++n) {transitionViews[n]->hide();field<unsigned>(nativeTransitions[n].data(),0x4c)=0;}
    views.updateTransitions([] {return 0.f;});require(views.transitions().empty(),"Contract fixture transitions remain queued");
    source->assign(*contracts[trial],false);setEvent(*contracts[trial]);function<void(__thiscall*)(void*,void*,bool)>(0xbd2f0)(original,nativeEvent.data(),false);
    source->dirty=false;field<unsigned char>(original,0x71)=0;elapsed=0;
    menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
    GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
    std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    const auto dialogCount=[] {return std::count_if(contractSourceCalls.begin(),contractSourceCalls.end(),[](const auto& call){return call[0]==6 || call[0]==7;});};const auto inputDialogs=dialogCount();
    const Point2 goalCenter{source->goalButton().geometry().x,source->goalButton().geometry().y};
    for(action=0;action<8;++action) {
        stage=256;menuMilliseconds+=40;menuPoint=(action==0 || action==4) ? Point2{790,590}:goalCenter;menuInputEvents=action==2 || action==6 ? std::vector<unsigned>{0x208}:action==3 || action==7 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};menuMouseDelta={1,0,0};
        contractTutorial=action>=5;contractTutorialStep=false;field<int>(contractTutorialObject.data(),0xbfc)=contractTutorial ? 0x13:-1;
        contractPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++contractFrames;check();
    }
    require(dialogCount()==inputDialogs+1,"Contract input release failed or tutorial blocked release opened a dialog");++contractInputClicks;
    menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    auto* loss=source->lossSound();auto* win=source->winSound();const auto lossRefs=loss->referenceCount(),winRefs=win->referenceCount();
    const auto beforeDelete=textureCounts();source.reset();require(loss->referenceCount()==lossRefs-1 && win->referenceCount()==winRefs-1,"Contract source destructor sound release differs");const auto middleDelete=textureCounts();call(0xb92c0,original);compareReferenceDeltas(beforeDelete,middleDelete);require(loss->referenceCount()==lossRefs-2 && win->referenceCount()==winRefs-2,"Contract native destructor sound release differs");
    require(controls.visible().empty(),"Contract destructor leaves controls visible");++contractObjects;
    field<WString>(nativeEvent.data(),0x68).~WString();field<WString>(nativeEvent.data(),0x84).~WString();
}
putPointer(0x25f73c,nullptr);
