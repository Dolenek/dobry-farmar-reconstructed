std::array<unsigned char,5> mowInvestmentEntry{};if(mowBuildingMode) std::memcpy(mowInvestmentEntry.data(),image+0x47b40,5);
std::array<unsigned char,5> waterInvestmentEntry{};if(waterSourceMode) std::memcpy(waterInvestmentEntry.data(),image+0x69400,5);
std::array<unsigned char,5> digInvestmentEntry{};if(digBuildingMode) std::memcpy(digInvestmentEntry.data(),image+0x26f50,5);
std::array<unsigned char,5> flyingExpenseEntry{};if(flyingMarketMode) std::memcpy(flyingExpenseEntry.data(),image+0xef00,5);
std::array<unsigned char,5> saleHistoryEntry{};if(salesHistoryMode) std::memcpy(saleHistoryEntry.data(),image+0x10f10,5);
std::map<unsigned,std::array<unsigned char,5>> marketEntries;if(marketMode) for(unsigned rva:{0x6f9e0U,0x6f370U,0x6eb70U}) std::memcpy(marketEntries[rva].data(),image+rva,5);
std::map<unsigned,std::array<unsigned char,5>> manufactureEntries;if(manufactureMode || warehouseMode) for(unsigned rva:{0x94a20U,0x93b70U,0x957c0U,0x95bd0U,0x98400U,0x94e70U,0x98c60U}) std::memcpy(manufactureEntries[rva].data(),image+rva,5);
std::map<unsigned,std::array<unsigned char,5>> warehouseTipEntries;if(warehouseTipMode) for(unsigned rva:{0x64b40U,0x65050U,0x65460U,0x654b0U}) std::memcpy(warehouseTipEntries[rva].data(),image+rva,5);
std::map<unsigned,std::array<unsigned char,5>> warehouseEntries;if(warehouseMode) for(unsigned rva:{0x67fa0U,0x65e70U,0x67ec0U,0x682d0U,0x66a40U}) std::memcpy(warehouseEntries[rva].data(),image+rva,5);
std::array<unsigned char,5> productionLevelEntry{};if(productionHouses) std::memcpy(productionLevelEntry.data(),image+0x17050,5);
std::map<unsigned,std::array<unsigned char,5>> tutorialEntries;
if(productionHouses || tutorialManagerMode || buySeedsMode || buyBuildingMode) for(unsigned rva:{0xd0a20U,0xcdef0U,0xcf3c0U,0xd0ac0U,0xd0c30U}) std::memcpy(tutorialEntries[rva].data(),image+rva,5);
stage=310;hudInputs={};new(image+0x260208) WString();field<unsigned>(image,0x260224)=1;new(image+0x25ff68) WString();field<unsigned>(image,0x25ff84)=1;redirect(0x1d680,reinterpret_cast<void*>(&hudLiteral));new(image+0x2601e8) String();field<unsigned char>(image,0x260204)=1;
redirect(0x8860,reinterpret_cast<void*>(&upgradeStreamConstruct));
redirect(0x10770,reinterpret_cast<void*>(&contractProperty));redirect(0x106a0,reinterpret_cast<void*>(&contractFloatProperty));redirect(0x105e0,reinterpret_cast<void*>(&contractBoolProperty));
redirect(0xa670,reinterpret_cast<void*>(&gameConfigurationLevel));
redirect(0x6b00,reinterpret_cast<void*>(&upgradeStreamConstruct));redirect(0x5060,reinterpret_cast<void*>(&listStreamNoop));redirect(0x6700,reinterpret_cast<void*>(&upgradeWriteLiteral));redirect(0x6570,reinterpret_cast<void*>(&upgradeWriteIndex));redirect(0xa450,reinterpret_cast<void*>(&upgradeStreamContents));redirect(0x59e0,reinterpret_cast<void*>(&listStreamNoop));
redirect(0x87d0,reinterpret_cast<void*>(&listStreamConstruct));redirect(0x3900,reinterpret_cast<void*>(&contractClearStream));redirect(0x50d0,reinterpret_cast<void*>(&contractSetStream));redirect(0xa8be0,reinterpret_cast<void*>(&contractWriteSigned));field<unsigned>(image,0x2601e0)=1;
redirect(0xd0a20,reinterpret_cast<void*>(&hudCan));redirect(0xcdef0,reinterpret_cast<void*>(&hudStep));redirect(0xcf3c0,reinterpret_cast<void*>(&hudShow));redirect(0xd0ac0,reinterpret_cast<void*>(&hudComplete));redirect(0x46d50,reinterpret_cast<void*>(&hudMessages));redirect(0xb5c60,reinterpret_cast<void*>(&hudGameplay));
redirect(0x2cde0,reinterpret_cast<void*>(&hudSavedEvent));redirect(0x10ce0,reinterpret_cast<void*>(&hudMoney));redirect(0x66a40,reinterpret_cast<void*>(&hudValue<1>));redirect(0x69400,reinterpret_cast<void*>(&hudValue<2>));redirect(0x47b40,reinterpret_cast<void*>(&hudValue<3>));redirect(0x26f50,reinterpret_cast<void*>(&hudValue<4>));redirect(0x17050,reinterpret_cast<void*>(&hudLevel));redirect(0x65e70,reinterpret_cast<void*>(&hudFree));
redirect(0x6eb70,reinterpret_cast<void*>(&hudProducts));redirect(0x6f370,reinterpret_cast<void*>(&hudAvailable));redirect(0x65050,reinterpret_cast<void*>(&hudTipShow));redirect(0x65460,reinterpret_cast<void*>(&hudTipHide));redirect(0x654b0,reinterpret_cast<void*>(&hudTipReset));
redirect(0x49820,reinterpret_cast<void*>(&hudStop<2>));redirect(0x4ad50,reinterpret_cast<void*>(&hudStopSix));redirect(0x49880,reinterpret_cast<void*>(&hudStop<1>));redirect(0x49860,reinterpret_cast<void*>(&hudStop<0>));redirect(0x49840,reinterpret_cast<void*>(&hudStop<4>));redirect(0x4a820,reinterpret_cast<void*>(&hudResume));
putPointer(0x25f73c,contractTutorialObject.data());field<int>(contractTutorialObject.data(),0xbfc)=-1;putPointer(0x25ff5c,nullptr);
xmlTable[23]=reinterpret_cast<void*>(&gameConfigurationUnsigned);xmlTable[34]=reinterpret_cast<void*>(&sectionCount);
interaction.loadSound=[&](const String& name)->cResource* {return manager.getSound(name);};LoadingDisabledAudio backend;ContractAudio audio(files,settings,backend,0);MusicPlayback music;
loadingFiles=&files;loadingSoundManager=&audio;managerTable[6]=reinterpret_cast<void*>(&loadingXML);managerTable[3]=reinterpret_cast<void*>(&loadingSoundResource);xmlTable[0]=reinterpret_cast<void*>(&loadingDeleteXML);
GameConfiguration configuration(manager);XmlDocument gameDefinition(files,String("game.xml"));configuration.loadXML(gameDefinition);
const auto common=std::filesystem::path(".local")/("hud-"+std::to_string(GetCurrentProcessId()));std::filesystem::create_directories(common);files.mountCommon(String(common.generic_string()));
String oldProfiles;settings.get(String("game"),String("profiles"),oldProfiles);settings.set(String("game"),String("profiles"),String("HudProfiles"));
ProfileSession profiles(files,settings,manager);profiles.open(WString(L"Alpha"));auto& profile=profileProperties();initializeGameProfile(profile);ProfileProperties nativeProfile=profile;contractNativeProfile=&nativeProfile;
ViewRegistry views;ControlRegistry controls;View game(nullptr,views),help(nullptr,views),overlay(nullptr,views),selection(nullptr,views),tip(nullptr,views);
auto* nativeGame=allocate(2716);function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeGame,nullptr);putPointer(0x25f6ec,nativeGame);
for(unsigned n=0;n<19;++n) {const auto& a=configuration.products[n].icon;auto* out=static_cast<unsigned char*>(nativeGame)+0x30+n*100+16;field<cTexture*>(out,4)=a.texture;field<unsigned>(out,8)=a.frames;field<float>(out,12)=a.length;field<unsigned char>(out,16)=a.loop;field<unsigned>(out,20)=a.columns;field<unsigned>(out,24)=a.rows;}
auto* nativeHelp=allocate(44);auto* nativeOverlay=allocate(900);auto* nativeSelection=allocate(900);auto* nativeTip=allocate(900);
for(auto* value:{nativeHelp,nativeOverlay,nativeSelection,nativeTip}) function<void*(__thiscall*)(void*,void*)>(0xd55b0)(value,nullptr);
putPointer(0x25f778,nativeHelp);putPointer(0x260010,nativeOverlay);putPointer(0x25ff64,nativeSelection);putPointer(0x25ffc4,nativeTip);field<unsigned char>(nativeTip,0x370)=0;auto* nativeWarehouse=allocate(8192);std::memset(nativeWarehouse,0,8192);putPointer(0x25f748,nativeWarehouse);
game.show();game.enable();game.absoluteEnable();call(0xd37d0,nativeGame);call(0xd3bb0,nativeGame);call(0xd3fb0,nativeGame);
std::array<std::unique_ptr<StateView>,3> scenes;std::array<void*,3> nativeScenes{};
const unsigned sceneGlobals[]={0x25f744,0x25f758,0x25ffc8};const char* sceneFiles[]={"UI/gameUI/Farm.xml","UI/gameUI/WareDetail.xml","UI/gameUI/Manufacture.xml"};
for(unsigned n=0;n<3;++n) {scenes[n]=std::make_unique<StateView>(&game,manager,settings,strings,views,controls);nativeScenes[n]=allocate(productionHouses ? 256:100);std::memset(nativeScenes[n],0,100);function<void*(__thiscall*)(void*,void*)>(0xc48e0)(nativeScenes[n],nativeGame);putPointer(sceneGlobals[n],nativeScenes[n]);XmlDocument xml(files,String(sceneFiles[n]));scenes[n]->loadXML(xml);function<void(__thiscall*)(void*,void*)>(0xc4a80)(nativeScenes[n],wrap(&xml));}
MainMenuContext mainContext(files,manager,settings,strings,audio,music,profiles,input,interaction,[] {return tooltipDimensions;},[](bool){},&mainMenuSourceComplete,views,controls);
GamePauseContext pauseContext{files,manager,settings,strings,audio,music,input,interaction,game,help,mainContext,[] {return tooltipDimensions;},[](bool){},[](const String&){},views,controls};
HudWorld world(profile,{scenes[0].get(),scenes[1].get(),scenes[2].get()},{&overlay,&selection,&tip});
GameMenuContext context{manager,settings,strings,input,interaction,profile,configuration,audio,world,pauseContext,world,views,controls};
// Exact money formatter is the original general unsigned stream primitive.
auto* hud=new GameMenu(&game,context);world.menu=hud;auto* original=allocate(232);std::memset(original,0,232);function<void*(__thiscall*)(void*,void*)>(0xc3430)(original,nativeGame);
const auto check=[&] {
    stage=311;require(GameMenu::instance()==hud && field<void*>(image,0x25f740)==original,"HUD singleton differs");mainMenuCompareView(*hud,original);compareDecorations(*hud,original);
    for(auto pair:std::initializer_list<std::pair<Control*,unsigned>>{{&hud->mainButton(),0x3c},{&hud->marketButton(),0x40},{&hud->farmButton(),0x44},{&hud->wareButton(),0x48},{&hud->journalButton(),0x68},{&hud->warehouseButton(),0x88},{&hud->warehouseProgress(),0x8c},{&hud->warehouseLighter(),0x90},{&hud->moneyText(),0x64}}) {auto* native=field<void*>(original,pair.second);if(pair.first->state().enabled!=bool(field<unsigned char>(native,0x26))) std::cerr<<"HUD state checkpoint "<<hudStates<<" slot "<<std::hex<<pair.second<<std::dec<<"\n";compareControl(*pair.first,native);}
    hudLetters+=compare(hud->moneyText(),field<void*>(original,0x64));for(auto pair:std::initializer_list<std::pair<TextAnimationButton*,unsigned>>{{&hud->mainButton(),0x3c},{&hud->marketButton(),0x40},{&hud->farmButton(),0x44},{&hud->wareButton(),0x48}}) hudLetters+=compare(pair.first->text(),field<void*>(field<void*>(original,pair.second),0x27c));
    require(hud->contractCount==field<unsigned>(original,0x60) && hud->gameplayPaused==bool(field<unsigned char>(original,0xa4)) && hud->hasUnread==bool(field<unsigned char>(original,0xe0)),"HUD scalar flags differ");
    require(!std::memcmp(&hud->blinkColor,static_cast<unsigned char*>(original)+0xa8,16) && !std::memcmp(&hud->normalColor,static_cast<unsigned char*>(original)+0xb8,16),"HUD blink colors differ");compareFloat(hud->blinkPeriod,field<float>(original,0xc8),"HUD blink period");compareFloat(hud->blinkRemaining,field<float>(original,0xcc),"HUD blink remaining");compareFloat(hud->lighterRemaining,field<float>(original,0x94),"HUD lighter time");
    compareFloat(hud->warehouseProgress().current,field<float>(field<void*>(original,0x8c),0x23c),"HUD warehouse percentage");for(unsigned n=0;n<3;++n) require(hud->resumeSceneButtons[n]==bool(field<unsigned char>(original,0x50+n)),"HUD saved scene button flags differ");
    for(unsigned n=0;n<5;++n) require(hud->resumeTools[n]==bool(field<unsigned char>(original,0xe1+n)),"HUD tool resume flags differ");
    const auto* first=field<GameMessage*>(original,0xd4);const auto* end=field<GameMessage*>(original,0xd8);require(hud->messages().size()==(first ? end-first:0),"HUD message count differs");for(unsigned n=0;n<hud->messages().size();++n) require(hud->messages()[n].text.view()==first[n].text.view() && hud->messages()[n].read==first[n].read,"HUD owned message differs");
    for(unsigned n=0;n<3;++n) {auto* native=nativeScenes[n];mainMenuCompareView(*scenes[n],native);require(scenes[n]->movement==field<unsigned>(native,0x4c),"HUD scene movement differs");compareControl(scenes[n]->movingPicture(),field<void*>(native,0x38));compareControl(hud->contract(n).substrate(),field<void*>(field<void*>(original,0x54+n*4),0x30));}
    mainMenuCompareView(hud->pauseMenu(),static_cast<unsigned char*>(field<void*>(original,0xa0))+4);mainMenuCompareView(game,nativeGame);
    require(profile.ints==nativeProfile.ints && profile.uints==nativeProfile.uints && profile.floats==nativeProfile.floats && profile.booleans==nativeProfile.booleans,"HUD profile state differs");
    if(hudSourceCalls!=hudNativeCalls) {std::cerr<<"HUD services checkpoint "<<hudStates<<" source/native "<<hudSourceCalls.size()<<'/'<<hudNativeCalls.size()<<'\n';for(const auto& value:hudSourceCalls) std::cerr<<"S "<<value[0]<<' '<<value[1]<<' '<<value[2]<<'\n';for(const auto& value:hudNativeCalls) std::cerr<<"N "<<value[0]<<' '<<value[1]<<' '<<value[2]<<'\n';}require(hudSourceCalls==hudNativeCalls,"HUD world services differ");hudSourceCalls.clear();hudNativeCalls.clear();++hudStates;
};
if(productionHouses) {
#include "production_house_oracle_cases.inc"
} else {
check();
// Whole native absolute-step body, wealth/level trigger gates and flash state.
std::mt19937 random(0x4c3920);
for(trial=0;trial<128;++trial) {
    hudInputs.wealth={};for(unsigned n=1;n<5;++n) hudInputs.wealth[n]=int(random());hudInputs.level=int(random()%8);hudInputs.free=trial%3;
    profile.ints["Money"]=nativeProfile.ints["Money"]=int(random());profile.ints["mTrigger"]=nativeProfile.ints["mTrigger"]=(trial%3 ? int(random()):0);profile.ints["wlTrigger"]=nativeProfile.ints["wlTrigger"]=int(trial%9)-1;
    hud->gameplayPaused=bool(trial%7==0);field<unsigned char>(original,0xa4)=hud->gameplayPaused;
    hud->refreshMoney();call(0xbfee0,original);check();
    hudInputs.capacity=trial%7 ? random():0;field<unsigned>(nativeWarehouse,0x538+hudInputs.level*0x70)=hudInputs.capacity;hud->refreshWarehouse();call(0xbeb40,original);check();
    hud->suspendButtons();call(0xbdf00,original);check();hud->resumeButtons();call(0xbe340,original);check();
    elapsed=float(trial%11)*0.037f;hud->absoluteStep();call(0xc3920,original);++hudSteps;check();
}
// Native tool capture/stop and resume, including missing-character path.
auto* nativeCharacter=allocate(0x28c0);std::memset(nativeCharacter,0,0x28c0);
for(trial=0;trial<32;++trial) {hudInputs.character=trial%3!=0;putPointer(0x25ff5c,hudInputs.character ? nativeCharacter:nullptr);for(unsigned n=0;n<5;++n) {hudInputs.tools[n]=bool(trial&(1U<<n));field<int>(nativeCharacter,0x28a8+n*4)=hudInputs.tools[n] ? -7:0;}hud->absoluteDisable();call(0xd3790,original);check();hud->absoluteEnable();call(0xd3fb0,original);check();}
hudInputs.character=false;putPointer(0x25ff5c,nullptr);
hudInputs.capacity=100;hudInputs.level=0;hudInputs.free=1;field<unsigned>(nativeWarehouse,0x538)=100;hud->refreshWarehouse();call(0xbeb40,original);check();
// Exact new-scene initialization and profile availability refresh.
hud->initializeScene();call(0xc2cc0,original);check();
// Original message records are deep-copied, including flags and empty vectors.
struct NativeMessages {unsigned allocator=0;GameMessage *begin=nullptr,*end=nullptr,*capacity=nullptr;};static_assert(sizeof(GameMessage)==32);
for(trial=0;trial<16;++trial) {std::vector<GameMessage> items;for(unsigned n=0;n<trial%6;++n) items.push_back({WString(std::wstring(n*19+1,L'A'+n)),bool(n%2)});NativeMessages values;values.begin=items.data();values.end=values.begin+items.size();values.capacity=values.end;hud->assignMessages(items);function<void(__thiscall*)(void*,NativeMessages*)>(0xc4390)(original,&values);check();
    WString note(std::wstring(35+trial,L'Z'));hud->notify(note,bool(trial%2));function<void(__thiscall*)(void*,const WString*,bool)>(0xc44a0)(original,&note,bool(trial%2));check();elapsed=0.31f;hud->absoluteStep();call(0xc3920,original);++hudSteps;check();}
// All 131 actual contract definitions activate the three real owned statuses.
XmlDocument timeline(files,String("Gameplay/Gameplay.xml"));GameplayEventDefinitions definitions;definitions.loadXML(timeline);
std::vector<GameplayEvent*> offers;std::vector<std::array<unsigned char,256>> nativeOffers;
for(const auto& item:definitions.events()) if(item->type==GameplayEventType::Contract) offers.push_back(item.get());nativeOffers.resize(offers.size());
for(unsigned n=0;n<offers.size();++n) {
    auto& e=*offers[n];const auto& c=std::get<ContractEvent>(e.data);auto* out=nativeOffers[n].data();new(out+0x14) WString(e.note);new(out+0x68) WString(c.winNote);new(out+0x84) WString(c.lossNote);
    field<unsigned>(out,0x34)=e.index;field<GameplayTime>(out,0x38)=c.winTime;field<int>(out,0x44)=c.bonus;field<int>(out,0x48)=c.penalty;std::memcpy(out+0x50,c.requirements.data(),24);
    world.events[int(e.index)]=&e;hudNativeEvents[int(e.index)]=out;
}
ContractDialogContext dialogContext{manager,settings,strings,input,interaction,configuration,*hud,views,controls};
auto* ready=new ContractPassConfirm(nullptr,dialogContext);auto* nativeReady=allocate(216);function<void*(__thiscall*)(void*,void*)>(0xb8ce0)(nativeReady,nullptr);
const auto clearContracts=[&] {hud->contractCount=field<unsigned>(original,0x60)=0;for(unsigned n=0;n<3;++n) {auto& status=hud->contract(n);auto* native=field<void*>(original,0x54+n*4);status.active=false;field<unsigned char>(native,0x70)=0;status.hide();status.disable();call(0xd3720,native);call(0xd3750,native);}};
for(trial=0;trial<offers.size();++trial) {
    stage=315;clearContracts();auto& event=*offers[trial];const auto& c=std::get<ContractEvent>(event.data);
    for(unsigned n=0;n<19;++n) profile.uints["WGoodCount"+std::to_string(n)]=nativeProfile.uints["WGoodCount"+std::to_string(n)]=0;
    for(unsigned n=0;n<4;++n) {hud->activateContract(event);function<void(__thiscall*)(void*,void*)>(0xc02c0)(original,nativeOffers[trial].data());++hudActions;check();}
    for(bool preview:{true,false}) {
        hud->stockChanged(preview);function<void(__thiscall*)(void*,bool)>(0xbd9d0)(original,preview);check();
        for(const auto& req:c.requirements) if(req.reqType==0) profile.uints["WGoodCount"+std::to_string(req.productType)]=nativeProfile.uints["WGoodCount"+std::to_string(req.productType)]=req.count;
    }
    hud->stockChanged(false);function<void(__thiscall*)(void*,bool)>(0xbd9d0)(original,false);mainMenuCompareView(*ready,nativeReady);check();
    ready->cancel();call(0xb8900,nativeReady);check();
    for(unsigned n=0;n<3;++n) {hud->removeContract(hud->contract(n));function<void(__thiscall*)(void*,void*)>(0xc06b0)(original,field<void*>(original,0x54+n*4));check();}
}
clearContracts();delete ready;call(0xb9290,nativeReady);
for(unsigned n=0;n<3;++n) {profile.ints["task"+std::to_string(n)]=nativeProfile.ints["task"+std::to_string(n)]=-1;}
// Saved task restoration executes the actual GameMenu constructors, not a selection fixture.
for(unsigned n=0;n<3;++n) {auto& e=*offers[n];profile.ints["task"+std::to_string(n)]=nativeProfile.ints["task"+std::to_string(n)]=int(e.index);profile.floats["taskTime"+std::to_string(n)]=nativeProfile.floats["taskTime"+std::to_string(n)]=123.f+n;}
{const auto before=textureCounts();delete hud;const auto middle=textureCounts();call(0xc31f0,original);compareReferenceDeltas(before,middle);}
hud=new GameMenu(&game,context);world.menu=hud;original=allocate(232);std::memset(original,0,232);function<void*(__thiscall*)(void*,void*)>(0xc3430)(original,nativeGame);check();clearContracts();
for(unsigned n=0;n<3;++n) profile.ints["task"+std::to_string(n)]=nativeProfile.ints["task"+std::to_string(n)]=-1;
// Controller callbacks retain the native tutorial short-circuit and input gates.
const auto resetScene=[&](unsigned index) {for(unsigned n=0;n<3;++n) {scenes[n]->movement=0;field<unsigned>(nativeScenes[n],0x4c)=0;if(n==index) {scenes[n]->show();call(0xd37d0,nativeScenes[n]);}else {scenes[n]->hide();call(0xd3720,nativeScenes[n]);}}};
for(trial=0;trial<40;++trial) {
    stage=316;hudInputs.active=trial%10!=0;hudInputs.blocked=trial%10==1;hudInputs.steps=trial%10<2 ? 0:1ULL<<std::array<unsigned,8>{6,7,0x1e,0x29,0x2c,0x27,5,0x2d}[trial%10-2];
    field<int>(contractTutorialObject.data(),0xbfc)=hudInputs.active ? 0:-1;field<unsigned char>(image,0x25f720)=hudInputs.blocked;field<unsigned char>(image,0x25f721)=0;
    resetScene(trial%3);auto* button=std::array<ButtonPrototype*,4>{&hud->marketButton(),&hud->farmButton(),&hud->wareButton(),&hud->journalButton()}[trial/10];
    button->clickEvent(button);function<void(__cdecl*)(void*)>(std::array<unsigned,4>{0xc0b50,0xc0f70,0xc1350,0xc1770}[trial/10])(nullptr);++hudActions;check();
}
hudInputs.active=false;hudInputs.blocked=false;hudInputs.steps=0;field<int>(contractTutorialObject.data(),0xbfc)=-1;field<unsigned char>(image,0x25f720)=0;resetScene(0);
// Absolute-step tutorial dispatch uses actual anchors and native ordering.
for(trial=0;trial<24;++trial) {resetScene(trial%3);hudInputs.can=trial%8<4 ? (1ULL<<6)|(1ULL<<7)|(1ULL<<0x1e)|(1ULL<<0x27):~0ULL;hud->marketButton().enable();hud->farmButton().enable();hud->wareButton().enable();for(unsigned slot:{0x40U,0x44U,0x48U}) call(0xd3bb0,field<void*>(original,slot));elapsed=0.07f;hud->absoluteStep();call(0xc3920,original);++hudSteps;check();}
hudInputs.can=0;resetScene(0);
// Source/native scene navigation executes actual StateView movement, not traces.
for(unsigned n=0;n<3;++n) {scenes[n]->hide();call(0xd3720,nativeScenes[n]);}scenes[0]->show();call(0xd37d0,nativeScenes[0]);
menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);timerTable[3]=reinterpret_cast<void*>(&tooltipUnscaledTimer);graphicsTable[3]=reinterpret_cast<void*>(&tooltipHeight);graphicsTable[4]=reinterpret_cast<void*>(&tooltipWidth);
const auto frame=[&](Point2 point=Point2{0,0},std::vector<unsigned> events={},std::array<int,3> delta={}) {stage=312;menuMilliseconds+=50;menuPoint=point;menuInputEvents=std::move(events);menuMouseDelta=delta;hudPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++hudFrames;check();};
if(!tutorialTips) for(trial=0;trial<6;++trial) {stage=313;switch(trial%3) {case 0:hud->switchMarket();call(0xbf4d0,original);break;case 1:hud->switchWare();call(0xbf9e0,original);break;default:hud->switchFarm();call(0xbef70,original);break;}++hudActions;check();for(unsigned n=0;n<9;++n) frame();}
hud->show();hud->enable();hud->absoluteEnable();call(0xd37d0,original);call(0xd3bb0,original);call(0xd3fb0,original);check();
const auto click=[&](ButtonPrototype& button) {const Point2 point{button.geometry().x,button.geometry().y};frame(point,{}, {1,0,0});frame(point,{0x208});frame(point,{0x200});frame({0,0},{},{1,0,0});++hudActions;};
if(!tutorialTips) {
for(trial=0;trial<3;++trial) {auto& button=*std::array<ButtonPrototype*,3>{&hud->marketButton(),&hud->wareButton(),&hud->farmButton()}[trial];button.enable();call(0xd3bb0,field<void*>(original,std::array<unsigned,3>{0x40,0x48,0x44}[trial]));click(button);for(unsigned n=0;n<9;++n) frame();}
// Actual warehouse hover input opens and closes its required world service.
frame({hud->warehouseButton().geometry().x,hud->warehouseButton().geometry().y},{},{1,0,0});frame({0,0},{},{1,0,0});
// Actual Escape suspends the parent game and the actual pause Enter binding resumes it.
frame({0,0},{1});require(!game.state().enabled && hud->pauseMenu().state().shown,"HUD Escape did not suspend actual game");++hudActions;
for(unsigned n=0;n<12;++n) frame();frame({0,0},{0x1c});++hudActions;for(unsigned n=0;n<12;++n) frame();require(game.state().enabled && !hud->pauseMenu().state().shown,"HUD pause Enter did not restore game");
}
if(tutorialTips && !tutorialManagerMode && !movableUnions && !buySeedsMode && !buyBuildingMode) {
#include "tutorial_tips_oracle_cases.inc"
}
if(tutorialManagerMode) {
#include "tutorial_manager_oracle_cases.inc"
}
if(movableUnions) {
#include "movable_union_oracle_cases.inc"
}
if(buySeedsMode) {
#include "buy_seeds_oracle_cases.inc"
}
if(buyBuildingMode) {
#include "buy_building_oracle_cases.inc"
}
for(auto& out:nativeOffers) {field<WString>(out.data(),0x14).~WString();field<WString>(out.data(),0x68).~WString();field<WString>(out.data(),0x84).~WString();}hudNativeEvents.clear();
}
stage=314;const auto before=textureCounts();delete hud;const auto middle=textureCounts();call(0xc31f0,original);compareReferenceDeltas(before,middle);require(!GameMenu::instance() && !field<void*>(image,0x25f740),"HUD destruction retains singleton");
for(unsigned n=0;n<3;++n) {scenes[n].reset();call(0x2c7d0,nativeScenes[n]);putPointer(sceneGlobals[n],nullptr);}for(auto* native:{nativeHelp,nativeOverlay,nativeSelection,nativeTip,nativeGame}) call(0xd58a0,native);
music.clear();profiles.close();settings.set(String("game"),String("profiles"),oldProfiles);menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);loadingDocuments.clear();loadingFiles=nullptr;loadingSoundManager=nullptr;
