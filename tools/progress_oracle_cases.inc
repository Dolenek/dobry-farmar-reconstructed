// Actual shipped Loading screen, plus explicit Lid/missing-Side XML cases.
XmlDocument definition(files,String("UI/Load.xml"));XmlNode* shippedProgress=nullptr;
require(definition.section(String("Progress"),shippedProgress),"Missing shipped Progress XML");
const auto fixtureDirectory=std::filesystem::path(L".local")/L"progress-xml-oracle";
std::filesystem::create_directories(fixtureDirectory);
{
    std::ofstream output(fixtureDirectory/L"progress.xml",std::ios::binary);
    output<<"<Progress Picture=\"UI/load_progres.jng\" position=\"401 392\" size=\"336 42\" deep=\"2002\" Visible=\"1\" Enable=\"1\" ProgressMaxValue=\"81.25\" ProgressCurrentValue=\"19.375\"><Lid Picture=\"UI/load_progres.jng\" position=\"0 0\" size=\"24 16\" deep=\"2003\" Visible=\"1\" Enable=\"1\"/></Progress>";
    require(bool(output),"Progress fixture write failed");
}
FileIndex fixtureFiles(toNarrow(WString(std::filesystem::absolute(fixtureDirectory).wstring())),String("pack"));
XmlDocument lidDefinition(fixtureFiles,String("progress.xml"));
std::mt19937 progressRandom(0x4f9b20);
for(trial=0;trial<12;++trial) {
    stage=120;ViewRegistry views;ControlRegistry controls;
    auto parent=std::make_unique<MenuView>(nullptr,manager,settings,strings,views,controls);std::array<unsigned char,56> nativeParent{};
    function<void*(__thiscall*)(void*,void*)>(0xfb040)(nativeParent.data(),nullptr);
    parent->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0xfa8c0)(nativeParent.data(),wrap(&definition));
    auto bar=std::make_unique<ProgressBar>(trial&2 ? parent.get():nullptr,manager,views,controls);auto* original=allocate(584);
    function<void*(__thiscall*)(void*,void*)>(0xf91e0)(original,trial&2 ? nativeParent.data():nullptr);
    require(bar->maximum==100 && bar->current==0 && bar->side==2 && !bar->lid(),"Progress constructor defaults differ");
    compareFloat(bar->maximum,field<float>(original,0x238),"Progress initial maximum");compareFloat(bar->current,field<float>(original,0x23c),"Progress initial current");require(bar->side==field<unsigned>(original,0x240) && !field<void*>(original,0x244),"Progress native initial side/lid differs");
    auto* xml=trial&1 ? static_cast<XmlNode*>(&lidDefinition):shippedProgress;
    bar->loadXML(*xml);function<void(__thiscall*)(void*,void*)>(0xf9f80)(original,wrap(xml));
    const auto check=[&] {
        stage=121;compareControl(*bar,original);
        compareFloat(bar->maximum,field<float>(original,0x238),"Progress maximum");compareFloat(bar->current,field<float>(original,0x23c),"Progress current before transform");require(bar->side==field<unsigned>(original,0x240),"Progress side differs");
        require(bar->dirty()==bool(field<unsigned char>(original,0x1b0)),"Progress dirty state differs");
        bar->updateTransform();call(0xf9290,original);
        require(!std::memcmp(&bar->sprite(),static_cast<unsigned char*>(original)+0x70,76),"Progress full sprite differs");
        compareFloat(bar->current,field<float>(original,0x23c),"Progress clamped current");
        require(bool(bar->lid())==bool(field<void*>(original,0x244)),"Progress lid presence differs");
        if(bar->lid()) {
            auto* lid=field<void*>(original,0x244);compareControl(*bar->lid(),lid);bar->lid()->updateTransform();call(0xf0b70,lid);
            require(!std::memcmp(&bar->lid()->sprite(),static_cast<unsigned char*>(lid)+0x70,76),"Progress lid sprite differs");
        }
        const auto* childHead=field<Node*>(original,0x18);std::vector<void*> children;collectControls(childHead->parent,children);
        require(bar->children().size()==children.size(),"Progress retained child count differs");++progressStates;
    };check();
    require(bar->side==(trial&1 ? 0U:1U),"Progress missing Side did not overwrite constructor default");
    for(action=0;action<300;++action) {
        stage=122;
        switch(action%10) {
        case 0:case 1:case 2:case 3: {
            bar->side=(action/10+trial)%8;bar->maximum=float(progressRandom()%16000+1)/83.f;bar->current=float(int(progressRandom()%32000)-8000)/71.f;
            if(action%90==0) bar->maximum=0;
            if(action%90==10) bar->maximum=-bar->maximum;
            field<unsigned>(original,0x240)=bar->side;field<float>(original,0x238)=bar->maximum;field<float>(original,0x23c)=bar->current;
            bar->updateLid();call(0xf9b20,original);break;
        }
        case 4: {const Point2 position{float(progressRandom()%6000)/7.f,float(progressRandom()%4000)/7.f};bar->setPosition(position);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(original,&position);bar->updateLid();call(0xf9b20,original);break;}
        case 5: {float angle=float(int(progressRandom()%4000)-2000)/17.f;bar->setAngle(angle);function<void(__thiscall*)(void*,const float*)>(0x13b20)(original,&angle);bar->updateLid();call(0xf9b20,original);break;}
        case 6:parent->hide();call(0xd3720,nativeParent.data());break;
        case 7:parent->show();call(0xd37d0,nativeParent.data());break;
        case 8: {float alpha=float(progressRandom()%1001)/1000.f;bar->setAlpha(alpha);function<void(__thiscall*)(void*,const float*)>(0x13b50)(original,&alpha);bar->updateLid();call(0xf9b20,original);break;}
        case 9:bar->enabledStep();call(0xf1810,original);break;
        }check();
    }
    for(unsigned reload=0;reload<2;++reload) {
        stage=123;bar->loadXML(*xml);function<void(__thiscall*)(void*,void*)>(0xf9f80)(original,wrap(xml));check();++progressReloads;
    }
    if(trial<4) {
        const Point2 position{401,392};const float angle=0,alpha=1;bar->setPosition(position);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(original,&position);bar->setAngle(angle);function<void(__thiscall*)(void*,const float*)>(0x13b20)(original,&angle);bar->setAlpha(alpha);function<void(__thiscall*)(void*,const float*)>(0x13b50)(original,&alpha);
        menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
        std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned frame=0;frame<12;++frame) {
            stage=124;action=frame;menuMilliseconds+=17;menuInputEvents.clear();menuMouseDelta={0,0,0};menuPoint={0,0};
            bar->side=frame%4;bar->maximum=100;bar->current=float(frame*13)-8;field<unsigned>(original,0x240)=bar->side;field<float>(original,0x238)=100;field<float>(original,0x23c)=bar->current;
            bar->updateLid();call(0xf9b20,original);progressPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++progressFrames;check();
        }
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }
    stage=125;const auto before=textureCounts();bar.reset();const auto middle=textureCounts();call(0xf9230,original);compareReferenceDeltas(before,middle);++progressObjects;
    parent.reset();call(0xfafc0,nativeParent.data());
}
