stage=270;new(image+0x2601e8) String();field<unsigned char>(image,0x260204)=1;
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
auto* nativeGameView=allocate(44);function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeGameView,nullptr);putPointer(0x25f740,nativeGameView);
gameView.show();gameView.enable();gameView.absoluteEnable();call(0xd37d0,nativeGameView);call(0xd3bb0,nativeGameView);call(0xd3fb0,nativeGameView);
struct DialogWorld final:ContractTrace {
    using ContractTrace::ContractTrace;
    ContractAim* aim=nullptr;ContractFail* fail=nullptr;ContractPassConfirm* confirm=nullptr;
    void openReadyDialog(ContractStatus& status) override {confirm->start(status);}
    void openRequirementsDialog(ContractStatus& status) override {aim->start(status);}
    void openResultDialog(ContractStatus& status) override {fail->start(status);}
};
ProfileProperties sourceProfile,nativeProfile;contractNativeProfile=&nativeProfile;
DialogWorld world(sourceProfile,{&depot,&farm,&manufacture});contractTutorial=false;contractCanShow=false;
contractSourceCalls.clear();contractNativeCalls.clear();actualSounds.clear();expectedSounds.clear();
auto status=std::make_unique<ContractStatus>(&gameView,0,manager,settings,strings,input,interaction,sourceProfile,configuration,audio,world,views,controls);world.source=status.get();
auto* nativeStatus=allocate(136);std::memset(nativeStatus,0,136);contractNativeStatus=nativeStatus;
function<void*(__thiscall*)(void*,void*,unsigned)>(0xbb830)(nativeStatus,nativeGameView,0);
ContractDialogContext context{manager,settings,strings,input,interaction,configuration,gameView,views,controls};
auto aim=std::make_unique<ContractAim>(nullptr,context);world.aim=aim.get();auto* nativeAim=allocate(252);std::memset(nativeAim,0,252);function<void*(__thiscall*)(void*,void*)>(0xaa040)(nativeAim,nullptr);
auto fail=std::make_unique<ContractFail>(nullptr,context);world.fail=fail.get();auto* nativeFail=allocate(216);std::memset(nativeFail,0,216);function<void*(__thiscall*)(void*,void*)>(0xb73f0)(nativeFail,nullptr);
auto confirm=std::make_unique<ContractPassConfirm>(nullptr,context);world.confirm=confirm.get();auto* nativeConfirm=allocate(216);std::memset(nativeConfirm,0,216);function<void*(__thiscall*)(void*,void*)>(0xb8ce0)(nativeConfirm,nullptr);
std::array<unsigned char,256> nativeEvent{};new(nativeEvent.data()+0x68) WString();new(nativeEvent.data()+0x84) WString();
const auto setEvent=[&](GameplayEvent& event) {
    status->event=&event;field<void*>(nativeStatus,0x6c)=nativeEvent.data();const auto& c=std::get<ContractEvent>(event.data);
    field<unsigned>(nativeEvent.data(),0x34)=event.index;field<GameplayTime>(nativeEvent.data(),0x38)=c.winTime;
    field<int>(nativeEvent.data(),0x44)=c.bonus;field<int>(nativeEvent.data(),0x48)=c.penalty;std::memcpy(nativeEvent.data()+0x50,c.requirements.data(),24);
    field<WString>(nativeEvent.data(),0x68)=c.winNote;field<WString>(nativeEvent.data(),0x84)=c.lossNote;
};
const auto compareView=[&](View& source,void* native) {
    const auto& s=source.state();compareFloat(s.transparency,field<float>(native,8),"Contract dialog transparency");compareFloat(s.transparencySpeed,field<float>(native,12),"Contract dialog fade speed");
    require(s.transition==field<unsigned>(native,16) && s.shown==bool(field<unsigned char>(native,0x24)) && s.parentShown==bool(field<unsigned char>(native,0x25)) && s.enabled==bool(field<unsigned char>(native,0x26)) && s.parentEnabled==bool(field<unsigned char>(native,0x27)) && s.absoluteEnabled==bool(field<unsigned char>(native,0x28)) && s.parentAbsoluteEnabled==bool(field<unsigned char>(native,0x29)),"Contract dialog View state differs");
};
const auto compareText=[&](TextLine& text,void* native) {compareControl(text,native);contractDialogLetters+=compare(text,native);};
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
    stage=271;compareView(gameView,nativeGameView);compareView(*status,nativeStatus);
    checkBase(*aim,nativeAim);checkBase(*fail,nativeFail);checkBase(*confirm,nativeConfirm);
    compareText(aim->caption(),field<void*>(nativeAim,0xc8));compareButton(aim->okButton(),field<void*>(nativeAim,0xcc));
    for(unsigned n=0;n<2;++n) compareLine(aim->missing(n),static_cast<unsigned char*>(nativeAim)+0xd0+n*20);
    compareText(fail->caption(),field<void*>(nativeFail,0xc8));compareMultiline(fail->resultText(),field<void*>(nativeFail,0xcc));compareButton(fail->okButton(),field<void*>(nativeFail,0xd0));
    compareText(confirm->caption(),field<void*>(nativeConfirm,0xc8));compareButton(confirm->okButton(),field<void*>(nativeConfirm,0xcc));compareButton(confirm->cancelButton(),field<void*>(nativeConfirm,0xd0));
    require((aim->status()!=nullptr)==(field<void*>(nativeAim,0xf8)!=nullptr) && (fail->status()!=nullptr)==(field<void*>(nativeFail,0xd4)!=nullptr) && (confirm->status()!=nullptr)==(field<void*>(nativeConfirm,0xd4)!=nullptr),"Contract dialog borrowed status differs");
    require(status->active==bool(field<unsigned char>(nativeStatus,0x70)) && status->dirty==bool(field<unsigned char>(nativeStatus,0x71)) && status->expired==bool(field<unsigned char>(nativeStatus,0x84)),"Dialog connected status flags differ");
    require(sourceProfile.ints==nativeProfile.ints && sourceProfile.uints==nativeProfile.uints && sourceProfile.floats==nativeProfile.floats && sourceProfile.booleans==nativeProfile.booleans,"Contract dialog profile effects differ");
    require(contractSourceCalls==contractNativeCalls,"Contract dialog world effects differ");require(actualSounds==expectedSounds,"Contract dialog sound effects differ");++contractDialogStates;
};check();
const auto close=[&](unsigned kind,bool accept) {
    if(kind==0) {aim->accept();function<void(__cdecl*)()>(0xa9c60)();}
    else if(kind==1) {fail->accept();function<void(__cdecl*)()>(0xb7010)();}
    else if(accept) {confirm->accept();function<void(__cdecl*)()>(0xb8510)();}
    else {confirm->cancel();function<void(__cdecl*)()>(0xb8900)();}
    ++contractDialogClicks;check();
};
const auto start=[&](unsigned kind) {
    stage=272;
    if(kind==0) {aim->start(*status);function<void(__thiscall*)(void*,void*)>(0xa94e0)(nativeAim,nativeStatus);}
    if(kind==1) {fail->start(*status);function<void(__thiscall*)(void*,void*)>(0xb6b90)(nativeFail,nativeStatus);}
    if(kind==2) {confirm->start(*status);function<void(__thiscall*)(void*,void*)>(0xb7fe0)(nativeConfirm,nativeStatus);}
    ++contractDialogStarts;check();
};
std::mt19937 random(0x4aa890);
for(trial=0;trial<contracts.size();++trial) {
    setEvent(*contracts[trial]);sourceProfile.ints["Money"]=nativeProfile.ints["Money"]=100;sourceProfile.uints["awQTask"]=nativeProfile.uints["awQTask"]=0;
    for(unsigned product=0;product<19;++product) sourceProfile.uints["WGoodCount"+std::to_string(product)]=nativeProfile.uints["WGoodCount"+std::to_string(product)]=random()%100;
    for(unsigned kind=0;kind<3;++kind) {start(kind);close(kind,trial%2==0);}
}
GameplayEvent edge(GameplayEventType::Contract);edge.index=0xffffffffU;auto& c=std::get<ContractEvent>(edge.data);c.winNote=WString(L"provided");c.lossNote=WString(L"not provided/nNext line");
for(trial=0;trial<128;++trial) {
    c.bonus=trial%2 ? INT_MIN:INT_MAX;c.penalty=-123;c.winTime={1,2,3};
    for(unsigned n=0;n<2;++n) {c.requirements[n].reqType=(trial+n)%3;c.requirements[n].productType=random()%19;c.requirements[n].count=trial%5 ? random():0xffffffffU;}
    setEvent(edge);for(unsigned product=0;product<19;++product) sourceProfile.uints["WGoodCount"+std::to_string(product)]=nativeProfile.uints["WGoodCount"+std::to_string(product)]=trial%7 ? random():0xffffffffU;
    sourceProfile.ints["Money"]=nativeProfile.ints["Money"]=INT_MAX;sourceProfile.uints["awQTask"]=nativeProfile.uints["awQTask"]=0xffffffffU;
    for(unsigned kind=0;kind<3;++kind) {start(kind);close(kind,trial%2==0);}
}
// Actual status-goal dispatch opens each real dialog; native starts are not adapters.
setEvent(*contracts[0]);c=std::get<ContractEvent>(contracts[0]->data);c.requirements[0].productType=0;c.requirements[0].count=5;c.requirements[0].reqType=0;c.requirements[1].productType=1;c.requirements[1].count=3;c.requirements[1].reqType=0;c.bonus=73;setEvent(edge);
for(action=0;action<2;++action) {
    for(unsigned product=0;product<19;++product) sourceProfile.uints["WGoodCount"+std::to_string(product)]=nativeProfile.uints["WGoodCount"+std::to_string(product)]=action ? 50:0;
    status->goalClicked();function<void(__cdecl*)(void*)>(0xb9350)(field<void*>(nativeStatus,0x34));++contractDialogStarts;check();close(action ? 2:0,false);
}
// Readiness and expiry run the actual controller into the actual dialogs.
setEvent(edge);status->assign(edge,false);function<void(__thiscall*)(void*,void*,bool)>(0xbd2f0)(nativeStatus,nativeEvent.data(),false);
elapsed=0;status->absoluteStep();call(0xbcc20,nativeStatus);++contractDialogStarts;check();
require(confirm->status()==status.get(),"Status readiness did not open real confirmation dialog");close(2,false);
status->expired=true;field<unsigned char>(nativeStatus,0x84)=1;status->absoluteStep();call(0xbcc20,nativeStatus);++contractDialogStarts;check();
require(fail->status()==status.get(),"Status expiry did not open real failure dialog");close(1,false);
// Partial XML retains previous line records and adds Place again without
// clearing missing prototypes. More than two places are ignored by the native loop.
const auto fixtureDirectory=std::filesystem::path(".local")/(std::string("contract-dialog-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));std::filesystem::create_directories(fixtureDirectory);
std::ofstream(fixtureDirectory/"dialog-fixture.xml")<<R"(<Root><Empty Visible="0" Enable="1"/><One Visible="0" Enable="1"><AimPlaces><P Place="-11.2 77"/></AimPlaces></One><Three Visible="0" Enable="1"><AimPlaces><P Place="11.3 -41.7"/><P Place="200 88.3"/><P Place="700 900"/></AimPlaces></Three><NoPlaces Visible="0" Enable="1"><AimProto><ReqText String="12345" Localized="0"/></AimProto></NoPlaces><MissingPlace Visible="0" Enable="1"><AimPlaces><P/></AimPlaces></MissingPlace></Root>)";
files.mountCommon(String(fixtureDirectory.generic_string()));XmlDocument partial(files,String("dialog-fixture.xml"));
for(const auto* name:{"Empty","One","Three","NoPlaces","MissingPlace"}) {
    XmlNode* node=nullptr;require(partial.section(String(name),node),"Contract dialog fixture missing");
    aim->loadXML(*node);function<void(__thiscall*)(void*,void*)>(0xa8d50)(nativeAim,wrap(node));++contractDialogReloads;check();
    fail->loadXML(*node);function<void(__thiscall*)(void*,void*)>(0xb65b0)(nativeFail,wrap(node));++contractDialogReloads;check();
    confirm->loadXML(*node);function<void(__thiscall*)(void*,void*)>(0xb7a00)(nativeConfirm,wrap(node));++contractDialogReloads;check();
}
for(const auto& item:std::array<std::tuple<ContractDialogBase*,void*,const char*,unsigned>,3>{{{aim.get(),nativeAim,"Gameplay/ContractAim.xml",0xa8d50},{fail.get(),nativeFail,"Gameplay/ContractFail.xml",0xb65b0},{confirm.get(),nativeConfirm,"Gameplay/ContractPassConfirm.xml",0xb7a00}}}) {
    XmlDocument xml(files,String(std::get<2>(item)));std::get<0>(item)->loadXML(xml);function<void(__thiscall*)(void*,void*)>(std::get<3>(item))(std::get<1>(item),wrap(&xml));++contractDialogReloads;check();
}
// GPU/input frames are supplied by actual timer/cursor/InputBindings and owned UI.
menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
status->active=false;field<unsigned char>(nativeStatus,0x70)=0;
for(unsigned kind=0;kind<4;++kind) {
    const unsigned dialogKind=(std::min)(kind,2U);
    if(kind==3) {status->assign(edge,false);function<void(__thiscall*)(void*,void*,bool)>(0xbd2f0)(nativeStatus,nativeEvent.data(),false);status->dirty=false;field<unsigned char>(nativeStatus,0x71)=0;}
    start(dialogKind);
    TextAnimationButton* button=kind==0 ? &aim->okButton():kind==1 ? &fail->okButton():kind==2 ? &confirm->cancelButton():&confirm->okButton();
    for(action=0;action<16;++action) {
        stage=273;menuMilliseconds+=50;menuPoint=action>=12 ? Point2{button->geometry().x,button->geometry().y}:Point2{790,590};
        menuInputEvents=action==13 ? std::vector<unsigned>{0x208}:action==14 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};menuMouseDelta={1,0,0};
        contractDialogPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++contractDialogFrames;check();
    }
    require(button->buttonState()!=ButtonState::Pressed,"Contract dialog input release stuck");++contractDialogInputClicks;
    // Hidden transition can still be active on the next frame; complete it with
    // the same UI frame on both sides before opening another dialog.
    for(action=0;action<8;++action) {menuMilliseconds+=50;menuPoint={790,590};menuInputEvents.clear();contractDialogPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++contractDialogFrames;check();}
    auto* current=kind==0 ? static_cast<ContractDialogBase*>(aim.get()):kind==1 ? static_cast<ContractDialogBase*>(fail.get()):static_cast<ContractDialogBase*>(confirm.get());
    require(!current->state().shown && current->state().transition==0,"Contract dialog input did not close actual dialog");
    if(kind>0) require(kind==1 ? fail->status()==nullptr:confirm->status()==nullptr,"Contract dialog input did not clear status");
}
menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
const auto before=textureCounts();confirm.reset();fail.reset();aim.reset();status.reset();const auto middle=textureCounts();
call(0xb9290,nativeConfirm);call(0xb7980,nativeFail);call(0xaa660,nativeAim);call(0xb92c0,nativeStatus);compareReferenceDeltas(before,middle);call(0xd58a0,nativeGameView);
require(!ContractAim::instance() && !ContractFail::instance() && !ContractPassConfirm::instance(),"Contract dialog singleton destructor differs");
require(!field<void*>(image,0x25f768) && !field<void*>(image,0x25f770) && !field<void*>(image,0x25f76c),"Native contract dialog singleton destructor differs");
require(controls.visible().empty(),"Contract dialog destruction leaves visible controls");contractDialogObjects+=3;
field<WString>(nativeEvent.data(),0x68).~WString();field<WString>(nativeEvent.data(),0x84).~WString();putPointer(0x25f740,nullptr);putPointer(0x25f73c,nullptr);
