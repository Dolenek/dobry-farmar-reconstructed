stage=750;cursorPosition={790,590};
// Previously native-verified SeedInfo data; the original 00403740 accessor
// and the original SeedPackage constructor execute their actual bodies.
for(unsigned i=0;i<8;++i) {const auto& info=configuration.seeds[i];auto* row=static_cast<unsigned char*>(nativeGame)+0x79c+i*44;field<unsigned>(row,0)=info.price;auto* a=row+4;
    field<cTexture*>(a,4)=info.icon.texture;field<unsigned>(a,8)=info.icon.frames;field<float>(a,12)=info.icon.length;field<unsigned char>(a,16)=info.icon.loop;field<unsigned>(a,20)=info.icon.columns;field<unsigned>(a,24)=info.icon.rows;field<unsigned char>(a,28)=info.icon.playing;
}
auto seedBuy=std::make_unique<ExtendedAnimationButton>(&fieldWorld.farmScene().origin(),manager,settings,strings,input,interaction,[&]{return productionTutorial->active();},views,controls);
auto* nativeSeedBuy=allocate(864);std::memset(nativeSeedBuy,0,864);function<void*(__thiscall*)(void*,void*)>(0x23550)(nativeSeedBuy,field<void*>(nativeScenes[0],0x60));
XmlDocument farmSeedXML(files,String("UI/GameUI/Farm.xml"));XmlNode* seedBuyXML=nullptr;require(farmSeedXML.section(String("SeedsBuyButton"),seedBuyXML),"Farm SeedsBuyButton XML missing");seedBuy->loadXML(*seedBuyXML);reinterpret_cast<void(__thiscall*)(void*,void*)>(field<void**>(nativeSeedBuy,0)[6])(nativeSeedBuy,wrap(seedBuyXML));
auto* previousSeedBuy=field<void*>(nativeScenes[0],0x9c);field<void*>(nativeScenes[0],0x9c)=nativeSeedBuy;
struct SeedFixture final:SeedPackageWorld {
    FieldFixture& fields;ExtendedAnimationButton& buy;ViewRegistry& views;SeedPackage* selected=nullptr;
    SeedFixture(FieldFixture& f,ExtendedAnimationButton& b,ViewRegistry& v):fields(f),buy(b),views(v) {}
    TutorialManager& tutorial() override {return fields.tutorial();}StateView& farmScene() override {return fields.farmScene();}
    void registerArea(ActionArea& a) override {fields.registerArea(a);}void queueAreaAction(ActionArea& a,const Point2& p) override {fields.queueAreaAction(a,p);}
    int warehouseLevel() override {return fields.warehouseLevel();}int digBuildingLevel() override {return fields.digBuildingLevel();}int mowBuildingLevel() override {return fields.mowBuildingLevel();}int waterSourceLevel() override {return fields.waterSourceLevel();}
    bool seedMoveActive() override {return fields.water.seedMove;}void setSeedMoveActive(bool value) override {fields.water.seedMove=value;}
    SeedPackage* selectedSeed() override {return selected;}void setSelectedSeed(SeedPackage* value) override {selected=value;}
    bool canPlaceSeed(SeedPackage& p,const Point2& at) override {return fields.graph.canPlaceSeed(&p,at);}ActionArea* topAt(const Point2& p,unsigned mask) override {return fields.graph.topAt(p,mask);}
    std::vector<ActionArea*> areasAt(const Point2& p,unsigned mask) override {std::vector<ActionArea*> result;fields.graph.appendAt(p,mask,result);return result;}
    void setFarmActionButtonEnabled(bool value) override {if(value) buy.enable();else buy.disable();}
    int money() override {return fields.water.money();}void setMoney(int value) override {fields.water.setMoney(value);}
    void removeSeed(SeedPackage& p) override {fields.graph.remove(p,views);}FarmField* fieldForSeedTutorial() override {return fields.graph.fieldForSeedTutorial();}
    Control& digBuilding() override {throw std::logic_error("Seed tutorial20 requires actual DigBuilding; this fixture leaves it disabled");}
} seedWorld(fieldWorld,*seedBuy,views);
SeedPackageContext seedContext{{manager,settings,strings,input,interaction,seedWorld,views,controls},configuration,profile,seedWorld};
const auto checkSeed=[&](SeedPackage& p,void* n) {
    stage=751;mainMenuCompareView(p,n);compareControl(p,n);require(p.status()==field<unsigned>(n,0x648) && p.kind()==field<unsigned>(n,0x64c) && p.count()==field<unsigned>(n,0x650),"SeedPackage identity/status/count differs");
    require(p.tipsDirection==field<int>(n,0x2b4) && p.tipsDisabled==bool(field<unsigned char>(n,0x280)),"Seed tooltip direction differs");compareFloat(p.waitRemaining,field<float>(n,0x2a8),"Seed tip timer");compareFloat(p.tipsAlpha,field<float>(n,0x2b0),"Seed tip alpha");
    require(p.unpaid==bool(field<unsigned char>(n,0x690)),"SeedPackage unpaid differs");for(unsigned i=0;i<2;++i) {compareFloat(p.pressPosition[i],field<float>(n,0x658+i*4),"Seed press");compareFloat(p.dragOffset[i],field<float>(n,0x660+i*4),"Seed drag offset");}
    for(auto pair:std::initializer_list<std::pair<const Color*,unsigned>>{{&p.normalColor,0x670},{&p.unfitColor,0x680}}) for(unsigned i=0;i<4;++i) compareFloat(reinterpret_cast<const float*>(pair.first)[i],field<float>(n,pair.second+i*4),"Seed appearance color");
    for(unsigned i=0;i<4;++i) compareFloat(reinterpret_cast<const float*>(&p.geometry().color)[i],field<float>(n,0x218+i*4),"Seed current color");
    checkUse(p.use(),field<void*>(n,0x654));require(seedWorld.seedMoveActive()==bool(field<unsigned char>(image,0x25f721)),"Seed move flag differs");require((seedWorld.selected==&p && field<void*>(image,0x25f724)==n) || (!seedWorld.selected && !field<void*>(image,0x25f724)),"Selected seed differs");
    require(profile.ints==nativeProfile.ints && profile.uints==nativeProfile.uints && profile.booleans==nativeProfile.booleans,"SeedPackage profile stores differ");require(actualSounds==expectedSounds,"SeedPackage sound events differ");actualSounds.clear();expectedSounds.clear();
    compareControl(*seedBuy,nativeSeedBuy);mainMenuCompareView(*seedBuy,nativeSeedBuy);require(actionAreaSourceTasks==actionAreaNativeTasks,"SeedPackage Person queue differs");actionAreaSourceTasks.clear();actionAreaNativeTasks.clear();++seedPackageStates;
};
for(unsigned kind=0;kind<8;++kind) {
    stage=752;trial=kind;fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
    seedWorld.setSeedMoveActive(false);seedWorld.selected=nullptr;field<unsigned char>(image,0x25f721)=0;putPointer(0x25f724,nullptr);
    const Point2 position{330+float(kind)*7,250};auto package=std::make_unique<SeedPackage>(nullptr,kind,4,position,2,true,seedContext);auto* np=allocate(1684);std::memset(np,0,1684);function<void*(__thiscall*)(void*,void*,unsigned,unsigned,const Point2*,unsigned,bool)>(0x5e130)(np,nullptr,kind,4,&position,2,true);checkSeed(*package,np);
    for(unsigned count:{0U,1U,4U,9U,0xffffffffU,4U}) {package->setCount(count);function<void(__thiscall*)(void*,unsigned)>(0x5cee0)(np,count);++seedPackageActions;checkSeed(*package,np);}
    for(unsigned mode:{0U,1U,2U,3U,0U}) {package->setStatus(mode);function<void(__thiscall*)(void*,unsigned)>(0x5cf50)(np,mode);++seedPackageActions;checkSeed(*package,np);}
    if(kind) {
        stage=753;package->show();call(0xd37d0,np);cursorPosition=position;profile.ints["Money"]=nativeProfile.ints["Money"]=10000;
        package->setStatus(2);function<void(__thiscall*)(void*,unsigned)>(0x5cf50)(np,2);package->clicked();call(0x5d660,np);++seedPackageActions;checkSeed(*package,np);require(!package->unpaid && package->status()==0,"Seed accepted click did not pay and install");
        for(unsigned click=0;click<3;++click) {package->clicked();call(0x5d660,np);++seedPackageActions;checkSeed(*package,np);package->unclicked();call(0x5dbe0,np);++seedPackageActions;checkSeed(*package,np);}
        // Exactly four pixels starts dragging; a three-pixel movement does not.
        cursorPosition=position;package->clicked();call(0x5d660,np);++seedPackageActions;checkSeed(*package,np);
        for(float distance:{3.f,4.f,10.f}) {cursorPosition={position[0]+distance,position[1]};elapsed=.05f;package->enabledStep();call(0x5e340,np);++seedPackageActions;checkSeed(*package,np);require(package->status()==(distance<4 ? 1U:2U),"Seed drag threshold differs");}
        package->unclicked();call(0x5dbe0,np);++seedPackageActions;checkSeed(*package,np);
        package->hovered();call(0x5e8d0,np);++seedPackageActions;checkSeed(*package,np);package->unhovered();call(0x5e100,np);++seedPackageActions;checkSeed(*package,np);
        package->clicked();call(0x5d660,np);++seedPackageActions;checkSeed(*package,np);
        cursorPosition={790,590};package->enabledStep();call(0x5e340,np);++seedPackageActions;checkSeed(*package,np);package->enabledStep();call(0x5e340,np);++seedPackageActions;checkSeed(*package,np);
        package->unclicked();call(0x5dbe0,np);++seedPackageActions;checkSeed(*package,np);require(package->status()==2,"Rejected seed drop unexpectedly completed");
        cursorPosition={340,260};package->enabledStep();call(0x5e340,np);++seedPackageActions;checkSeed(*package,np);package->clicked();call(0x5d660,np);++seedPackageActions;checkSeed(*package,np);
    }
    if(kind==1 || kind==7) {
        const auto hover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
        menuMilliseconds=0;GameTimer timer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor cursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&cursor;
        std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned frame=0;frame<8;++frame) {stage=754;action=frame;menuMilliseconds+=50;
            menuPoint=frame>=5 ? Point2{790,590}:Point2{340+float(frame>=3 ? 10:frame==2 ? 5:0),260};menuMouseDelta={1,0,0};
            menuInputEvents=frame==1 ? std::vector<unsigned>{0x208}:frame==4 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
            seedPackagePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++seedPackageFrames;checkSeed(*package,np);
        }
        interaction.playHover=hover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
        require(package->status()==0,"Actual SeedPackage mouse drag did not complete");
    }
    seedWorld.setSeedMoveActive(false);seedWorld.selected=nullptr;field<unsigned char>(image,0x25f721)=0;putPointer(0x25f724,nullptr);
    const auto before=textureCounts();package.reset();const auto middle=textureCounts();call(0x5ce00,np);compareReferenceDeltas(before,middle);++seedPackageObjects;
}
fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
{
    stage=757;auto& farm=seedWorld.farmScene();auto* parent=farm.parent();auto* nativeParent=field<void*>(nativeScenes[0],0x20);const bool wasShown=farm.state().shown;
    farm.setParent(nullptr);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nullptr);farm.show();call(0xd37d0,nativeScenes[0]);farm.movement=field<unsigned>(nativeScenes[0],0x4c)=0;
    cursorPosition={340,260};auto package=std::make_unique<SeedPackage>(nullptr,1,4,cursorPosition,2,false,seedContext);auto* np=allocate(1684);std::memset(np,0,1684);function<void*(__thiscall*)(void*,void*,unsigned,unsigned,const Point2*,unsigned,bool)>(0x5e130)(np,nullptr,1,4,&cursorPosition,2,false);
    package->clicked();call(0x5d660,np);++seedPackageActions;checkSeed(*package,np);
    profile.ints["fcu"]=nativeProfile.ints["fcu"]=0;profile.ints["fua"]=nativeProfile.ints["fua"]=0;profile.uints["FFC0"]=nativeProfile.uints["FFC0"]=0;profile.uints["FFS0"]=nativeProfile.uints["FFS0"]=3;
    profile.floats["FFP0"]=nativeProfile.floats["FFP0"]=100;profile.floats["FFW0"]=nativeProfile.floats["FFW0"]=0;profile.floats["FFF0"]=nativeProfile.floats["FFF0"]=100;
    auto plot=std::make_unique<FarmField>(nullptr,0,0,0,fieldContext);auto* nf=allocate(2384);std::memset(nf,0,2384);fieldNativeIds[nf]=0;function<void*(__thiscall*)(void*,void*,unsigned,unsigned,int)>(0x3c120)(nf,nullptr,0,0,0);
    const Point2 position{540,300};plot->setPosition(position);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(nf,&position);
    fieldWorld.fields={plot.get()};std::array<void*,1> fields{nf};field<void*>(nativeScenes[0],0x7c)=fields.data();field<void*>(nativeScenes[0],0x80)=field<void*>(nativeScenes[0],0x84)=fields.data()+1;plot->setWater(0);function<void(__thiscall*)(void*,float)>(0x37dc0)(nf,0);checkField(*plot,nf);
    for(unsigned variant=0;variant<3;++variant) {
        plot->active=bool(field<unsigned char>(nf,0x510)=variant!=2);plot->setCulture(variant==1 ? 1:0);function<void(__thiscall*)(void*,unsigned)>(0x3b5f0)(nf,variant==1 ? 1:0);
        auto* found=fieldWorld.graph.fieldForSeedTutorial();auto* nativeFound=function<void*(__thiscall*)(void*)>(0x2e7c0)(nativeScenes[0]);require((found==plot.get() && nativeFound==nf) || (!found && !nativeFound),"Actual Farm seed tutorial field query differs");++seedPackageActions;checkField(*plot,nf);
    }
    plot->active=true;field<unsigned char>(nf,0x510)=1;
    unsigned rejected=0;for(unsigned i=0;i<16;++i) {
        const Point2 point{position[0]+float(int(i%4)-2)*10,position[1]+float(int(i/4)-2)*10};package->setPosition(point);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(np,&point);
        const bool actual=seedWorld.canPlaceSeed(*package,point),expected=function<unsigned char(__thiscall*)(void*,void*,const Point2*)>(0x2ce30)(nativeScenes[0],np,&point)!=0;require(actual==expected,"Actual seed/Field placement masks differ");if(!actual) ++rejected;++seedPackageActions;checkSeed(*package,np);
    }require(rejected,"Seed placement did not reject actual Field mask overlap");const Point2 back{340,260};package->setPosition(back);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(np,&back);
    productionTutorial->tutorialEnabled=true;field<unsigned char>(nativeProductionTutorial,0xc09)=1;productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
    for(unsigned step:{21U,22U,23U}) profile.booleans["tip"+std::to_string(step)]=nativeProfile.booleans["tip"+std::to_string(step)]=step==21;
    elapsed=.05f;plot->actions().step();call(0x153b0,static_cast<unsigned char*>(nf)+0x504);++seedPackageActions;checkField(*plot,nf);checkSeed(*package,np);
    require(productionTutorial->currentStep==22 && field<int>(nativeProductionTutorial,0xbfc)==22,"Actual Field did not anchor tutorial22 to actual SeedPackage");
    auto& tip=productionTutorial->action();auto* nativeTip=field<void*>(nativeProductionTutorial,0xbf4);mainMenuCompareView(tip,nativeTip);tip.hide();call(0xd3720,nativeTip);productionTutorial->finishTip(tip);function<void(__thiscall*)(void*,void*)>(0xd0c30)(nativeProductionTutorial,nativeTip);checkField(*plot,nf);
    productionTutorial->tutorialEnabled=false;field<unsigned char>(nativeProductionTutorial,0xc09)=0;
    fieldWorld.fields.clear();for(unsigned offset:{0x78U,0x7cU,0x80U,0x84U}) field<unsigned>(nativeScenes[0],offset)=0;
    const auto beforePlot=textureCounts();plot.reset();const auto middlePlot=textureCounts();call(0x36340,nf);compareReferenceDeltas(beforePlot,middlePlot);
    const auto beforePackage=textureCounts();package.reset();const auto middlePackage=textureCounts();call(0x5ce00,np);compareReferenceDeltas(beforePackage,middlePackage);++seedPackageObjects;
    fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;if(!wasShown) {farm.hide();call(0xd3720,nativeScenes[0]);}farm.setParent(parent);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nativeParent);
}
{
    stage=755;require(!BuySeeds::instance(),"Seed factory fixture requires no prior BuySeeds singleton");auto* previousPanel=field<void*>(image,0x25ff64);putPointer(0x25ff64,nullptr);
    struct SeedPurchaseFixture final:SeedPurchaseWorld {
        SeedFixture& seed;SeedPackageContext& context;GameMenu& menu;SeedPackage* created=nullptr;
        SeedPurchaseFixture(SeedFixture& s,SeedPackageContext& c,GameMenu& h):seed(s),context(c),menu(h) {}
        GameMenu& hud() override {return menu;}TutorialManager& tutorial() override {return seed.tutorial();}int money() override {return seed.money();}
        void chooseSeed(unsigned kind) override {created=createFarmSeed(kind,context);}
    } purchase(seedWorld,seedContext,*hud);
    BuySeedsContext purchaseContext{manager,settings,strings,input,interaction,profile,configuration,audio,purchase,views,controls};
    auto panel=std::make_unique<BuySeeds>(nullptr,purchaseContext);auto* nativePanel=allocate(1332);std::memset(nativePanel,0,1332);function<void*(__thiscall*)(void*,void*)>(0x20cb0)(nativePanel,nullptr);
    for(unsigned i=1;i<8;++i) profile.uints["SeedAvail"+std::to_string(i)]=nativeProfile.uints["SeedAvail"+std::to_string(i)]=1;
    profile.ints["Money"]=nativeProfile.ints["Money"]=10000;cursorPosition={340,260};panel->open();call(0x20f50,nativePanel);
    for(unsigned tick=0;tick<3;++tick) {elapsed=.25f;panel->absoluteStep();call(0x148e0,nativePanel);}require(panel->movement==0 && field<unsigned>(nativePanel,0x248)==0,"Actual BuySeeds opening did not finish");
    auto& chosen=*panel->cells()[0].button;cursorPosition={chosen.geometry().x,chosen.geometry().y};chosen.clicked();call(0xf41f0,field<void*>(nativePanel,0x4dc));chosen.unclicked();call(0xf4620,field<void*>(nativePanel,0x4dc));++seedPackageActions;
    auto* created=purchase.created;auto* nativeCreated=field<void*>(image,0x25f724);require(created && nativeCreated,"Actual BuySeeds callback did not create a seed package");checkSeed(*created,nativeCreated);
    require(created->parent()==&seedWorld.farmScene().movingPicture() && field<void*>(nativeCreated,0x20)==field<void*>(nativeScenes[0],0x38),"Actual Farm seed factory parent differs");
    require(created->kind()==1 && created->count()==4 && created->status()==2 && created->unpaid && profile.ints["Money"]==10000,"Actual seed factory purchase initialization differs");
    cursorPosition={340,260};created->enabledStep();call(0x5e340,nativeCreated);checkSeed(*created,nativeCreated);
    created->clicked();call(0x5d660,nativeCreated);++seedPackageActions;checkSeed(*created,nativeCreated);require(!created->unpaid && profile.ints["Money"]==int(10000-configuration.seeds[1].price),"Actual factory seed placement payment differs");
    seedWorld.setSeedMoveActive(false);seedWorld.selected=nullptr;field<unsigned char>(image,0x25f721)=0;putPointer(0x25f724,nullptr);
    const auto beforeCreated=textureCounts();delete created;const auto middleCreated=textureCounts();call(0x5ce00,nativeCreated);compareReferenceDeltas(beforeCreated,middleCreated);++seedPackageObjects;
    fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U}) field<unsigned>(nativeScenes[0],offset)=0;
    const auto beforePanel=textureCounts();panel.reset();const auto middlePanel=textureCounts();call(0x200f0,nativePanel);compareReferenceDeltas(beforePanel,middlePanel);putPointer(0x25ff64,previousPanel);
}
// Invalid placement of a newly bought seed removes it through the actual
// Farm registry and deferred discard. The money setter is never called.
{
    stage=756;auto& farm=seedWorld.farmScene();auto* parent=farm.parent();auto* nativeParent=field<void*>(nativeScenes[0],0x20);const bool shown=farm.state().shown;
    farm.setParent(nullptr);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nullptr);farm.show();call(0xd37d0,nativeScenes[0]);farm.movement=field<unsigned>(nativeScenes[0],0x4c)=0;
    cursorPosition={790,590};auto* created=createFarmSeed(2,seedContext);function<void(__thiscall*)(void*,unsigned)>(0x2c960)(nativeScenes[0],2);auto* nativeCreated=field<void*>(image,0x25f724);checkSeed(*created,nativeCreated);
    redirect(0x32c20,reinterpret_cast<void*>(&nativeDiscard));const int money=profile.ints["Money"];created->clicked();call(0x5d660,nativeCreated);++seedPackageActions;checkSeed(*created,nativeCreated);
    require(views.discarded().size()==1 && nativeDiscards.size()==1 && views.discarded().at(created).view()==nativeDiscards.at(nativeCreated),"Rejected seed discard origin differs");require(profile.ints["Money"]==money && fieldWorld.graph.areas.empty(),"Rejected unpaid seed changed money or stayed registered");
    const auto before=textureCounts();views.deleteDiscarded();const auto middle=textureCounts();call(0x5ce00,nativeCreated);compareReferenceDeltas(before,middle);nativeDiscards.clear();++seedPackageObjects;redirect(0x32c20,reinterpret_cast<void*>(&waterNativeDiscardTask));
    if(!shown) {farm.hide();call(0xd3720,nativeScenes[0]);}farm.setParent(parent);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nativeParent);
}
const auto beforeSeedBuy=textureCounts();seedBuy.reset();const auto middleSeedBuy=textureCounts();mainMenuDeleteNative(nativeSeedBuy);compareReferenceDeltas(beforeSeedBuy,middleSeedBuy);field<void*>(nativeScenes[0],0x9c)=previousSeedBuy;
