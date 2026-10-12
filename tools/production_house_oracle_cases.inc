stage=410;write(0x17050,productionLevelEntry.data(),5);for(const auto& entry:tutorialEntries) write(entry.first,entry.second.data(),5);
redirect(0x2fe80,reinterpret_cast<void*>(&decimalString));redirect(0x1cc0,reinterpret_cast<void*>(&concatenateString));
redirect(0x180ed3,reinterpret_cast<void*>(&tutorialVectorDestroy));redirect(0xd0070,reinterpret_cast<void*>(&nativeDiscard));
redirect(0x67fa0,reinterpret_cast<void*>(&productionGoods));redirect(0x65e70,reinterpret_cast<void*>(&productionFree));
redirect(0x682d0,reinterpret_cast<void*>(&productionTake));redirect(0x67ec0,reinterpret_cast<void*>(&productionGive));
redirect(0x94a20,reinterpret_cast<void*>(&productionPlacement));redirect(0x93b70,reinterpret_cast<void*>(&productionManipulationBlocked));
redirect(0x957c0,reinterpret_cast<void*>(&productionBegin));redirect(0x95bd0,reinterpret_cast<void*>(&productionEnd));redirect(0x98400,reinterpret_cast<void*>(&productionList));
redirect(0x94e70,reinterpret_cast<void*>(&productionFocus));redirect(0x98c60,reinterpret_cast<void*>(&productionRemove));
redirect(0x10ce0,reinterpret_cast<void*>(&productionMoney));redirect(0x10d60,reinterpret_cast<void*>(&productionSetMoney));
for(unsigned n=0;n<5;++n) new(static_cast<unsigned char*>(nativeGame)+0xa10+n*28) WString(configuration.upgradeStrings[n]);
for(unsigned n=0;n<19;++n) {const auto& a=configuration.products[n].iconLarge;auto* out=static_cast<unsigned char*>(nativeGame)+0x30+n*100+56;field<cTexture*>(out,4)=a.texture;field<unsigned>(out,8)=a.frames;field<float>(out,12)=a.length;field<unsigned char>(out,16)=a.loop;field<unsigned>(out,20)=a.columns;field<unsigned>(out,24)=a.rows;}
for(unsigned n=0;n<15;++n) for(unsigned j=0;j<4;++j) field<unsigned>(nativeGame,0x8fc+(n*4+j)*4)=configuration.costs[n][j];
Control productionAnchor(nullptr,manager,views,controls);TutorialWorldFixture productionTutorialWorld(world,productionAnchor);
TutorialManagerContext productionTutorialContext{manager,settings,strings,input,interaction,profile,productionTutorialWorld,views,controls};
putPointer(0x25f73c,nullptr);auto* productionTutorial=new TutorialManager(productionTutorialContext);world.tutorial=productionTutorial;
auto* nativeProductionTutorial=allocate(3104);std::memset(nativeProductionTutorial,0,3104);field<int>(nativeProductionTutorial,0xc00)=-1;function<void*(__thiscall*)(void*)>(0xcfce0)(nativeProductionTutorial);
productionTutorial->disable();call(0xd3750,nativeProductionTutorial);productionTutorial->tutorialEnabled=false;field<unsigned char>(nativeProductionTutorial,0xc09)=0;
ProductionWorldFixture productionWorld(world,*productionTutorial,profile);UpgradeContext productionUpgrades(configuration,profile,audio);
ProductionBuildingContext productionContext{manager,settings,strings,input,interaction,profile,configuration,audio,productionUpgrades,productionWorld,views,controls};
const unsigned productionConstructors[]={0xa7850,0xa54b0,0xa42e0,0xa6f60,0xa39f0,0xa4bc0,0xa8a30,0xa5d90,0xa8140,0xa3100,0xa6670};
// Isolate the actual house's input/frame lifecycle from the separate HUD
// controller's diagnostic warehouse-value providers.
game.disable();game.absoluteDisable();game.hide();call(0xd3750,nativeGame);call(0xd3790,nativeGame);call(0xd3720,nativeGame);
const auto appearanceCheck=[&](const AnimableAppearance& a,void* n) {
    require(a.texture==field<cTexture*>(n,4) && a.frames==field<unsigned>(n,8) && a.columns==field<unsigned>(n,20) && a.rows==field<unsigned>(n,24),"Production level appearance differs");
    compareFloat(a.length,field<float>(n,12),"Production level duration");require(a.loop==bool(field<unsigned char>(n,16)) && a.playing==bool(field<unsigned char>(n,28)),"Production level animation flags differ");
};
#include "production_placement_oracle_cases.inc"
if(!manufactureMode && !actionAreaMode) for(unsigned type=0;type<11;++type) for(unsigned initialLevel=0;initialLevel<5;++initialLevel) {
    trial=type*5+initialLevel;stage=411;const unsigned slot=type;const auto key=[&](const char* prefix){return std::string(prefix)+std::to_string(slot);};
    profile.ints[key("Upgrade")]=nativeProfile.ints[key("Upgrade")]=initialLevel==4 ? -1:int(initialLevel);
    profile.uints[key("ProdCount")]=nativeProfile.uints[key("ProdCount")]=initialLevel==4 ? 0:initialLevel%2;
    profile.uints[key("ProdStop")]=nativeProfile.uints[key("ProdStop")]=initialLevel==2 ? 3:0;
    profile.floats[key("ProdPct")]=nativeProfile.floats[key("ProdPct")]=0;
    profile.floats[key("XProdPos")]=nativeProfile.floats[key("XProdPos")]=400;profile.floats[key("YProdPos")]=nativeProfile.floats[key("YProdPos")]=300;
    productionSourceGoods.fill(100);productionNativeGoods=productionSourceGoods;productionSourceFree=productionNativeFree=100;
    productionMoving=nullptr;productionFocused=nullptr;putPointer(0x260150,nullptr);field<void*>(nativeScenes[1],0xf8)=nullptr;
    cursorPosition={790,590};actualSounds.clear();expectedSounds.clear();productionSourceCalls.clear();productionNativeCalls.clear();
    auto building=std::make_unique<ProductionBuilding>(type,slot,nullptr,productionContext);
    auto* nativeBuilding=allocate(1588);std::memset(nativeBuilding,0,1588);function<void*(__thiscall*)(void*,unsigned,void*)>(productionConstructors[type])(nativeBuilding,slot,nullptr);
    productionSourceBuildings={building.get()};productionNativeBuildings={nativeBuilding};
    const auto houseCheck=[&] {
        stage=412;mainMenuCompareView(*building,nativeBuilding);compareControl(*building,nativeBuilding);compareControl(building->overlay(),field<void*>(nativeBuilding,0x27c));
        require(building->flags==field<unsigned>(nativeBuilding,0x574) && building->built==bool(field<unsigned char>(nativeBuilding,0x578)),"Production flags/built state differs");
        require(building->buildingType==field<unsigned>(nativeBuilding,0x390) && building->itemType==field<unsigned>(nativeBuilding,0x368) && building->propertyId==field<unsigned>(nativeBuilding,0x388),"Production type/property id differs");
        require(building->levelCount==field<int>(nativeBuilding,0x38c),"Production upgrade count differs");compareFloat(building->productionSpeed,field<float>(nativeBuilding,0x394),"Production speed");compareFloat(building->originalDepth,field<float>(nativeBuilding,0x57c),"Production original depth");
        require(!std::memcmp(&building->normalColor,static_cast<unsigned char*>(nativeBuilding)+0x5c8,16) && !std::memcmp(&building->unfitColor,static_cast<unsigned char*>(nativeBuilding)+0x5d8,16) && !std::memcmp(&building->fitSubstrate,static_cast<unsigned char*>(nativeBuilding)+0x5e8,16) && !std::memcmp(&building->unfitSubstrate,static_cast<unsigned char*>(nativeBuilding)+0x5f8,16),"Production placement colours differ");
        require(!std::memcmp(&building->geometry().color,static_cast<unsigned char*>(nativeBuilding)+0x218,16),"Production current colour differs");
        require(unsigned(building->buttonState())==field<unsigned>(nativeBuilding,0x238) && building->tipsDirection==field<int>(nativeBuilding,0x2b4),"Production root hover state differs");
        for(unsigned i=0;i<4;++i) {const auto& level=building->levels[i];auto* n=static_cast<unsigned char*>(nativeBuilding)+0x398+i*116;require(level.productionCount==field<unsigned>(n,0),"Production capacity differs");compareFloat(level.maximumProductionTime,field<float>(n,4),"Production time");appearanceCheck(level.appearance,n+8);appearanceCheck(level.hovered,n+48);require(level.maskName.view()==field<String>(n,88).view(),"Production level mask name differs");}
        for(auto pair:std::initializer_list<std::pair<Control*,unsigned>>{{&building->takeIcon(),0x580},{&building->givenIcon(),0x584},{&building->startStopButton(),0x588},{&building->warehouseQuantity(),0x58c},{&building->loadedQuantity(),0x590},{&building->loadedProgress(),0x594},{&building->loadedSlider(),0x598},{&building->addStockButton(),0x59c},{&building->addDepotButton(),0x5a0},{&building->sensorPanel(),0x5a4},{&building->inverseSubstrate(),0x5a8},{&building->loadedSubstrate(),0x5ac},{&building->lighter(),0x620}}) {
            action=pair.second;auto* n=field<void*>(nativeBuilding,pair.second);compareControl(*pair.first,n);if(auto* b=dynamic_cast<ButtonPrototype*>(pair.first)) require(unsigned(b->buttonState())==field<unsigned>(n,0x238),"Production child button state differs");
            if(auto* line=dynamic_cast<TextLine*>(pair.first)) {if(line->textDirty()) line->makeLetters();compare(*line,n);}
        }
        compareControl(building->loadedSlider().thumb(),field<void*>(field<void*>(nativeBuilding,0x598),0x27c));
        compareFloat(building->loadedSlider().value(),field<float>(field<void*>(nativeBuilding,0x598),0x290),"Production slider percent");compareFloat(building->loadedProgress().current,field<float>(field<void*>(nativeBuilding,0x594),0x23c),"Production progress percent");
        for(unsigned i=0;i<5;++i) require(building->sounds()[i]==field<cSound*>(nativeBuilding,0x608+i*4),"Production sound ownership differs");
        require(profile.ints==nativeProfile.ints && profile.uints==nativeProfile.uints && profile.floats.size()==nativeProfile.floats.size() && profile.booleans==nativeProfile.booleans,"Production profile mutation differs");
        for(const auto& entry:profile.floats) {const auto found=nativeProfile.floats.find(entry.first);require(found!=nativeProfile.floats.end(),"Production float profile key differs");compareFloat(entry.second,found->second,"Production float profile value");}
        require(productionSourceGoods==productionNativeGoods && productionSourceFree==productionNativeFree,"Production warehouse observations differ");
        if(productionSourceCalls!=productionNativeCalls) {for(const auto& c:productionSourceCalls) std::cerr<<"PS "<<c[0]<<' '<<c[1]<<' '<<c[2]<<'\n';for(const auto& c:productionNativeCalls) std::cerr<<"PN "<<c[0]<<' '<<c[1]<<' '<<c[2]<<'\n';}
        require(productionSourceCalls==productionNativeCalls,"Production world call order differs");productionSourceCalls.clear();productionNativeCalls.clear();require(actualSounds==expectedSounds,"Production sounds differ");actualSounds.clear();expectedSounds.clear();++productionHouseStates;
    };houseCheck();
    if(initialLevel==4) {
        // The normal test heap has fresh zero padding. Separately vary the
        // exact native overlapping loop/padding word, never a guessed level.
        for(unsigned raw:{0U,0x100U,0x10000U,0x1000000U}) {
            auto& appearance=const_cast<AnimableAppearance&>(building->tipAppearances()[2]);appearance.loop=false;
            for(unsigned j=0;j<3;++j) appearance.loopPadding[j]=static_cast<unsigned char>(raw>>(8*(j+1)));
            field<unsigned>(nativeBuilding,0x324)=raw;building->setProductionCount(0);function<void(__thiscall*)(void*,unsigned)>(0x9fdf0)(nativeBuilding,0);houseCheck();++productionHouseActions;
        }
        // Placement is an explicitly observed boundary; the complete house
        // click body builds level zero and applies the actual money rule.
        productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=37;
        productionValid=false;building->clicked();call(0x9f8c0,nativeBuilding);houseCheck();++productionHouseActions;
        productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=type%2 ? 37:-1;
        productionValid=true;building->clicked();call(0x9f8c0,nativeBuilding);houseCheck();++productionHouseActions;
        productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
    }
    for(unsigned count:{0U,1U,2U,5U,100U,0xffffffffU}) {stage=413;building->setProductionCount(count);function<void(__thiscall*)(void*,unsigned)>(0x9fdf0)(nativeBuilding,count);houseCheck();++productionHouseActions;}
    building->setProductionCount(100);function<void(__thiscall*)(void*,unsigned)>(0x9fdf0)(nativeBuilding,100);houseCheck();++productionHouseActions;
    for(int requested:{0,1,2,7}) {building->setLevel(requested,false);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(static_cast<unsigned char*>(nativeBuilding)+0x364,requested,false);houseCheck();++productionHouseActions;}
    building->hovered();call(0x9d340,nativeBuilding);houseCheck();building->unhovered();call(0x9d7b0,nativeBuilding);houseCheck();
    for(unsigned i=0;i<24;++i) {
        stage=414;action=i;productionSourceFree=productionNativeFree=i%3 ? 100:0;productionSourceGoods[productionKind(type).inputProduct()]=productionNativeGoods[productionKind(type).inputProduct()]=i%4;
        auto& button=i%2 ? building->addDepotButton():building->addStockButton();button.clickEvent(&button);
        function<void(__cdecl*)(void*)>(i%2 ? 0xa0230:0xa02c0)(field<void*>(nativeBuilding,i%2 ? 0x5a0:0x59c));houseCheck();++productionHouseActions;
    }
    for(unsigned i=0;i<20;++i) {
        stage=415;action=i;const float value=float(i)*5.7f;productionSourceFree=productionNativeFree=i%3;productionSourceGoods[productionKind(type).inputProduct()]=productionNativeGoods[productionKind(type).inputProduct()]=i%5;
        building->loadedSlider().valueChanged(&building->loadedSlider(),value);function<void(__cdecl*)(void*,float)>(0xa0000)(field<void*>(nativeBuilding,0x598),value);houseCheck();++productionHouseActions;
    }
    productionSourceFree=productionNativeFree=100;building->setProductionCount(2);function<void(__thiscall*)(void*,unsigned)>(0x9fdf0)(nativeBuilding,2);houseCheck();
    for(unsigned i=0;i<64;++i) {
        stage=416;action=i;elapsed=float(i%5)*0.73f;
        if(i%11==0) {building->startStopButton().clickEvent(&building->startStopButton());function<void(__cdecl*)(void*)>(0x9fdb0)(field<void*>(nativeBuilding,0x588));}
        building->absoluteStep();call(0xa1c90,nativeBuilding);houseCheck();++productionHouseActions;
    }
    for(unsigned i=0;i<16;++i) {
        // Frozen tutorial/each scene, full warehouse and progress edge values.
        action=i;productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=i%4==0 ? 5:-1;
        for(unsigned scene=0;scene<3;++scene) scenes[scene]->movement=field<unsigned>(nativeScenes[scene],0x4c)=i%4==scene+1 ? 1:0;
        building->setProductionStopped(false);function<void(__thiscall*)(void*,bool)>(0x9f210)(nativeBuilding,false);
        productionSourceFree=productionNativeFree=i%3 ? 100:0;building->setProductionCount(1);function<void(__thiscall*)(void*,unsigned)>(0x9fdf0)(nativeBuilding,1);
        const unsigned nanWord=0x7fc00001;float nan;std::memcpy(&nan,&nanWord,4);const float values[]={99.999f,100.f,-1.f,nan};building->setProductionProgress(values[i%4]);function<void(__thiscall*)(void*,float)>(0x9f300)(nativeBuilding,values[i%4]);
        elapsed=0.73f;building->absoluteStep();call(0xa1c90,nativeBuilding);houseCheck();++productionHouseActions;
    }
    productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;for(unsigned scene=0;scene<3;++scene) scenes[scene]->movement=field<unsigned>(nativeScenes[scene],0x4c)=0;
    building->setProductionProgress(0);function<void(__thiscall*)(void*,float)>(0x9f300)(nativeBuilding,0);
    // Real completed objects remain movable. Exercise the original squared
    // distance >32 gate, colour/position tick, valid release and focus click.
    for(auto* control:std::initializer_list<ButtonPrototype*>{&building->loadedSlider(),&building->startStopButton(),&building->loadedSlider().thumb(),&building->addStockButton(),&building->addDepotButton()}) control->setState(ButtonState::Normal);
    for(unsigned offset:{0x598U,0x588U,0x59cU,0x5a0U}) function<void(__thiscall*)(void*,unsigned)>(0xf30e0)(field<void*>(nativeBuilding,offset),1);
    function<void(__thiscall*)(void*,unsigned)>(0xf30e0)(field<void*>(field<void*>(nativeBuilding,0x598),0x27c),1);
    cursorPosition={building->geometry().x,building->geometry().y};building->clicked();call(0x9f8c0,nativeBuilding);houseCheck();++productionHouseActions;
    cursorPosition[0]+=5;building->absoluteStep();call(0xa1c90,nativeBuilding);houseCheck();++productionHouseActions;
    cursorPosition[0]+=1;building->absoluteStep();call(0xa1c90,nativeBuilding);houseCheck();++productionHouseActions;
    building->absoluteStep();call(0xa1c90,nativeBuilding);houseCheck();++productionHouseActions;
    building->unclicked();call(0x9fc30,nativeBuilding);houseCheck();++productionHouseActions;
    building->clicked();call(0x9f8c0,nativeBuilding);building->unclicked();call(0x9fc30,nativeBuilding);houseCheck();++productionHouseActions;
    if(initialLevel==0) {
        menuMilliseconds=0;GameTimer houseTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor houseCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&houseCursor;
        std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned i=0;i<4;++i) {
            stage=418;menuMilliseconds+=50;menuPoint=i==3 ? Point2{790,590}:Point2{building->geometry().x,building->geometry().y};menuMouseDelta=i==0 || i==3 ? std::array<int,3>{1,0,0}:std::array<int,3>{};menuInputEvents=i==1 ? std::vector<unsigned>{0x208}:i==2 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
            productionHousePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++productionHouseFrames;houseCheck();
        }
        menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }
    if(type==0 && initialLevel==0) {
        // Distinct signs make the byte-selection contract observable even
        // though shipped building XML omits this dormant button section.
        const auto signDirectory=common/"signs";std::filesystem::create_directories(signDirectory);
        std::ofstream(signDirectory/"production-signs.xml")<<R"(<Sign position="7 9" size="35 31" Picture="Manufacture/icon_button.jng" frames_count="4" anim_columns="2" anim_rows="2" Visible="1" Enable="1"><StateStopSign Picture="Manufacture/line_shariki_back.jng" anim_length="0.75" anim_loop="0"/><StateStartSign Picture="Manufacture/icon_button.jng" frames_count="4" anim_columns="2" anim_rows="2" anim_length="2"/></Sign>)";
        files.mountCommon(String(signDirectory.generic_string()));
        XmlDocument signDefinition(files,String("production-signs.xml"));building->startStopButton().loadXML(signDefinition);function<void(__thiscall*)(void*,void*)>(0x9ab60)(field<void*>(nativeBuilding,0x588),wrap(&signDefinition));houseCheck();
        for(unsigned char value:{0,1,2,3,127,128,254,255}) {
            const bool dirty=building->startStopButton().dirty();const auto nativeDirty=field<unsigned char>(field<void*>(nativeBuilding,0x588),0x1b0);
            building->startStopButton().setStoppedSign(value);function<void(__thiscall*)(void*,unsigned char)>(0x99b90)(field<void*>(nativeBuilding,0x588),value);
            require(building->startStopButton().dirty()==dirty && field<unsigned char>(field<void*>(nativeBuilding,0x588),0x1b0)==nativeDirty,"Production sign dirties its control");houseCheck();++productionHouseActions;
        }
    }
    stage=417;productionSourceBuildings.clear();productionNativeBuildings.clear();productionFocused=nullptr;field<void*>(nativeScenes[1],0xf8)=nullptr;
    const auto before=textureCounts();building.reset();const auto middle=textureCounts();call(0x9b080,nativeBuilding);compareReferenceDeltas(before,middle);++productionHouseObjects;
}
if(manufactureMode) {
#include "manufacture_oracle_cases.inc"
}
if(actionAreaMode) {
#include "action_area_oracle_cases.inc"
}
std::array<TutorialCommonTip*,5> keptProductionTips{&productionTutorial->common(),&productionTutorial->arrow(),&productionTutorial->disabledAnchor(),&productionTutorial->action(),&productionTutorial->repeatedAction()};std::array<void*,5> nativeKeptProductionTips{};for(unsigned i=0;i<5;++i) nativeKeptProductionTips[i]=field<void*>(nativeProductionTutorial,0xbe8+i*4);
delete productionTutorial;world.tutorial=nullptr;call(0xd0490,nativeProductionTutorial);const auto tipBefore=textureCounts();views.deleteDiscarded();delete keptProductionTips[4];const auto tipMiddle=textureCounts();for(auto* n:nativeKeptProductionTips) call(0xcc770,n);compareReferenceDeltas(tipBefore,tipMiddle);nativeDiscards.clear();
for(unsigned n=0;n<5;++n) field<WString>(nativeGame,0xa10+n*28).~WString();
productionSourceCalls.clear();productionNativeCalls.clear();hudSourceCalls.clear();hudNativeCalls.clear();
