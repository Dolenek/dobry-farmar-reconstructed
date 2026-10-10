XmlDocument definition(files,String("UI/YesNoExit.xml"));exitDefinition=&definition;std::mt19937 motion(0x47ac90);
for(trial=0;trial<12;++trial) {
    stage=90;rtl=bool(trial&1);settings.set(String("game"),String("RightToLeftEnable"),rtl);
    ViewRegistry views;ControlRegistry controls;auto parent=std::make_unique<View>(nullptr,views);std::array<unsigned char,44> nativeParent{};
    function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeParent.data(),nullptr);parent->show();parent->enable();call(0xd37d0,nativeParent.data());call(0xd3bb0,nativeParent.data());
    auto dialog=std::make_unique<ExitDialog>(trial&2 ? parent.get():nullptr,manager,settings,strings,input,interaction,views,controls);auto* original=allocate(100);function<void*(__thiscall*)(void*,void*)>(0x7b1c0)(original,trial&2 ? nativeParent.data():nullptr);
    actualExitDialog=dialog.get();nativeExitDialog=original;actualExitEvents.clear();nativeExitEvents.clear();
    if(trial) { dialog->yesEvent=&exitYes;dialog->noEvent=&exitNo;field<void*>(original,0x54)=reinterpret_cast<void*>(&nativeExitYes);field<void*>(original,0x58)=reinterpret_cast<void*>(&nativeExitNo); }
    const auto check=[&] {
        stage=91;const auto& state=dialog->state();require(state.shown==bool(field<unsigned char>(original,0x24)) && state.parentShown==bool(field<unsigned char>(original,0x25)) && state.enabled==bool(field<unsigned char>(original,0x26)) && state.parentEnabled==bool(field<unsigned char>(original,0x27)),"Exit root activation differs");
        require(!std::memcmp(dialog->originalYesPosition().data(),static_cast<unsigned char*>(original)+0x5c,8),"Exit original Yes position differs");compareDecorations(*dialog,original);
        for(unsigned i=0;i<3;++i) compareControl(dialog->bee(i),field<void*>(original,0x3c+i*4));
        for(const auto& pair:std::initializer_list<std::pair<TextAnimationButton*,unsigned>>{{&dialog->yesButton(),0x48},{&dialog->noButton(),0x4c}}) {
            auto* native=field<void*>(original,pair.second);compareControl(*pair.first,native);require(unsigned(pair.first->buttonState())==field<unsigned>(native,0x238),"Exit button state differs");exitLetters+=compare(pair.first->text(),field<void*>(native,0x27c));
        }
        auto* text=field<void*>(original,0x50);dialog->text().enabledStep();call(0xf8e10,text);compareControl(dialog->text(),text);auto* head=field<Link*>(text,0x230);auto* node=head->next;
        require(dialog->text().lines().size()==field<unsigned>(text,0x234),"Exit text rows differ");for(auto* line:dialog->text().lines()) { require(node!=head,"Exit text rows ended early");compareControl(*line,node->control);exitLetters+=compare(*line,node->control);node=node->next; }require(node==head,"Exit text rows ended late");
        require(actualExitEvents==nativeExitEvents,"Exit callback order differs");++exitStates;
    };
    check();dialog->show();dialog->enable();call(0xd37d0,original);call(0xd3bb0,original);check();
    for(action=0;action<600;++action) {
        stage=92;const auto position=dialog->yesButton().geometry();cursorPosition=action%5==0 ? Point2{position.x,position.y}:action%5==1 ? Point2{position.x+20,position.y-35}:action%5==2 ? Point2{position.x+80,position.y}:Point2{float(motion()%10000)/10.f-100,float(motion()%8000)/10.f-100};
        elapsed=action%7==0 ? 0:float(motion()%2000)/1000;
        const auto state=ButtonState(action%4);dialog->yesButton().setState(state);function<void(__thiscall*)(void*,unsigned)>(0x1ed10)(field<void*>(original,0x48),unsigned(state));
        dialog->enabledStep();call(0x7ac90,original);
        if(action%11==0) { dialog->yesButton().hovered();call(0xf3a00,field<void*>(original,0x48)); }
        if(action%13==0) { dialog->noButton().hovered();call(0xf3a00,field<void*>(original,0x4c)); }
        if(action%17==0) { dialog->yesButton().unhovered();call(0xf3e00,field<void*>(original,0x48)); }
        if(action%19==0) { dialog->noButton().unhovered();call(0xf3e00,field<void*>(original,0x4c)); }
        if(action%23==0) { dialog->yesButton().clickEvent(&dialog->yesButton());reinterpret_cast<void(__cdecl*)(void*)>(image+0x79d40)(field<void*>(original,0x48)); }
        if(action%29==0) { dialog->noButton().clickEvent(&dialog->noButton());reinterpret_cast<void(__cdecl*)(void*)>(image+0x7a110)(field<void*>(original,0x4c)); }
        if(action%31==0) { dialog->show();dialog->enable();call(0xd37d0,original);call(0xd3bb0,original); }
        check();
    }
    if(trial==0 || trial==1) {
        dialog->show();dialog->enable();call(0xd37d0,original);call(0xd3bb0,original);
        menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
        std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned frame=0;frame<16;++frame) {
            stage=93;action=frame;menuMilliseconds+=17;menuInputEvents.clear();menuMouseDelta={};menuPoint={0,0};
            if(frame<8) { auto& button=frame<4 ? dialog->yesButton():dialog->noButton();if(frame%4!=3) menuPoint={button.geometry().x,button.geometry().y};if(frame%4==0 || frame%4==3) menuMouseDelta={1,0,0};if(frame%4==1) menuInputEvents={0x208};if(frame%4==2) menuInputEvents={0x200}; }
            if(frame>=8) { dialog->show();dialog->enable();call(0xd37d0,original);call(0xd3bb0,original);menuPoint={float(460+(frame-8)*8),float(420+(frame%3)*4)};menuMouseDelta={1,0,0}; }
            // The default handler hides this view after click; restore before
            // rendering the next input sample, preserving native callback state.
            if(frame==3 || frame==7) { dialog->show();dialog->enable();call(0xd37d0,original);call(0xd3bb0,original); }
            exitPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++exitFrames;check();
        }
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }
    exitCallbacks+=unsigned(actualExitEvents.size());require(actualSounds==expectedSounds,"Exit sound request order differs");actualSounds.clear();expectedSounds.clear();
    stage=94;const auto before=textureCounts();dialog.reset();const auto middle=textureCounts();call(0x79c70,original);compareReferenceDeltas(before,middle);++exitObjects;parent.reset();call(0xd58a0,nativeParent.data());
    require(input.mouse().empty() && nativeInput.mouse().empty(),"Exit destructor retained mouse bindings");
}
exitDefinition=nullptr;
