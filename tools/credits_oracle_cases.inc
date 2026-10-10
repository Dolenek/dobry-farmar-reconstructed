// Shipped Credits tree plus a genuine recovered MenuView return target. The
// main-menu controller's additional hooks are outside this explicit boundary.
XmlDocument definition(files,String("UI/Credits.xml"));creditsDefinition=&definition;
auto menuDefinition=manager.loadSettingsXML(String("UI/MainMenu.xml"));
for(trial=0;trial<8;++trial) {
    stage=80;rtl=bool(trial&1);settings.set(String("game"),String("RightToLeftEnable"),rtl);
    ViewRegistry views;ControlRegistry controls;auto main=std::make_unique<MenuView>(nullptr,manager,settings,strings,views,controls);
    std::array<unsigned char,60> nativeMain{};auto* nativeMainView=nativeMain.data()+4;function<void*(__thiscall*)(void*,void*)>(0xfb040)(nativeMainView,nullptr);putPointer(0x25ffd0,nativeMain.data());
    main->loadXML(*menuDefinition);function<void(__thiscall*)(void*,void*)>(0xfa8c0)(nativeMainView,wrap(menuDefinition.get()));
    main->hide();main->disable();call(0xd3720,nativeMainView);call(0xd3750,nativeMainView);
    auto credits=std::make_unique<Credits>(*main,manager,settings,strings,input,interaction,views,controls);auto* original=static_cast<unsigned char*>(allocate(64));function<void*(__thiscall*)(void*)>(0x79b30)(original);auto* nativeView=original+4;
    const auto compareView=[&](View& view,void* native) {
        const auto& state=view.state();compareFloat(state.transparency,field<float>(native,8),"Credits View transparency");compareFloat(state.transparencySpeed,field<float>(native,12),"Credits View speed");
        require(state.transition==field<unsigned>(native,16) && state.shown==bool(field<unsigned char>(native,0x24)) && state.parentShown==bool(field<unsigned char>(native,0x25)) && state.enabled==bool(field<unsigned char>(native,0x26)) && state.parentEnabled==bool(field<unsigned char>(native,0x27)),"Credits View state differs");
    };
    const auto check=[&] {
        stage=81;compareView(*credits,nativeView);compareView(*main,nativeMainView);creditsDecorations+=compareDecorations(*credits,nativeView);
        compareControl(credits->returnButton(),field<void*>(original,0x3c));require(unsigned(credits->returnButton().buttonState())==field<unsigned>(field<void*>(original,0x3c),0x238),"Credits ReturnButton state differs");compare(credits->returnButton().text(),field<void*>(field<void*>(original,0x3c),0x27c));++creditsStates;
    };
    check();
    std::vector<Sound*> overwrittenSounds;
    if(trial&2) {
        require(sounds.size()>=4,"Credits diagnostic sound fixture count differs");for(unsigned i=unsigned(sounds.size())-4;i<sounds.size();++i) overwrittenSounds.push_back(sounds[i].get());
        credits->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0x795b0)(nativeView,wrap(&definition));check();
    }
    credits->showTransparently();call(0xd4590,nativeView);check();
    for(action=0;action<20;++action) {
        stage=82;
        switch(action%4) {
        case 0:credits->hide();call(0xd3720,nativeView);break;
        case 1:credits->show();call(0xd37d0,nativeView);break;
        case 2:credits->disable();call(0xd3750,nativeView);break;
        case 3:credits->enable();call(0xd3bb0,nativeView);break;
        }check();
    }
    if(trial==0 || trial==1) {
        menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
        std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned frame=0;frame<10;++frame) {
            stage=83;action=frame;menuMilliseconds+=17;menuInputEvents.clear();menuMouseDelta={};menuPoint={0,0};
            if(frame>=6 && frame<=8) menuPoint={credits->returnButton().geometry().x,credits->returnButton().geometry().y};
            if(frame==6 || frame==9) menuMouseDelta={1,0,0};if(frame==7) menuInputEvents={0x208};if(frame==8) menuInputEvents={0x200};
            creditsPixels+=compareMenuFrame(d3d,device,window,views,controls,input);++creditsFrames;check();
        }
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }else { credits->returnToMenu();reinterpret_cast<void(__cdecl*)()>(image+0x79110)();check(); }
    require(credits->state().transition==3 && main->state().transition==1 && main->state().enabled,"Credits return did not restore main menu");
    stage=84;const auto before=textureCounts();credits.reset();const auto middle=textureCounts();call(0x78e90,original);compareReferenceDeltas(before,middle);++creditsObjects;
    require(Credits::instance()==nullptr && field<void*>(image,0x25ffd4)==nullptr,"Credits destructor retained singleton");
    const auto menuBefore=textureCounts();main.reset();const auto menuMiddle=textureCounts();call(0xfafc0,nativeMainView);compareReferenceDeltas(menuBefore,menuMiddle);putPointer(0x25ffd0,nullptr);
    require(input.mouse().empty() && nativeInput.mouse().empty(),"Credits destructor retained mouse bindings");
    require(actualSounds==expectedSounds,"Credits sound requests differ");actualSounds.clear();expectedSounds.clear();
    for(auto* sound:overwrittenSounds) { require(sound->referenceCount()==1,"Credits XML sound overwrite does not match original retained reference");++creditsSoundResidues;sound->releaseReference(); } // Diagnostic cleanup only, after confirming both sides leak identically.
}
creditsDefinition=nullptr;
