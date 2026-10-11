stage=260;xmlTable[34]=reinterpret_cast<void*>(&sectionCount);
const auto directory=std::filesystem::path(".local")/(std::string("price-text-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));std::filesystem::create_directories(directory);
std::ofstream(directory/"price-fixture.xml")<<R"(<Root><Price position="101.25 200.75" Font="UI/Font/font_tips.fnt" Texture="UI/Font/font_tips.jng" Localized="0" String="123456789" DegreeOffset="1.75"><Coin position="-13 3.5" size="16 16" Picture="ui/gameUI/contract.jng" frames_count="2" anim_columns="2" anim_rows="1"/></Price><Empty position="51 60" Font="UI/Font/font_tips.fnt" Texture="UI/Font/font_tips.jng" Localized="0" String="" DegreeOffset="-2.3"><Coin position="-8 -7" size="11 13" Picture="ui/gameUI/contract.jng"/></Empty></Root>)";
files.mountCommon(String(directory.generic_string()));
const std::pair<const char*,std::vector<const char*>> definitions[]={
    {"price-fixture.xml",{"Price","Empty"}},
    {"Farm/ActionAreas/DigBuilding.xml",{"CostText"}},
    {"Farm/ActionAreas/MowBuilding.xml",{"CostText"}},
    {"Farm/ActionAreas/WareHouse.xml",{"CostText"}},
    {"Farm/ActionAreas/WaterSource.xml",{"SprinkleCostText","CostText"}},
    {"Gameplay/ContractAim.xml",{"BonusMoneyText"}},
    {"Gameplay/ContractPassConfirm.xml",{"BonusMoneyText"}},
    {"Gameplay/ContractProvide.xml",{"BonusMoneyText","PenaltyMoneyText"}},
    {"UI/gameUI/BuyBuilding.xml",{"ItemProto/PriceText"}},
    {"UI/gameUI/BuySeeds.xml",{"ItemProto/PriceText"}},
    {"UI/gameUI/Farm.xml",{"FieldsBuyCostText"}},
    {"UI/gameUI/Manufacture.xml",{"UpgradeText"}},
    {"UI/gameUI/Market.xml",{"ProductLine/PriceLabel"}}
};
const auto runPrice=[&](auto* tag) {
    using PriceType=std::remove_pointer_t<decltype(tag)>;constexpr bool money=std::is_same_v<PriceType,TextMoney>;
    for(const auto& file:definitions) {XmlDocument document(files,String(file.first));for(const auto* name:file.second) {
        XmlNode* definition=nullptr;XmlNode* parent=&document;std::istringstream parts(name);std::string part;while(std::getline(parts,part,'/')) {if(!parent->section(String(part.c_str()),definition)) throw std::runtime_error(std::string("Missing actual money/price definition: ")+file.first+"/"+name);parent=definition;}
        ViewRegistry views;ControlRegistry controls;auto source=std::make_unique<PriceType>(nullptr,manager,settings,strings,views,controls);
        auto* original=allocate(money ? 704:700);function<void*(__thiscall*)(void*,void*)>(money ? 0x15b70:0x8bd30)(original,nullptr);
        source->loadXML(*definition);function<void(__thiscall*)(void*,void*)>(money ? 0x15d10:0x8be00)(original,wrap(definition));
        const auto check=[&] {
            stage=261;compareControl(*source,original);priceLetters+=compare(*source,original);
            compareFloat(source->degreeOffset,field<float>(original,0x2b8),"Price degree offset");
            if constexpr(std::is_same_v<PriceType,TextMoney>) {compareControl(source->coin(),field<void*>(original,0x2bc));source->coin().updateTransform();call(0xf0b70,field<void*>(original,0x2bc));
                require(!std::memcmp(&source->coin().sprite(),static_cast<unsigned char*>(field<void*>(original,0x2bc))+0x70,76),"Money coin sprite differs");}
            ++priceStates;
        };check();
        for(action=0;action<256;++action) {
            stage=262;std::wstring text;
            const std::array<const wchar_t*,12> words{L"",L"0",L"1",L"123",L"1234567890",L"-2147483648",L"4294967295",L"12 345 678",L"1a23.456b789",L"-12:345",L"abc",L"1 2 3 4"};
            text=words[action%words.size()];if(action>=12 && action%7==0) text=std::to_wstring(action*10000001U);
            source->degreeOffset=field<float>(original,0x2b8)=float(int(action%19)-9)/3.7f;
            const unsigned alignment=action%3;source->setAlignment(alignment);function<void(__thiscall*)(void*,const unsigned*)>(0x15cf0)(original,&alignment);
            source->setMultiplier(float(action%13+1)/7.f);
            const float multiplier=float(action%13+1)/7.f;function<void(__thiscall*)(void*,const float*)>(0x15cc0)(original,&multiplier);
            const WString value(text);source->setString(value);function<void(__thiscall*)(void*,const WString*)>(0xdd030)(original,&value);
            if(action%5==0) {source->disable();call(0xd3750,original);}else {source->enable();call(0xd3bb0,original);}
            source->show();call(0xd37d0,original);source->enabledStep();call(0xdae40,original);check();
            const Point2 point{float(int(action%53)-20)/11.f+301.5f,float(action%23)+200};source->setPosition(point);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(original,&point);
            source->makeLetters();call(0x8c210,original);check();
        }
        source->loadXML(*definition);function<void(__thiscall*)(void*,void*)>(money ? 0x15d10:0x8be00)(original,wrap(definition));check();++priceReloads;
        menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
        GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
        std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(action=0;action<4;++action) {stage=263;source->setString(WString(action%2 ? L"123456789":L"-987654321"));const WString value(action%2 ? L"123456789":L"-987654321");function<void(__thiscall*)(void*,const WString*)>(0xdd030)(original,&value);
            if(action%2) source->enable(),call(0xd3bb0,original);else source->disable(),call(0xd3750,original);
            menuMilliseconds+=40;menuPoint={790,590};menuInputEvents.clear();menuMouseDelta={1,0,0};pricePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++priceFrames;check();}
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
        const auto before=textureCounts();source.reset();const auto middle=textureCounts();call(money ? 0x15bf0:0x8bd60,original);compareReferenceDeltas(before,middle);
        require(controls.visible().empty() && views.enabled().empty(),"Price/money destructor leaves owned UI");++priceObjects;
    }}
};
if(moneyTexts) runPrice(static_cast<TextMoney*>(nullptr));else runPrice(static_cast<PriceText*>(nullptr));
