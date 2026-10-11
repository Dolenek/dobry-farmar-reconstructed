stage=280;new(image+0x2601e8) String();field<unsigned char>(image,0x260204)=1;
redirect(0x8860,reinterpret_cast<void*>(&upgradeStreamConstruct));
redirect(0x10770,reinterpret_cast<void*>(&contractProperty));redirect(0x106a0,reinterpret_cast<void*>(&contractFloatProperty));redirect(0x105e0,reinterpret_cast<void*>(&contractBoolProperty));
redirect(0xa670,reinterpret_cast<void*>(&gameConfigurationLevel));
redirect(0x6b00,reinterpret_cast<void*>(&upgradeStreamConstruct));redirect(0x5060,reinterpret_cast<void*>(&listStreamNoop));
redirect(0x6700,reinterpret_cast<void*>(&upgradeWriteLiteral));redirect(0x6570,reinterpret_cast<void*>(&upgradeWriteIndex));
redirect(0xa450,reinterpret_cast<void*>(&upgradeStreamContents));redirect(0x59e0,reinterpret_cast<void*>(&listStreamNoop));
redirect(0x87d0,reinterpret_cast<void*>(&listStreamConstruct));redirect(0x3900,reinterpret_cast<void*>(&contractClearStream));
redirect(0x50d0,reinterpret_cast<void*>(&contractSetStream));redirect(0xa8be0,reinterpret_cast<void*>(&contractWriteSigned));field<unsigned>(image,0x2601e0)=1;
redirect(0xd0a20,reinterpret_cast<void*>(&offerNativeCanShow));redirect(0xcdef0,reinterpret_cast<void*>(&contractNativeStep));redirect(0xcf3c0,reinterpret_cast<void*>(&offerNativeShow));
redirect(0x10d60,reinterpret_cast<void*>(&contractNativeMoney));redirect(0x682d0,reinterpret_cast<void*>(&contractNativeStock));
redirect(0xc06b0,reinterpret_cast<void*>(&contractNativeRemove));redirect(0xc44a0,reinterpret_cast<void*>(&contractNativeNotify));

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

