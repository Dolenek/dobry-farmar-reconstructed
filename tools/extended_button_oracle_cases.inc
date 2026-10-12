using ExtendedButtonType=std::remove_pointer_t<decltype(tag)>;
constexpr bool hasText=std::is_same_v<ExtendedButtonType,ExtendedTextAnimationButton>;
constexpr bool hasPicture=std::is_same_v<ExtendedButtonType,ExtendedPictureButton>;
const auto offset=[hasText,hasPicture](unsigned value) {return !hasText && !hasPicture && value>=0x27c ? value-4:value;};
const auto nativeRva=[](unsigned text,unsigned animation) {
    if constexpr(std::is_same_v<ExtendedButtonType,ExtendedPictureButton>) switch(text) {
        case 0x28370:return 0x9dc20U;case 0x284c0:return 0x9a270U;case 0x2a1b0:return 0x9ddc0U;
        case 0x28a90:return 0x9a650U;case 0x28ff0:return 0x9bcc0U;case 0x293e0:return 0x9c0b0U;
        case 0x28590:return 0x9b7c0U;default:return text;
    }
    return std::is_same_v<ExtendedButtonType,ExtendedTextAnimationButton> ? text:animation;
};
stage=240;xmlTable[34]=reinterpret_cast<void*>(&sectionCount);
std::array<unsigned char,3104> tutorial{};field<int>(tutorial.data(),0xbfc)=-1;putPointer(0x25f73c,tutorial.data());
const auto directory=std::filesystem::path(".local")/(std::string("extended-button-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));std::filesystem::create_directories(directory);
std::ofstream(directory/"extended-button-fixture.xml")<<R"(<Root><Button position="401 301" Visible="1" Enable="1"><Tip Offset="17 23" WaitTime="0.125" TipsAlphaChangeTime="0.3"><Image position="3 4" size="120 140" deep="2100" PlayAnimation="1"/><UpLeftImage Picture="ui/gameUI/tips-l_l_u.jng" frames_count="3" anim_columns="2" anim_rows="2" anim_length="0.7"/><UpRightImage Picture="ui/gameUI/tips-l_r_u.jng"/><DownLeftImage Picture="ui/gameUI/tips-l_l_d.jng"/><DownRightImage Picture="ui/gameUI/tips-l_r_d.jng"/><Texts><T position="8 -9" deep="2101" Font="UI/Font/font_tips.fnt" Texture="UI/Font/font_tips.jng" Localized="0" String="Numbers 123"/></Texts><Pics><P position="-7 11" size="20 24" deep="2102" Picture="ui/gameUI/tips-l_l_u.jng"/></Pics></Tip></Button><Missing position="200 100"/></Root>)";
files.mountCommon(String(directory.generic_string()));
std::vector<std::pair<const char*,std::vector<const char*>>> definitions={
    {"extended-button-fixture.xml",{"Button","Missing"}},
    {"Farm/ActionAreas/DigBuilding.xml",{"UpgradeButton"}},
    {"Farm/ActionAreas/MowBuilding.xml",{"UpgradeButton"}},
    {"Farm/ActionAreas/WareHouse.xml",{"UpgradeButton"}},
    {"Farm/ActionAreas/WaterSource.xml",{"SprinkleButton","UpgradeButton"}},
    {"UI/gameUI/ContractStatus.xml",{"GoalButton"}},
    {"UI/gameUI/Farm.xml",{"SeedsBuyButton","FieldsBuyButton"}},
    {"UI/gameUI/GameMenu.xml",{"MainMenuButton","MarketButton","FarmButton","ProducingButton","JournalButton"}},
    {"UI/gameUI/Manufacture.xml",{"DestroyButton","BuildButton","UpgradeButton"}}
};
if constexpr(std::is_same_v<ExtendedButtonType,ExtendedPictureButton>) {definitions={};for(const char* path:{"Manufacture/SheepFold.xml","Manufacture/HenHouse.xml","Manufacture/CowShed.xml","Manufacture/PigPen.xml","Manufacture/CheeseDairy.xml","Manufacture/Creamery.xml","Manufacture/Weavery.xml","Manufacture/Incubator.xml","Manufacture/TextileDep.xml","Manufacture/Bakery.xml","Manufacture/Mill.xml"}) definitions.push_back({path,{""}});}
for(const auto& entry:definitions) {
    XmlDocument document(files,String(entry.first));
    for(const auto* name:entry.second) {
        stage=241;trial=extendedObjects;XmlNode* definition=nullptr;if constexpr(std::is_same_v<ExtendedButtonType,ExtendedPictureButton>) definition=&document;else require(document.section(String(name),definition),"Extended button definition missing");
        ViewRegistry views;ControlRegistry controls;interaction.active=nullptr;interaction.pending=0;putPointer(0x2604dc,nullptr);field<unsigned>(image,0x2604e0)=0;
        cursorPosition={790,590};field<int>(tutorial.data(),0xbfc)=-1;
        auto button=std::make_unique<ExtendedButtonType>(nullptr,manager,settings,strings,input,interaction,[&] {return field<int>(tutorial.data(),0xbfc)!=-1;},views,controls);
        auto* original=allocate(hasText || hasPicture ? 868:864);function<void*(__thiscall*)(void*,void*)>(nativeRva(0x28370,0x23550))(original,nullptr);
        const auto firstSound=sounds.size();
        button->loadXML(*definition);function<void(__thiscall*)(void*,void*)>(nativeRva(0x2a1b0,0x236f0))(original,wrap(definition));
        const auto compareLine=[&](TextLine& text,void* native) {
            if(text.textDirty()) text.makeLetters();compareControl(text,native);extendedLetters+=compare(text,native);
        };
        const auto check=[&] {
            stage=242;compareControl(*button,original);if constexpr(std::is_same_v<ExtendedButtonType,ExtendedTextAnimationButton>) compareLine(button->text(),field<void*>(original,0x27c));
            if constexpr(std::is_same_v<ExtendedButtonType,ExtendedPictureButton>) compareControl(button->overlay(),field<void*>(original,0x27c));
            require(unsigned(button->buttonState())==field<unsigned>(original,0x238),"Extended button state differs");
            require(button->tipsDisabled==bool(field<unsigned char>(original,offset(0x280))) && button->tipsDirection==field<int>(original,offset(0x2b4)),"Extended tip direction differs");
            compareFloat(button->waitTime,field<float>(original,offset(0x2a4)),"Tip wait time");compareFloat(button->waitRemaining,field<float>(original,offset(0x2a8)),"Tip remaining time");
            compareFloat(button->alphaChangeTime,field<float>(original,offset(0x2ac)),"Tip fade time");compareFloat(button->tipsAlpha,field<float>(original,offset(0x2b0)),"Tip alpha");
            require(!std::memcmp(button->tipOffset.data(),static_cast<unsigned char*>(original)+offset(0x2b8),8),"Tip offset differs");
            auto** begin=field<void**>(original,offset(0x288));auto** end=field<void**>(original,offset(0x28c));
            require(button->tips().size()==std::size_t(end-begin),"Tip list size differs");
            auto* layouts=field<unsigned char*>(original,offset(0x298));require(button->tipLayouts().size()==std::size_t(field<unsigned char*>(original,offset(0x29c))-layouts)/12,"Tip layout count differs");
            for(std::size_t i=0;i<button->tipLayouts().size();++i) require(!std::memcmp(&button->tipLayouts()[i],layouts+i*12,12),"Tip saved layout differs");
            for(unsigned i=0;i<4;++i) {
                const auto& appearance=button->tipAppearances()[i];auto* native=static_cast<unsigned char*>(original)+offset(0x2c4)+i*40;
                require(appearance.texture==field<cTexture*>(native,4) && appearance.frames==field<unsigned>(native,8) && appearance.columns==field<unsigned>(native,20) && appearance.rows==field<unsigned>(native,24),"Tip appearance differs");
                compareFloat(appearance.length,field<float>(native,12),"Tip appearance duration");require(appearance.loop==bool(field<unsigned char>(native,16)) && appearance.playing==bool(field<unsigned char>(native,28)),"Tip appearance flags differ");
            }
            for(std::size_t i=0;i<button->tips().size();++i) {
                auto* control=button->tips()[i];auto* native=begin[i];compareControl(*control,native);
                if(auto* line=dynamic_cast<TextLine*>(control)) compareLine(*line,native);
                else if(auto* text=dynamic_cast<MultilineText*>(control)) {
                    text->enabledStep();call(0xf8e10,native);require(text->string().view()==field<WString>(native,0x23c).view(),"Tip multiline text differs");
                    auto* head=field<Link*>(native,0x230);auto* node=head->next;require(text->lines().size()==field<unsigned>(native,0x234),"Tip multiline line count differs");
                    for(auto* line:text->lines()) {require(node!=head,"Tip lines ended early");compareLine(*line,node->control);node=node->next;}require(node==head,"Tip lines ended late");
                } else {
                    control->updateTransform();call(0xf0b70,native);require(!std::memcmp(&control->sprite(),static_cast<unsigned char*>(native)+0x70,76),"Tip sprite differs");
                }
            }
            require(actualSounds==expectedSounds,"Extended button sound requests differ");++extendedStates;
        };check();
        for(action=0;action<3;++action) {
            const auto before=textureCounts();button->loadXML(*definition);const auto middle=textureCounts();function<void(__thiscall*)(void*,void*)>(nativeRva(0x2a1b0,0x236f0))(original,wrap(definition));compareReferenceDeltas(before,middle);check();++extendedReloads;
        }
        // Exercise every screen quadrant including the exact native split.
        for(Point2 position:{Point2{400,300},Point2{401,300},Point2{400,301},Point2{401,301},Point2{-50,-10},Point2{850,650}}) {
            stage=243;button->setPosition(position);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(original,&position);
            button->positionTips();call(nativeRva(0x28a90,0x227b0),original);check();
        }
        const Point2 center{400,300};button->setPosition(center);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(original,&center);cursorPosition=center;
        actualSounds.clear();expectedSounds.clear();button->enable();call(0xd3bb0,original);button->show();call(0xd37d0,original);check();
        for(action=0;action<96;++action) {
            stage=244;actualSounds.clear();expectedSounds.clear();field<int>(tutorial.data(),0xbfc)=action%8==5 ? 0:-1;
            switch(action%8) {
            case 0:button->hovered();call(nativeRva(0x28ff0,0x22cc0),original);break;
            case 1:button->unhovered();call(nativeRva(0x293e0,0x21800),original);break;
            case 2:button->clicked();call(nativeRva(0x99ca0,0x215e0),original);break;
            case 3:button->unclicked();call(nativeRva(0x99cc0,0x8fb30),original);break;
            case 4:button->disable();call(0xd3750,original);break;
            case 5:button->enable();call(0xd3bb0,original);break;
            case 6:button->tipsDisabled=true;field<unsigned char>(original,offset(0x280))=1;button->hovered();call(nativeRva(0x28ff0,0x22cc0),original);break;
            case 7:button->hideTips();call(nativeRva(0x9a340,0x8fb50),original);break;
            }
            elapsed=(action%4)*0.025f;button->enabledStep();call(nativeRva(0x28590,0x222b0),original);++extendedSteps;check();
        }
        // Deterministic fade/wait boundary states, nonstandard directions,
        // zero durations and NaN; compare actual native x87 arithmetic.
        std::mt19937 random(0x428590+trial);
        for(action=0;action<2048;++action) {
            stage=245;actualSounds.clear();expectedSounds.clear();field<int>(tutorial.data(),0xbfc)=action%17==0 ? 4:-1;
            button->tipsAlpha=float(int(random()%400)-150)*0.01f;button->tipsDirection=int(random()%4)-1;
            button->alphaChangeTime=action%31==0 ? 0:float(random()%199+1)*0.01f;
            button->waitRemaining=action%3==0 ? 0:float(int(random()%100)-50)*0.01f;
            elapsed=float(random()%200)*0.01f;
            const unsigned nanBits=0x7fc00000;float nan;std::memcpy(&nan,&nanBits,4);
            if(action%101==0) button->tipsAlpha=nan;
            if(action%103==0) button->waitRemaining=nan;
            if(action%107==0) button->alphaChangeTime=nan;
            field<float>(original,offset(0x2b0))=button->tipsAlpha;field<int>(original,offset(0x2b4))=button->tipsDirection;
            field<float>(original,offset(0x2ac))=button->alphaChangeTime;field<float>(original,offset(0x2a8))=button->waitRemaining;
            button->enabledStep();call(nativeRva(0x28590,0x222b0),original);++extendedSteps;check();
        }
        field<int>(tutorial.data(),0xbfc)=-1;button->alphaChangeTime=0.1f;field<float>(original,offset(0x2ac))=0.1f;
        button->waitRemaining=0;field<float>(original,offset(0x2a8))=0;button->tipsDirection=0;field<int>(original,offset(0x2b4))=0;button->tipsAlpha=1;field<float>(original,offset(0x2b0))=1;
        elapsed=0;button->positionTips();call(nativeRva(0x28a90,0x227b0),original);button->show();call(0xd37d0,original);button->enable();call(0xd3bb0,original);
        menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
        GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
        std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(action=0;action<4;++action) {
            stage=246;elapsed=action==0 ? 2.01f:0.05f;button->enabledStep();call(nativeRva(0x28590,0x222b0),original);++extendedSteps;check();
            if(action==3) button->unhovered(),call(nativeRva(0x293e0,0x21800),original);
            menuMilliseconds+=action==0 ? 2010:50;menuPoint=center;menuInputEvents.clear();menuMouseDelta={1,0,0};
            extendedPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++extendedFrames;
        }
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
        const auto beforeDelete=textureCounts();button.reset();const auto middleDelete=textureCounts();call(nativeRva(0x284c0,0x21600),original);compareReferenceDeltas(beforeDelete,middleDelete);
        require(views.enabled().empty() && controls.visible().empty(),"Extended button destructor leaves owned controls");++extendedObjects;
        const auto soundCount=sounds.size()-firstSound;require(soundCount==0 || soundCount==16,"Extended button reload sound count differs");
        for(std::size_t i=firstSound;i+4<sounds.size();++i) {require(sounds[i]->referenceCount()==1,"Extended button prior sound retention differs");sounds[i]->releaseReference();++extendedSoundResidues;}
        actualSounds.clear();expectedSounds.clear();
    }
}
putPointer(0x25f73c,nullptr);
