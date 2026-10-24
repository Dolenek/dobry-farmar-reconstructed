// Runs with the actual GameMenu, TutorialManager and moving Farm StateView.
stage=880;redirect(0x65e70,reinterpret_cast<void*>(&hudFree));redirect(0x10ce0,reinterpret_cast<void*>(&hudMoney));static unsigned sourceRemoved=0;sourceRemoved=contractLetterNativeRemoved=0;
std::array<unsigned char,5> oldRemove{};std::memcpy(oldRemove.data(),image+0x341b0,5);redirect(0x341b0,reinterpret_cast<void*>(&contractLetterRemoved));
struct LetterWorld final:ContractLetterWorld,ContractOfferWorld {
    HudWorld& hud;TutorialManager& tips;ProfileProperties& profile;unsigned& removed;
    LetterWorld(HudWorld& h,TutorialManager& t,ProfileProperties& p,unsigned& r):hud(h),tips(t),profile(p),removed(r) {}
    TutorialManager& tutorial() override {return tips;}StateView& farmScene() override {return hud.farm();}
    void removeLetter(ContractLetterSlot&) override {++removed;}
    int activeContracts() const override {return int(hud.menu->contractCount);}
    int money() const override {return hud.money();}
    void setMoney(int value) override {productionSourceCalls.push_back({13,unsigned(value),0,0,0});profile.ints["Money"]=value;}
    void consumeLetter(ContractOfferLetter& letter) override {removeLetter(dynamic_cast<ContractLetterSlot&>(letter));}
    void activateContract(GameplayEvent& event) override {hud.menu->activateContract(event);}
    bool tutorialCanShow(unsigned step) const override {return tips.canShow(step);}
    void showTutorial(unsigned step,View& anchor) override {tips.showAction(step,dynamic_cast<Control*>(&anchor));}
} letterWorld(world,*productionTutorial,profile,sourceRemoved);
ContractLetterContext letterContext{manager,settings,strings,input,interaction,profile,audio,letterWorld,views,controls};
ContractDialogContext dialogContext{manager,settings,strings,input,interaction,configuration,*hud,views,controls};
auto offer=std::make_unique<ContractProvide>(nullptr,dialogContext,letterWorld);auto* nativeOffer=allocate(220);std::memset(nativeOffer,0,220);function<void*(__thiscall*)(void*,void*)>(0xad4b0)(nativeOffer,nullptr);
productionTutorialWorld.dialogs[3]=offer.get();
std::array<std::unique_ptr<ContractLetterSlot>,3> letters;std::array<void*,3> nativeLetters{};
for(unsigned i=0;i<3;++i) {letters[i]=std::make_unique<ContractLetterSlot>(&scenes[0]->origin(),i,letterContext);nativeLetters[i]=allocate(1020);std::memset(nativeLetters[i],0,1020);function<void*(__thiscall*)(void*,void*,unsigned)>(0x24ba0)(nativeLetters[i],field<void*>(nativeScenes[0],0x60),i);}
std::array<unsigned char,256> nativeEvent{};new(nativeEvent.data()+0x14) WString();new(nativeEvent.data()+0x68) WString();new(nativeEvent.data()+0x84) WString();
const auto writeEvent=[&](GameplayEvent& event) {const auto& c=std::get<ContractEvent>(event.data);field<WString>(nativeEvent.data(),0x14)=event.note;field<unsigned>(nativeEvent.data(),0x34)=event.index;field<GameplayTime>(nativeEvent.data(),0x38)=c.winTime;field<GameplayTime>(nativeEvent.data(),0xa0)=c.letterTime;field<int>(nativeEvent.data(),0x44)=c.bonus;field<int>(nativeEvent.data(),0x48)=c.penalty;std::memcpy(nativeEvent.data()+0x50,c.requirements.data(),24);field<WString>(nativeEvent.data(),0x68)=c.winNote;field<WString>(nativeEvent.data(),0x84)=c.lossNote;};
const auto check=[&](unsigned i,bool assigned=true) {
    stage=881;auto& s=*letters[i];auto* n=nativeLetters[i];compareControl(s,n);mainMenuCompareView(s,n);compareControl(s.image(),field<void*>(n,0x378));compareControl(s.progress(),field<void*>(n,0x370));
    require(s.id==field<unsigned>(n,0x360) && s.letterState==field<unsigned>(n,0x364),"Letter id/state differs");
    for(auto v:std::initializer_list<std::pair<float,unsigned>>{{s.warningTime,0x368},{s.blinkPeriod,0x36c},{s.moveRemaining,0x3f4},{s.moveSpeed,0x3f8}}) compareFloat(v.first,field<float>(n,v.second),"Letter configuration/motion scalar");
    require(!std::memcmp(s.appear.data(),static_cast<unsigned char*>(n)+0x3e4,8) && !std::memcmp(s.place.data(),static_cast<unsigned char*>(n)+0x3ec,8),"Letter movement endpoints differ");
    appearanceCheck(s.readAppearance(),static_cast<unsigned char*>(n)+0x38c);appearanceCheck(s.unreadAppearance(),static_cast<unsigned char*>(n)+0x3b4);
    require(s.nearSound()==field<cSound*>(n,0x3dc) && s.expiredSound()==field<cSound*>(n,0x3e0),"Letter sound resources differ");
    if(assigned) {compareFloat(s.timeRemaining,field<float>(n,0x380),"Letter time remaining");compareFloat(s.fullDuration,field<float>(n,0x384),"Letter full duration");require(s.warning==bool(field<unsigned char>(n,0x388)),"Letter warning flag differs");if(s.warning) compareFloat(s.blinkRemaining,field<float>(n,0x374),"Letter blink remaining");compareFloat(s.progress().current,field<float>(field<void*>(n,0x370),0x23c),"Letter progress");}
    require(profile.ints==nativeProfile.ints && profile.uints==nativeProfile.uints && profile.floats==nativeProfile.floats && profile.booleans==nativeProfile.booleans,"Letter profile differs");
    require(sourceRemoved==contractLetterNativeRemoved && actualSounds==expectedSounds,"Letter removal/sound calls differ");if(productionSourceCalls!=productionNativeCalls) {for(const auto& row:productionSourceCalls) std::cerr<<"S call "<<row[0]<<' '<<row[1]<<"\n";for(const auto& row:productionNativeCalls) std::cerr<<"N call "<<row[0]<<' '<<row[1]<<"\n";}require(productionSourceCalls==productionNativeCalls,"Letter connected offer money calls differ");require(hudSourceCalls==hudNativeCalls,"Letter connected HUD calls differ");hudSourceCalls.clear();hudNativeCalls.clear();productionSourceCalls.clear();productionNativeCalls.clear();actualSounds.clear();expectedSounds.clear();++contractLetterStates;
};
for(unsigned i=0;i<3;++i) check(i,false);
const auto reloadPath=common/"letter-reload";std::filesystem::create_directories(reloadPath);
std::ofstream(reloadPath/"letter-reload.xml")<<R"(<Root><Empty/><Changed FlashTimeRemaind="4.25" FlashPeriod="0.13" MoveSpeed="213.7" AppearPosition="-11.2 77.3"><Pic size="48 53"/></Changed><NoSound NearPuffSound="" PuffSound=""/></Root>)";
files.mountCommon(String(reloadPath.generic_string()));XmlDocument reloadDefinition(files,String("letter-reload.xml")),originalLetterXML(files,String("UI/gameUI/Farm/ContractLetterSlot.xml"));
for(unsigned i=0;i<3;++i) {for(const auto* name:{"Empty","Changed","NoSound"}) {XmlNode* node=nullptr;require(reloadDefinition.section(String(name),node),"Letter reload fixture missing");letters[i]->loadXML(*node);function<void(__thiscall*)(void*,void*)>(0x24320)(nativeLetters[i],wrap(node));check(i,false);}letters[i]->loadXML(originalLetterXML);function<void(__thiscall*)(void*,void*)>(0x24320)(nativeLetters[i],wrap(&originalLetterXML));check(i,false);}
XmlDocument timeline(files,String("Gameplay/Gameplay.xml"));GameplayEventDefinitions definitions;definitions.loadXML(timeline);std::vector<GameplayEvent*> contracts;for(auto& p:definitions.events()) if(p->type==GameplayEventType::Contract) contracts.push_back(p.get());require(contracts.size()==131,"Letter timeline contract count differs");
const auto assign=[&](unsigned i,GameplayEvent& event,unsigned state,bool restore) {writeEvent(event);letters[i]->assign(event,state,restore);function<void(__thiscall*)(void*,void*,unsigned,bool)>(0x25770)(nativeLetters[i],nativeEvent.data(),state,restore);++contractLetterAssignments;check(i);};
const auto step=[&](unsigned i,float dt) {elapsed=dt;letters[i]->absoluteStep();call(0x250d0,nativeLetters[i]);++contractLetterActions;check(i);};
for(trial=0;trial<contracts.size();++trial) {
    const unsigned i=trial%3;auto& s=*letters[i];auto* n=nativeLetters[i];s.place={float(140+i*220),110};field<Point2>(n,0x3ec)=s.place;
    assign(i,*contracts[trial],trial%2,false);
    step(i,0);step(i,.013f);step(i,100);
    require(s.moveRemaining==0 && s.state().enabled,"Letter did not finish incoming motion");
    step(i,0);step(i,.013f);step(i,1);step(i,10000);
    // Restore reads profile coordinates exactly, without adding Farm movement.
    profile.floats["letTime"+std::to_string(i)]=nativeProfile.floats["letTime"+std::to_string(i)]=45.25f;
    profile.ints["letState"+std::to_string(i)]=nativeProfile.ints["letState"+std::to_string(i)]=trial%2;
    assign(i,*contracts[trial],0,true);step(i,.013f);step(i,.5f);step(i,.5f);
}
// Exact motion expiry stays disabled for that frame; the next countdown frame
// has no implicit enable. Tutorial freezes motion and deadline at zero elapsed.
GameplayEvent edge(GameplayEventType::Contract);edge.index=707;auto& ec=std::get<ContractEvent>(edge.data);ec.letterTime={0,1,30};ec.winTime={0,2,0};ec.penalty=0;ec.bonus=123;edge.note=WString(L"A contract offer");
for(unsigned i=0;i<3;++i) {
    auto& s=*letters[i];auto* n=nativeLetters[i];s.place=s.appear;field<Point2>(n,0x3ec)=s.place;assign(i,edge,0,false);step(i,0);step(i,-.01f);
    productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=7;step(i,2);productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
    s.place={s.appear[0]+100,s.appear[1]};field<Point2>(n,0x3ec)=s.place;s.moveSpeed=field<float>(n,0x3f8)=100;assign(i,edge,0,false);step(i,1);require(!s.state().enabled,"Letter exact arrival unexpectedly enables");step(i,0);
    profile.floats["letTime"+std::to_string(i)]=nativeProfile.floats["letTime"+std::to_string(i)]=.25f;profile.ints["letState"+std::to_string(i)]=nativeProfile.ints["letState"+std::to_string(i)]=0;assign(i,edge,0,true);step(i,.25f);
}
// Actual click invokes complete tutorial17 and the real ContractProvide body.
assign(0,edge,0,true);letters[0]->clicked();call(0x25d90,nativeLetters[0]);++contractLetterActions;check(0);require(offer->letter()==letters[0].get() && field<void*>(nativeOffer,0xd8)==nativeLetters[0],"Letter did not open actual offer");mainMenuCompareView(*offer,nativeOffer);offer->wait();function<void(__cdecl*)()>(0xad0d0)();check(0);
productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=7;letters[0]->clicked();call(0x25d90,nativeLetters[0]);++contractLetterActions;check(0);productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
// Whole frames: incoming letter, actual hover/press/release, opened offer.
for(unsigned i=0;i<3;++i) {letters[i]->letterState=field<unsigned>(nativeLetters[i],0x364)=2;letters[i]->hideTips();call(0x8fb50,nativeLetters[i]);}
auto* previousParent=scenes[0]->parent();auto* nativePreviousParent=field<void*>(nativeScenes[0],0x20);
scenes[0]->setParent(hud);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],original);
game.show();game.enable();game.absoluteEnable();call(0xd37d0,nativeGame);call(0xd3bb0,nativeGame);call(0xd3fb0,nativeGame);
scenes[0]->show();scenes[0]->enable();scenes[0]->absoluteEnable();call(0xd37d0,nativeScenes[0]);call(0xd3bb0,nativeScenes[0]);call(0xd3fb0,nativeScenes[0]);
letters[0]->place={360,140};field<Point2>(nativeLetters[0],0x3ec)=letters[0]->place;letters[0]->appear={100,140};field<Point2>(nativeLetters[0],0x3e4)=letters[0]->appear;letters[0]->moveSpeed=field<float>(nativeLetters[0],0x3f8)=400;assign(0,edge,0,false);
menuMilliseconds=0;explicitTipElapsed=false;GameTimer timer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor cursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&cursor;
std::array<unsigned char,24> frameCursor{};field<void*>(frameCursor.data(),0)=cursorTable.data();nativeCursorFrame=frameCursor.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
for(unsigned frame=0;frame<48;++frame) {stage=882;action=frame;menuMilliseconds+=250;menuPoint=frame>=17 && frame<=19 ? Point2{360,140}:Point2{790,590};menuMouseDelta=frame==17 || frame==20 ? std::array<int,3>{1,0,0}:std::array<int,3>{};menuInputEvents=frame==18 ? std::vector<unsigned>{0x208}:frame==19 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};contractLetterPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++contractLetterFrames;check(0);mainMenuCompareView(*offer,nativeOffer);if(frame==35) {offer->wait();function<void(__cdecl*)()>(0xad0d0)();check(0);}}
require(letters[0]->letterState==1 && offer->letter()==letters[0].get() && field<void*>(nativeOffer,0xd8)==nativeLetters[0],"Letter mouse input did not open and read actual offer");
// Actual tutorial17 starts from the letter deadline and observes this exact
// actor's read state; real mouse input acknowledges it and opens the offer.
productionTutorial->tutorialEnabled=true;field<unsigned char>(nativeProductionTutorial,0xc09)=1;profile.booleans["tip17"]=nativeProfile.booleans["tip17"]=false;profile.floats["letTime0"]=nativeProfile.floats["letTime0"]=80;profile.ints["letState0"]=nativeProfile.ints["letState0"]=0;assign(0,edge,0,true);bool showedTutorial=false;
for(unsigned frame=0;frame<32;++frame) {stage=883;action=frame;menuMilliseconds+=250;menuPoint=frame>=5 && frame<=7 ? Point2{360,140}:Point2{790,590};menuMouseDelta=frame==5 || frame==8 ? std::array<int,3>{1,0,0}:std::array<int,3>{};menuInputEvents=frame==6 ? std::vector<unsigned>{0x208}:frame==7 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};contractLetterPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++contractLetterFrames;check(0);showedTutorial=showedTutorial || productionTutorial->activeStep(17);require(productionTutorial->currentStep==field<int>(nativeProductionTutorial,0xbfc),"Letter connected tutorial step differs");auto& tip=productionTutorial->action();auto* nativeTip=field<void*>(nativeProductionTutorial,0xbf4);mainMenuCompareView(tip,nativeTip);compareControl(tip.image(),field<void*>(nativeTip,0x40));compareControl(tip.text(),field<void*>(nativeTip,0x44));mainMenuCompareView(*offer,nativeOffer);}
require(showedTutorial && !productionTutorial->active() && profile.booleans["tip17"] && letters[0]->letterState==1,"Letter did not complete actual tutorial17 through mouse input");offer->wait();function<void(__cdecl*)()>(0xad0d0)();check(0);productionTutorial->tutorialEnabled=false;field<unsigned char>(nativeProductionTutorial,0xc09)=0;
menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);explicitTipElapsed=true;
scenes[0]->setParent(previousParent);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nativePreviousParent);game.disable();game.absoluteDisable();game.hide();call(0xd3750,nativeGame);call(0xd3790,nativeGame);call(0xd3720,nativeGame);
for(unsigned i=0;i<3;++i) {const auto before=textureCounts();letters[i].reset();const auto middle=textureCounts();call(0x21760,nativeLetters[i]);compareReferenceDeltas(before,middle);}
const auto before=textureCounts();offer.reset();const auto middle=textureCounts();call(0xada90,nativeOffer);compareReferenceDeltas(before,middle);productionTutorialWorld.dialogs[3]=nullptr;
for(unsigned at:{0x14U,0x68U,0x84U}) field<WString>(nativeEvent.data(),at).~WString();write(0x341b0,oldRemove.data(),5);
