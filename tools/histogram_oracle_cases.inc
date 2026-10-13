// Real Market histogram plus reloads and owned Lid controls.
XmlDocument marketXml(files,String("UI/gameUI/Market.xml"));XmlNode *line=nullptr,*shipped=nullptr;
require(marketXml.section(String("ProductLine"),line) && line->section(String("PriceGrowHistogram"),shipped),"Market histogram XML missing");
const auto directory=std::filesystem::path(L".local")/L"histogram-xml-oracle";std::filesystem::create_directories(directory);
for(unsigned count: {0U,1U,3U,17U}) {
    std::ofstream out(directory/(std::to_wstring(count)+L".xml"),std::ios::binary);
    out<<"<Histogram position=\"-19.125 49.375\" Step=\"7.125 -2.875\" BarCount=\""<<count<<"\" Visible=\"1\" Enable=\"1\" Threshold=\"13.75\"><Bar position=\"5.375 -9.125\" size=\"4 33\" deep=\"-999\" Picture=\"UI/GameUI/Market/gistogram.jng\" Visible=\"1\" Enable=\"0\" Side=\"2\" ProgressMaxValue=\"1\" ProgressCurrentValue=\"0\"><Lid Picture=\"UI/GameUI/Market/gistogram.jng\" size=\"3 3\" deep=\"-998\" Visible=\"1\"/></Bar></Histogram>";
    require(bool(out),"Histogram fixture write failed");
}
FileIndex fixtureFiles(toNarrow(WString(std::filesystem::absolute(directory).wstring())),String("pack"));
std::array<std::unique_ptr<XmlDocument>,4> definitions;
const unsigned counts[]{0,1,3,17};for(unsigned i=0;i<4;++i) definitions[i]=std::make_unique<XmlDocument>(fixtureFiles,String(std::to_string(counts[i])+".xml"));
std::mt19937 histogramRandom(0x46b730);
for(trial=0;trial<4;++trial) {
    stage=210;ViewRegistry views;ControlRegistry controls;
    auto histogram=std::make_unique<Histogram>(nullptr,manager,views,controls);auto* original=allocate(576);
    function<void*(__thiscall*)(void*,void*)>(0x6d420)(original,nullptr);
    const auto check=[&] {
        stage=211;compareControl(*histogram,original);compareFloat(histogram->threshold,field<float>(original,0x22c),"Histogram threshold");
        auto** begin=field<void**>(original,0x234);auto** end=field<void**>(original,0x238);
        const auto count=begin ? std::size_t(end-begin):0;require(count==histogram->bars().size(),"Histogram bar count differs");
        const auto* head=field<Node*>(original,0x18);std::vector<void*> children;collectControls(head->parent,children);
        require(children.size()==histogram->children().size() && children.size()==count,"Histogram reload retained deleted bars");
        for(std::size_t i=0;i<count;++i) {
            auto& bar=*histogram->bars()[i];void* native=begin[i];compareControl(bar,native);
            compareFloat(bar.maximum,field<float>(native,0x238),"Histogram bar maximum");compareFloat(bar.current,field<float>(native,0x23c),"Histogram bar current");
            require(bar.dirty()==bool(field<unsigned char>(native,0x1b0)),"Histogram bar dirty differs");
            bar.updateTransform();call(0xf9290,native);
            require(!std::memcmp(&bar.sprite(),static_cast<unsigned char*>(native)+0x70,76),"Histogram bar sprite differs");
            compareFloat(bar.current,field<float>(native,0x23c),"Histogram clamped bar current");
            require(bool(bar.lid())==bool(field<void*>(native,0x244)),"Histogram bar lid presence differs");
            if(bar.lid()) {auto* lid=field<void*>(native,0x244);compareControl(*bar.lid(),lid);bar.lid()->updateTransform();call(0xf0b70,lid);require(!std::memcmp(&bar.lid()->sprite(),static_cast<unsigned char*>(lid)+0x70,76),"Histogram lid sprite differs");}
        }++histogramStates;
    };check();
    for(unsigned reload=0;reload<5;++reload) {
        stage=212;XmlNode* xml=reload ? static_cast<XmlNode*>(definitions[(reload+trial-1)%4].get()):shipped;
        histogram->loadXML(*xml);function<void(__thiscall*)(void*,void*)>(0x6d560)(original,wrap(xml));++histogramReloads;check();
        for(action=0;action<120;++action) {
            stage=213;std::vector<float> values((action+trial)%24);
            for(auto& value:values) value=float(int(histogramRandom()%4000)-1000)/17.f;
            if(action%20==0) values.clear();
            if(action%20==1) values.assign(30,-1);
            if(action%20==2) values.assign(30,0);
            struct NativeFloats {unsigned allocator=0;float *begin,*end,*capacity;} nativeValues{0,values.data(),values.data()+values.size(),values.data()+values.size()};
            histogram->applyValues(values);function<void(__thiscall*)(void*,void*)>(0x6b730)(original,&nativeValues);check();
            if(action%10==0) {const float maximum=float(int(histogramRandom()%1000)-100)/9.f;histogram->setMaximum(maximum);function<void(__thiscall*)(void*,float)>(0x6b6f0)(original,maximum);check();}
        }
        if(reload==0) {
            histogram->show();call(0xd37d0,original);
            menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
            std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
            for(unsigned frame=0;frame<8;++frame) {
                stage=214;menuMilliseconds+=17;menuInputEvents.clear();menuMouseDelta={0,0,0};menuPoint={0,0};
                std::vector<float> values(10);for(unsigned i=0;i<10;++i) values[i]=float((i+frame)*13%100);
                struct NativeFloats {unsigned allocator;float *begin,*end,*capacity;} nativeValues{0,values.data(),values.data()+10,values.data()+10};
                histogram->applyValues(values);function<void(__thiscall*)(void*,void*)>(0x6b730)(original,&nativeValues);histogram->setMaximum(100);function<void(__thiscall*)(void*,float)>(0x6b6f0)(original,100);
                histogramPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++histogramFrames;check();
            }
            menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
        }
    }
    stage=215;const auto before=textureCounts();histogram.reset();const auto middle=textureCounts();call(0x6d4c0,original);compareReferenceDeltas(before,middle);++histogramObjects;
}
