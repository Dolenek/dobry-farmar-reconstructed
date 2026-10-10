XmlDocument definition(files,String("UI/Options.xml"));optionsDefinition=&definition;
auto menuDefinition=manager.loadSettingsXML(String("UI/MainMenu.xml"));graphicsTable[5]=reinterpret_cast<void*>(&optionsReset);
for(trial=0;trial<12;++trial) {
    stage=120;rtl=bool(trial&1);settings.set(String("game"),String("RightToLeftEnable"),rtl);
    nativeOptionsFull=bool(trial&2);settings.set(String("graphics"),String("fullscreen"),nativeOptionsFull);
    actualOptionsAudio={bool(trial&4),trial%3==0 ? 0U:trial%3==1 ? 73U:100U,trial%3==0 ? 100U:trial%3==1 ? 19U:0U,{}};nativeOptionsAudio=actualOptionsAudio;actualOptionsResets.clear();nativeOptionsResets.clear();
    OptionsServices services;
    services.soundDisabled=[] { actualOptionsAudio.events.emplace_back(0,actualOptionsAudio.disabled);return actualOptionsAudio.disabled; };
    services.soundVolume=[] { actualOptionsAudio.events.emplace_back(1,actualOptionsAudio.sound);return actualOptionsAudio.sound; };
    services.musicVolume=[] { actualOptionsAudio.events.emplace_back(2,actualOptionsAudio.music);return actualOptionsAudio.music; };
    services.setSoundVolume=[](unsigned value) { actualOptionsAudio.events.emplace_back(3,value);actualOptionsAudio.sound=value; };
    services.setMusicVolume=[](unsigned value) { actualOptionsAudio.events.emplace_back(4,value);actualOptionsAudio.music=value; };
    services.resetGraphics=[&](bool reset) { bool full=false;require(settings.get(String("graphics"),String("fullscreen"),full),"Source fullscreen setting missing");actualOptionsResets.emplace_back(reset,full); };
    ViewRegistry views;ControlRegistry controls;auto menu=std::make_unique<MenuView>(nullptr,manager,settings,strings,views,controls);std::array<unsigned char,56> nativeMenu{};
    function<void*(__thiscall*)(void*,void*)>(0xfb040)(nativeMenu.data(),nullptr);menu->loadXML(*menuDefinition);function<void(__thiscall*)(void*,void*)>(0xfa8c0)(nativeMenu.data(),wrap(menuDefinition.get()));menu->hide();menu->disable();call(0xd3720,nativeMenu.data());call(0xd3750,nativeMenu.data());
    auto options=std::make_unique<Options>(manager,settings,strings,input,interaction,services,views,controls);auto* original=static_cast<unsigned char*>(allocate(108));function<void*(__thiscall*)(void*)>(0x89770)(original);auto* nativeView=original+4;
    options->setReturnView(menu.get());function<void(__thiscall*)(void*,void*)>(0x87aa0)(original,nativeMenu.data());
    const auto compareView=[&](View& view,void* native) {
        const auto& state=view.state();compareFloat(state.transparency,field<float>(native,8),"Options View transparency");compareFloat(state.transparencySpeed,field<float>(native,12),"Options View speed");
        require(state.transition==field<unsigned>(native,16) && state.shown==bool(field<unsigned char>(native,0x24)) && state.parentShown==bool(field<unsigned char>(native,0x25)) && state.enabled==bool(field<unsigned char>(native,0x26)) && state.parentEnabled==bool(field<unsigned char>(native,0x27)),"Options View state differs");
    };
    const auto compareButton=[&](ButtonPrototype& button,void* native) { compareControl(button,native);require(unsigned(button.buttonState())==field<unsigned>(native,0x238),"Options Button state differs"); };
    const auto check=[&] {
        stage=121;compareView(*options,nativeView);compareView(*menu,nativeMenu.data());compareDecorations(*options,nativeView);compareDecorations(*menu,nativeMenu.data());
        for(unsigned i=0;i<3;++i) compareControl(options->picture(i),field<void*>(original,0x44+i*4));
        compareButton(options->okButton(),field<void*>(original,0x50));optionsLetters+=compare(options->okButton().text(),field<void*>(field<void*>(original,0x50),0x27c));
        auto* nativeCheck=field<void*>(original,0x54);compareButton(options->fullscreen(),nativeCheck);compareControl(options->fullscreen().tick(),field<void*>(nativeCheck,0x280));require(options->fullscreen().checked()==bool(field<unsigned char>(nativeCheck,0x284)),"Options checked value differs");
        for(const auto& pair:std::initializer_list<std::pair<Slider*,unsigned>>{{&options->soundSlider(),0x3c},{&options->musicSlider(),0x40}}) {
            auto* native=field<void*>(original,pair.second);compareButton(*pair.first,native);compareButton(pair.first->thumb(),field<void*>(native,0x27c));compareFloat(pair.first->value(),field<float>(native,0x290),"Options slider value");require(pair.first->dragging()==bool(field<unsigned char>(native,0x284)),"Options slider dragging differs");
        }
        for(unsigned i=0;i<4;++i) { compareControl(options->label(i),field<void*>(original,0x58+i*4));optionsLetters+=compare(options->label(i),field<void*>(original,0x58+i*4)); }
        bool full=false;settings.get(String("graphics"),String("fullscreen"),full);require(full==nativeOptionsFull,"Options persisted setting value differs");require(actualOptionsResets==nativeOptionsResets,"Options reset flags/order/setting differs");require(actualOptionsAudio.events==nativeOptionsAudio.events && actualOptionsAudio.sound==nativeOptionsAudio.sound && actualOptionsAudio.music==nativeOptionsAudio.music,"Options audio service sequence differs");++optionsStates;
    };check();
    for(action=0;action<60;++action) {
        stage=122;cursorPosition=action%2 ? Point2{514,350}:Point2{-100,-100};
        switch(action%6) {
        case 0:options->fullscreen().unclicked();call(0x104b80,field<void*>(original,0x54));break;
        case 1:options->fullscreen().enabledStep();call(0x104780,field<void*>(original,0x54));break;
        case 2:case 3: {
            const float values[]={.125f,50.875f,100.f,-2.9f,123.75f};const float value=values[(action/6)%5];auto& slider=action%6==2 ? options->soundSlider():options->musicSlider();slider.setValue(value);slider.valueChanged(&slider,value);
            const unsigned offset=action%6==2 ? 0x3c:0x40;field<float>(field<void*>(original,offset),0x290)=value;function<void(__cdecl*)(void*,float)>(action%6==2 ? 0x88770:0x88b40)(field<void*>(original,offset),value);break;
        }
        case 4:options->returnToMenu();reinterpret_cast<void(__cdecl*)(void*)>(image+0x88f10)(field<void*>(original,0x50));break;
        case 5:menu->hide();menu->disable();call(0xd3720,nativeMenu.data());call(0xd3750,nativeMenu.data());options->show();options->enable();call(0xd37d0,nativeView);call(0xd3bb0,nativeView);break;
        }check();
    }
    if(trial<2) {
        menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
        std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned frame=0;frame<24;++frame) {
            stage=123;action=frame;menuMilliseconds+=17;menuInputEvents.clear();menuMouseDelta={1,0,0};menuPoint={0,0};
            if(frame<8) { auto& slider=frame<4 ? options->soundSlider():options->musicSlider();const auto& thumb=slider.thumb().geometry();menuPoint={thumb.x,thumb.y};if(frame%4==0) menuInputEvents={0x208};if(frame%4==1) menuPoint={460,slider.geometry().y};if(frame%4==2) menuInputEvents={0x200};if(frame%4==3) menuPoint={0,0}; }
            if(frame>=8 && frame<12) { menuPoint={514,350};if(frame==9) menuInputEvents={0x208};if(frame==10) menuInputEvents={0x200};if(frame==11) menuPoint={0,0}; }
            if(frame>=12 && frame<16) { menuPoint={400,456};if(frame==13) menuInputEvents={0x208};if(frame==14) menuInputEvents={0x200};if(frame==15) menuPoint={0,0}; }
            optionsPixels+=compareMenuFrame(d3d,device,window,views,controls,input,100000);++optionsFrames;check();
        }
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }
    stage=124;optionsEvents+=unsigned(actualOptionsAudio.events.size());optionsResets+=unsigned(actualOptionsResets.size());require(actualSounds==expectedSounds,"Options sound request sequence differs");actualSounds.clear();expectedSounds.clear();
    const auto before=textureCounts();options.reset();const auto middle=textureCounts();call(0x87a30,original);compareReferenceDeltas(before,middle);++optionsObjects;require(!Options::instance() && !field<void*>(image,0x26000c),"Options singleton retained after deletion");
    menu.reset();call(0xfafc0,nativeMenu.data());require(input.mouse().empty() && nativeInput.mouse().empty(),"Options destruction retained input bindings");
}
optionsDefinition=nullptr;graphicsTable[5]=nullptr;
