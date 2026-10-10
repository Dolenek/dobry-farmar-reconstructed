XmlDocument definition(files,String("UI/Options.xml"));XmlNode* checkDefinition=nullptr;
require(definition.section(String("FullScreen"),checkDefinition),"Missing shipped FullScreen XML");
for(trial=0;trial<12;++trial) {
    stage=100;ViewRegistry views;ControlRegistry controls;
    auto parent=std::make_unique<MenuView>(nullptr,manager,settings,strings,views,controls);std::array<unsigned char,56> nativeParent{};
    function<void*(__thiscall*)(void*,void*)>(0xfb040)(nativeParent.data(),nullptr);
    parent->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0xfa8c0)(nativeParent.data(),wrap(&definition));
    auto box=std::make_unique<CheckBox>(trial&2 ? parent.get():nullptr,manager,input,interaction,views,controls);
    auto* original=allocate(648);function<void*(__thiscall*)(void*,void*)>(0x104180)(original,trial&2 ? nativeParent.data():nullptr);
    actualCheckBox=box.get();nativeCheckBox=original;actualCheckEvents.clear();nativeCheckEvents.clear();
    box->setChecked(bool(trial&1));field<unsigned char>(original,0x284)=bool(trial&1);
    box->loadXML(*checkDefinition);function<void(__thiscall*)(void*,void*)>(0x1042a0)(original,wrap(checkDefinition));
    box->clickEvent=&checkClicked;field<void*>(original,0x23c)=reinterpret_cast<void*>(&nativeCheckClicked);
    box->changed=&checkChanged;field<void*>(original,0x27c)=reinterpret_cast<void*>(&nativeCheckChanged);
    const auto check=[&] {
        stage=101;compareControl(*box,original);compareControl(box->tick(),field<void*>(original,0x280));
        require(box->checked()==bool(field<unsigned char>(original,0x284)),"Checkbox value differs");
        require(unsigned(box->buttonState())==field<unsigned>(original,0x238),"Checkbox button state differs");
        for(const auto& pair:std::initializer_list<std::pair<Control*,void*>>{{box.get(),original},{&box->tick(),field<void*>(original,0x280)}}) {
            pair.first->updateTransform();call(0xf0b70,pair.second);require(!std::memcmp(&pair.first->sprite(),static_cast<unsigned char*>(pair.second)+0x70,76),"Checkbox sprite differs");
        }
        require(actualCheckEvents==nativeCheckEvents,"Checkbox callback sequence differs");++checkStates;
    };check();
    for(action=0;action<120;++action) {
        stage=102;cursorPosition=action%2 ? Point2{514,350}:Point2{-100,-100};
        switch(action%10) {
        case 0:box->setChecked(!box->checked());field<unsigned char>(original,0x284)=!field<unsigned char>(original,0x284);break;
        case 1:box->enabledStep();call(0x104780,original);break;
        case 2:box->clicked();call(0xf41f0,original);break;
        case 3:box->unclicked();call(0x104b80,original);break;
        case 4:parent->hide();call(0xd3720,nativeParent.data());break;
        case 5:box->enabledStep();call(0x104780,original);break;
        case 6:parent->show();call(0xd37d0,nativeParent.data());break;
        case 7:box->disable();call(0xd3750,original);break;
        case 8:box->enable();call(0xd3bb0,original);break;
        case 9:box->unclicked();call(0x104b80,original);box->enabledStep();call(0x104780,original);break;
        }check();
    }
    if(trial<2) {
        menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
        std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned frame=0;frame<16;++frame) {
            stage=103;action=frame;menuMilliseconds+=17;menuInputEvents.clear();menuMouseDelta={};menuPoint={0,0};
            if(frame%4!=3) menuPoint={514,350};if(frame%4==0 || frame%4==3) menuMouseDelta={1,0,0};if(frame%4==1) menuInputEvents={0x208};if(frame%4==2) menuInputEvents={0x200};
            checkPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++checkFrames;check();
        }
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }
    stage=104;checkCallbacks+=unsigned(actualCheckEvents.size());require(actualSounds==expectedSounds,"Checkbox sound sequence differs");actualSounds.clear();expectedSounds.clear();
    const auto before=textureCounts();box.reset();const auto middle=textureCounts();call(0x104210,original);compareReferenceDeltas(before,middle);++checkObjects;
    parent.reset();call(0xfafc0,nativeParent.data());require(input.mouse().empty() && nativeInput.mouse().empty(),"Checkbox destruction retained mouse bindings");
}
