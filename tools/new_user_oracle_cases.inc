stage=190;
const auto common=std::filesystem::path(".local")/(std::string("new-user-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
std::filesystem::create_directories(common);files.mountCommon(String(common.generic_string()));
String previousProfiles;settings.get(String("game"),String("profiles"),previousProfiles);settings.set(String("game"),String("profiles"),String("NewUserProfiles"));
LoadingDisabledAudio disabledBackend;SoundManager audio(files,settings,disabledBackend,0);MusicPlayback music;
ProfileSession profiles(files,settings,manager);selectorNames.clear();selectorNextID=1;selectorEvents.clear();putPointer(0x25f738,nullptr);
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

newUserMainContext=&context;newUserSourceCompletions=newUserNativeCompletions=0;
context.ensureFixedBack();function<void(__cdecl*)()>(0x81990)();
menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
std::array<void*,6> logicTable{};std::copy(cursorTable.begin(),cursorTable.end(),logicTable.begin());logicTable[5]=image+0x1306c0;
std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=logicTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
NewUserContext userContext{manager,settings,strings,profiles,input,interaction,views,controls};
NewUser* user=nullptr;void* nativeUser=nullptr;void* nativeUserView=nullptr;
const auto check=[&] {
    stage=952;require(NewUser::instance()==user && field<void*>(image,0x2604f4)==nativeUser,"NewUser singleton differs");
    mainMenuCompareView(*user,nativeUserView);compareDecorations(*user,nativeUserView);
    auto* nativeName=field<void*>(nativeUser,0x40);auto* nativeOK=field<void*>(nativeUser,0x3c);
    compareControl(user->nameEdit(),nativeName);compare(user->nameEdit(),nativeName);
    compareControl(user->okButton(),nativeOK);compare(user->okButton().text(),field<void*>(nativeOK,0x27c));
    require(unsigned(user->okButton().buttonState())==field<unsigned>(nativeOK,0x238),"NewUser OK frame differs");
    require(user->nameEdit().rightInput()==bool(field<unsigned char>(nativeName,0x2b8)) && user->nameEdit().caretShown()==bool(field<unsigned char>(nativeName,0x2cc)),"NewUser caret state differs");
    compareFloat(user->nameEdit().maximumWidth(),field<float>(nativeName,0x2c4),"NewUser maximum name width");
    require(input.controls(0x1c).size()==nativeInput.controls(0x1c).size() && input.keyboard().size()==nativeInput.keyboard().size(),"NewUser Enter/character bindings differ");
    std::vector<void*> children,decorations;collectControls(field<Node*>(nativeUserView,0x18)->parent,children);collectControls(field<Node*>(nativeUserView,0x30)->parent,decorations);
    auto* textTable=image+0x1fb818; //004daca0, actual cTextLine rather than animated OK text
    children.erase(std::remove_if(children.begin(),children.end(),[&](void* item){return field<void*>(item,0)!=textTable;}),children.end());
    require(children.size()==user->labels().size(),"NewUser appended labels differ");
    for(unsigned i=0;i<children.size();++i) {compareControl(*user->labels()[i],children[i]);compare(*user->labels()[i],children[i]);}
    require(newUserSourceCompletions==newUserNativeCompletions,"NewUser completion count differs");++newUserStates;
};
const auto frame=[&](Point2 point=Point2{0,0},std::vector<unsigned> events={},std::array<int,3> delta={}) {
    stage=953;menuMilliseconds+=50;menuPoint=point;menuInputEvents=std::move(events);menuMouseDelta=delta;
    newUserPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++newUserFrames;if(user) check();
};
const auto settle=[&] {for(unsigned i=0;i<24 && !views.transitions().empty();++i) frame();require(views.transitions().empty() && field<unsigned>(image,0x260528)==0,"NewUser transition did not finish");};
const auto type=[&](const std::vector<wchar_t>& text) {input.dispatchCharacters(text);nativeInput.dispatchCharacters(text);user->enabledStep();call(0x1053a0,nativeUserView);++newUserActions;check();frame();};
const auto click=[&](ButtonPrototype& button) {Point2 point{button.geometry().x,button.geometry().y};frame(point,{}, {1,0,0});frame(point,{0x208});frame(point,{0x200});frame({0,0},{},{1,0,0});++newUserActions;};
const auto discard=[&] {
    auto expected=nativeDiscards;mainMenuNativeDiscardTree(field<void*>(field<void*>(image,0x2604e8),4),expected);
    require(expected.size()==views.discarded().size(),"NewUser discard count differs");
    std::multiset<std::string> actualOrigins,expectedOrigins;for(const auto& item:views.discarded()) actualOrigins.emplace(item.second.view());for(const auto& item:expected) expectedOrigins.emplace(item.second);
    require(actualOrigins==expectedOrigins,"NewUser discard origins differ");views.deleteDiscarded();for(const auto& item:expected) mainMenuDeleteNative(item.first);nativeDiscards.clear();
};
for(unsigned cycle=0;cycle<2;++cycle) {
    if(auto* menu=MainMenu::instance()) {menu->hide();call(0xd3720,static_cast<unsigned char*>(field<void*>(image,0x25ffd0))+4);}
    user=new NewUser(userContext);nativeUser=allocate(76);function<void*(__thiscall*)(void*)>(0x106780)(nativeUser);nativeUserView=static_cast<unsigned char*>(nativeUser)+4;
    user->completed=&newUserSourceComplete;field<void*>(nativeUser,0x48)=reinterpret_cast<void*>(&newUserNativeComplete);++newUserObjects;check();
    user->enabledStep();call(0x1053a0,nativeUserView);check();
    input.dispatchControls({0x1c});nativeInput.dispatchControls({0x1c});++newUserActions;require(newUserSourceCompletions==cycle && newUserNativeCompletions==cycle,"Empty NewUser accepted");check();
    type({L'A',L'B',8,L'\u017d',L'\u65e5',13});
    std::vector<wchar_t> erase(20,8);type(erase);require(user->nameEdit().string().empty() && !user->okButton().state().shown,"NewUser empty changed callback differs");
    type({L'N',L'a',L'm',L'e'});
    require(sounds.size()>=4,"NewUser sound fixture count differs");
    std::vector<cResource*> overwrittenSounds;for(unsigned i=unsigned(sounds.size())-4;i<sounds.size();++i) overwrittenSounds.push_back(sounds[i].get());
    const auto definition=manager.loadSettingsXML(String("UI/NewUser.xml"));user->loadXML(*definition);function<void(__thiscall*)(void*,void*)>(0x105bf0)(nativeUserView,wrap(definition.get()));check();
    require(user->labels().size()==6,"NewUser XML reload must retain and append old owned labels");
    type(erase); // Empty XML String does not necessarily replace existing TextLine content.
    const WString requested(cycle ? L"Second Farm":L"First Farm"),name(cycle ? L"SecondFarm":L"FirstFarm");
    std::vector<wchar_t> characters(requested.data(),requested.data()+requested.size());type(characters);
    require(user->nameEdit().string().view()==name.view(),"NewUser must preserve the shipped font's rejection of spaces");
    if(cycle) click(user->okButton());else {frame({0,0},{0x1c});++newUserActions;}
    require(newUserSourceCompletions==cycle+1 && newUserNativeCompletions==cycle+1,"NewUser completion missing");
    require(MainMenu::instance() && field<void*>(image,0x25ffd0),"NewUser did not construct/restore actual MainMenu");
    require(profiles.current() && profiles.current()->headers().currentName().view()==name.view(),"NewUser source persistence name differs");
    require(field<WString>(field<void*>(image,0x25f738),0x24).view()==name.view(),"NewUser native profile request differs");
    settle();discard();user=nullptr;nativeUser=nativeUserView=nullptr;
    require(!NewUser::instance() && !field<void*>(image,0x2604f4),"NewUser destruction retained singleton");
    mainMenuCompareView(*MainMenu::instance(),static_cast<unsigned char*>(field<void*>(image,0x25ffd0))+4);
    frame();
    for(auto* sound:overwrittenSounds) {require(sound->referenceCount()==1,"NewUser XML sound overwrite reference differs");++newUserSoundResidues;sound->releaseReference();} // Diagnostic cleanup after checking both native/source retained references.
}
require(profiles.current()->headers().names().size()==2 && selectorNames.size()==2,"NewUser profile names did not persist");
delete MainMenu::instance();mainMenuDeleteNative(static_cast<unsigned char*>(field<void*>(image,0x25ffd0))+4);
profiles.close();selectorProfileDestroy(field<void*>(image,0x25f738),nullptr,1);
music.clear();if(auto* resource=field<cSound*>(image,0x25ffe0)) resource->releaseReference();putPointer(0x25ffe0,nullptr);putPointer(0x25ffdc,nullptr);
settings.set(String("game"),String("profiles"),previousProfiles);
require(views.enabled().empty() && controls.visible().empty() && input.controls(0x1c).empty() && nativeInput.controls(0x1c).empty(),"NewUser retained UI/input");
menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
loadingDocuments.clear();loadingFiles=nullptr;loadingSoundManager=nullptr;profileProperties().booleans.clear();newUserMainContext=nullptr;
