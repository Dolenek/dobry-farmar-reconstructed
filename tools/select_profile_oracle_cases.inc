// Included inside the existing D3D/UI oracle; all files stay under .local.
XmlDocument selectorXML(files,String("UI/SelectUser.xml")),deleteXML(files,String("UI/YesNo.xml"));
selectorDefinition=&selectorXML;dialogDefinition=&deleteXML;
redirect(0xecb20,reinterpret_cast<void*>(&selectorProfileConstruct));redirect(0xe8900,reinterpret_cast<void*>(&selectorProfileNames));redirect(0xe8d20,reinterpret_cast<void*>(&selectorProfileRemove));
redirect(0x2fe80,reinterpret_cast<void*>(&decimalString));redirect(0x1cc0,reinterpret_cast<void*>(&concatenateString));redirect(0x7d0b0,reinterpret_cast<void*>(&nativeDiscard));
wchar_t selectorModule[32768]{};require(GetModuleFileNameW(nullptr,selectorModule,32768)!=0,"Selector fixture module path missing");
const auto selectorRoot=(std::filesystem::path(selectorModule).parent_path()/L"../../.."/L".local/selector-profile-validation-work").lexically_normal();
require(selectorRoot.filename()==L"selector-profile-validation-work" && selectorRoot.parent_path().filename()==L".local","Selector cleanup target escaped diagnostic directory");
std::filesystem::remove_all(selectorRoot);std::filesystem::create_directories(selectorRoot);files.mountCommon(toNarrow(WString(selectorRoot.native())));
String selectorPreviousDirectory;settings.get(String("game"),String("profiles"),selectorPreviousDirectory);
for(trial=0;trial<16;++trial) {
    stage=70;rtl=bool(trial&2);settings.set(String("game"),String("RightToLeftEnable"),rtl);editLanguage=trial&2 ? 0x040d:0x0405;
    const String directory("SelectProfiles"+std::to_string(trial));settings.set(String("game"),String("profiles"),directory);
    FileIndex::createDirectories(String(std::string(toNarrow(WString(selectorRoot.native())).view())+"/"+std::string(directory.view())+"/"));
    ProfileSession profiles(files,settings,manager);
    const wchar_t* seedNames[]={L"Alpha",L"Zebra",L"\u017dofie"};
    const unsigned seedCount=trial%4==0 ? 0:trial%4==1 ? 1:3;
    for(unsigned i=0;i<seedCount;++i) profiles.open(WString(seedNames[i]));
    profiles.open(WString(seedCount ? seedNames[0]:L""));
    selectorNames=profiles.current()->headers().names();selectorNextID=profiles.current()->headers().nextID();selectorEvents.clear();actualSelectorCallbacks.clear();nativeSelectorCallbacks.clear();
    auto* nativeProfile=allocate(88);selectorProfileConstruct(nativeProfile,nullptr,&profiles.current()->headers().currentName());selectorEvents.clear();
    ViewRegistry views;ControlRegistry controls;
    auto selector=std::make_unique<SelectProfile>(bool(trial&1),true,profiles,manager,settings,strings,input,interaction,views,controls);
    auto* original=static_cast<unsigned char*>(allocate(196));function<void*(__thiscall*)(void*,bool,bool)>(0xfffb0)(original,bool(trial&1),true);auto* originalView=original+4;
    selector->accepted=&selectorAccepted;selector->cancelled=&selectorCancelled;field<void*>(original,0x68)=reinterpret_cast<void*>(&nativeSelectorAccepted);field<void*>(original,0x6c)=reinterpret_cast<void*>(&nativeSelectorCancelled);
    const auto check=[&] {
        stage=71;const auto& state=selector->state();require(state.shown==bool(field<unsigned char>(originalView,0x24)) && state.parentShown==bool(field<unsigned char>(originalView,0x25)) && state.enabled==bool(field<unsigned char>(originalView,0x26)) && state.parentEnabled==bool(field<unsigned char>(originalView,0x27)),"Selector root activation differs");
        require(selector->selectedIndex()==field<unsigned>(original,0x64),"Selector selected index differs");selectorLetters+=compareDecorations(*selector,originalView);
        for(const auto& pair:std::initializer_list<std::pair<TextAnimationButton*,unsigned>>{{&selector->okButton(),0x44},{&selector->exitButton(),0x48},{&selector->deleteButton(),0x4c}}) {
            auto* native=field<void*>(original,pair.second);compareControl(*pair.first,native);require(unsigned(pair.first->buttonState())==field<unsigned>(native,0x238),"Selector button state differs");selectorLetters+=compare(pair.first->text(),field<void*>(native,0x27c));
        }
        for(const auto& pair:std::initializer_list<std::pair<Control*,unsigned>>{{&selector->background(),0x50},{&selector->logo(),0x54},{&selector->nameBacking(),0x58},{&selector->nameEdit(),0x40},{&selector->enterNameText(),0x5c}}) compareControl(*pair.first,field<void*>(original,pair.second));
        selectorLetters+=compare(selector->nameEdit(),field<void*>(original,0x40));selectorLetters+=compare(selector->enterNameText(),field<void*>(original,0x5c));
        auto& list=selector->list();auto* nativeList=field<void*>(original,0x3c);compareControl(list,nativeList);auto** first=field<void**>(nativeList,0x23c);auto** end=field<void**>(nativeList,0x240);
        require(list.rows().size()==unsigned(end-first) && list.firstRow()==field<unsigned>(nativeList,0x268),"Selector list count/page differs");
        compareControl(const_cast<Button&>(list.up()),field<void*>(nativeList,0x248));compareControl(const_cast<Button&>(list.down()),field<void*>(nativeList,0x24c));
        for(unsigned i=0;i<list.rows().size();++i) { compareControl(*list.rows()[i],first[i]);selectorLetters+=compare(list.rows()[i]->text(),field<void*>(first[i],0x27c)); }
        auto* current=field<void*>(image,0x25f738);require(bool(profiles.current())==bool(current),"Selector profile singleton existence differs");
        if(current) {
            require(profiles.current()->headers().currentName().view()==field<WString>(current,0x24).view(),"Selector open profile name differs");
            const auto& names=profiles.current()->headers().names();require(names.size()==selectorNames.size() && profiles.current()->headers().nextID()==selectorNextID,"Selector persisted profile count/next ID differs");
            for(const auto& item:names) require(selectorNames.count(item.first) && selectorNames.at(item.first)==item.second,"Selector persisted profile names/IDs differ");
        }
        require(actualSelectorCallbacks==nativeSelectorCallbacks,"Selector accept/cancel callback order differs");
        for(unsigned control:{1U,0x1cU,0x39U}) require(input.controls(control).size()==nativeInput.controls(control).size(),"Selector keyboard binding count differs");
        require(input.keyboard().size()==nativeInput.keyboard().size(),"Selector character binding count differs");++selectorStates;
    };
    check();selector->show();selector->enable();call(0xd37d0,originalView);call(0xd3bb0,originalView);check();
    const auto setName=[&](const WString& name) {
        selector->nameEdit().setString(name);auto* edit=field<void*>(original,0x40);function<void(__thiscall*)(void*,const WString*)>(0xdd030)(edit,&name);
        selector->nameEdit().changed(&selector->nameEdit());reinterpret_cast<void(__cdecl*)(void*)>(image+0xfe9b0)(edit);
        selector->enabledStep();call(0xfe070,originalView);check();
    };
    for(action=0;action<30;++action) {
        stage=72;
        switch(action%6) {
        case 0:setName(WString(L""));break;
        case 1:setName(WString(L"New Farmer"));break;
        case 2:setName(WString(seedCount ? seedNames[action%seedCount]:L"Alpha"));break;
        case 3:
            input.dispatchCharacters({L'A',8,L'\u65e5',13});nativeInput.dispatchCharacters({L'A',8,L'\u65e5',13});selector->enabledStep();call(0xfe070,originalView);check();break;
        case 4:
            if(!selector->list().rows().empty()) { const unsigned index=(action/6)%selector->list().rows().size();selector->list().selected(index,&selector->list());reinterpret_cast<void(__cdecl*)(unsigned,void*)>(image+0xfeaa0)(index,field<void*>(original,0x3c)); }check();break;
        case 5:selector->disable();call(0xd3750,originalView);check();selector->enable();call(0xd3bb0,originalView);check();break;
        }
    }
    if(trial==0 || trial==2) {
        menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
        std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned frame=0;frame<12;++frame) {
            stage=73;action=frame;menuMilliseconds+=17;menuInputEvents.clear();menuMouseDelta={};menuPoint={0,0};
            if(frame==2) setName(WString(L"New Farmer"));if(frame==4 && seedCount) setName(WString(seedNames[0]));if(frame==6) { menuPoint={selector->okButton().geometry().x,selector->okButton().geometry().y};menuMouseDelta={1,0,0}; }
            if(frame==8) { setName(WString(L"")); }
            selectorPixels+=compareMenuFrame(d3d,device,window,views,controls,input);++selectorFrames;check();
        }
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }
    stage=74;input.dispatchControls({1});nativeInput.dispatchControls({1});check();selector->enable();call(0xd3bb0,originalView);check();
    if(seedCount) {
        setName(WString(seedNames[0]));
        const String removedID=formatProfileID(profiles.current()->headers().names().at(WString(seedNames[0])));
        const String removedPrefix(std::string(directory.view())+"/"+std::string(removedID.view()));
        for(const char* suffix:{".sav",".sav.bak",".pack"}) { auto output=files.create(String(std::string(removedPrefix.view())+suffix));const unsigned marker=0xaabbccdd;output->write(&marker,4); }
        for(unsigned confirm=0;confirm<2;++confirm) {
            stage=74;auto* dialog=selector->requestDelete();const auto previous=allocations;
            reinterpret_cast<void(__cdecl*)(void*)>(image+0xffb70)(nullptr);void* nativeDialogObject=nullptr;
            for(auto* candidate:allocations) if(!previous.count(candidate) && field<void*>(candidate,0)==image+0x1fd360) nativeDialogObject=candidate;
            require(nativeDialogObject!=nullptr,"Selector did not construct native YesNo dialog");check();
            if(trial==2) {
                menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
                std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
                for(unsigned frame=0;frame<4;++frame) { stage=74;menuMilliseconds+=17;menuInputEvents.clear();menuMouseDelta={};menuPoint=frame&1 ? Point2{dialog->yesButton().geometry().x,dialog->yesButton().geometry().y}:Point2{0,0};menuMouseDelta={1,0,0};selectorPixels+=compareMenuFrame(d3d,device,window,views,controls,input,60000);++selectorFrames;check(); }
                menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
            }
            input.dispatchControls({confirm ? 0x1cU:1U});nativeInput.dispatchControls({confirm ? 0x1cU:1U});
            require(views.discarded().size()==1 && nativeDiscards.size()==1 && views.discarded().at(dialog).view()==nativeDiscards.at(nativeDialogObject),"Selector dialog deletion origin differs");
            if(confirm) {
                ++selectorDeletions;require(!selector->exitButton().state().shown,"Native delete hides selector ExitButton");
                for(const char* suffix:{".xml",".sav",".sav.bak"}) require(!files.isExist(String(std::string(removedPrefix.view())+suffix)),"Selector confirmed deletion retained profile file");
                require(files.isExist(String(std::string(removedPrefix.view())+".pack")),"Selector deletion changed original pack-extension exclusion");
            }else for(const char* suffix:{".xml",".sav",".sav.bak",".pack"}) require(files.isExist(String(std::string(removedPrefix.view())+suffix)),"Selector cancelled deletion removed profile file");
            check();views.deleteDiscarded();call(0x101970,nativeDialogObject);nativeDiscards.clear();check();
        }
    }
    stage=75;setName(WString(L""));input.dispatchControls({0x1c});nativeInput.dispatchControls({0x1c});check();
    input.dispatchControls({1});nativeInput.dispatchControls({1});check();
    selector->enable();call(0xd3bb0,originalView);setName(WString(L"Created Profile"));input.dispatchControls({0x1c});nativeInput.dispatchControls({0x1c});check();
    require(profiles.current()->headers().currentName().view()==L"Created Profile","Selector Enter did not create selected profile");
    selectorCallbacks+=unsigned(actualSelectorCallbacks.size());selectorServiceEvents+=unsigned(selectorEvents.size());
    stage=76;const auto before=textureCounts();selector.reset();const auto middle=textureCounts();call(0xfe870,original);compareReferenceDeltas(before,middle);++selectorObjects;
    require(SelectProfile::instance()==nullptr && field<void*>(image,0x2604f0)==nullptr,"Selector destruction retained singleton");
    require(input.keyboard().empty() && nativeInput.keyboard().empty() && input.mouse().empty() && nativeInput.mouse().empty(),"Selector destruction retained input bindings");
    for(unsigned control:{1U,0x1cU,0x39U}) require(input.controls(control).empty() && nativeInput.controls(control).empty(),"Selector destruction retained key bindings");
    profiles.close();selectorProfileDestroy(field<void*>(image,0x25f738),nullptr,1);
    const auto saved=manager.loadSettingsXML(String(std::string(directory.view())+"/profiles.xml"));require(saved->attribute(WString(L"CurrentUser")).view()==L"Created Profile" && saved->attribute(WString(L"NowPlaying")).view()==L"0","Selector close did not persist selected header");
    const auto properties=manager.loadSettingsXML(String(std::string(directory.view())+"/"+std::string(formatProfileID(selectorNames.at(WString(L"Created Profile"))).view())+".xml"));require(!properties->children().empty(),"Selector created profile properties were not persisted");
    require(actualSounds==expectedSounds,"Selector sound requests differ");actualSounds.clear();expectedSounds.clear();
}
settings.set(String("game"),String("profiles"),selectorPreviousDirectory);selectorDefinition=nullptr;dialogDefinition=nullptr;
