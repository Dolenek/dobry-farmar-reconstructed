stage=350;for(const auto& entry:tutorialEntries) write(entry.first,entry.second.data(),5);
redirect(0xd0070,reinterpret_cast<void*>(&nativeDiscard));putPointer(0x25f73c,nullptr);
redirect(0x180ed3,reinterpret_cast<void*>(&tutorialVectorDestroy));
redirect(0x2fe80,reinterpret_cast<void*>(&decimalString));redirect(0x1cc0,reinterpret_cast<void*>(&concatenateString));
Control managerAnchor(nullptr,manager,views,controls);auto* nativeManagerAnchor=allocate(3000);std::memset(nativeManagerAnchor,0,3000);function<void*(__thiscall*)(void*,void*)>(0xedc80)(nativeManagerAnchor,nullptr);
TutorialWorldFixture tutorialWorld(world,managerAnchor);
TutorialManagerContext tutorialContext{manager,settings,strings,input,interaction,profile,tutorialWorld,views,controls};
auto* tutorial=new TutorialManager(tutorialContext);world.tutorial=tutorial;auto* nativeTutorial=allocate(3104);std::memset(nativeTutorial,0,3104);field<int>(nativeTutorial,0xc00)=-1;
function<void*(__thiscall*)(void*)>(0xcfce0)(nativeTutorial);
const auto managerCheck=[&] {
    stage=351;mainMenuCompareView(*tutorial,nativeTutorial);
    require(TutorialManager::instance()==tutorial && field<void*>(image,0x25f73c)==nativeTutorial,"Tutorial manager singleton differs");
    require(tutorial->currentStep==field<int>(nativeTutorial,0xbfc) && tutorial->justProcessed==field<int>(nativeTutorial,0xc00),"Tutorial progression differs");
    require(tutorial->cancelRequested==bool(field<unsigned char>(nativeTutorial,0xc08)) && tutorial->tutorialEnabled==bool(field<unsigned char>(nativeTutorial,0xc09)),"Tutorial enable/cancel differs");
    for(auto pair:std::initializer_list<std::pair<bool,unsigned>>{{tutorial->resumeSelection,0xc04},{tutorial->resumeOverlay,0xc05},{tutorial->resumeHUD,0xc06},{tutorial->resumeHUDabsolute,0xc07}}) require(pair.first==bool(field<unsigned char>(nativeTutorial,pair.second)),"Tutorial saved control flags differ");
    compareFloat(tutorial->elapsed,field<float>(nativeTutorial,0xc0c),"Tutorial elapsed");compareFloat(tutorial->valueC10,field<float>(nativeTutorial,0xc10),"Tutorial c10");require(tutorial->valueC14==field<unsigned>(nativeTutorial,0xc14),"Tutorial c14 differs");compareFloat(tutorial->valueC18,field<float>(nativeTutorial,0xc18),"Tutorial c18");
    for(unsigned i=0;i<50;++i) {const auto& s=tutorial->steps[i];auto* n=static_cast<unsigned char*>(nativeTutorial)+0x30+i*60;require(s.index==field<int>(n,0) && s.text.view()==field<WString>(n,4).view(),"Tutorial step XML text/index differs");for(unsigned d=0;d<2;++d) {compareFloat(s.offset[d],field<float>(n,0x20+d*4),"Tutorial step offset");compareFloat(s.arrowOffset[d],field<float>(n,0x28+d*4),"Tutorial step arrow offset");}compareFloat(s.multiplier,field<float>(n,0x30),"Tutorial step multiplier");compareFloat(s.interlinear,field<float>(n,0x34),"Tutorial step interlinear");require(bool(s.predicate)==bool(field<void*>(n,0x38)),"Tutorial installed predicate differs");}
    for(auto pair:std::initializer_list<std::pair<TutorialCommonTip*,unsigned>>{{&tutorial->common(),0xbe8},{&tutorial->arrow(),0xbec},{&tutorial->disabledAnchor(),0xbf0},{&tutorial->action(),0xbf4},{&tutorial->repeatedAction(),0xbf8}}) {auto* n=field<void*>(nativeTutorial,pair.second);mainMenuCompareView(*pair.first,n);compareControl(pair.first->image(),field<void*>(n,0x40));compareControl(pair.first->text(),field<void*>(n,0x44));compareControl(pair.first->okButton(),field<void*>(n,0x48));}
    mainMenuCompareView(overlay,nativeOverlay);mainMenuCompareView(selection,nativeSelection);compareControl(managerAnchor,nativeManagerAnchor);check();++tutorialManagerStates;
};managerCheck();
// Prevent scheduler execution until controlled world bindings are supplied.
tutorial->disable();call(0xd3750,nativeTutorial);managerCheck();
for(trial=0;trial<128;++trial) {
    stage=352;tutorial->tutorialEnabled=bool(trial%3);field<unsigned char>(nativeTutorial,0xc09)=tutorial->tutorialEnabled;
    tutorial->currentStep=trial%4 ? -1:7;field<int>(nativeTutorial,0xbfc)=tutorial->currentStep;
    const unsigned index=trial%4==0 ? ~0U:trial%50;
    require(tutorial->isProcessed(index)==function<bool(__thiscall*)(void*,unsigned)>(0xd0990)(nativeTutorial,index),"Tutorial processed query differs");managerCheck();
    require(tutorial->canShow(index)==function<bool(__thiscall*)(void*,unsigned)>(0xd0a20)(nativeTutorial,index),"Tutorial availability query differs");managerCheck();
    tutorial->complete(index);function<void(__thiscall*)(void*,unsigned)>(0xd0ac0)(nativeTutorial,index);managerCheck();
}
tutorial->tutorialEnabled=true;field<unsigned char>(nativeTutorial,0xc09)=1;
for(trial=0;trial<50;++trial) {
    stage=353;tutorial->currentStep=field<int>(nativeTutorial,0xbfc)=-1;
    tutorial->showCommon(trial);function<void(__thiscall*)(void*,unsigned)>(0xce650)(nativeTutorial,trial);++tutorialManagerStarts;managerCheck();
    tutorial->common().okButton().clickEvent(&tutorial->common().okButton());function<void(__cdecl*)(void*)>(0xcda00)(field<void*>(field<void*>(nativeTutorial,0xbe8),0x48));managerCheck();
    const Point2 at{float(trial%2 ? 500:200),float(trial%3 ? 500:200)};managerAnchor.setPosition(at);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(nativeManagerAnchor,&at);
    tutorial->showArrow(trial,&managerAnchor);function<void(__thiscall*)(void*,unsigned,void*)>(0xceb00)(nativeTutorial,trial,nativeManagerAnchor);++tutorialManagerStarts;managerCheck();
    tutorial->arrow().okButton().clickEvent(&tutorial->arrow().okButton());function<void(__cdecl*)(void*)>(0xcda00)(field<void*>(field<void*>(nativeTutorial,0xbec),0x48));managerCheck();
    if(trial%2) {managerAnchor.disable();call(0xd3750,nativeManagerAnchor);}else {managerAnchor.enable();call(0xd3bb0,nativeManagerAnchor);}
    tutorial->showDisabledAnchor(trial,&managerAnchor);function<void(__thiscall*)(void*,unsigned,void*)>(0xcef60)(nativeTutorial,trial,nativeManagerAnchor);++tutorialManagerStarts;managerCheck();
    tutorial->disabledAnchor().okButton().clickEvent(&tutorial->disabledAnchor().okButton());function<void(__cdecl*)(void*)>(0xcc320)(field<void*>(field<void*>(nativeTutorial,0xbf0),0x48));managerCheck();
}
// Installed predicate 15 reads the actual global seed-move flag; finish uses
// the actual source/native manager and profile, including the repeated pass.
for(trial=0;trial<12;++trial) {
    stage=354;tutorialInputs.seed=bool(trial%3==0);field<unsigned char>(image,0x25f721)=tutorialInputs.seed;
    tutorial->currentStep=field<int>(nativeTutorial,0xbfc)=-1;
    if(trial%2) {tutorial->showRepeatedAction(15,&managerAnchor);function<void(__thiscall*)(void*,unsigned,void*)>(0xcf7d0)(nativeTutorial,15,nativeManagerAnchor);}else {tutorial->showAction(15,&managerAnchor);function<void(__thiscall*)(void*,unsigned,void*)>(0xcf3c0)(nativeTutorial,15,nativeManagerAnchor);}++tutorialManagerStarts;managerCheck();
    tutorial->requestCancel();call(0xcdf10,nativeTutorial);managerCheck();explicitTipElapsed=true;elapsed=0.09f;
    if(trial%2) {tutorial->repeatedAction().enabledStep();call(0xcac10,field<void*>(nativeTutorial,0xbf8));}else {tutorial->action().enabledStep();call(0xca690,field<void*>(nativeTutorial,0xbf4));}++tutorialManagerSteps;explicitTipElapsed=false;managerCheck();
}
field<unsigned char>(image,0x25f721)=0;tutorialInputs.seed=false;
// Every installed original predicate, including ordered repeated activity
// reads, short circuits and the unspilled original growth/capacity results.
tutorialNativeControl=nativeManagerAnchor;tutorialNativeMower=allocate(1400);tutorialNativeItemUpgrade=allocate(64);
putPointer(0x25f750,tutorialNativeControl);putPointer(0x25f754,tutorialNativeMower);
std::array<void*,94> tutorialCharacterTable{};tutorialCharacterTable[92]=reinterpret_cast<void*>(&tutorialCharacterActivity);field<void*>(nativeCharacter,0)=tutorialCharacterTable.data();putPointer(0x25ff5c,nativeCharacter);hudInputs.character=true;
for(auto pair:std::initializer_list<std::pair<unsigned,void*>>{{0x17050,reinterpret_cast<void*>(&tutorialUpgrade)},{0x4ec00,reinterpret_cast<void*>(&tutorialHeldType)},{0x4f120,reinterpret_cast<void*>(&tutorialHeldWater)},{0x683a0,reinterpret_cast<void*>(&tutorialCapacity)},
    {0x31df0,reinterpret_cast<void*>(&tutorialList<9>)},{0x32260,reinterpret_cast<void*>(&tutorialList<7>)},{0x98400,reinterpret_cast<void*>(&tutorialList<12>)},{0x37d90,reinterpret_cast<void*>(&tutorialProgress)},{0x5cea0,reinterpret_cast<void*>(&tutorialPlant)},{0x36d90,reinterpret_cast<void*>(&tutorialPlotKind)},
    {0x9f010,reinterpret_cast<void*>(&tutorialParentProducts)},{0x4edd0,reinterpret_cast<void*>(&tutorialTool)},{0x67fa0,reinterpret_cast<void*>(&tutorialWarehouseProducts)},{0x6eb70,reinterpret_cast<void*>(&tutorialManufactureProducts)},{0x65e70,reinterpret_cast<void*>(&tutorialWarehouseFree)},{0x99c80,reinterpret_cast<void*>(&tutorialWareSelection)}}) redirect(pair.first,pair.second);