ViewRegistry views;ControlRegistry controls;View gameView(nullptr,views),depot(nullptr,views),farm(nullptr,views),manufacture(nullptr,views);
auto* nativeGameView=allocate(112);function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeGameView,nullptr);putPointer(0x25f740,nativeGameView);
gameView.show();gameView.enable();gameView.absoluteEnable();call(0xd37d0,nativeGameView);call(0xd3bb0,nativeGameView);call(0xd3fb0,nativeGameView);
redirect(0x10ce0,reinterpret_cast<void*>(&offerNativeMoney));redirect(0x341b0,reinterpret_cast<void*>(&offerNativeConsume));
struct OfferWorld final:ContractTrace,ContractOfferWorld {
    using ContractTrace::ContractTrace;
    std::array<ContractStatus*,3> statuses{};int count=0;GameplayEvent* replacement=nullptr;
    int activeContracts() const override {return count;}
    int money() const override {contractSourceCalls.push_back({9,0,0,0,0});return profile.ints["Money"];}
    void setMoney(int value) override {ContractTrace::setMoney(value);}
    void consumeLetter(ContractOfferLetter& value) override {contractSourceCalls.push_back({10,0,0,0,0});if(replacement) static_cast<OfferLetterFixture&>(value).current=replacement;}
    void activateContract(GameplayEvent& event) override {if(count!=3) {for(auto* status:statuses) if(!status->active) {status->assign(event,false);++count;return;}throw std::runtime_error("Invalid offer fixture: no free slot");}}
    bool tutorialCanShow(unsigned step) const override {if(step==0x12) return false;require(step==0x2a,"Offer source tutorial query step differs");contractSourceCalls.push_back({11,step,0,0,0});return offerTutorial;}
    void showTutorial(unsigned step,View&) override {require(step==0x2a,"Offer source tutorial step differs");contractSourceCalls.push_back({1,step,0,0,0});}
};
ProfileProperties sourceProfile,nativeProfile;contractNativeProfile=&nativeProfile;
OfferWorld world(sourceProfile,{&depot,&farm,&manufacture});contractTutorial=false;contractCanShow=false;
contractSourceCalls.clear();contractNativeCalls.clear();actualSounds.clear();expectedSounds.clear();
std::array<std::unique_ptr<ContractStatus>,3> statuses;std::array<void*,3> nativeStatuses{};
for(unsigned n=0;n<3;++n) {
    statuses[n]=std::make_unique<ContractStatus>(&gameView,n,manager,settings,strings,input,interaction,sourceProfile,configuration,audio,world,views,controls);
    world.statuses[n]=statuses[n].get();nativeStatuses[n]=allocate(136);std::memset(nativeStatuses[n],0,136);
    function<void*(__thiscall*)(void*,void*,unsigned)>(0xbb830)(nativeStatuses[n],nativeGameView,n);field<void*>(nativeGameView,0x54+n*4)=nativeStatuses[n];
}
field<int>(nativeGameView,0x60)=0;world.source=statuses[0].get();contractNativeStatus=nativeStatuses[0];
ContractDialogContext context{manager,settings,strings,input,interaction,configuration,gameView,views,controls};
auto offer=std::make_unique<ContractProvide>(nullptr,context,world);auto* nativeOffer=allocate(220);std::memset(nativeOffer,0,220);function<void*(__thiscall*)(void*,void*)>(0xad4b0)(nativeOffer,nullptr);
offerNativeYes=field<void*>(nativeOffer,0xc8);
std::array<std::array<unsigned char,256>,2> nativeEvents{};
for(auto& e:nativeEvents) {new(e.data()+0x14) WString();new(e.data()+0x68) WString();new(e.data()+0x84) WString();}
OfferLetterFixture letter;std::array<unsigned char,900> nativeLetter{};offerNativeLetter=nativeLetter.data();
const auto writeEvent=[&](GameplayEvent& event,unsigned index) {
    auto* e=nativeEvents[index].data();const auto& c=std::get<ContractEvent>(event.data);
    field<WString>(e,0x14)=event.note;field<unsigned>(e,0x34)=event.index;field<GameplayTime>(e,0x38)=c.winTime;
    field<int>(e,0x44)=c.bonus;field<int>(e,0x48)=c.penalty;std::memcpy(e+0x50,c.requirements.data(),24);
    field<WString>(e,0x68)=c.winNote;field<WString>(e,0x84)=c.lossNote;
};
const auto setEvent=[&](GameplayEvent& event) {writeEvent(event,0);letter.current=&event;field<void*>(nativeLetter.data(),0x37c)=nativeEvents[0].data();};
const auto compareView=[&](View& source,void* native) {
    const auto& s=source.state();compareFloat(s.transparency,field<float>(native,8),"Contract dialog transparency");compareFloat(s.transparencySpeed,field<float>(native,12),"Contract dialog fade speed");
    require(s.transition==field<unsigned>(native,16) && s.shown==bool(field<unsigned char>(native,0x24)) && s.parentShown==bool(field<unsigned char>(native,0x25)) && s.enabled==bool(field<unsigned char>(native,0x26)) && s.parentEnabled==bool(field<unsigned char>(native,0x27)) && s.absoluteEnabled==bool(field<unsigned char>(native,0x28)) && s.parentAbsoluteEnabled==bool(field<unsigned char>(native,0x29)),"Contract dialog View state differs");
};
const auto compareText=[&](TextLine& text,void* native) {compareControl(text,native);contractOfferLetters+=compare(text,native);};
const auto compareMultiline=[&](MultilineText& text,void* native) {
    text.enabledStep();call(0xf8e10,native);compareControl(text,native);
    require(text.string().view()==field<WString>(native,0x23c).view() && text.textDirty()==bool(field<unsigned char>(native,0x258)),"Contract dialog multiline string/dirty differs");
    auto* head=field<Link*>(native,0x230);auto* item=head->next;require(text.lines().size()==field<unsigned>(native,0x234),"Contract dialog multiline line count differs");
    for(auto* line:text.lines()) {require(item!=head,"Contract dialog multiline list ended early");compareText(*line,item->control);item=item->next;}
    require(item==head,"Contract dialog multiline list ended late");
};
const auto compareLine=[&](ContractRequirementLine& line,void* native) {
    compareControl(*line.icon,field<void*>(native,0));compareText(*line.label,field<void*>(native,4));compareText(*line.number,field<void*>(native,8));
    require(!std::memcmp(line.place.data(),static_cast<unsigned char*>(native)+12,8),"Contract dialog line placement differs");
};
const auto compareButton=[&](TextAnimationButton& button,void* native) {compareControl(button,native);compareText(button.text(),field<void*>(native,0x27c));require(unsigned(button.buttonState())==field<unsigned>(native,0x238),"Contract dialog button state differs");};
const auto compareMoney=[&](TextMoney& text,void* native) {compareText(text,native);compareControl(text.coin(),field<void*>(native,0x2bc));compareFloat(text.degreeOffset,field<float>(native,0x2b8),"Contract dialog money degree");};
const auto compareAppearance=[&](AnimableAppearance& appearance,void* native) {
    require(appearance.texture==field<cTexture*>(native,4) && appearance.frames==field<unsigned>(native,8) && appearance.columns==field<unsigned>(native,20) && appearance.rows==field<unsigned>(native,24) && appearance.loop==bool(field<unsigned char>(native,16)),"Contract dialog navigation appearance differs");
    compareFloat(appearance.length,field<float>(native,12),"Contract dialog navigation duration");
};
const auto checkBase=[&](ContractDialogBase& source,void* native) {
    compareAppearance(source.marketState(),static_cast<unsigned char*>(native)+0x64);compareAppearance(source.warehouseState(),static_cast<unsigned char*>(native)+0x8c);
    compareView(source,native);compareDecorations(source,native);compareMultiline(source.note(),field<void*>(native,0x38));
    for(unsigned n=0;n<2;++n) compareLine(source.requirement(n),static_cast<unsigned char*>(native)+0x3c+n*20);
    compareText(source.bonusLabel(),field<void*>(native,0xb4));compareText(source.penaltyLabel(),field<void*>(native,0xb8));
    compareMoney(source.bonusMoney(),field<void*>(native,0xbc));compareMoney(source.penaltyMoney(),field<void*>(native,0xc0));
};
const auto check=[&] {
    stage=281;compareView(gameView,nativeGameView);checkBase(*offer,nativeOffer);
    compareButton(offer->yesButton(),field<void*>(nativeOffer,0xc8));compareButton(offer->waitButton(),field<void*>(nativeOffer,0xcc));
    compareText(offer->timeLabel(),field<void*>(nativeOffer,0xd0));compareText(offer->timeValue(),field<void*>(nativeOffer,0xd4));
    require((offer->letter()!=nullptr)==(field<void*>(nativeOffer,0xd8)!=nullptr),"Offer borrowed letter differs");
    require(world.count==field<int>(nativeGameView,0x60),"Offer native activation count differs");
    for(unsigned n=0;n<3;++n) {
        auto& source=*statuses[n];auto* native=nativeStatuses[n];compareView(source,native);
        require(source.active==bool(field<unsigned char>(native,0x70)) && source.dirty==bool(field<unsigned char>(native,0x71)) && source.expired==bool(field<unsigned char>(native,0x84)),"Offer connected status flags differ");
        compareFloat(source.accumulator,field<float>(native,0x58),"Offer status accumulator");compareFloat(source.fullDuration,field<float>(native,0x68),"Offer status duration");
        require(source.remaining==field<GameplayTime>(native,0x5c) && !std::memcmp(source.initialStocks.data(),static_cast<unsigned char*>(native)+0x74,8),"Offer status time/stocks differ");
        compareControl(source.substrate(),field<void*>(native,0x30));compareControl(source.goalButton(),field<void*>(native,0x34));
        for(unsigned j=0;j<2;++j) {compareControl(source.productIcon(j),field<void*>(native,0x38+j*4));compareText(source.requirementText(j),field<void*>(native,0x40+j*4));compareControl(source.requirementCheck(j),field<void*>(native,0x48+j*4));}
        compareText(source.timeText(),field<void*>(native,0x50));compareControl(source.progress(),field<void*>(native,0x54));
        compareFloat(source.progress().current,field<float>(field<void*>(native,0x54),0x23c),"Offer status progress current");compareFloat(source.progress().maximum,field<float>(field<void*>(native,0x54),0x238),"Offer status progress maximum");
    }
    require(sourceProfile.ints==nativeProfile.ints && sourceProfile.uints==nativeProfile.uints && sourceProfile.floats==nativeProfile.floats && sourceProfile.booleans==nativeProfile.booleans,"Offer profile effects differ");
    require(contractSourceCalls==contractNativeCalls,"Offer ordered world effects differ");require(actualSounds==expectedSounds,"Offer sound effects differ");++contractOfferStates;
};check();
const auto reset=[&](int count,int money,unsigned occupied) {
    world.count=count;field<int>(nativeGameView,0x60)=count;world.replacement=nullptr;offerNativeReplacement=nullptr;
    sourceProfile.ints["Money"]=nativeProfile.ints["Money"]=money;
    for(unsigned n=0;n<3;++n) {statuses[n]->active=n<occupied;field<unsigned char>(nativeStatuses[n],0x70)=n<occupied;}
};
const auto start=[&] {stage=282;offer->start(letter);function<void(__thiscall*)(void*,void*)>(0xac570)(nativeOffer,nativeLetter.data());++contractOfferStarts;check();};
const auto close=[&](bool accept) {if(accept) {offer->accept();function<void(__cdecl*)()>(0xaccc0)();}else {offer->wait();function<void(__cdecl*)()>(0xad0d0)();}++contractOfferClicks;check();};
std::mt19937 random(0x4ac570);
for(trial=0;trial<contracts.size();++trial) {
    setEvent(*contracts[trial]);for(unsigned product=0;product<19;++product) sourceProfile.uints["WGoodCount"+std::to_string(product)]=nativeProfile.uints["WGoodCount"+std::to_string(product)]=random()%100;
    for(action=0;action<4;++action) {reset(action,action%2 ? INT_MAX:0,action);start();close(action%2);}
}
GameplayEvent edge(GameplayEventType::Contract),replacement(GameplayEventType::Contract);edge.index=0xffffffffU;edge.note=WString(L"A contract/nSecond line");replacement.index=0x80000001U;replacement.note=WString(L"Re-read after consume");
auto& c=std::get<ContractEvent>(edge.data);auto& r=std::get<ContractEvent>(replacement.data);
r.winTime={1,2,3};r.penalty=71;r.bonus=17;for(unsigned n=0;n<2;++n) {r.requirements[n].productType=n;r.requirements[n].count=5+n*2;r.requirements[n].reqType=0;}
for(trial=0;trial<128;++trial) {
    c.bonus=trial%2 ? INT_MIN:INT_MAX;c.penalty=std::array<int,8>{INT_MIN,INT_MAX,-1,0,1,50,71,1000}[trial%8];c.winTime={trial%7,trial%61,trial%63};
    for(unsigned n=0;n<2;++n) {c.requirements[n].reqType=(trial+n)%3;c.requirements[n].productType=random()%19;c.requirements[n].count=trial%5 ? random():0xffffffffU;}
    setEvent(edge);reset(std::array<int,6>{-1,0,1,2,3,4}[trial%6],std::array<int,6>{INT_MIN,INT_MAX,-1,0,71,1000}[trial%6],trial%3);
    start();if(trial%3==0) {writeEvent(replacement,1);world.replacement=&replacement;offerNativeReplacement=nativeEvents[1].data();}close(trial%2==0);
}
// Exact disabled own/parent conditions, tutorial query and anchor, without visibility checks.
for(trial=0;trial<8;++trial) {
    const bool own=trial&1,parent=trial&2;offerTutorial=trial&4;
    if(parent) offer->enable();else offer->disable();call(parent ? 0xd3bb0:0xd3750,nativeOffer);
    if(own) offer->yesButton().enable();else offer->yesButton().disable();call(own ? 0xd3bb0:0xd3750,offerNativeYes);
    offer->enabledStep();call(0xabea0,nativeOffer);check();
}
offerTutorial=false;
// Partial/repeated XML ownership and retained values.
const auto fixtureDirectory=std::filesystem::path(".local")/(std::string("contract-offer-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));std::filesystem::create_directories(fixtureDirectory);
std::ofstream(fixtureDirectory/"offer-fixture.xml")<<R"(<Root><Empty Visible="0" Enable="1"/><One Visible="0" Enable="1"><AimPlaces><P Place="-11.2 77"/></AimPlaces></One><Three Visible="0" Enable="1"><AimPlaces><P Place="11.3 -41.7"/><P Place="200 88.3"/><P Place="700 900"/></AimPlaces></Three><NoPlaces Visible="0" Enable="1"><AimProto><ReqText String="12345" Localized="0"/></AimProto></NoPlaces><MissingPlace Visible="0" Enable="1"><AimPlaces><P/></AimPlaces></MissingPlace></Root>)";
files.mountCommon(String(fixtureDirectory.generic_string()));XmlDocument partial(files,String("offer-fixture.xml"));
for(const auto* name:{"Empty","One","Three","NoPlaces","MissingPlace"}) {XmlNode* node=nullptr;require(partial.section(String(name),node),"Offer XML fixture missing");offer->loadXML(*node);function<void(__thiscall*)(void*,void*)>(0xabef0)(nativeOffer,wrap(node));++contractOfferReloads;check();}
XmlDocument originalOffer(files,String("Gameplay/ContractProvide.xml"));offer->loadXML(originalOffer);function<void(__thiscall*)(void*,void*)>(0xabef0)(nativeOffer,wrap(&originalOffer));++contractOfferReloads;check();
menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
for(trial=0;trial<2;++trial) {
    setEvent(replacement);reset(0,1000,0);for(unsigned product=0;product<19;++product) sourceProfile.uints["WGoodCount"+std::to_string(product)]=nativeProfile.uints["WGoodCount"+std::to_string(product)]=0;
    start();auto& button=trial ? offer->yesButton():offer->waitButton();
    for(action=0;action<24;++action) {
        stage=283;menuMilliseconds+=50;menuPoint=action>=12 && action<16 ? Point2{button.geometry().x,button.geometry().y}:Point2{790,590};
        menuInputEvents=action==13 ? std::vector<unsigned>{0x208}:action==14 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};menuMouseDelta={1,0,0};
        contractOfferPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++contractOfferFrames;check();
    }
    require(!offer->state().shown && offer->state().transition==0,"Offer actual input did not close dialog");require(world.count==int(trial),"Offer input did not activate expected contract");++contractOfferInputClicks;
}
menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
const auto before=textureCounts();offer.reset();for(auto& status:statuses) status.reset();const auto middle=textureCounts();
call(0xada90,nativeOffer);for(auto* status:nativeStatuses) call(0xb92c0,status);compareReferenceDeltas(before,middle);call(0xd58a0,nativeGameView);
require(!ContractProvide::instance() && !field<void*>(image,0x25f760),"Offer singleton destruction differs");require(controls.visible().empty(),"Offer destruction leaves visible controls");++contractOfferObjects;
for(auto& e:nativeEvents) {field<WString>(e.data(),0x14).~WString();field<WString>(e.data(),0x68).~WString();field<WString>(e.data(),0x84).~WString();}
putPointer(0x25f740,nullptr);putPointer(0x25f73c,nullptr);offerNativeLetter=offerNativeReplacement=offerNativeYes=nullptr;
