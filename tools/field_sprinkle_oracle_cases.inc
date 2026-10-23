// Actual Field + FarmAreas + Sprinkle + emitter/particles. Only remaining
// FieldTip and Person primitives retain the explicitly observed boundaries.
stage=710;
auto buy=std::make_unique<ExtendedAnimationButton>(&fieldWorld.farmScene().origin(),manager,settings,strings,input,interaction,[&]{return productionTutorial->active();},views,controls);
auto* nb=allocate(864);std::memset(nb,0,864);function<void*(__thiscall*)(void*,void*)>(0x23550)(nb,field<void*>(nativeScenes[0],0x60));
XmlDocument farmXML(files,String("UI/GameUI/Farm.xml"));XmlNode* buyXML=nullptr;require(farmXML.section(String("SeedsBuyButton"),buyXML),"Actual Farm seeds button XML missing");buy->loadXML(*buyXML);reinterpret_cast<void(__thiscall*)(void*,void*)>(field<void**>(nb,0)[6])(nb,wrap(buyXML));
auto* priorBuy=field<void*>(nativeScenes[0],0x9c);field<void*>(nativeScenes[0],0x9c)=nb;
struct ConnectedSprinkleWorld final:SprinkleWorld {
    FieldFixture& field;ExtendedAnimationButton& buy;ViewRegistry& views;
    ConnectedSprinkleWorld(FieldFixture& f,ExtendedAnimationButton& b,ViewRegistry& v):field(f),buy(b),views(v) {}
    TutorialManager& tutorial() override {return field.tutorial();}void registerArea(ActionArea& a) override {field.registerArea(a);}void queueAreaAction(ActionArea& a,const Point2& p) override {field.queueAreaAction(a,p);}
    int warehouseLevel() override {return field.warehouseLevel();}int digBuildingLevel() override {return field.digBuildingLevel();}int mowBuildingLevel() override {return field.mowBuildingLevel();}int waterSourceLevel() override {return field.waterSourceLevel();}
    bool sprinkleMoveActive() override {return field.water.sprinkleMove;}void setSprinkleMoveActive(bool v) override {field.water.sprinkleMove=v;}
    ActionArea* areaAt(const Point2& p,unsigned mask) override {return field.graph.topAt(p,mask);}bool buildingAt(const Point2& p) override {return field.graph.firstAt(p,1)!=nullptr;}
    std::vector<ActionArea*> areasAt(const Point2& p,unsigned mask) override {std::vector<ActionArea*> result;field.graph.appendAt(p,mask,result);return result;}
    bool canPlaceSprinkle(Sprinkle& s,const Point2& p) override {return field.graph.canPlaceSprinkle(&s,p);}
    std::vector<ActionArea*> wateredFields(const Point2& p,float radius) override {return field.graph.wateredFields(p,radius);}
    void clearWaterPreview() override {field.graph.clearWaterPreview();}void previewWater(ActionArea& a,bool v) override {static_cast<FarmField&>(a).preview(v);}
    double addWater(ActionArea& a,float amount) override {return static_cast<FarmField&>(a).addWater(amount);}
    void setFarmActionButtonEnabled(bool v) override {if(v) buy.enable();else buy.disable();}
    int money() override {return field.water.money();}void setMoney(int value) override {field.water.setMoney(value);}
    void removeSprinkle(Sprinkle& s) override {field.graph.remove(s,views);}
} connectedWorld(fieldWorld,*buy,views);
SprinkleContext connectedContext{{manager,settings,strings,input,interaction,connectedWorld,views,controls},profile,fieldRandom,connectedWorld};
profile.floats["SPXPos0"]=nativeProfile.floats["SPXPos0"]=320;profile.floats["SPYPos0"]=nativeProfile.floats["SPYPos0"]=250;profile.booleans["SPExist0"]=nativeProfile.booleans["SPExist0"]=true;
auto sprinkler=std::make_unique<Sprinkle>(nullptr,0,0,connectedContext);auto* nspr=allocate(1380);std::memset(nspr,0,1380);function<void*(__thiscall*)(void*,void*,unsigned,unsigned)>(0x60180)(nspr,nullptr,0,0);
const auto checkConnected=[&] {
    checkField(*f,n);stage=713;compareControl(*sprinkler,nspr);mainMenuCompareView(*sprinkler,nspr);auto* ne=field<void*>(nspr,0x520);compareControl(sprinkler->effect(),ne);
    require(sprinkler->fields().size()==std::size_t((field<unsigned>(nspr,0x52c)-field<unsigned>(nspr,0x528))/4),"Actual Field/Sprinkle affected fields differ");
    require(sprinkler->effect().emission.generating==bool(field<unsigned char>(ne,0x250)),"Connected Sprinkle generation differs");
    stage=714;mainMenuCompareView(*buy,nb);compareControl(*buy,nb);
};checkConnected();require(sprinkler->fields().size()==1,"Connected Sprinkle did not select the actual Field");
for(float initial:{0.f,5.f,119.f,120.f}) for(float dt:{0.f,.017f,.05f,1.f}) {
    stage=711;f->setWater(initial);function<void(__thiscall*)(void*,float)>(0x37dc0)(n,initial);elapsed=dt;sprinkler->enabledStep();call(0x60450,nspr);++farmFieldActions;checkConnected();
}
// Include the actual Warehouse as a collidable area. Field and Sprinkle
// types are excluded by the original placement rules; Warehouse is not.
fieldWorld.graph.registerArea(*warehouse);function<void(__thiscall*)(void*,void*)>(0x31530)(nativeScenes[0],nw);unsigned blocked=0;
for(unsigned i=0;i<16;++i) {
    stage=715;const Point2 point{warehouse->geometry().x+float(int(i%4)-2)*20,warehouse->geometry().y+float(int(i/4)-2)*20};
    sprinkler->setPosition(point);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(nspr,&point);
    const bool actual=fieldWorld.graph.canPlaceSprinkle(sprinkler.get(),point),expected=function<unsigned(__thiscall*)(void*,void*,const Point2*)>(0x2d290)(nativeScenes[0],nspr,&point)!=0;
    require(actual==expected,"Actual Farm Warehouse/Sprinkle placement masks differ");if(!actual) ++blocked;++farmFieldActions;checkConnected();
}require(blocked,"Farm placement did not test a rejected Warehouse overlap");
fieldWorld.graph.areas.pop_back();field<unsigned>(nativeScenes[0],0x70)-=4;
const Point2 restored{320,250};sprinkler->setPosition(restored);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(nspr,&restored);
sprinkler->setStatus(2);function<void(__thiscall*)(void*,unsigned)>(0x5f3d0)(nspr,2);checkConnected();cursorPosition={330,250};sprinkler->enabledStep();call(0x60450,nspr);checkConnected();require(f->hiddenPicture().state().shown,"Actual Field water preview was not shown");
sprinkler->setStatus(0);function<void(__thiscall*)(void*,unsigned)>(0x5f3d0)(nspr,0);checkConnected();
{
    const auto savedHover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
    menuMilliseconds=0;GameTimer coupledTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor coupledCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&coupledCursor;
    std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    for(unsigned frame=0;frame<16;++frame) {stage=712;action=frame;menuMilliseconds+=50;menuPoint={790,590};menuMouseDelta={};menuInputEvents.clear();const auto framePixels=compareMenuFrame(d3d,device,window,views,controls,input,0);farmFieldPixels+=framePixels;fieldTipPixels+=framePixels;++farmFieldFrames;++fieldTipFrames;checkConnected();}
    interaction.playHover=savedHover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
}
const auto before=textureCounts();sprinkler.reset();buy.reset();views.deleteDiscarded();const auto middle=textureCounts();call(0x5e9d0,nspr);mainMenuDeleteNative(nb);compareReferenceDeltas(before,middle);nativeDiscards.clear();field<void*>(nativeScenes[0],0x9c)=priorBuy;
fieldWorld.graph.areas={f.get()};
