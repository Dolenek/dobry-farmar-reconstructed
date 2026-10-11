stage=190;
const auto common=std::filesystem::path(".local")/(std::string("main-menu-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
std::filesystem::create_directories(common);files.mountCommon(String(common.generic_string()));
String previousProfiles;settings.get(String("game"),String("profiles"),previousProfiles);settings.set(String("game"),String("profiles"),String("MainMenuProfiles"));
LoadingDisabledAudio disabledBackend;SoundManager audio(files,settings,disabledBackend,0);MusicPlayback music;
ProfileSession profiles(files,settings,manager);profiles.open(WString(L"Alpha"));profiles.open(WString(L"Zebra"));profiles.open(WString(L"Alpha"));
selectorNames=profiles.current()->headers().names();selectorNextID=profiles.current()->headers().nextID();selectorEvents.clear();
auto* nativeProfile=allocate(88);selectorProfileConstruct(nativeProfile,nullptr,&profiles.current()->headers().currentName());selectorEvents.clear();
loadingFiles=&files;loadingSoundManager=&audio;
managerTable[6]=reinterpret_cast<void*>(&loadingXML);managerTable[14]=reinterpret_cast<void*>(&mainMenuNativeLog);
managerTable[15]=image+0x141700;managerTable[17]=reinterpret_cast<void*>(&loadingClear);
xmlTable[0]=reinterpret_cast<void*>(&loadingDeleteXML);xmlTable[34]=reinterpret_cast<void*>(&sectionCount);
settingsTable[10]=reinterpret_cast<void*>(&mainMenuSettingsBool);
redirect(0xecb20,reinterpret_cast<void*>(&selectorProfileConstruct));redirect(0xe8900,reinterpret_cast<void*>(&selectorProfileNames));redirect(0xe8d20,reinterpret_cast<void*>(&selectorProfileRemove));
redirect(0x2fe80,reinterpret_cast<void*>(&decimalString));redirect(0x1cc0,reinterpret_cast<void*>(&concatenateString));redirect(0x7d0b0,reinterpret_cast<void*>(&nativeDiscard));
redirect(0x842d0,reinterpret_cast<void*>(&mainMenuNativeDiscard));redirect(0x832f0,reinterpret_cast<void*>(&mainMenuNativeComplete));
redirect(0xa670,reinterpret_cast<void*>(&awardsProfileKey));redirect(0x105e0,reinterpret_cast<void*>(&awardsProfileBool));
graphicsTable[3]=reinterpret_cast<void*>(&tooltipHeight);graphicsTable[4]=reinterpret_cast<void*>(&tooltipWidth);timerTable[3]=reinterpret_cast<void*>(&tooltipUnscaledTimer);
nativeOptionsAudio={audio.disabled(),audio.soundVolume(),audio.musicVolume(),{}};nativeOptionsFull=false;settings.set(String("graphics"),String("fullscreen"),false);
std::array<void*,7> loadingFileTable{};loadingFileTable[6]=reinterpret_cast<void*>(&loadingExists);auto* loadingFilePointer=loadingFileTable.data();putPointer(0x25f72c,&loadingFilePointer);
soundTable[3]=reinterpret_cast<void*>(&loadingMusicChannel);graphicsTable[5]=reinterpret_cast<void*>(&optionsReset);
void* quitFunction=reinterpret_cast<void*>(&PostQuitMessage);write(0x1ee268,&quitFunction,4);
ViewRegistry views;ControlRegistry controls;unsigned resets=0;
MainMenuContext context(files,manager,settings,strings,audio,music,profiles,input,interaction,[] {return tooltipDimensions;},
    [&](bool) {++resets;},&mainMenuSourceComplete,views,controls);
auto* menu=new MainMenu(context);auto* original=allocate(96);function<void*(__thiscall*)(void*)>(0x84da0)(original);
void* nativeView=static_cast<unsigned char*>(original)+4;
const auto check=[&] {
    stage=191;require(MainMenu::instance()==menu && field<void*>(image,0x25ffd0)==original,"MainMenu singleton differs");
    mainMenuCompareView(*menu,nativeView);mainMenuCompareView(*context.fixedBack(),field<void*>(image,0x25ffec));
    compareDecorations(*menu,nativeView);compareDecorations(*context.fixedBack(),field<void*>(image,0x25ffec));
    const auto& content=menu->content();
    for(const auto& pair:std::initializer_list<std::pair<TextAnimationButton*,unsigned>>{
        {content.start,0x3c},{content.exit,0x40},{content.changeUser,0x44},{content.options,0x4c},{content.awards,0x54},{content.credits,0x50},{content.unused,0x48}}) {
        auto* native=field<void*>(original,pair.second);compareControl(*pair.first,native);
        require(unsigned(pair.first->buttonState())==field<unsigned>(native,0x238),"MainMenu button state differs");mainMenuLetters+=compare(pair.first->text(),field<void*>(native,0x27c));
    }
    mainMenuLetters+=compare(*content.userName,field<void*>(original,0x58));mainMenuLetters+=compare(*content.userText,field<void*>(original,0x5c));
    require(input.controls(1).size()==nativeInput.controls(1).size(),"MainMenu Escape binding count differs");
    const bool sourceRegistered=views.enabled().count(menu)!=0;
    std::vector<void*> nativeEnabled;auto* head=field<Node*>(image,0x26050c);collectControls(head->parent,nativeEnabled);
    require(sourceRegistered==(std::find(nativeEnabled.begin(),nativeEnabled.end(),nativeView)!=nativeEnabled.end()),"MainMenu reversed base-hook registration differs");
    ++mainMenuStates;
};
check();
context.restore(false);function<void(__cdecl*)()>(0x85490)();check();
menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
std::array<void*,6> logicTable{};std::copy(cursorTable.begin(),cursorTable.end(),logicTable.begin());logicTable[5]=image+0x1306c0;
std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=logicTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
const auto frame=[&](Point2 cursor,std::vector<unsigned> events={},std::array<int,3> delta={}) {
    stage=192;menuMilliseconds+=50;menuPoint=cursor;menuInputEvents=std::move(events);menuMouseDelta=delta;
    mainMenuPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++mainMenuFrames;
    for(auto* view:views.enabled()) if(dynamic_cast<LoadingFromLog*>(view)) {++mainMenuLoadingSteps;break;}
};
// Explicit source/native deletion after the original transition queue has
// produced its discard requests. This frame oracle's native discard map is
// an existing diagnostic service boundary rather than original STL storage.
const auto finishTransitions=[&] {
    for(unsigned i=0;i<24 && !views.transitions().empty();++i) frame({0,0});
    require(views.transitions().empty() && field<unsigned>(image,0x260528)==0,"MainMenu transition did not finish");
};
const auto deleteDiscarded=[&] {
    auto expected=nativeDiscards;
    mainMenuNativeDiscardTree(field<void*>(field<void*>(image,0x2604e8),4),expected);
    require(expected.size()==views.discarded().size(),"MainMenu discard queue size differs");
    std::multiset<std::string> sourceOrigins,nativeOrigins;
    for(const auto& entry:views.discarded()) sourceOrigins.emplace(entry.second.view());
    for(const auto& entry:expected) nativeOrigins.emplace(entry.second);
    require(sourceOrigins==nativeOrigins,"MainMenu discard origin strings differ");
    views.deleteDiscarded();for(const auto& entry:expected) mainMenuDeleteNative(entry.first);nativeDiscards.clear();
    require(field<unsigned>(image,0x2604ec)==0,"MainMenu native destruction retained original discard tree");
};
const auto findExit=[&]() -> ExitDialog* {for(auto* view:views.enabled()) if(auto* exit=dynamic_cast<ExitDialog*>(view)) return exit;return nullptr;};
const auto click=[&](TextAnimationButton& button) {
    const Point2 point{button.geometry().x,button.geometry().y};
    frame(point,{}, {1,0,0});frame(point,{0x208});frame(point,{0x200});frame({0,0},{},{1,0,0});++mainMenuActions;
};
finishTransitions();check();
const WString emptyName;menu->content().userName->setString(emptyName);
function<void(__thiscall*)(void*,const WString*)>(0xdd030)(field<void*>(original,0x58),&emptyName);check();
menu->enabledStep();call(0x81770,nativeView);check();
const auto* fixed=context.fixedBack();const auto* nativeFixed=field<void*>(image,0x25ffec);
const String partialName("main-menu-partial.xml");const wchar_t partialText[]=L"<MainMenu Visible=\"1\" Enable=\"1\"/>";
{auto file=files.create(partialName);file->write(partialText,sizeof(partialText)-2);}
XmlDocument partial(files,partialName);menu->loadXML(partial);function<void(__thiscall*)(void*,void*)>(0x81e10)(nativeView,wrap(&partial));check();
require(context.fixedBack()==fixed && field<void*>(image,0x25ffec)==nativeFixed,"MainMenu XML created FixedBack repeatedly");
// Complete real Options navigation and singleton guard, then actual OK input.
click(*menu->content().options);auto* options=Options::instance();auto* nativeOptions=field<void*>(image,0x26000c);
require(options && nativeOptions,"MainMenu did not create Options");mainMenuCompareView(*options,static_cast<unsigned char*>(nativeOptions)+4);check();
require(!menu->openOptions(),"MainMenu duplicate Options guard differs");function<void(__cdecl*)()>(0x826c0)();
finishTransitions();click(options->okButton());finishTransitions();deleteDiscarded();check();
// MainMenuContent field names retain the native allocation-order naming. The
// actual CreditsButton XML is content.awards; AwardsButton is content.credits.
click(*menu->content().awards);auto* credits=Credits::instance();auto* nativeCredits=field<void*>(image,0x25ffd4);
require(credits && nativeCredits,"MainMenu did not create Credits");mainMenuCompareView(*credits,static_cast<unsigned char*>(nativeCredits)+4);check();
require(!menu->openCredits(),"MainMenu duplicate Credits guard differs");function<void(__cdecl*)()>(0x82ef0)();
finishTransitions();click(credits->returnButton());finishTransitions();deleteDiscarded();check();
click(*menu->content().credits);auto* awards=Awards::instance();auto* nativeAwards=field<void*>(image,0x25ffcc);
require(awards && nativeAwards,"MainMenu did not create Awards");mainMenuCompareView(*awards,static_cast<unsigned char*>(nativeAwards)+4);check();
require(!menu->openAwards(),"MainMenu duplicate Awards guard differs");function<void(__cdecl*)()>(0x82ae0)();
finishTransitions();click(awards->returnButton());finishTransitions();deleteDiscarded();check();
// Actual persisted profiles on source, explicit native profile service on oracle.
click(*menu->content().changeUser);auto* selector=SelectProfile::instance();auto* nativeSelector=field<void*>(image,0x2604f0);
require(selector && nativeSelector,"MainMenu did not create profile selector");mainMenuCompareView(*selector,static_cast<unsigned char*>(nativeSelector)+4);check();
finishTransitions();selector->select(1);function<void(__cdecl*)(unsigned,void*)>(0xfeaa0)(1,field<void*>(nativeSelector,0x3c));
click(selector->okButton());finishTransitions();deleteDiscarded();check();
require(profiles.current()->headers().currentName().view()==L"Zebra" && menu->content().userName->string().view()==L"Zebra","MainMenu accepted profile name differs");
click(*menu->content().changeUser);selector=SelectProfile::instance();finishTransitions();click(selector->exitButton());finishTransitions();deleteDiscarded();check();
// Actual Exit input and Escape binding. No resumes MainMenu, Yes posts the same
// WM_QUIT with code 0 as original cLogic slot +14; message is consumed here.
click(*menu->content().exit);auto* exit=findExit();require(exit!=nullptr,"Source Exit factory capture missing");
auto* nativeExit=mainMenuLastNativeExit;mainMenuCompareView(*exit,nativeExit);check();
finishTransitions();click(exit->noButton());finishTransitions();deleteDiscarded();check();
frame({0,0},{1});exit=findExit();nativeExit=mainMenuLastNativeExit;require(exit && nativeExit,"MainMenu Escape did not create Exit");check();
MSG message{};
const auto consumeQuit=[&] {
    for(unsigned i=0;i<1000 && PeekMessageW(&message,nullptr,0,0,PM_REMOVE);++i) {
        if(message.message==WM_QUIT) return message.wParam==0;
        TranslateMessage(&message);DispatchMessageW(&message);
    }
    return false;
};
while(PeekMessageW(&message,nullptr,0,0,PM_REMOVE)) {if(message.message!=WM_QUIT) {TranslateMessage(&message);DispatchMessageW(&message);}}
exit->yesEvent(exit);require(consumeQuit(),"MainMenu source quit differs");
function<void(__cdecl*)(void*)>(0x817c0)(nativeExit);require(consumeQuit(),"MainMenu native quit differs");++mainMenuQuitChecks;
delete exit;mainMenuDeleteNative(nativeExit);menu->showTransparently();menu->enable();call(0xd4590,nativeView);call(0xd3bb0,nativeView);finishTransitions();check();
// Absent-menu restoration branch constructs both complete menus and FixedBack.
delete menu;mainMenuDeleteNative(nativeView);require(!MainMenu::instance() && !context.fixedBack() && !field<void*>(image,0x25ffd0) && !field<void*>(image,0x25ffec),"MainMenu destruction retained singleton/background");
menu=&context.restore(true);function<void(__cdecl*)()>(0x81860)();original=field<void*>(image,0x25ffd0);nativeView=static_cast<unsigned char*>(original)+4;check();finishTransitions();check();
// Start writes the real isolated resource snapshot and creates real Loading.
// The callback adapter counts completion and explicitly does not instantiate a
// fake game. Actual cGame construction remains the next recovery dependency.
mainMenuLogging=true;settings.set(String("game"),String("LogResources"),true);
managerTable[3]=reinterpret_cast<void*>(&loadingSoundResource);
click(*menu->content().start);LoadingFromLog* loading=nullptr;
for(auto* view:views.enabled()) if(auto* candidate=dynamic_cast<LoadingFromLog*>(view)) {loading=candidate;break;}
auto* nativeLoading=mainMenuLastNativeLoading;
require(loading && nativeLoading && mainMenuNativeLogs==1 && files.isCommonExist(String("menureslog.xml")),"MainMenu Start log/loading differs");
require(views.discarded().at(menu).view()==nativeDiscards.at(nativeView) && views.discarded().at(menu).view()==".\\Game\\Ui\\MainMenu.cpp153","MainMenu Start discard origin differs");
check();deleteDiscarded();menu=nullptr;original=nativeView=nullptr;
for(unsigned i=0;i<40 && mainMenuSourceCompleted==0;++i) {
    loading->enabledStep();call(0x81130,nativeLoading);
    require(loading->nextIndex()==field<unsigned>(nativeLoading,0x78) && loading->total()==field<unsigned>(nativeLoading,0x7c) && loading->batch()==30 && field<unsigned>(nativeLoading,0x80)==30,"MainMenu integrated Loading range differs");
    require(mainMenuSourceCompleted==mainMenuNativeCompleted,"MainMenu game completion order differs");++mainMenuLoadingSteps;
}
require(mainMenuSourceCompleted==1 && mainMenuNativeCompleted==1,"MainMenu Loading completion missing");deleteDiscarded();
music.clear();if(auto* resource=field<cSound*>(image,0x25ffe0)) resource->releaseReference();putPointer(0x25ffe0,nullptr);putPointer(0x25ffdc,nullptr);
profiles.close();selectorProfileDestroy(field<void*>(image,0x25f738),nullptr,1);settings.set(String("game"),String("profiles"),previousProfiles);
require(views.enabled().empty() && controls.visible().empty() && input.controls(1).empty() && nativeInput.controls(1).empty(),"MainMenu navigation retained registries/input");
require(!resets && nativeOptionsResets.empty(),"MainMenu navigation unexpectedly reset graphics");
menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
loadingDocuments.clear();loadingFiles=nullptr;loadingSoundManager=nullptr;profileProperties().booleans.clear();
