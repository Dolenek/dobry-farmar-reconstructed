stage=700;loadingFiles=&files;managerTable[6]=reinterpret_cast<void*>(&loadingXML);xmlTable[0]=reinterpret_cast<void*>(&loadingDeleteXML);xmlTable[34]=reinterpret_cast<void*>(&sectionCount);redirect(0x185a13,reinterpret_cast<void*>(&gameplayGetThreadData));
const auto discardFieldParticle=+[](void* p,const String* origin) {nativeDiscards[p]=std::string(origin->view());};redirect(0x62af0,reinterpret_cast<void*>(discardFieldParticle));
redirect(0x2c9e0,reinterpret_cast<void*>(&fieldIndicate));redirect(0x2c800,reinterpret_cast<void*>(&fieldClearIndication));
// Numeric configuration is adapted data, using the previously native-verified
// XML configuration. The original two product getters execute their real body.
for(unsigned i=0;i<19;++i) {field<float>(nativeGame,0x30+i*100)=configuration.products[i].dieTime;field<float>(nativeGame,0x30+i*100+96)=configuration.products[i].creationTime;}
void* priorNativeFieldTip=field<void*>(image,0x25f774);putPointer(0x25f774,nullptr);
FieldTipContext fieldTipContext{manager,settings,strings,configuration,views,controls};auto fieldTip=std::make_unique<FieldTip>(nullptr,fieldTipContext);auto* nativeFieldTip=allocate(1012);std::memset(nativeFieldTip,0,1012);function<void*(__thiscall*)(void*,void*)>(0x41360)(nativeFieldTip,nullptr);
// Keep the proven StateView scene dispatch, with the four actual Farm flag
// accessors used by Field input. Complete Farm UI composition is still pending.
auto* priorFieldFarmVtable=field<void**>(nativeScenes[0],0);std::array<void*,19> fieldFarmVtable{};std::copy_n(priorFieldFarmVtable,15,fieldFarmVtable.begin());
for(unsigned i=0;i<4;++i) fieldFarmVtable[15+i]=image+0x34c70+i*16;field<void*>(nativeScenes[0],0)=fieldFarmVtable.data();field<unsigned char>(nativeScenes[0],0x108)=0;
struct FieldFixture final:FarmFieldWorld {
    WaterFixture& water;PersonTasks* tasks=nullptr;FarmAreas graph;std::vector<FarmField*>& fields;bool held=false;
    explicit FieldFixture(WaterFixture& w):water(w),fields(graph.fields) {}
    TutorialManager& tutorial() override {return water.tutorial();}StateView& farmScene() override {return water.farmScene();}
    void registerArea(ActionArea& a) override {water.registerArea(a);graph.registerArea(a);}void queueAreaAction(ActionArea& a,const Point2& p) override {if(tasks) tasks->add(a,p);else water.queueAreaAction(a,p);}
    int warehouseLevel() override {return water.warehouseLevel();}int digBuildingLevel() override {return water.digBuildingLevel();}int mowBuildingLevel() override {return water.mowBuildingLevel();}int waterSourceLevel() override {return water.waterSourceLevel();}
    bool hasDryField() override {return graph.hasDryField();}
    bool farmClickHeld() override {return held;}void setFarmClickHeld(bool value) override {held=value;}
    bool areaGate(unsigned type) override {return water.cursorOverAreaType(type,nullptr);}
    ActionArea* firstAreaAt(const Point2& p,unsigned mask) override {return graph.firstAt(p,mask);}
    void indicateField(FarmField& f) override {fieldSourceEvents.push_back({4,f.id,0});}void clearFieldIndication(FarmField& f) override {fieldSourceEvents.push_back({5,f.id,0});}
    std::vector<ActionArea*> seedPackages() override {return graph.seedPackages();}
} fieldWorld(waterWorld);
std::memcpy(static_cast<unsigned char*>(nativeScenes[0])+0xb0,fieldWorld.graph.bounds.data(),16);
GameRandom fieldRandom(0x436120);field<unsigned>(gameplayThreadData.data(),0x14)=fieldRandom.state();
FarmFieldContext fieldContext{{manager,settings,strings,input,interaction,fieldWorld,views,controls},profile,configuration,fieldRandom,fieldWorld};
for(unsigned offset:{0x78U,0x7cU,0x80U,0x84U}) field<unsigned>(nativeScenes[0],offset)=0;
const auto checkFieldTip=[&] {
    stage=730;auto& t=*fieldTip;auto* nt=nativeFieldTip;mainMenuCompareView(t,nt);compareControl(t,nt);compareControl(t.origin(),field<void*>(nt,0x26c));
    require(t.movement==field<unsigned>(nt,0x248),"FieldTip movement differs");compareFloat(t.moveSpeed,field<float>(nt,0x254),"FieldTip move speed");compareFloat(t.remaining,field<float>(nt,0x258),"FieldTip movement timer");
    for(unsigned d=0;d<2;++d) {compareFloat(t.hiddenPlace[d],field<float>(nt,0x25c+d*4),"FieldTip hidden place");compareFloat(t.originalPosition[d],field<float>(nt,0x264+d*4),"FieldTip original position");}
    auto* selected=field<void*>(nt,0x2a4);require(bool(t.selected)==bool(selected),"FieldTip selection presence differs");if(selected) require(t.selected->id==fieldNativeIds.at(selected),"FieldTip selected owner differs");
    require(t.showPending==bool(field<unsigned char>(nt,0x2ac)),"FieldTip pending flag differs");compareFloat(t.hideRemaining,field<float>(nt,0x2a8),"FieldTip hide timer");if(t.showPending) compareFloat(t.showRemaining,field<float>(nt,0x2b0),"FieldTip show timer");
    for(auto row:std::initializer_list<std::pair<ProgressBar*,unsigned>>{{&t.waterBar(),0x280},{&t.fertilBar(),0x284},{&t.readyBar(),0x288}}) {
        auto* bar=field<void*>(nt,row.second);compareControl(*row.first,bar);mainMenuCompareView(*row.first,bar);compareFloat(row.first->maximum,field<float>(bar,0x238),"FieldTip bar maximum");compareFloat(row.first->current,field<float>(bar,0x23c),"FieldTip bar current");require(row.first->side==field<unsigned>(bar,0x240),"FieldTip bar side differs");
    }
    compare(t.cultureText(),field<void*>(nt,0x28c));compareControl(t.cultureIcon(),field<void*>(nt,0x290));mainMenuCompareView(t.cultureIcon(),field<void*>(nt,0x290));
    auto** first=field<void**>(nt,0x298);auto** last=field<void**>(nt,0x29c);require(t.labels().size()==std::size_t(last-first),"FieldTip labels count differs");for(unsigned i=0;i<t.labels().size();++i) compare(*t.labels()[i],first[i]);
    for(unsigned i=0;i<8;++i) appearanceCheck(t.icons[i],static_cast<unsigned char*>(nt)+0x2b4+i*40);++fieldTipStates;
};
checkFieldTip();
const auto checkField=[&](FarmField& f,void* n) {
    stage=701;mainMenuCompareView(f,n);compareControl(f,n);require(f.id==field<unsigned>(n,0x508) && f.upgradeIndex==field<int>(n,0x50c) && f.active==bool(field<unsigned char>(n,0x510)),"FarmField identity/activation differs");
    for(auto pair:std::initializer_list<std::pair<float,unsigned>>{{f.waterCapacity,0x8a8},{f.fertilCapacity,0x8ac},{f.waterSignLevel,0x8b0},{f.fertilSignLevel,0x8b4},{f.waterAbsorption,0x8b8},{f.fertilAbsorption,0x8bc},{f.ploughedToGrass,0x8c0}}) compareFloat(pair.first,field<float>(n,pair.second),"FarmField numeric properties");
    require(f.digCost==field<unsigned>(n,0x8a0) && f.beginGrowFrame==field<unsigned>(n,0x8a4) && f.harvested==bool(field<unsigned char>(n,0x944)),"FarmField controls/harvest state differs");
    for(auto pair:std::initializer_list<std::pair<Picture*,unsigned>>{{&f.waterSign(),0x890},{&f.readySign(),0x894},{&f.fertilSign(),0x898},{&f.hiddenPicture(),0x89c}}) {stage=720+(pair.second-0x890)/4;auto* nn=field<void*>(n,pair.second);mainMenuCompareView(*pair.first,nn);compareControl(*pair.first,nn);}stage=701;
    for(unsigned i=0;i<8;++i) {appearanceCheck(f.cultureAppearances[i],static_cast<unsigned char*>(n)+0x590+i*40);auto* info=static_cast<unsigned char*>(n)+0x6d0+i*56;const auto& s=f.cultures[i];require(s.firstFrames==field<unsigned>(info,0) && s.secondFrames==field<unsigned>(info,4) && s.notGrowFrame==field<unsigned>(info,20) && s.mask.view()==field<String>(info,28).view(),"FarmField culture visual metadata differs");for(auto p:std::initializer_list<std::pair<float,unsigned>>{{s.firstTime,8},{s.secondTime,12},{s.lastTime,16},{s.notGrowProgress,24}}) compareFloat(p.first,field<float>(info,p.second),"FarmField culture times");}
    if(profile.ints!=nativeProfile.ints || profile.uints!=nativeProfile.uints || profile.floats!=nativeProfile.floats || profile.booleans!=nativeProfile.booleans) {
        auto* task=field<void*>(n,0x504);std::cerr<<"field divergence kind/state "<<f.culture()<<'/'<<f.actionState()<<" elapsed "<<currentElapsed()<<" animation time/length "<<f.animation().time<<'/'<<f.animation().length<<" native "<<field<float>(n,0x1ac)<<'/'<<field<float>(n,0x38)<<" creation "<<configuration.products[f.culture()].creationTime<<'/'<<field<float>(nativeGame,0x30+f.culture()*100+96)<<" task "<<(task ? field<void*>(task,0):nullptr)<<'\n';
        const auto differences=[](const auto& a,const auto& b,const char* type) {for(const auto& x:a) {const auto y=b.find(x.first);if(y==b.end() || y->second!=x.second) std::cerr<<"source "<<type<<' '<<x.first<<' '<<x.second<<'\n';}for(const auto& x:b) {const auto y=a.find(x.first);if(y==a.end() || y->second!=x.second) std::cerr<<"native "<<type<<' '<<x.first<<' '<<x.second<<'\n';}};
        differences(profile.ints,nativeProfile.ints,"int");differences(profile.uints,nativeProfile.uints,"uint");differences(profile.floats,nativeProfile.floats,"float");differences(profile.booleans,nativeProfile.booleans,"bool");
    }
    require(profile.ints==nativeProfile.ints && profile.uints==nativeProfile.uints && profile.floats==nativeProfile.floats && profile.booleans==nativeProfile.booleans,"FarmField profile stores differ");
    require(fieldRandom.state()==field<unsigned>(gameplayThreadData.data(),0x14),"FarmField/action random sequence differs");require(GameAction::count()==field<unsigned>(image,0x25ff58),"GameAction original instance count differs");
    auto* task=field<void*>(n,0x504);require(bool(f.actions().current())==bool(task),"FarmField current action presence differs");if(task) require(f.actions().current()->owner==&f && field<void*>(task,8)==n && f.actions().current()->type==field<unsigned>(task,4),"FarmField action ownership/type differs");
    require(fieldSourceEvents==fieldNativeEvents,"FarmField remaining Farm/tip boundary order differs");fieldSourceEvents.clear();fieldNativeEvents.clear();require(actualSounds==expectedSounds,"FarmField sounds differ");actualSounds.clear();expectedSounds.clear();
    require(actionAreaSourceTasks==actionAreaNativeTasks,"FarmField Person queue differs");actionAreaSourceTasks.clear();actionAreaNativeTasks.clear();
    require(fieldWorld.held==bool(field<unsigned char>(nativeScenes[0],0x108)),"FarmField actual Farm held flag differs");++farmFieldStates;checkFieldTip();
};
for(unsigned kind=0;kind<8;++kind) for(unsigned state=0;state<4;++state) {
    stage=702;trial=farmFieldObjects;cursorPosition={790,590};fieldWorld.fields.clear();fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U,0x78U,0x7cU,0x80U,0x84U}) field<unsigned>(nativeScenes[0],offset)=0;
    profile.ints["fcu"]=nativeProfile.ints["fcu"]=0;profile.ints["fua"]=nativeProfile.ints["fua"]=0;profile.uints["FFC0"]=nativeProfile.uints["FFC0"]=kind;profile.uints["FFS0"]=nativeProfile.uints["FFS0"]=state;profile.floats["FFP0"]=nativeProfile.floats["FFP0"]=configuration.products[kind].creationTime;profile.floats["FFW0"]=nativeProfile.floats["FFW0"]=100;profile.floats["FFF0"]=nativeProfile.floats["FFF0"]=100;
    auto f=std::make_unique<FarmField>(nullptr,0,0,0,fieldContext);auto* n=allocate(2384);std::memset(n,0,2384);fieldNativeIds[n]=0;function<void*(__thiscall*)(void*,void*,unsigned,unsigned,int)>(0x3c120)(n,nullptr,0,0,0);checkField(*f,n);
    fieldTip->select(f.get());function<void(__thiscall*)(void*,void*)>(0x402a0)(nativeFieldTip,n);checkFieldTip();
    for(float dt:{0.f,.25f,.25f,.2f}) {elapsed=dt;fieldTip->absoluteStep();call(0x405e0,nativeFieldTip);checkFieldTip();}
    fieldTip->hideFor(nullptr);function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,nullptr);checkFieldTip();
    fieldTip->select(nullptr);function<void(__thiscall*)(void*,void*)>(0x402a0)(nativeFieldTip,nullptr);checkFieldTip();
    fieldTip->hideFor(nullptr);function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,nullptr);checkFieldTip();
    fieldWorld.fields={f.get()};std::array<void*,1> fields{n};field<void*>(nativeScenes[0],0x7c)=fields.data();field<void*>(nativeScenes[0],0x80)=field<void*>(nativeScenes[0],0x84)=fields.data()+1;
    for(unsigned sample=0;sample<32;++sample) {
        stage=706;Point2 point{float(int(sample%8)-4)*13.125f,float(int(sample/8)-2)*17.5f};const unsigned mask=sample%2 ? 1:127;
        const auto matches=[&](ActionArea* a,void* b) {require((a==f.get() && b==n) || (!a && !b),"Actual Farm area query differs");};
        matches(fieldWorld.graph.topAt(point,mask),function<void*(__thiscall*)(void*,const Point2*,unsigned)>(0x2df60)(nativeScenes[0],&point,mask));
        matches(fieldWorld.graph.firstAt(point,mask),function<void*(__thiscall*)(void*,const Point2*,unsigned)>(0x2e3a0)(nativeScenes[0],&point,mask));
        const float radius=float(sample%7)*11.5f;const auto selected=fieldWorld.graph.wateredFields(point,radius);std::array<unsigned,4> out{};function<void(__thiscall*)(void*,const Point2*,float,void*)>(0x326c0)(nativeScenes[0],&point,radius,out.data());
        require(selected.size()==std::size_t((out[2]-out[1])/4),"Actual Farm radius field count differs");if(!selected.empty()) require(selected.front()==f.get() && field<void*>(reinterpret_cast<void*>(out[1]),0)==n,"Actual Farm radius field selection differs");release(reinterpret_cast<void*>(out[1]));++farmFieldActions;checkField(*f,n);
    }
    for(unsigned step=0;step<24;++step) {stage=703;action=step;elapsed=std::array<float,6>{0,.001f,.017f,.05f,.2f,1.f}[step%6];f->absoluteStep();call(0x3b0d0,n);++farmFieldActions;checkField(*f,n);}
    // Long steps exercise actual processor completion, replacement actions,
    // water/fertil exhaustion, grass fallback and repeated ready/dead cycles.
    for(float dt:{5.f,30.f,60.f,120.f,180.f,.017f,0.f,60.f}) {stage=704;elapsed=dt;f->absoluteStep();call(0x3b0d0,n);++farmFieldActions;checkField(*f,n);}
    // SeedPackage has its own connected GPU scene. Its prefix retains all
    // CPU comparisons; the mandatory farm-field mode covers these frames.
    if(state==0 && !seedPackageMode) {
        const Point2 pos{300,250};f->setPosition(pos);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(n,&pos);
        const auto savedHover=interaction.playHover;interaction.playHover=[&](cResource* sound,bool loop,unsigned start) {audio.playSound(static_cast<cSound*>(sound),loop,bool(start));};
        menuMilliseconds=0;GameTimer fieldTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor fieldCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&fieldCursor;
        std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
        for(unsigned frame=0;frame<(kind<2 ? 24U:1U);++frame) {stage=705;action=frame;menuMilliseconds+=50;
            menuPoint=kind==0 && frame<4 ? pos:Point2{790,590};menuMouseDelta=kind==0 && (frame==0 || frame==4) ? std::array<int,3>{1,0,0}:std::array<int,3>{};
            menuInputEvents=kind==0 && frame==1 ? std::vector<unsigned>{0x208}:kind==0 && frame==3 ? std::vector<unsigned>{0x200}:std::vector<unsigned>{};
            const auto framePixels=compareMenuFrame(d3d,device,window,views,controls,input,0);farmFieldPixels+=framePixels;fieldTipPixels+=framePixels;++farmFieldFrames;++fieldTipFrames;checkField(*f,n);
        }
        fieldWorld.held=false;field<unsigned char>(nativeScenes[0],0x108)=0;
        interaction.playHover=savedHover;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    }
    if(kind==0 && state==0) {
        if(seedPackageMode) {const Point2 plotPosition{300,250};f->setPosition(plotPosition);function<void(__thiscall*)(void*,const Point2*)>(0x192a0)(n,&plotPosition);}
#include "field_sprinkle_oracle_cases.inc"
        const auto tipReloadDirectory=common/"field-tip-reload";std::filesystem::create_directories(tipReloadDirectory);
        std::ofstream(tipReloadDirectory/"partial.xml")<<"<Tip MoveSpeed='2175' HiddenPlace='-211 275'/>";files.mountCommon(String(tipReloadDirectory.generic_string()));
        for(const char* path:{"partial.xml","UI/gameUI/FarmFieldTip.xml"}) {
            stage=731;XmlDocument definition(files,String(path));fieldTip->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0x408d0)(nativeFieldTip,wrap(&definition));checkFieldTip();
            fieldTip->select(f.get());function<void(__thiscall*)(void*,void*)>(0x402a0)(nativeFieldTip,n);elapsed=.6f;fieldTip->absoluteStep();call(0x405e0,nativeFieldTip);checkFieldTip();
            fieldTip->hideFor(f.get());function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,n);checkFieldTip();
        }
    }
    if(kind==0 && state==3) {
        // 00440350 destroys the temporary vector on the stack. It is not a
        // FieldTip callback; execute its original body, including empty lists.
        auto& scene=fieldWorld.farmScene();auto* previousParent=scene.parent();auto* nativeParent=field<void*>(nativeScenes[0],0x20);
        const bool shown=scene.state().shown;scene.setParent(nullptr);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nullptr);
        scene.show();call(0xd37d0,nativeScenes[0]);scene.movement=field<unsigned>(nativeScenes[0],0x4c)=0;
        productionTutorial->tutorialEnabled=true;field<unsigned char>(nativeProductionTutorial,0xc09)=1;
        for(unsigned flags=0;flags<8;++flags) {
            stage=708;for(unsigned i=0;i<3;++i) profile.booleans["tip"+std::to_string(21+i)]=nativeProfile.booleans["tip"+std::to_string(21+i)]=bool(flags&(1U<<i));
            productionTutorial->currentStep=field<int>(nativeProductionTutorial,0xbfc)=-1;
            f->installDead(100);auto* dead=allocate(20);function<void*(__thiscall*)(void*,void*,float)>(0x36180)(dead,n,100);
            function<void(__thiscall*)(void*,void*)>(0x15790)(static_cast<unsigned char*>(n)+0x504,dead);checkField(*f,n);
            elapsed=.05f;f->actions().step();call(0x153b0,static_cast<unsigned char*>(n)+0x504);++farmFieldActions;checkField(*f,n);
            const int expected=(flags&2) && !(flags&4) ? 23:-1;
            require(productionTutorial->currentStep==expected && field<int>(nativeProductionTutorial,0xbfc)==expected,"Field dead tutorial branches differ");
            if(expected==23) {
                auto& tip=productionTutorial->action();auto* nativeTip=field<void*>(nativeProductionTutorial,0xbf4);
                mainMenuCompareView(tip,nativeTip);tip.hide();call(0xd3720,nativeTip);
                productionTutorial->finishTip(tip);function<void(__thiscall*)(void*,void*)>(0xd0c30)(nativeProductionTutorial,nativeTip);checkField(*f,n);
            }
        }
        productionTutorial->tutorialEnabled=false;field<unsigned char>(nativeProductionTutorial,0xc09)=0;
        if(!shown) {scene.hide();call(0xd3720,nativeScenes[0]);}scene.setParent(previousParent);function<void(__thiscall*)(void*,void*)>(0xd2e50)(nativeScenes[0],nativeParent);
    }
    fieldTip->hideFor(f.get());function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,n);
    const auto before=textureCounts();f.reset();const auto middle=textureCounts();call(0x36340,n);compareReferenceDeltas(before,middle);require(GameAction::count()==field<unsigned>(image,0x25ff58),"FarmField action destructor count differs");++farmFieldObjects;
}
for(int current:{-1,0,1,2}) for(int available:{1,3}) {
    stage=707;trial=farmFieldObjects;fieldWorld.fields.clear();fieldWorld.graph.areas.clear();for(unsigned offset:{0x68U,0x6cU,0x70U,0x74U,0x78U,0x7cU,0x80U,0x84U}) field<unsigned>(nativeScenes[0],offset)=0;
    profile.ints["fcu"]=nativeProfile.ints["fcu"]=current;profile.ints["fua"]=nativeProfile.ints["fua"]=available;
    profile.uints["FFC0"]=nativeProfile.uints["FFC0"]=0;profile.uints["FFS0"]=nativeProfile.uints["FFS0"]=0;profile.floats["FFP0"]=nativeProfile.floats["FFP0"]=configuration.products[0].creationTime;profile.floats["FFW0"]=nativeProfile.floats["FFW0"]=100;profile.floats["FFF0"]=nativeProfile.floats["FFF0"]=100;
    auto f=std::make_unique<FarmField>(nullptr,0,0,2,fieldContext);auto* n=allocate(2384);std::memset(n,0,2384);fieldNativeIds[n]=0;function<void*(__thiscall*)(void*,void*,unsigned,unsigned,int)>(0x3c120)(n,nullptr,0,0,2);checkField(*f,n);
    require(f->active==(current>=2),"FarmField locked/purchase/active initialization differs");
    for(float dt:{0.f,.05f,1.f}) {elapsed=dt;f->absoluteStep();call(0x3b0d0,n);checkField(*f,n);++farmFieldActions;}
    fieldTip->hideFor(f.get());function<void(__thiscall*)(void*,void*)>(0x402d0)(nativeFieldTip,n);
    const auto before=textureCounts();f.reset();const auto middle=textureCounts();call(0x36340,n);compareReferenceDeltas(before,middle);++farmFieldObjects;
}
fieldWorld.fields.clear();for(unsigned offset:{0x78U,0x7cU,0x80U,0x84U}) field<unsigned>(nativeScenes[0],offset)=0;field<void*>(nativeScenes[0],0)=priorFieldFarmVtable;fieldNativeIds.clear();

if(seedPackageMode && !personVisualsMode) {
#include "seed_package_oracle_cases.inc"
}
if(personVisualsMode) {
#include "person_visuals_oracle_cases.inc"
}
const auto tipBefore=textureCounts();fieldTip.reset();const auto tipMiddle=textureCounts();call(0x415e0,nativeFieldTip);compareReferenceDeltas(tipBefore,tipMiddle);putPointer(0x25f774,priorNativeFieldTip);
