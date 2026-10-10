std::mt19937 sliderRandom(0x5036d0);
for(trial=0;trial<12;++trial) {
    stage=110;ViewRegistry views;ControlRegistry controls;
    auto parent=std::make_unique<MenuView>(nullptr,manager,settings,strings,views,controls);std::array<unsigned char,56> nativeParent{};
    function<void*(__thiscall*)(void*,void*)>(0xfb040)(nativeParent.data(),nullptr);parent->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0xfa8c0)(nativeParent.data(),wrap(&definition));
    auto slider=std::make_unique<Slider>(trial&2 ? parent.get():nullptr,manager,input,interaction,views,controls);auto* original=allocate(668);
    function<void*(__thiscall*)(void*,void*)>(0x102970)(original,trial&2 ? nativeParent.data():nullptr);
    XmlNode* xml=nullptr;require(definition.section(String(trial&1 ? "MusicVolume":"SoundVolume"),xml),"Missing shipped volume XML");
    slider->loadXML(*xml);function<void(__thiscall*)(void*,void*)>(0x103bf0)(original,wrap(xml));actualSlider=slider.get();nativeSlider=original;mutateSlider=false;
    slider->valueChanged=&sliderValue;slider->dragChanged=&sliderDrag;field<void*>(original,0x294)=reinterpret_cast<void*>(&nativeSliderValue);field<void*>(original,0x298)=reinterpret_cast<void*>(&nativeSliderDrag);actualSliderEvents.clear();nativeSliderEvents.clear();
    const auto check=[&] {
        stage=111;compareControl(*slider,original);compareControl(slider->thumb(),field<void*>(original,0x27c));
        require(slider->layout==field<unsigned>(original,0x280) && slider->dragging()==bool(field<unsigned char>(original,0x284)),"Slider layout/drag differs");
        compareFloat(slider->value(),field<float>(original,0x290),"Slider value");compareFloat(slider->maximum,field<float>(original,0x288),"Slider max");compareFloat(slider->minimum,field<float>(original,0x28c),"Slider min");
        for(const auto& pair:std::initializer_list<std::pair<Control*,void*>>{{slider.get(),original},{&slider->thumb(),field<void*>(original,0x27c)}}) {
            auto* button=static_cast<Button*>(pair.first);require(unsigned(button->buttonState())==field<unsigned>(pair.second,0x238),"Slider button state differs");pair.first->updateTransform();call(0xf0b70,pair.second);require(!std::memcmp(&pair.first->sprite(),static_cast<unsigned char*>(pair.second)+0x70,76),"Slider sprite differs");
        }
        require(actualSliderEvents==nativeSliderEvents,"Slider callback order/value differs");++sliderStates;
    };check();
    for(action=0;action<600;++action) {
        stage=112;cursorPosition={float(sliderRandom()%18000)/10.f-900,float(sliderRandom()%14000)/10.f-700};mutateSlider=action%17==0;
        switch(action%12) {
        case 0:slider->clicked();call(0x102dc0,original);break;
        case 1:slider->thumb().clicked();call(0x1021a0,field<void*>(original,0x27c));break;
        case 2:case 3:case 4:slider->enabledStep();call(0x1036d0,original);break;
        case 5:slider->thumb().unclicked();call(0x102590,field<void*>(original,0x27c));break;
        case 6:slider->unclicked();call(0x102170,original);break;
        case 7:parent->hide();call(0xd3720,nativeParent.data());break;
        case 8:parent->show();call(0xd37d0,nativeParent.data());break;
        case 9:slider->thumb().hide();call(0xd3720,field<void*>(original,0x27c));break;
        case 10:slider->thumb().show();call(0xd37d0,field<void*>(original,0x27c));break;
        case 11: {
            slider->layout=(action/12+trial)%2;field<unsigned>(original,0x280)=slider->layout;
            slider->minimum=float(sliderRandom()%4000)/13.f-100;slider->maximum=slider->minimum+float(sliderRandom()%2000+1)/7.f;
            field<float>(original,0x28c)=slider->minimum;field<float>(original,0x288)=slider->maximum;
            const float value=float(sliderRandom()%4000)/11.f-150;slider->setValue(value);field<float>(original,0x290)=value;
            const Point2 size{float(sliderRandom()%3000+1)/7.f,float(sliderRandom()%3000+1)/7.f};slider->setSize(size);function<void(__thiscall*)(void*,const Point2*)>(0x13b80)(original,&size);
            const Point2 position{float(sliderRandom()%8000)/11.f-100,float(sliderRandom()%6000)/11.f-100};slider->setPosition(position);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(original,&position);
            slider->updateThumb();call(0x1031a0,original);break;
        }
        }check();
    }
    if(trial<2) {
        mutateSlider=false;slider->layout=trial;field<unsigned>(original,0x280)=trial;slider->minimum=0;slider->maximum=100;slider->setValue(50);field<float>(original,0x28c)=0;field<float>(original,0x288)=100;field<float>(original,0x290)=50;
        const Point2 size=trial ? Point2{10,190}:Point2{190,10},position{480,276};slider->setSize(size);function<void(__thiscall*)(void*,const Point2*)>(0x13b80)(original,&size);slider->setPosition(position);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(original,&position);slider->updateThumb();call(0x1031a0,original);
        menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
        std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned frame=0;frame<16;++frame) {
            stage=113;action=frame;menuMilliseconds+=17;menuInputEvents.clear();menuMouseDelta={1,0,0};
            const auto& thumb=slider->thumb().geometry();menuPoint={thumb.x,thumb.y};if(frame%8==1) menuInputEvents={0x208};if(frame%8>=2 && frame%8<6) menuPoint=trial ? Point2{480,float(215+frame%8*20)}:Point2{float(410+frame%8*20),276};if(frame%8==6) menuInputEvents={0x200};if(frame%8==7) menuPoint={0,0};
            sliderPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++sliderFrames;check();
        }
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }
    stage=114;sliderCallbacks+=unsigned(actualSliderEvents.size());require(actualSounds==expectedSounds,"Slider sound sequence differs");actualSounds.clear();expectedSounds.clear();
    const auto before=textureCounts();slider.reset();const auto middle=textureCounts();call(0x1020c0,original);compareReferenceDeltas(before,middle);++sliderObjects;
    parent.reset();call(0xfafc0,nativeParent.data());require(input.mouse().empty() && nativeInput.mouse().empty(),"Slider destruction retained mouse bindings");
}