std::array<std::unique_ptr<View>,4> predicateDialogs;std::array<void*,4> nativePredicateDialogs{};const unsigned predicateDialogGlobals[]={0x25f76c,0x25f768,0x25f764,0x25f760};
for(unsigned i=0;i<4;++i) {predicateDialogs[i]=std::make_unique<View>(nullptr,views);tutorialWorld.dialogs[i]=predicateDialogs[i].get();nativePredicateDialogs[i]=allocate(44);function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativePredicateDialogs[i],nullptr);putPointer(predicateDialogGlobals[i],nativePredicateDialogs[i]);}
std::mt19937 tutorialRandom(0x4c9ff0);
for(trial=0;trial<256;++trial) {
    stage=356;tutorialInputs.wellUpgrade=int(tutorialRandom()%3)-1;tutorialInputs.mowerUpgrade=int(tutorialRandom()%3)-1;tutorialInputs.heldType=int(tutorialRandom()%12)-1;
    tutorialInputs.activity=int(tutorialRandom()%14)-2;tutorialInputs.varyingActivity=trial%2;for(auto& a:tutorialInputs.activities) a=int(tutorialRandom()%14)-2;
    tutorialInputs.plant=int(trial%3)-1;tutorialInputs.itemUpgrade=int(trial%3)-1;tutorialInputs.itemFlags=trial;tutorialInputs.anchorState=int(trial%3);tutorialInputs.plotKind=int(trial%3);
    tutorialInputs.parentProducts=tutorialInputs.warehouseProducts=tutorialInputs.manufactureProducts=trial%3;tutorialInputs.free=trial%3;
    tutorialInputs.water=float(trial%7);tutorialInputs.capacity=trial%3 ? tutorialInputs.water:double(tutorialInputs.water)+0.5;
    tutorialInputs.previous=float(trial%5);tutorialInputs.progress=trial%3 ? tutorialInputs.previous:double(tutorialInputs.previous)+0.5;
    tutorialInputs.plots=trial%4;tutorialInputs.items=trial%4;tutorialInputs.plants=1;tutorialInputs.seed=trial%2;tutorialInputs.tool=trial%2;tutorialInputs.selection=trial%2;tutorialInputs.sceneSelection=trial%2;tutorialInputs.wareSelection=trial%2;
    field<unsigned char>(image,0x25f721)=tutorialInputs.seed;field<void*>(nativeSelection,0x248)=tutorialInputs.selection ? nativeManagerAnchor:nullptr;field<void*>(nativeOverlay,0x248)=tutorialInputs.sceneSelection ? nativeManagerAnchor:nullptr;
    field<float>(nativeManagerAnchor,0x8a8)=tutorialInputs.previous;field<unsigned char>(nativeManagerAnchor,0x574)=static_cast<unsigned char>(tutorialInputs.itemFlags);
    for(unsigned i=0;i<3;++i) {scenes[i]->movement=(trial>>(i+1))&1;field<unsigned>(nativeScenes[i],0x4c)=scenes[i]->movement;}
    for(unsigned i=0;i<4;++i) {if((trial>>i)&1) {predicateDialogs[i]->show();call(0xd37d0,nativePredicateDialogs[i]);}else {predicateDialogs[i]->hide();call(0xd3720,nativePredicateDialogs[i]);}}
    for(unsigned i=0;i<50;++i) if(tutorial->steps[i].predicate) {
        if(i==34 || i==37) tutorialInputs.items=1;else tutorialInputs.items=trial%4;
        field<void*>(nativeManagerAnchor,0x364)=i==34 ? tutorialNativeItemUpgrade:reinterpret_cast<void*>(tutorialInputs.anchorState);
        tutorialSourceQueries.clear();tutorialNativeQueries.clear();tutorialSourceActivity=tutorialNativeActivity=0;
        const bool actual=tutorial->evaluate(i,&managerAnchor);auto* test=field<void*>(static_cast<unsigned char*>(nativeTutorial)+0x30+i*60,0x38);
        const bool expected=reinterpret_cast<bool(__cdecl*)(void*)>(test)(nativeManagerAnchor);
        if(actual!=expected || tutorialSourceQueries!=tutorialNativeQueries) {std::cerr<<"Tutorial predicate "<<i<<" result "<<actual<<'/'<<expected<<" queries";for(auto q:tutorialSourceQueries) std::cerr<<" S"<<q;for(auto q:tutorialNativeQueries) std::cerr<<" N"<<q;std::cerr<<'\n';}
        require(actual==expected && tutorialSourceQueries==tutorialNativeQueries,"Tutorial predicate result/order differs");require(tutorialSourceActivity==tutorialNativeActivity,"Tutorial activity query count differs");++tutorialManagerPredicates;
    }
    managerCheck();
}
// Actual enabled-step dispatch: initial water tutorial, plot completion,
// warehouse follow-up and production-complete modal, with typed profile state.
for(trial=0;trial<128;++trial) {
    stage=357;tutorial->currentStep=field<int>(nativeTutorial,0xbfc)=trial%7 ? -1:42;
    tutorial->justProcessed=field<int>(nativeTutorial,0xc00)=trial%3 ? 4:0;
    tutorial->tutorialEnabled=bool(trial%11);field<unsigned char>(nativeTutorial,0xc09)=tutorial->tutorialEnabled;
    for(unsigned step:{1U,2U,5U,43U}) profile.booleans[TutorialManager::profileKey(step)]=nativeProfile.booleans[TutorialManager::profileKey(step)]=bool((trial>>(step%5))&1);
    profile.booleans["TProdCompl"]=nativeProfile.booleans["TProdCompl"]=bool(trial%2);profile.floats["GameTime"]=nativeProfile.floats["GameTime"]=float(trial%13);
    tutorialInputs.plots=1+trial%3;tutorialInputs.progress=trial%4 ? 0:1;tutorialInputs.heldType=int(trial%3);
    field<float>(nativeWarehouse,0x5c)=managerAnchor.geometry().x;field<float>(nativeWarehouse,0x60)=managerAnchor.geometry().y;
    for(unsigned i=0;i<3;++i) {scenes[i]->movement=(trial>>(i+1))&1;field<unsigned>(nativeScenes[i],0x4c)=scenes[i]->movement;}
    if(trial%5) {scenes[0]->show();call(0xd37d0,nativeScenes[0]);}else {scenes[0]->hide();call(0xd3720,nativeScenes[0]);}
    tutorialSourceQueries.clear();tutorialNativeQueries.clear();explicitTipElapsed=true;elapsed=0.031f*(1+trial%4);
    tutorial->enabledStep();call(0xd1160,nativeTutorial);explicitTipElapsed=false;++tutorialManagerSteps;
    require(tutorialSourceQueries==tutorialNativeQueries,"Tutorial enabled-step world query order differs");managerCheck();
    // Hide whichever actual tip was opened through its manager; this cleanup
    // keeps the next controlled world scenario independent.
    if(tutorial->active()) {
        TutorialCommonTip* sourceTip=tutorial->currentStep==43 ? &tutorial->common():&tutorial->action();auto* counterpart=field<void*>(nativeTutorial,tutorial->currentStep==43 ? 0xbe8:0xbf4);
        sourceTip->hide();call(0xd3720,counterpart);tutorial->finishTip(*sourceTip);function<void(__thiscall*)(void*,void*)>(0xd0c30)(nativeTutorial,counterpart);tutorial->setGameplayPaused(false);field<unsigned char>(original,0xa4)=0;managerCheck();
    }
}
for(unsigned i=0;i<4;++i) {predicateDialogs[i].reset();call(0xd58a0,nativePredicateDialogs[i]);putPointer(predicateDialogGlobals[i],nullptr);}
putPointer(0x25ff5c,nullptr);hudInputs.character=false;putPointer(0x25f750,nullptr);putPointer(0x25f754,nullptr);field<unsigned char>(image,0x25f721)=0;
redirect(0x17050,reinterpret_cast<void*>(&hudLevel));redirect(0x6eb70,reinterpret_cast<void*>(&hudProducts));redirect(0x65e70,reinterpret_cast<void*>(&hudFree));
// Actual source/native manager, HUD and five windows in the frame scheduler,
// with visible GPU pixels, OK mouse input and real profile completion effects.
tutorial->tutorialEnabled=true;field<unsigned char>(nativeTutorial,0xc09)=1;
tutorial->cancelRequested=false;field<unsigned char>(nativeTutorial,0xc08)=0;tutorialInputs.seed=false;field<unsigned char>(image,0x25f721)=0;
for(auto pair:std::initializer_list<std::pair<TutorialCommonTip*,unsigned>>{{&tutorial->common(),0xbe8},{&tutorial->arrow(),0xbec},{&tutorial->disabledAnchor(),0xbf0},{&tutorial->action(),0xbf4},{&tutorial->repeatedAction(),0xbf8}}) {pair.first->hide();call(0xd3720,field<void*>(nativeTutorial,pair.second));}
resetScene(0);hud->show();hud->enable();hud->absoluteEnable();call(0xd37d0,original);call(0xd3bb0,original);call(0xd3fb0,original);managerCheck();
const auto managerFrame=[&](Point2 p=Point2{0,0},std::vector<unsigned> e={},std::array<int,3> d={}) {try {frame(p,std::move(e),d);} catch(...) {for(auto pair:std::initializer_list<std::pair<TutorialCommonTip*,unsigned>>{{&tutorial->common(),0xbe8},{&tutorial->arrow(),0xbec},{&tutorial->disabledAnchor(),0xbf0},{&tutorial->action(),0xbf4},{&tutorial->repeatedAction(),0xbf8}}) {auto* n=field<void*>(nativeTutorial,pair.second);std::cerr<<"Tip slot "<<std::hex<<pair.second<<std::dec<<" root "<<pair.first->state().transparency<<"/"<<field<float>(n,8)<<" transition "<<pair.first->state().transition<<"/"<<field<unsigned>(n,16)<<" depth "<<pair.first->originalDepth<<"/"<<field<float>(n,0x34)<<" alpha "<<pair.first->image().alpha()<<"/"<<*field<float*>(field<void*>(n,0x40),0x58)<<" phase "<<(pair.second==0xbe8 ? 0:static_cast<TutorialArrowTip*>(pair.first)->phase)<<"/"<<(pair.second==0xbe8 ? 0:field<float>(n,0x74))<<char(10);}throw;}};
for(trial=0;trial<5;++trial) {
    stage=358;tutorial->currentStep=field<int>(nativeTutorial,0xbfc)=-1;TutorialCommonTip* tip=nullptr;
    switch(trial) {
    case 0:tutorial->showCommon(0);function<void(__thiscall*)(void*,unsigned)>(0xce650)(nativeTutorial,0);tip=&tutorial->common();break;
    case 1:tutorial->showArrow(0,&managerAnchor);function<void(__thiscall*)(void*,unsigned,void*)>(0xceb00)(nativeTutorial,0,nativeManagerAnchor);tip=&tutorial->arrow();break;
    case 2:tutorial->showDisabledAnchor(0,&managerAnchor);function<void(__thiscall*)(void*,unsigned,void*)>(0xcef60)(nativeTutorial,0,nativeManagerAnchor);tip=&tutorial->disabledAnchor();break;
    case 3:tutorial->showAction(15,&managerAnchor);function<void(__thiscall*)(void*,unsigned,void*)>(0xcf3c0)(nativeTutorial,15,nativeManagerAnchor);tip=&tutorial->action();break;
    default:tutorial->showRepeatedAction(15,&managerAnchor);function<void(__thiscall*)(void*,unsigned,void*)>(0xcf7d0)(nativeTutorial,15,nativeManagerAnchor);tip=&tutorial->repeatedAction();break;
    }++tutorialManagerStarts;managerCheck();
    for(unsigned i=0;i<8;++i) {managerFrame();managerCheck();++tutorialManagerFrames;}
    if(trial<3) {const Point2 at{tip->okButton().geometry().x,tip->okButton().geometry().y};managerFrame(at,{}, {1,0,0});managerCheck();managerFrame(at,{0x208});managerCheck();managerFrame(at,{0x200});managerCheck();managerFrame({0,0},{},{1,0,0});managerCheck();tutorialManagerFrames+=4;}
    else if(trial==3) {tutorial->requestCancel();call(0xcdf10,nativeTutorial);}else {tutorialInputs.seed=true;field<unsigned char>(image,0x25f721)=1;}
    for(unsigned i=0;i<10;++i) {managerFrame();managerCheck();++tutorialManagerFrames;}
    require(!tutorial->active(),"Tutorial real frame/input did not complete the step");
}
field<unsigned char>(image,0x25f721)=0;tutorialInputs.seed=false;
stage=355;std::array<TutorialCommonTip*,5> sourceTips{&tutorial->common(),&tutorial->arrow(),&tutorial->disabledAnchor(),&tutorial->action(),&tutorial->repeatedAction()};std::array<void*,5> nativeTips{};for(unsigned i=0;i<5;++i) nativeTips[i]=field<void*>(nativeTutorial,0xbe8+i*4);
delete tutorial;world.tutorial=nullptr;call(0xd0490,nativeTutorial);require(!TutorialManager::instance() && !field<void*>(image,0x25f73c),"Tutorial destruction retains singleton");
for(unsigned i=0;i<4;++i) {
    if(!views.discarded().count(sourceTips[i]) || !nativeDiscards.count(nativeTips[i])) {std::cerr<<"Tutorial discard missing "<<i<<" source "<<sourceTips[i]<<" native "<<nativeTips[i]<<" counts "<<views.discarded().size()<<'/'<<nativeDiscards.size()<<'\n';for(const auto& entry:nativeDiscards) std::cerr<<entry.first<<' '<<entry.second<<'\n';}
    require(views.discarded().count(sourceTips[i]) && nativeDiscards.count(nativeTips[i]),"Tutorial deferred discard object differs");
    require(views.discarded().at(sourceTips[i]).view()==nativeDiscards.at(nativeTips[i]),"Tutorial deferred discard origin differs");
}
require(views.discarded().size()==4 && nativeDiscards.size()==4 && views.contains(sourceTips[4]),"Tutorial repeated tip retention differs");mainMenuCompareView(*sourceTips[4],nativeTips[4]);
// Diagnostic cleanup of the retained fifth tip is explicitly outside the
// manager destructor; native 004d0490 never performs this operation.
const auto tipBefore=textureCounts();views.deleteDiscarded();delete sourceTips[4];const auto tipMiddle=textureCounts();for(auto* n:nativeTips) call(0xcc770,n);compareReferenceDeltas(tipBefore,tipMiddle);nativeDiscards.clear();
call(0xf1560,nativeManagerAnchor);putPointer(0x25f73c,contractTutorialObject.data());field<int>(contractTutorialObject.data(),0xbfc)=-1;
redirect(0xd0a20,reinterpret_cast<void*>(&hudCan));redirect(0xcdef0,reinterpret_cast<void*>(&hudStep));redirect(0xcf3c0,reinterpret_cast<void*>(&hudShow));redirect(0xd0ac0,reinterpret_cast<void*>(&hudComplete));
