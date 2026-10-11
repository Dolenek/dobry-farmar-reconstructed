// Complete owned cAwardTip and four cAnimable appearances, actual shipped XML.
XmlDocument definition(files,String("UI/AwardTip.xml"));
XmlDocument awardsDefinition(files,String("UI/Awards.xml"));XmlNode* awards=nullptr;
require(awardsDefinition.section(String("Awards"),awards),"Awards descriptions missing");
const auto directory=std::filesystem::absolute(std::filesystem::path(L".local")/L"tooltip-xml-oracle");std::filesystem::create_directories(directory);
{
    std::ofstream xml(directory/L"alternate.xml",std::ios::binary);
    xml<<"<AwardTip><Podlogka Picture=\"ui/gameUI/avard_tip_up_l.jng\" Offset=\"13.25 -9.75\" size=\"310.5 194.25\" position=\"150 160\" deep=\"3382\" Visible=\"1\" Enable=\"1\" frames_count=\"1\" PlayAnimation=\"1\">";
    constexpr const char* names[]={"UpLeftImage","UpRightImage","DownLeftImage","DownRightImage"};
    for(unsigned i=0;i<4;++i) xml<<'<'<<names[i]<<" Picture=\"ui/gameUI/avard_tip_"<<(i<2 ? "up_":"dn_")<<(i&1 ? "r":"l")<<".jng\" frames_count=\""<<i+2<<"\" anim_length=\""<<i+1<<"\" anim_loop=\""<<(i&1)<<"\" PlayAnimation=\"0\" anim_columns=\"2\" anim_rows=\"2\"/>";
    xml<<"</Podlogka></AwardTip>";require(bool(xml),"Tooltip fixture write failed");
    std::ofstream(directory/L"empty.xml")<<"<AwardTip/>";
}
FileIndex fixtureFiles(toNarrow(WString(directory.wstring())),String("pack"));XmlDocument alternate(fixtureFiles,String("alternate.xml")),empty(fixtureFiles,String("empty.xml"));
std::mt19937 random(0x4c95c0);menuMilliseconds=0;
GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
timerTable[3]=reinterpret_cast<void*>(&tooltipUnscaledTimer);graphicsTable[3]=reinterpret_cast<void*>(&tooltipHeight);graphicsTable[4]=reinterpret_cast<void*>(&tooltipWidth);
for(trial=0;trial<12;++trial) {
    stage=130;ViewRegistry views;ControlRegistry controls;tooltipDimensions={800,600};
    auto parent=std::make_unique<Picture>(nullptr,manager,views,controls);auto* nativeParent=allocate(568);function<void*(__thiscall*)(void*,void*)>(0xf16f0)(nativeParent,nullptr);
    auto tip=std::make_unique<TooltipFrame>(trial&1 ? parent.get():nullptr,manager,settings,strings,[] { return tooltipDimensions; },views,controls);
    auto* original=allocate(744);function<void*(__thiscall*)(void*,void*)>(0xc8d10)(original,trial&1 ? nativeParent:nullptr);
    compareControl(*tip,original);require(tip->fadeDirection==0 && !field<float>(original,0x244),"Tooltip initial fade differs");
    tip->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0xc8e80)(original,wrap(&definition));
    const auto check=[&] {
        stage=131;compareControl(*tip,original);compareFloat(tip->fadeDirection,field<float>(original,0x244),"Tooltip fade direction");
        require(!std::memcmp(tip->offset().data(),static_cast<unsigned char*>(original)+0x23c,8),"Tooltip offset differs");
        require(tip->animation().frames==field<unsigned>(original,0x34) && tip->animation().loop==bool(field<unsigned char>(original,0x3c)),"Tooltip assigned animation differs");
        compareFloat(tip->animation().length,field<float>(original,0x38),"Tooltip appearance animation length");
        require(tip->geometry().columns==field<unsigned>(original,0x40) && tip->geometry().rows==field<unsigned>(original,0x44),"Tooltip assigned atlas differs");
        require(tip->playing()==bool(field<unsigned char>(original,0x48)),"Tooltip cAnimable assignment changed playing");
        require(tip->dirty()==bool(field<unsigned char>(original,0x1b0)),"Tooltip dirty differs");
        tip->updateTransform();call(0xf0b70,original);require(!std::memcmp(&tip->sprite(),static_cast<unsigned char*>(original)+0x70,76),"Tooltip full sprite differs");
        auto* nativeText=field<void*>(original,0x238);tip->text().enabledStep();call(0xf8e10,nativeText);compareControl(tip->text(),nativeText);
        require(tip->text().string().view()==field<WString>(nativeText,0x23c).view(),"Tooltip description differs");
        auto* head=field<Link*>(nativeText,0x230);auto* node=head->next;require(tip->text().lines().size()==field<unsigned>(nativeText,0x234),"Tooltip multiline count differs");
        for(auto* line:tip->text().lines()) { require(node!=head,"Tooltip line ended early");compareControl(*line,node->control);tooltipLetters+=compare(*line,node->control);node=node->next; }require(node==head,"Tooltip line ended late");
        ++tooltipStates;
    };check();
    for(action=0;action<300;++action) {
        stage=132;
        switch(action%10) {
        case 0:case 1:case 2:case 3: {
            Point2 point{float(random()%9000)/11.f,float(random()%7000)/13.f};
            if(action%70==0) point={400,300};
            if(action%70==10) point={0,0};if(action%70==20) point={800,600};
            const auto before=textureCounts();tip->placeBeside(point);const auto middle=textureCounts();function<void(__thiscall*)(void*,const Point2*)>(0xc95c0)(original,&point);compareReferenceDeltas(before,middle);++tooltipPlacements;break;
        }
        case 4: {
            menuMilliseconds+=random()%30+1;frameTimer.setMultiplier(action%3 ? .7f:2.f);frameTimer.process();
            tip->fadeDirection=float(int(random()%9)-4)/3.f;field<float>(original,0x244)=tip->fadeDirection;
            tip->enabledStep();call(0xc9a80,original);break;
        }
        case 5: {
            XmlNode* award=nullptr;require(awards->section((action/10+trial)%21,award),"Award description index missing");WString value;
            XmlNode* description=nullptr;require(award->section(String("DescText"),description),"Award description text missing");value=description->text();tip->text().setString(value);function<void(__thiscall*)(void*,const WString*,bool)>(0xf8ce0)(field<void*>(original,0x238),&value,false);break;
        }
        case 6: {
            XmlNode* node=action%30==6 ? static_cast<XmlNode*>(&alternate):action%30==16 ? static_cast<XmlNode*>(&empty):static_cast<XmlNode*>(&definition);
            const auto before=textureCounts();tip->loadXML(*node);const auto middle=textureCounts();function<void(__thiscall*)(void*,void*)>(0xc8e80)(original,wrap(node));compareReferenceDeltas(before,middle);++tooltipReloads;break;
        }
        case 7:tip->show();call(0xd37d0,original);parent->show();call(0xd37d0,nativeParent);break;
        case 8:tip->hide();call(0xd3720,original);parent->hide();call(0xd3720,nativeParent);break;
        case 9: {
            const Point2 position{float(int(random()%16000)-4000)/11.f,float(int(random()%12000)-3000)/13.f};
            tip->setPosition(position);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(original,&position);
            tip->enabledStep();call(0xc9a80,original);break;
        }
        }check();
    }
    if(trial<4) {
        stage=133;tip->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0xc8e80)(original,wrap(&definition));
        parent->show();call(0xd37d0,nativeParent);tip->show();call(0xd37d0,original);tip->fadeDirection=1;field<float>(original,0x244)=1;
        GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
        std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned frame=0;frame<12;++frame) {
            stage=134;action=frame;menuMilliseconds+=25;menuInputEvents.clear();menuMouseDelta={0,0,0};menuPoint={0,0};
            const Point2 point{frame&1 ? 630.f:170.f,frame&2 ? 450.f:120.f};tip->placeBeside(point);function<void(__thiscall*)(void*,const Point2*)>(0xc95c0)(original,&point);
            tooltipPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++tooltipFrames;check();
        }
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }
    stage=135;const auto before=textureCounts();tip.reset();const auto middle=textureCounts();call(0xc8e00,original);compareReferenceDeltas(before,middle);++tooltipObjects;
    parent.reset();call(0xf1780,nativeParent);
}
