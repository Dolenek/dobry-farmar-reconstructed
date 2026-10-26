stage=190;
const auto common=std::filesystem::path(".local")/(std::string("startup-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
std::filesystem::create_directories(common);files.mountCommon(String(common.generic_string()));
String previousProfiles;settings.get(String("game"),String("profiles"),previousProfiles);settings.set(String("game"),String("profiles"),String("StartupProfiles"));
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

startupSettings=&settings;managerTable[3]=reinterpret_cast<void*>(&startupSoundResource);
redirect(0xe14e0,reinterpret_cast<void*>(&startupNativeHeadersExist));redirect(0xe15e0,reinterpret_cast<void*>(&startupNativeStoredName));
menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
std::array<void*,6> logicTable{};std::copy(cursorTable.begin(),cursorTable.end(),logicTable.begin());logicTable[5]=image+0x1306c0;
std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=logicTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
StartupContext startupContext(files,manager,settings,strings,audio,music,profiles,input,interaction,context,views,controls);
StartupView* startup=nullptr;void* nativeStartup=nullptr;
const auto check=[&] {
    if(startup) {
        stage=960;mainMenuCompareView(*startup,nativeStartup);compareControl(startup->logo(),field<void*>(nativeStartup,0x2c));
        require(startup->stage()==field<unsigned>(nativeStartup,0x38) && startup->started()==bool(field<unsigned char>(nativeStartup,0x30)),"Startup stage/completion differs");
        compareFloat(startup->elapsed(),field<float>(nativeStartup,0x34),"Startup elapsed");
    }
    require(bool(NewUser::instance())==bool(field<void*>(image,0x2604f4)) && bool(MainMenu::instance())==bool(field<void*>(image,0x25ffd0)),"Startup resulting screen differs");
    require(input.controls(InputBindings::AllControls).size()==nativeInput.controls(InputBindings::AllControls).size(),"Startup all-control binding differs");++startupStates;
};
const auto collect=[&] {
    // Generic View discards use the explicit recorded-map boundary. Native
    // Loading uses the original queue and original frame's actual collector.
    unsigned sourceGeneric=0;
    for(const auto& entry:views.discarded()) if(!dynamic_cast<LoadingFromLog*>(entry.first)) ++sourceGeneric;
    require(sourceGeneric==nativeDiscards.size(),"Startup generic discard count differs");
    std::multiset<std::string> actualOrigins,expectedOrigins;
    for(const auto& entry:views.discarded()) if(!dynamic_cast<LoadingFromLog*>(entry.first)) actualOrigins.emplace(entry.second.view());
    for(const auto& entry:nativeDiscards) expectedOrigins.emplace(entry.second);
    require(actualOrigins==expectedOrigins,"Startup generic discard origin differs");
    if(startup && views.discarded().count(startup)) {startup=nullptr;nativeStartup=nullptr;}
    const auto expected=nativeDiscards;views.deleteDiscarded();for(const auto& entry:expected) mainMenuDeleteNative(entry.first);nativeDiscards.clear();
};
const auto frame=[&](std::vector<unsigned> events={}) {
    stage=961;menuMilliseconds+=50;menuPoint={0,0};menuInputEvents=std::move(events);menuMouseDelta={};
    try {startupPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);}catch(...) {
        std::cerr<<"Startup frame "<<startupFrames<<" source "<<(startup ? startup->stage():99)<<" elapsed "<<(startup ? startup->elapsed():-1)<<" started "<<(startup && startup->started())
            <<" native "<<(nativeStartup ? field<unsigned>(nativeStartup,0x38):99)<<" elapsed "<<(nativeStartup ? field<float>(nativeStartup,0x34):-1)<<" started "<<(nativeStartup && field<unsigned char>(nativeStartup,0x30))<<" toggle "<<unsigned(image[0x25d290])<<"\n";throw;
    }++startupFrames;check();collect();
};
for(unsigned cycle=0;cycle<2;++cycle) {
    startup=new StartupView(startupContext);nativeStartup=allocate(60);function<void*(__thiscall*)(void*)>(0x8b4a0)(nativeStartup);++startupObjects;check();
    startup->hide();startup->showTransparently();call(0xd3720,nativeStartup);call(0xd4590,nativeStartup);check();
    if(cycle) {
        // Actual key-down/key-up dispatch reaches the process-wide toggle.
        for(unsigned pair=0;pair<4;++pair) {frame({0x1e});frame({0x11e});++startupActions;}
    }
    for(unsigned i=0;i<800 && !MainMenu::instance() && !NewUser::instance();++i) frame();
    require(cycle ? MainMenu::instance()!=nullptr:NewUser::instance()!=nullptr,"Startup failed to reach the proper profile/menu branch");
    if(!cycle) {
        for(unsigned i=0;i<30 && !views.transitions().empty();++i) frame();
        const WString name(L"StartupFarm");std::vector<wchar_t> text(name.data(),name.data()+name.size());input.dispatchCharacters(text);nativeInput.dispatchCharacters(text);++startupActions;
        frame({0x1c});++startupActions;require(MainMenu::instance(),"Startup first profile did not restore real MainMenu");
    }
    for(unsigned i=0;i<40 && !views.transitions().empty();++i) frame();frame();
    require(!startup && !nativeStartup && !NewUser::instance() && !field<void*>(image,0x2604f4),"Startup retained discarded screens");
    mainMenuCompareView(*MainMenu::instance(),static_cast<unsigned char*>(field<void*>(image,0x25ffd0))+4);
    require(profiles.current() && profiles.current()->headers().currentName().view()==L"StartupFarm","Startup persisted/loaded profile differs");
    require(field<WString>(field<void*>(image,0x25f738),0x24).view()==L"StartupFarm","Startup native profile request differs");
    delete MainMenu::instance();mainMenuDeleteNative(static_cast<unsigned char*>(field<void*>(image,0x25ffd0))+4);
    profiles.close();selectorProfileDestroy(field<void*>(image,0x25f738),nullptr,1);
}
music.clear();if(auto* resource=field<cSound*>(image,0x25ffe0)) resource->releaseReference();putPointer(0x25ffe0,nullptr);putPointer(0x25ffdc,nullptr);
settings.set(String("game"),String("profiles"),previousProfiles);
require(views.enabled().empty() && controls.visible().empty() && input.controls(InputBindings::AllControls).empty() && nativeInput.controls(InputBindings::AllControls).empty(),"Startup retained UI/all-control input");
menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
loadingDocuments.clear();loadingFiles=nullptr;loadingSoundManager=nullptr;profileProperties().booleans.clear();startupSettings=nullptr;
