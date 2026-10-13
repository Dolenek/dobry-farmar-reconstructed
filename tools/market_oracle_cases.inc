stage=520;
for(const auto& entry:marketEntries) write(entry.first,entry.second.data(),5);
redirect(0xef00,reinterpret_cast<void*>(&marketExpense));redirect(0x10f10,reinterpret_cast<void*>(&marketSale));
redirect(0x185a13,reinterpret_cast<void*>(&gameplayGetThreadData));
redirect(0xb0b70,reinterpret_cast<void*>(&gameplayTimeAttribute));redirect(0xd9720,reinterpret_cast<void*>(&typed<int>));
SalesHistory saleHistory(profile);std::function<void()> saleHistoryCheck;
if(salesHistoryMode) {
#include "sales_history_oracle_setup.inc"
}
struct MarketFixture final:MarketWorld {
    WarehouseFixture& stock;SalesHistory* history;
    explicit MarketFixture(WarehouseFixture& s,SalesHistory* h):stock(s),history(h) {}
    int money() override {return stock.money();}void setMoney(int value) override {stock.setMoney(value);}
    void productExpense(unsigned product) override {marketSourceEffects.push_back({0,product,0});}
    void recordSale(unsigned amount,float time) override {if(history) history->record(amount,time);else marketSourceEffects.push_back({1,amount,floatBits(time)});}
} marketWorld(warehouseWorld,salesHistoryMode ? &saleHistory:nullptr);
const auto savedTime=profile.floats["GameTime"];profile.floats["GameTime"]=nativeProfile.floats["GameTime"]=0;
XmlDocument timeline(files,String("Gameplay/Gameplay.xml"));GameRandom marketRandom(0x471d40);GameplayTrace marketExecutor(profile);
auto marketGameplay=std::make_unique<Gameplay>(timeline,profile,marketRandom,marketExecutor);
gameplayRuntimeDefinition=&timeline;field<unsigned>(gameplayThreadData.data(),0x14)=0x471d40;
const auto savedXML=managerTable[6],savedRelease=xmlTable[0];managerTable[6]=reinterpret_cast<void*>(&gameplayRuntimeXML);xmlTable[0]=reinterpret_cast<void*>(&gameplayReleaseXML);
std::array<unsigned char,64> nativeGameplay{};function<void*(__thiscall*)(void*)>(0xb59c0)(nativeGameplay.data());managerTable[6]=savedXML;xmlTable[0]=savedRelease;
require(marketRandom.state()==field<unsigned>(gameplayThreadData.data(),0x14),"Market Gameplay initialization RNG differs");
for(unsigned i=0;i<19;++i) {
    const auto& info=configuration.products[i];auto* native=static_cast<unsigned char*>(nativeGame)+0x30+i*100;
    field<float>(native,0)=info.dieTime;field<int>(native,4)=info.priceRadius;field<int>(native,8)=info.goodAttPriceRadius;field<float>(native,12)=info.maxSellFrequency;
    for(unsigned large=0;large<2;++large) {const auto& a=large ? info.iconLarge:info.icon;auto* out=native+16+large*40;field<cTexture*>(out,4)=a.texture;field<unsigned>(out,8)=a.frames;field<float>(out,12)=a.length;field<unsigned char>(out,16)=a.loop;field<unsigned>(out,20)=a.columns;field<unsigned>(out,24)=a.rows;}
}
MarketContext marketContext{manager,settings,strings,input,interaction,profile,configuration,*marketGameplay,marketRandom,audio,*productionTutorial,marketWorld,views,controls};
const auto oldNativeMarket=field<void*>(image,0x25ffc8);putPointer(0x25ffc8,nullptr);
auto market=std::make_unique<Market>(nullptr,marketContext);auto* nativeMarket=allocate(1320);std::memset(nativeMarket,0,1320);function<void*(__thiscall*)(void*,void*)>(0x71d40)(nativeMarket,nullptr);
const auto priorMarket=world.scenes[1];world.scenes[1]=market.get();hudUseActualMarket=true;
const auto marketCheck=[&] {
    stage=521;require(Market::instance()==market.get() && field<void*>(image,0x25ffc8)==nativeMarket,"Market singleton differs");mainMenuCompareView(*market,nativeMarket);
    compareControl(market->movingPicture(),field<void*>(nativeMarket,0x38));compareControl(market->origin(),field<void*>(nativeMarket,0x60));
    require(market->iconType==field<int>(nativeMarket,0x50c) && market->priceIncrement==field<unsigned>(nativeMarket,0x524),"Market icon/increment differs");
    compareFloat(market->priceCheckPeriod,field<float>(nativeMarket,0x510),"Market PriceCheckPeriod");compareFloat(market->priceUpdatePeriod,field<float>(nativeMarket,0x514),"Market PriceUpdatePeriod");
    compareFloat(market->remainingPriceUpdate,field<float>(nativeMarket,0x518),"Market remaining update");compareFloat(market->priceClickElapsed,field<float>(nativeMarket,0x51c),"Market click timer");compareFloat(market->changePricePeriod,field<float>(nativeMarket,0x520),"Market ChangePricePeriod");
    if(profile.uints!=nativeProfile.uints || profile.ints!=nativeProfile.ints || profile.floats!=nativeProfile.floats || profile.booleans!=nativeProfile.booleans) {
        std::cerr<<"Market profile at state "<<marketStates<<'\n';
        const auto differences=[](const auto& source,const auto& native) {
            for(const auto& pair:source) {const auto found=native.find(pair.first);if(found==native.end() || pair.second!=found->second) std::cerr<<pair.first<<" source "<<pair.second<<" native "<<(found==native.end() ? -999:found->second)<<'\n';}
            for(const auto& pair:native) if(!source.count(pair.first)) std::cerr<<pair.first<<" native-only "<<pair.second<<'\n';
        };differences(profile.uints,nativeProfile.uints);differences(profile.ints,nativeProfile.ints);differences(profile.floats,nativeProfile.floats);differences(profile.booleans,nativeProfile.booleans);
    }
    require(profile.uints==nativeProfile.uints && profile.ints==nativeProfile.ints && profile.floats==nativeProfile.floats && profile.booleans==nativeProfile.booleans,"Market profile stores differ");
    require(marketRandom.state()==field<unsigned>(gameplayThreadData.data(),0x14),"Market random state differs");require(marketSourceEffects==marketNativeEffects,"Market cGame effects differ");require(actualSounds==expectedSounds,"Market sound requests differ");
    for(unsigned i=0;i<19;++i) {
        require(!std::memcmp(market->places[i].data(),static_cast<unsigned char*>(nativeMarket)+0x3f8+i*8,8),"Market row places differ");
        const auto& row=market->rows()[i];auto* native=static_cast<unsigned char*>(nativeMarket)+0x68+i*48;
        std::array<Control*,12> source{row.background,row.icon,row.warehouseQuantity,row.addOne,row.addTen,row.addAll,row.sendingQuantity,row.increasePrice,row.price,row.decreasePrice,row.attitude,row.histogram};
        for(unsigned n=0;n<12;++n) compareControl(*source[n],field<void*>(native,n*4));
        for(unsigned n:{3U,4U,5U,7U,9U}) require(static_cast<MarketNumberButton*>(source[n])->product==field<unsigned>(field<void*>(native,n*4),0x280),"Market button product differs");
        for(unsigned n:{2U,6U,8U}) {auto& text=*static_cast<TextLine*>(source[n]);if(text.textDirty()) text.makeLetters();compare(text,field<void*>(native,n*4));}
        auto* histogram=field<void*>(native,44);auto** begin=field<void**>(histogram,0x234);auto** end=field<void**>(histogram,0x238);require(row.histogram->bars().size()==std::size_t(end-begin),"Market histogram count differs");
        for(unsigned bar=0;bar<row.histogram->bars().size();++bar) {auto& actual=*row.histogram->bars()[bar];compareControl(actual,begin[bar]);compareFloat(actual.maximum,field<float>(begin[bar],0x238),"Market histogram maximum");compareFloat(actual.current,field<float>(begin[bar],0x23c),"Market histogram current");}
    }if(saleHistoryCheck) saleHistoryCheck();++marketStates;
};marketCheck();
stage=522;market->refresh();call(0x6f9e0,nativeMarket);marketCheck();
for(unsigned product=0;product<19;++product) {
    require(market->goods(product)==function<unsigned(__thiscall*)(void*,unsigned)>(0x6eb70)(nativeMarket,product),"Market goods getter differs");
    require(market->price(product)==function<unsigned(__thiscall*)(void*,unsigned)>(0x6ef70)(nativeMarket,product),"Market price getter differs");
    require(market->available(product)==function<bool(__thiscall*)(void*,unsigned)>(0x6f370)(nativeMarket,product),"Market availability differs");
    for(unsigned price:{0U,1U,5U,10U,50U,80U,99U,0xffffffffU}) require(market->priceAttitude(product,price)==function<unsigned(__thiscall*)(void*,unsigned,unsigned)>(0x6b850)(nativeMarket,product,price),"Market attitude differs");
}
marketCheck();
const auto savedUInts=profile.uints;
std::mt19937 availabilityRandom(0x46f370);
for(unsigned pattern=0;pattern<64;++pattern) {
    stage=531;
    for(unsigned product=0;product<19;++product) {
        profile.uints["WGoodCount"+std::to_string(product)]=nativeProfile.uints["WGoodCount"+std::to_string(product)]=(pattern%4==0 ? availabilityRandom()%2:0);
        profile.uints["MGoodCount"+std::to_string(product)]=nativeProfile.uints["MGoodCount"+std::to_string(product)]=(pattern%4==1 ? availabilityRandom()%2:0);
    }
    for(unsigned seed=1;seed<8;++seed) profile.uints["SeedAvail"+std::to_string(seed)]=nativeProfile.uints["SeedAvail"+std::to_string(seed)]=(pattern%4==2 ? availabilityRandom()%2:0);
    for(unsigned type=0;type<11;++type) profile.uints["UpAvail"+std::to_string(type)]=nativeProfile.uints["UpAvail"+std::to_string(type)]=(pattern%4==3 && availabilityRandom()%2 ? 0:0xffffffffU);
    market->refresh();call(0x6f9e0,nativeMarket);
    for(unsigned product=0;product<19;++product) require(market->available(product)==function<bool(__thiscall*)(void*,unsigned)>(0x6f370)(nativeMarket,product),"Market compact product availability differs");
    marketCheck();
}
profile.uints=nativeProfile.uints=savedUInts;market->refresh();call(0x6f9e0,nativeMarket);marketCheck();
stage=523;market->show();call(0xd37d0,nativeMarket);marketCheck();
// Real Warehouse supplies the market, and original/source quantity callbacks
// remove its goods before updating market quantities and resetting sale time.
for(unsigned product=0;product<19;++product) profile.uints["WGoodCount"+std::to_string(product)]=nativeProfile.uints["WGoodCount"+std::to_string(product)]=0;
warehouse->occupied=field<unsigned>(nw,0x534)=0;warehouse->refresh();call(0x66530,static_cast<unsigned char*>(nw)+0x508);
for(unsigned product=0;product<19;++product) {
    stage=524;
    const auto actual=warehouse->give(product,17,true,false);const auto expected=function<unsigned(__thiscall*)(void*,unsigned,unsigned,bool,bool)>(0x67ec0)(nw,product,17,true,false);
    require(actual==expected,"Market supply Warehouse give differs");marketCheck();
    market->setGoods(product,2);function<void(__thiscall*)(void*,unsigned,unsigned)>(0x707e0)(nativeMarket,product,2);marketCheck();
}
for(unsigned row=0;row<19;++row) {
    auto* native=static_cast<unsigned char*>(nativeMarket)+0x68+row*48;const auto& r=market->rows()[row];
    for(const auto& button:std::array<std::pair<MarketNumberButton*,unsigned>,3>{{{r.addOne,0x713c0},{r.addTen,0x71420},{r.addAll,0x71480}}}) {
        stage=525;button.first->clickEvent(button.first);
        const auto offset=button.second==0x713c0 ? 12U:button.second==0x71420 ? 16U:20U;
        function<void(__cdecl*)(void*)>(button.second)(field<void*>(native,offset));++marketActions;marketCheck();
    }
    for(unsigned click=0;click<8;++click) {
        stage=526;r.increasePrice->clickEvent(r.increasePrice);function<void(__cdecl*)(void*)>(0x71910)(field<void*>(native,28));++marketActions;marketCheck();
        r.decreasePrice->clickEvent(r.decreasePrice);function<void(__cdecl*)(void*)>(0x714e0)(field<void*>(native,36));++marketActions;marketCheck();
    }
}
// Every real sale must change the market profile, Money, RNG, timer, award flag,
// and the ordered cGame expense/history boundary observations identically.
for(unsigned product=0;product<19;++product) {
    stage=527;market->setGoods(product,9);function<void(__thiscall*)(void*,unsigned,unsigned)>(0x707e0)(nativeMarket,product,9);
    profile.ints["MEffPrice"+std::to_string(product)]=nativeProfile.ints["MEffPrice"+std::to_string(product)]=product%3==0 ? 100:10;
    market->setPrice(product,product%3==0 ? 80:product%3==1 ? 2:10);function<void(__thiscall*)(void*,unsigned,unsigned)>(0x70c00)(nativeMarket,product,product%3==0 ? 80:product%3==1 ? 2:10);marketCheck();
}
for(unsigned tick=0;tick<200;++tick) {
    stage=528;elapsed=tick%7==0 ? 100.f:tick%7==1 ? 0.f:.125f;market->absoluteStep();call(0x721d0,nativeMarket);++marketActions;marketCheck();
}
require(!marketSourceEffects.empty(),"Market did not actually sell supplied goods");
require(profile.booleans["awHighSold"] && nativeProfile.booleans["awHighSold"],"Market high-price award was not triggered");
{
    stage=529;menuMilliseconds=0;GameTimer marketTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor marketCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&marketCursor;
    std::array<unsigned char,24> marketCursorFrame{};field<void*>(marketCursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=marketCursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    for(unsigned icon=0;icon<2;++icon) {
        market->iconType=field<int>(nativeMarket,0x50c)=icon;market->refresh();call(0x6f9e0,nativeMarket);marketCheck();
        for(unsigned frame=0;frame<12;++frame) {
            stage=530;action=frame;menuMilliseconds+=17;menuMouseDelta={0,0,0};menuPoint={790,590};menuInputEvents.clear();
            marketPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++marketFrames;marketCheck();
        }
    }
    for(unsigned movement:{2U,1U}) {
        const Point2 from=movement==2 ? Point2{-800,0}:Point2{0,0},to=movement==2 ? Point2{0,0}:Point2{-800,0};
        market->move(movement,from,to);function<void(__thiscall*)(void*,unsigned,const Point2*,const Point2*)>(0xc5630)(nativeMarket,movement,&from,&to);marketCheck();
        for(unsigned frame=0;frame<28;++frame) {
            stage=532;action=frame;menuMilliseconds+=17;menuMouseDelta={0,0,0};menuPoint={790,590};menuInputEvents.clear();
            marketPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++marketFrames;marketCheck();
        }
        require(market->movement==0 && !field<unsigned>(nativeMarket,0x4c),"Market movement did not finish");
    }
    menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
}
const auto marketXmlDirectory=common/"market-reload";std::filesystem::create_directories(marketXmlDirectory);
std::ofstream(marketXmlDirectory/"partial-market.xml")<<"<Market><ProductLine/></Market>";files.mountCommon(String(marketXmlDirectory.generic_string()));
for(const char* path:{"UI/gameUI/Market.xml","partial-market.xml","UI/gameUI/Market.xml"}) {
    stage=533;XmlDocument definition(files,String(path));market->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0x6bec0)(nativeMarket,wrap(&definition));marketCheck();
    market->refresh();call(0x6f9e0,nativeMarket);marketCheck();
}
stage=534;market->show();call(0xd37d0,nativeMarket);marketCheck();
productionTutorial->tutorialEnabled=true;field<unsigned char>(nativeProductionTutorial,0xc09)=1;
for(unsigned step:{8U,9U,45U,11U,12U}) {
    productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
    profile.ints["marketIt"]=nativeProfile.ints["marketIt"]=step==11 ? 2:step==12 ? 3:1;
    for(unsigned previous:{8U,9U,45U,11U,12U}) profile.booleans["tip"+std::to_string(previous)]=nativeProfile.booleans["tip"+std::to_string(previous)]=previous!=step;
    stage=535;market->enabledStep();call(0x6e760,nativeMarket);marketCheck();
    require(productionTutorial->currentStep==int(step) && field<int>(nativeProductionTutorial,0xbfc)==int(step),"Market tutorial branch/anchor did not start");
    auto* sourceTip=step==8 ? &productionTutorial->common():step==45 ? static_cast<TutorialCommonTip*>(&productionTutorial->action()):static_cast<TutorialCommonTip*>(&productionTutorial->arrow());
    const auto slot=step==8 ? 0xbe8U:step==45 ? 0xbf4U:0xbecU;auto* nativeTip=field<void*>(nativeProductionTutorial,slot);
    mainMenuCompareView(*sourceTip,nativeTip);compareControl(sourceTip->image(),field<void*>(nativeTip,0x40));compareControl(sourceTip->text(),field<void*>(nativeTip,0x44));
    sourceTip->hide();call(0xd3720,nativeTip);productionTutorial->finishTip(*sourceTip);function<void(__thiscall*)(void*,void*)>(0xd0c30)(nativeProductionTutorial,nativeTip);marketCheck();
}
productionTutorial->tutorialEnabled=false;field<unsigned char>(nativeProductionTutorial,0xc09)=0;
{
    // This connected game fixture binds actual SoundManager::playSound;
    // the standalone menu hover adapter uses a different observation code.
    const auto savedHover=interaction.playHover;
    interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
    stage=536;menuMilliseconds=0;GameTimer mouseTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor mouseCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&mouseCursor;
    std::array<unsigned char,24> mouseFrame{};field<void*>(mouseFrame.data(),0)=cursorTable.data();nativeCursorFrame=mouseFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    market->refresh();call(0x6f9e0,nativeMarket);const auto before=market->price(0);const auto& button=*market->rows()[0].increasePrice;
    const Point2 point{button.geometry().x,button.geometry().y};
    for(unsigned frame=0;frame<4;++frame) {
        stage=537;action=frame;menuMilliseconds+=17;menuPoint=frame==3 ? Point2{790,590}:point;menuMouseDelta=frame==0 || frame==3 ? std::array<int,3>{1,0,0}:std::array<int,3>{};
        menuInputEvents=frame==1 ? std::vector<unsigned>{0x208}:frame==2 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
        marketPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++marketFrames;marketCheck();
    }
    require(market->price(0)!=before,"Market real mouse click did not change price");
    interaction.playHover=savedHover;
    menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
}
if(salesHistoryMode) {
#include "sales_history_oracle_cases.inc"
}
// Teardown is compared before any legacy Warehouse reloads resume.
hudUseActualMarket=false;world.scenes[1]=priorMarket;
const auto beforeMarket=textureCounts();market.reset();const auto middleMarket=textureCounts();call(0x6b630,nativeMarket);compareReferenceDeltas(beforeMarket,middleMarket);
putPointer(0x25ffc8,oldNativeMarket);redirect(0x6f9e0,reinterpret_cast<void*>(&warehouseMarket));redirect(0x6f370,reinterpret_cast<void*>(&hudAvailable));redirect(0x6eb70,reinterpret_cast<void*>(&hudProducts));
marketGameplay.reset();call(0xaeb10,nativeGameplay.data());profile.floats["GameTime"]=nativeProfile.floats["GameTime"]=savedTime;
hudSourceCalls.clear();hudNativeCalls.clear();warehouseSourceEvents.clear();warehouseNativeEvents.clear();actualSounds.clear();expectedSounds.clear();
