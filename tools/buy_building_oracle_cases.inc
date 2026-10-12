stage=390;for(const auto& entry:tutorialEntries) write(entry.first,entry.second.data(),5);
redirect(0xd0070,reinterpret_cast<void*>(&nativeDiscard));redirect(0x180ed3,reinterpret_cast<void*>(&tutorialVectorDestroy));redirect(0x2fe80,reinterpret_cast<void*>(&decimalString));redirect(0x1cc0,reinterpret_cast<void*>(&concatenateString));putPointer(0x25f73c,nullptr);
Control buildingAnchor(nullptr,manager,views,controls);TutorialWorldFixture buildingTutorialWorld(world,buildingAnchor);
TutorialManagerContext buildingTutorialContext{manager,settings,strings,input,interaction,profile,buildingTutorialWorld,views,controls};
auto* buildingTutorial=new TutorialManager(buildingTutorialContext);world.tutorial=buildingTutorial;auto* nativeBuildingTutorial=allocate(3104);std::memset(nativeBuildingTutorial,0,3104);field<int>(nativeBuildingTutorial,0xc00)=-1;function<void*(__thiscall*)(void*)>(0xcfce0)(nativeBuildingTutorial);buildingTutorial->disable();call(0xd3750,nativeBuildingTutorial);
BuildingWorldFixture buildingWorld(*hud,*buildingTutorial,profile);BuyBuildingContext buildingContext{manager,settings,strings,input,interaction,profile,configuration,audio,buildingWorld,views,controls};
putPointer(0x260010,nullptr);auto* buildingPanel=new BuyBuilding(nullptr,buildingContext);auto* nativeBuildingPanel=allocate(1700);std::memset(nativeBuildingPanel,0,1700);function<void*(__thiscall*)(void*,void*)>(0x91690)(nativeBuildingPanel,nullptr);
world.overlays[0]=buildingPanel;redirect(0x94390,reinterpret_cast<void*>(&buildingNativeChoose));
for(unsigned i=0;i<15;++i) for(unsigned j=0;j<4;++j) field<unsigned>(nativeGame,0x8fc+(i*4+j)*4)=configuration.costs[i][j];
const auto buildingCheck=[&] {
    stage=391;require(BuyBuilding::instance()==buildingPanel && field<void*>(image,0x260010)==nativeBuildingPanel,"BuyBuilding singleton differs");mainMenuCompareView(*buildingPanel,nativeBuildingPanel);compareControl(*buildingPanel,nativeBuildingPanel);compareControl(buildingPanel->origin(),field<void*>(nativeBuildingPanel,0x26c));
    require(buildingPanel->movement==field<unsigned>(nativeBuildingPanel,0x248),"BuyBuilding movement differs");compareFloat(buildingPanel->remaining,field<float>(nativeBuildingPanel,0x258),"BuyBuilding remaining");
    for(unsigned d=0;d<2;++d) {compareFloat(buildingPanel->originalPosition[d],field<float>(nativeBuildingPanel,0x264+d*4),"BuyBuilding original position");compareFloat(buildingPanel->hiddenPlace[d],field<float>(nativeBuildingPanel,0x25c+d*4),"BuyBuilding hidden position");}
    for(unsigned i=0;i<11;++i) {
        const auto& def=buildingPanel->definitions[i];auto* n=static_cast<unsigned char*>(nativeBuildingPanel)+0x2d8+i*68;require(def.name.view()==field<WString>(n,0).view(),"BuyBuilding definition name differs");const auto& a=def.appearance;require(a.texture==field<cTexture*>(n,0x20) && a.frames==field<unsigned>(n,0x24) && a.columns==field<unsigned>(n,0x30) && a.rows==field<unsigned>(n,0x34),"BuyBuilding definition appearance differs");
        const auto& cell=buildingPanel->cells()[i];auto* button=field<void*>(nativeBuildingPanel,0x5c8+i*20);auto* mask=field<void*>(nativeBuildingPanel,0x5cc+i*20);auto* price=field<void*>(nativeBuildingPanel,0x5d0+i*20);auto* name=field<void*>(nativeBuildingPanel,0x5d4+i*20);
        for(unsigned d=0;d<2;++d) compareFloat(buildingPanel->places[i][d],field<float>(nativeBuildingPanel,0x280+i*8+d*4),"BuyBuilding item place");compareControl(*cell.button,button);require(cell.button->building==field<unsigned>(button,0x280) && unsigned(cell.button->buttonState())==field<unsigned>(button,0x238),"BuyBuilding button identity/state differs");require(cell.button->deepProcessing==bool(field<unsigned char>(button,0x26a)) && cell.button->dirty()==bool(field<unsigned char>(button,0x1b0)),"BuyBuilding button deep/dirty flags differ");compareControl(cell.button->overlay(),field<void*>(button,0x27c));
        compareControl(*cell.price,price);buyBuildingLetters+=compare(*cell.price,price);compareControl(cell.price->coin(),field<void*>(price,0x2bc));compareControl(*cell.name,name);buyBuildingLetters+=compare(*cell.name,name);
        compareControl(*cell.mask,mask);require(unsigned(cell.mask->buttonState())==field<unsigned>(mask,0x238),"BuyBuilding mask button state differs");
        require(cell.mask->tipsDisabled==bool(field<unsigned char>(mask,0x27c)) && cell.mask->tipsDirection==field<int>(mask,0x2b0),"BuyBuilding mask tip direction differs");
        compareFloat(cell.mask->waitTime,field<float>(mask,0x2a0),"BuyBuilding mask wait");compareFloat(cell.mask->waitRemaining,field<float>(mask,0x2a4),"BuyBuilding mask wait remaining");compareFloat(cell.mask->alphaChangeTime,field<float>(mask,0x2a8),"BuyBuilding mask fade");compareFloat(cell.mask->tipsAlpha,field<float>(mask,0x2ac),"BuyBuilding mask alpha");
        auto** begin=field<void**>(mask,0x284);auto** end=field<void**>(mask,0x288);require(cell.mask->tips().size()==std::size_t(end-begin),"BuyBuilding mask tip count differs");
        auto* layouts=field<unsigned char*>(mask,0x294);require(cell.mask->tipLayouts().size()==std::size_t(field<unsigned char*>(mask,0x298)-layouts)/12,"BuyBuilding mask layout count differs");for(std::size_t j=0;j<cell.mask->tips().size();++j) {compareControl(*cell.mask->tips()[j],begin[j]);require(!std::memcmp(&cell.mask->tipLayouts()[j],layouts+j*12,12),"BuyBuilding mask saved layout differs");}
    }
    require(buildingPanel->buySound()==field<cSound*>(nativeBuildingPanel,0x6a0),"BuyBuilding sound ownership differs");require(buildingSourceChoices==buildingNativeChoices,"BuyBuilding Manufacture choice dispatch differs");require(buildingTutorial->currentStep==field<int>(nativeBuildingTutorial,0xbfc) && buildingTutorial->justProcessed==field<int>(nativeBuildingTutorial,0xc00),"BuyBuilding tutorial progression differs");for(unsigned j=0;j<5;++j) mainMenuCompareView(*std::array<TutorialCommonTip*,5>{&buildingTutorial->common(),&buildingTutorial->arrow(),&buildingTutorial->disabledAnchor(),&buildingTutorial->action(),&buildingTutorial->repeatedAction()}[j],field<void*>(nativeBuildingTutorial,0xbe8+j*4));check();++buyBuildingStates;
};buildingCheck();
buildingTutorial->tutorialEnabled=false;field<unsigned char>(nativeBuildingTutorial,0xc09)=0;
const char* shardText=std::getenv("FARMAR_BUILDING_TEST_SHARD");const unsigned buildingShard=shardText ? unsigned(std::strtoul(shardText,nullptr,10)):0;require(buildingShard<16,"BuyBuilding diagnostic shard out of range");
// The existing shared source/native monotonic diagnostic heap is 512 MiB.
// Partition all 2048 masks across sixteen independent processes; do not let heap
// exhaustion replace any production behavior. Validator requires all shards.
for(trial=buildingShard*128;trial<(buildingShard+1)*128;++trial) {
    stage=392;const unsigned mask=(trial*67)&2047;
    for(unsigned i=0;i<11;++i) profile.uints["UpAvail"+std::to_string(i)]=nativeProfile.uints["UpAvail"+std::to_string(i)]=(mask&(1U<<i)) ? 0:0xffffffffU;
    profile.ints["Money"]=nativeProfile.ints["Money"]=trial%4 ? int(trial*231):-1;
    hud->disable();hud->absoluteDisable();call(0xd3750,original);call(0xd3790,original);buildingPanel->open();call(0x928e0,nativeBuildingPanel);++buyBuildingStarts;buildingCheck();
    explicitTipElapsed=true;elapsed=0.25f;for(unsigned i=0;i<3;++i) {buildingPanel->absoluteStep();call(0x148e0,nativeBuildingPanel);buildingCheck();}explicitTipElapsed=false;
    if(trial%3==0) {buildingPanel->cancel();hud->enable();hud->absoluteEnable();call(0xd3bb0,original);call(0xd3fb0,original);call(0x8e130,nativeBuildingPanel);}else {buildingPanel->choose(*buildingPanel->cells()[trial%11].button);function<void(__cdecl*)(void*)>(0x8e980)(field<void*>(nativeBuildingPanel,0x5c8+(trial%11)*20));++buyBuildingChoices;}buildingCheck();actualSounds.clear();expectedSounds.clear();tutorialInputs.wareSelection=false;field<unsigned>(image,0x260150)=0;
}
// Actual dispatcher, D3D8 drawing, hover tooltips and first/last building choice.
if(buildingShard==0) {
auto* savedBuildingCursor=menuCursor;auto* savedNativeBuildingCursor=nativeCursorFrame;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);
buildingPanel->setPosition(buildingPanel->originalPosition);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(nativeBuildingPanel,&buildingPanel->originalPosition);buildingPanel->enable();call(0xd3bb0,nativeBuildingPanel);
for(trial=0;trial<80;++trial) {cursorPosition={float(int(trial%10)*89-1),float(int(trial/10)*79-1)};require(buildingPanel->cursorInside()==function<bool(__thiscall*)(void*)>(0x8e550)(nativeBuildingPanel),"BuyBuilding cursor rectangle differs");buildingCheck();}
menuCursor=savedBuildingCursor;nativeCursorFrame=savedNativeBuildingCursor;putPointer(0x25ff60,nativeCursorFrame);
for(unsigned i=0;i<11;++i) profile.uints["UpAvail"+std::to_string(i)]=nativeProfile.uints["UpAvail"+std::to_string(i)]=0;
// isProcessed short-circuits the second key; disabled tutorial still performs
// the calls and then retains normal prices. Compare all combinations in profile.
for(trial=0;trial<8;++trial) {
    buildingTutorial->tutorialEnabled=bool(trial&4);field<unsigned char>(nativeBuildingTutorial,0xc09)=trial&4 ? 1:0;
    profile.booleans["tip35"]=nativeProfile.booleans["tip35"]=bool(trial&1);profile.booleans["tip36"]=nativeProfile.booleans["tip36"]=bool(trial&2);
    profile.ints["Money"]=nativeProfile.ints["Money"]=2;buildingPanel->open();call(0x928e0,nativeBuildingPanel);++buyBuildingStarts;buildingCheck();buildingPanel->close();call(0x8e130,nativeBuildingPanel);buildingCheck();
}
buildingTutorial->tutorialEnabled=false;field<unsigned char>(nativeBuildingTutorial,0xc09)=0;
profile.ints["Money"]=nativeProfile.ints["Money"]=100000;
const auto buildingFrame=[&](Point2 point=Point2{0,0},std::vector<unsigned> events={},std::array<int,3> delta={}) {action=buyBuildingFrames;frame(point,std::move(events),delta);buildingCheck();++buyBuildingFrames;};
for(trial=0;trial<3;++trial) {
    stage=394;hud->disable();hud->absoluteDisable();call(0xd3750,original);call(0xd3790,original);buildingPanel->open();call(0x928e0,nativeBuildingPanel);++buyBuildingStarts;buildingCheck();
    for(unsigned i=0;i<8;++i) buildingFrame();const auto choicesBefore=buildingSourceChoices.size();
    if(trial<2) {auto& button=*buildingPanel->cells()[trial ? 10:0].button;const Point2 at{button.geometry().x,button.geometry().y};buildingFrame(at,{}, {1,0,0});buildingFrame(at);buildingFrame(at,{0x208});buildingFrame(at,{0x200});buildingFrame({0,0},{},{1,0,0});require(buildingSourceChoices.size()==choicesBefore+1 && buildingSourceChoices.back()==std::pair<unsigned,int>{trial ? 10U:0U,-1},"BuyBuilding real input selected wrong building");++buyBuildingChoices;}
    else {buildingFrame({0,0},{},{1,0,0});buildingFrame({0,0},{0x208});buildingFrame({0,0},{0x200});}
    for(unsigned i=0;i<8;++i) buildingFrame();require(!buildingPanel->state().shown,"BuyBuilding real input did not close panel");tutorialInputs.wareSelection=false;field<unsigned>(image,0x260150)=0;
}
// Actual two purchase guides, handled by the owned windows and real dispatcher.
buildingTutorial->tutorialEnabled=true;field<unsigned char>(nativeBuildingTutorial,0xc09)=1;buildingTutorial->currentStep=field<int>(nativeBuildingTutorial,0xbfc)=-1;
buildingTutorial->cancelRequested=false;field<unsigned char>(nativeBuildingTutorial,0xc08)=0;profile.booleans["tip35"]=nativeProfile.booleans["tip35"]=false;profile.booleans["tip36"]=nativeProfile.booleans["tip36"]=false;
for(unsigned step:{6U,7U,30U,39U}) profile.booleans[TutorialManager::profileKey(step)]=nativeProfile.booleans[TutorialManager::profileKey(step)]=true;
hud->disable();hud->absoluteDisable();call(0xd3750,original);call(0xd3790,original);buildingPanel->open();call(0x928e0,nativeBuildingPanel);++buyBuildingStarts;
for(unsigned i=0;i<8;++i) buildingFrame();require(buildingTutorial->activeStep(35) && !buildingPanel->state().enabled,"BuyBuilding did not open disabled-anchor guide 35");
{auto& button=buildingTutorial->disabledAnchor().okButton();const Point2 at{button.geometry().x,button.geometry().y};buildingFrame(at,{}, {1,0,0});buildingFrame(at,{0x208});buildingFrame(at,{0x200});buildingFrame({0,0},{},{1,0,0});}
for(unsigned i=0;i<6;++i) buildingFrame();require(buildingTutorial->activeStep(36) && buildingPanel->state().enabled,"BuyBuilding did not open repeated-action guide 36");
{auto& button=*buildingPanel->cells()[0].button;const Point2 at{button.geometry().x,button.geometry().y};const auto before=buildingSourceChoices.size();buildingFrame(at,{}, {1,0,0});buildingFrame(at,{0x208});buildingFrame(at,{0x200});buildingFrame({0,0},{},{1,0,0});require(buildingSourceChoices.size()==before+1 && buildingSourceChoices.back()==std::pair<unsigned,int>{0,-1},"BuyBuilding guide did not dispatch first building");++buyBuildingChoices;}
for(unsigned i=0;i<8;++i) buildingFrame();require(!buildingTutorial->active() && profile.booleans["tip35"] && profile.booleans["tip36"] && buildingTutorial->justProcessed==-1,"BuyBuilding guide progression differs");tutorialInputs.wareSelection=false;field<unsigned>(image,0x260150)=0;
}
stage=393;const auto buildingBefore=textureCounts();delete buildingPanel;const auto buildingMiddle=textureCounts();call(0x8edf0,nativeBuildingPanel);compareReferenceDeltas(buildingBefore,buildingMiddle);require(!BuyBuilding::instance() && !field<void*>(image,0x260010),"BuyBuilding destruction retains singleton");
world.overlays[0]=&overlay;putPointer(0x260010,nativeOverlay);
std::array<TutorialCommonTip*,5> keptBuildingTips{&buildingTutorial->common(),&buildingTutorial->arrow(),&buildingTutorial->disabledAnchor(),&buildingTutorial->action(),&buildingTutorial->repeatedAction()};std::array<void*,5> nativeKeptBuildingTips{};for(unsigned i=0;i<5;++i) nativeKeptBuildingTips[i]=field<void*>(nativeBuildingTutorial,0xbe8+i*4);
delete buildingTutorial;world.tutorial=nullptr;call(0xd0490,nativeBuildingTutorial);const auto buildingTipBefore=textureCounts();views.deleteDiscarded();delete keptBuildingTips[4];const auto buildingTipMiddle=textureCounts();for(auto* n:nativeKeptBuildingTips) call(0xcc770,n);compareReferenceDeltas(buildingTipBefore,buildingTipMiddle);nativeDiscards.clear();
putPointer(0x25f73c,contractTutorialObject.data());field<int>(contractTutorialObject.data(),0xbfc)=-1;redirect(0xd0a20,reinterpret_cast<void*>(&hudCan));redirect(0xcdef0,reinterpret_cast<void*>(&hudStep));redirect(0xcf3c0,reinterpret_cast<void*>(&hudShow));redirect(0xd0ac0,reinterpret_cast<void*>(&hudComplete));
