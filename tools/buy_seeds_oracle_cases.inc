stage=380;for(const auto& entry:tutorialEntries) write(entry.first,entry.second.data(),5);
redirect(0xd0070,reinterpret_cast<void*>(&nativeDiscard));redirect(0x180ed3,reinterpret_cast<void*>(&tutorialVectorDestroy));redirect(0x2fe80,reinterpret_cast<void*>(&decimalString));redirect(0x1cc0,reinterpret_cast<void*>(&concatenateString));putPointer(0x25f73c,nullptr);
Control seedAnchor(nullptr,manager,views,controls);TutorialWorldFixture seedTutorialWorld(world,seedAnchor);
TutorialManagerContext seedTutorialContext{manager,settings,strings,input,interaction,profile,seedTutorialWorld,views,controls};
auto* seedTutorial=new TutorialManager(seedTutorialContext);world.tutorial=seedTutorial;auto* nativeSeedTutorial=allocate(3104);std::memset(nativeSeedTutorial,0,3104);field<int>(nativeSeedTutorial,0xc00)=-1;function<void*(__thiscall*)(void*)>(0xcfce0)(nativeSeedTutorial);seedTutorial->disable();call(0xd3750,nativeSeedTutorial);
SeedWorldFixture seedWorld(*hud,*seedTutorial,profile);BuySeedsContext seedContext{manager,settings,strings,input,interaction,profile,configuration,audio,seedWorld,views,controls};
putPointer(0x25ff64,nullptr);auto* seedPanel=new BuySeeds(nullptr,seedContext);auto* nativeSeedPanel=allocate(1332);std::memset(nativeSeedPanel,0,1332);function<void*(__thiscall*)(void*,void*)>(0x20cb0)(nativeSeedPanel,nullptr);
world.overlays[1]=seedPanel;redirect(0x2c960,reinterpret_cast<void*>(&seedNativeChoose));
for(unsigned i=0;i<8;++i) field<unsigned>(nativeGame,0x79c+i*44)=configuration.seeds[i].price;
const auto seedCheck=[&] {
    stage=381;require(BuySeeds::instance()==seedPanel && field<void*>(image,0x25ff64)==nativeSeedPanel,"BuySeeds singleton differs");mainMenuCompareView(*seedPanel,nativeSeedPanel);compareControl(*seedPanel,nativeSeedPanel);compareControl(seedPanel->origin(),field<void*>(nativeSeedPanel,0x26c));compareControl(seedPanel->cancelButton(),field<void*>(nativeSeedPanel,0x4d8));buySeedsLetters+=compare(seedPanel->cancelButton().text(),field<void*>(field<void*>(nativeSeedPanel,0x4d8),0x27c));
    require(seedPanel->movement==field<unsigned>(nativeSeedPanel,0x248),"BuySeeds movement differs");compareFloat(seedPanel->remaining,field<float>(nativeSeedPanel,0x258),"BuySeeds remaining");
    for(unsigned d=0;d<2;++d) {compareFloat(seedPanel->originalPosition[d],field<float>(nativeSeedPanel,0x264+d*4),"BuySeeds original position");compareFloat(seedPanel->hiddenPlace[d],field<float>(nativeSeedPanel,0x25c+d*4),"BuySeeds hidden position");}
    for(unsigned i=0;i<8;++i) {const auto& def=seedPanel->definitions[i];auto* n=static_cast<unsigned char*>(nativeSeedPanel)+0x2b8+i*68;require(def.name.view()==field<WString>(n,0).view(),"BuySeeds definition name differs");const auto& a=def.appearance;require(a.texture==field<cTexture*>(n,0x20) && a.frames==field<unsigned>(n,0x24) && a.columns==field<unsigned>(n,0x30) && a.rows==field<unsigned>(n,0x34),"BuySeeds definition appearance differs");}
    for(unsigned i=0;i<7;++i) {const auto& cell=seedPanel->cells()[i];auto* button=field<void*>(nativeSeedPanel,0x4dc+i*12);auto* price=field<void*>(nativeSeedPanel,0x4e0+i*12);auto* name=field<void*>(nativeSeedPanel,0x4e4+i*12);
        for(unsigned d=0;d<2;++d) compareFloat(seedPanel->places[i][d],field<float>(nativeSeedPanel,0x280+i*8+d*4),"BuySeeds item place");compareControl(*cell.button,button);require(cell.button->seed==field<unsigned>(button,0x27c) && unsigned(cell.button->buttonState())==field<unsigned>(button,0x238),"BuySeeds button identity/state differs");
        require(cell.button->deepProcessing==bool(field<unsigned char>(button,0x26a)) && cell.button->dirty()==bool(field<unsigned char>(button,0x1b0)),"BuySeeds button deep/dirty flags differ");compareControl(*cell.price,price);buySeedsLetters+=compare(*cell.price,price);compareControl(cell.price->coin(),field<void*>(price,0x2bc));compareControl(*cell.name,name);
        require(cell.name->string().view()==field<WString>(name,0x23c).view() && cell.name->lines().size()==field<unsigned>(name,0x234),"BuySeeds item name differs");auto* head=field<Link*>(name,0x230);auto* item=head->next;for(auto* line:cell.name->lines()) {require(item!=head,"BuySeeds multiline list ended early");buySeedsLetters+=compare(*line,item->control);item=item->next;}require(item==head,"BuySeeds multiline list ended late");
    }
    require(seedPanel->buySound()==field<cSound*>(nativeSeedPanel,0x530),"BuySeeds sound ownership differs");require(seedSourceChoices==seedNativeChoices,"BuySeeds Farm choice dispatch differs");check();++buySeedsStates;
};seedCheck();
for(trial=0;trial<128;++trial) {
    stage=382;for(unsigned i=1;i<8;++i) profile.uints["SeedAvail"+std::to_string(i)]=nativeProfile.uints["SeedAvail"+std::to_string(i)]=(trial&(1U<<(i-1))) ? 1:0;
    profile.ints["Money"]=nativeProfile.ints["Money"]=trial%4 ? int(trial*23):-1;
    hud->disable();hud->absoluteDisable();call(0xd3750,original);call(0xd3790,original);
    seedPanel->open();call(0x20f50,nativeSeedPanel);++buySeedsStarts;seedCheck();
    explicitTipElapsed=true;elapsed=0.25f;for(unsigned i=0;i<3;++i) {seedPanel->absoluteStep();call(0x148e0,nativeSeedPanel);seedCheck();}explicitTipElapsed=false;
    if(trial%3==0) {seedPanel->cancel();function<void(__cdecl*)()>(0x1d5e0)();}else {
        const auto& cell=seedPanel->cells()[trial%7];seedPanel->choose(*cell.button);function<void(__cdecl*)(void*)>(0x1d610)(field<void*>(nativeSeedPanel,0x4dc+(trial%7)*12));++buySeedsChoices;
    }seedCheck();actualSounds.clear();expectedSounds.clear();tutorialInputs.seed=false;field<unsigned char>(image,0x25f721)=0;
}
// Actual dispatcher renders all seven available slots and routes real mouse
// input through the source/native manager, panel, HUD and Farm boundary.
seedTutorial->tutorialEnabled=false;field<unsigned char>(nativeSeedTutorial,0xc09)=0;
for(unsigned i=1;i<8;++i) profile.uints["SeedAvail"+std::to_string(i)]=nativeProfile.uints["SeedAvail"+std::to_string(i)]=1;
profile.ints["Money"]=nativeProfile.ints["Money"]=10000;
const auto seedFrame=[&](Point2 point=Point2{0,0},std::vector<unsigned> events={},std::array<int,3> delta={}) {frame(point,std::move(events),delta);seedCheck();++buySeedsFrames;};
for(trial=0;trial<3;++trial) {
    stage=384;hud->disable();hud->absoluteDisable();call(0xd3750,original);call(0xd3790,original);seedPanel->open();call(0x20f50,nativeSeedPanel);++buySeedsStarts;seedCheck();
    for(unsigned i=0;i<8;++i) seedFrame();
    const auto choicesBefore=seedSourceChoices.size();
    if(trial<2) {auto& button=*seedPanel->cells()[trial ? 6:0].button;const Point2 at{button.geometry().x,button.geometry().y};seedFrame(at,{}, {1,0,0});seedFrame(at,{0x208});seedFrame(at,{0x200});seedFrame({0,0},{},{1,0,0});require(seedSourceChoices.size()==choicesBefore+1 && seedSourceChoices.back()==(trial ? 7U:1U),"BuySeeds real input selected the wrong seed");++buySeedsChoices;}
    else {const auto& button=seedPanel->cancelButton();const Point2 at{button.geometry().x,button.geometry().y};seedFrame(at,{}, {1,0,0});seedFrame(at,{0x208});seedFrame(at,{0x200});seedFrame({0,0},{},{1,0,0});}
    for(unsigned i=0;i<8;++i) seedFrame();require(!seedPanel->state().shown,"BuySeeds real input did not close panel");tutorialInputs.seed=false;field<unsigned char>(image,0x25f721)=0;
}
// Compare original hit-test float bounds and its unusual skipped-first-cell /
// sound-slot path. Both diagnostic controllers reference the same actual
// cSound object, so this does not establish cross-process heap independence.
auto* savedSeedCursor=menuCursor;auto* savedNativeSeedCursor=nativeCursorFrame;menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);
seedPanel->setPosition(seedPanel->originalPosition);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(nativeSeedPanel,&seedPanel->originalPosition);seedPanel->enable();call(0xd3bb0,nativeSeedPanel);
for(trial=0;trial<80;++trial) {
    stage=385;cursorPosition={float(int(trial%10)*40-4),float(220+int(trial/10)*45)};
    for(unsigned i=0;i<7;++i) {const auto state=trial%9==i ? ButtonState::Hovered:ButtonState::Normal;seedPanel->cells()[i].button->setState(state);function<void(__thiscall*)(void*,unsigned)>(0xf30e0)(field<void*>(nativeSeedPanel,0x4dc+i*12),unsigned(state));}
    seedPanel->cancelButton().setState(ButtonState::Normal);function<void(__thiscall*)(void*,unsigned)>(0xf30e0)(field<void*>(nativeSeedPanel,0x4d8),unsigned(ButtonState::Normal));
    require(seedPanel->cursorInside()==function<bool(__thiscall*)(void*)>(0x1e050)(nativeSeedPanel),"BuySeeds cursor/legacy slot check differs");seedCheck();
}
menuCursor=savedSeedCursor;nativeCursorFrame=savedNativeSeedCursor;putPointer(0x25ff60,nativeCursorFrame);actualSounds.clear();expectedSounds.clear();
// Real panel enabled-step opens the two original guides in sequence. The
// source/native TutorialManager and actual owned tip windows handle their OK
// input and the seed selection completion, without tutorial notification mocks.
seedTutorial->tutorialEnabled=true;field<unsigned char>(nativeSeedTutorial,0xc09)=1;seedTutorial->currentStep=field<int>(nativeSeedTutorial,0xbfc)=-1;
seedTutorial->cancelRequested=false;field<unsigned char>(nativeSeedTutorial,0xc08)=0;profile.booleans["tip14"]=nativeProfile.booleans["tip14"]=false;profile.booleans["tip15"]=nativeProfile.booleans["tip15"]=false;
// Earlier HUD navigation guides are already processed in this controlled
// purchase tutorial scenario; otherwise the resumed HUD correctly opens them.
for(unsigned step:{6U,7U,30U,39U}) profile.booleans[TutorialManager::profileKey(step)]=nativeProfile.booleans[TutorialManager::profileKey(step)]=true;
hud->disable();hud->absoluteDisable();call(0xd3750,original);call(0xd3790,original);seedPanel->open();call(0x20f50,nativeSeedPanel);++buySeedsStarts;
for(unsigned i=0;i<20;++i) seedFrame();require(seedTutorial->activeStep(14) && !seedPanel->state().enabled,"BuySeeds did not open original disabled-anchor guide");
{auto& button=seedTutorial->disabledAnchor().okButton();const Point2 at{button.geometry().x,button.geometry().y};seedFrame(at,{}, {1,0,0});seedFrame(at,{0x208});seedFrame(at,{0x200});seedFrame({0,0},{},{1,0,0});}
for(unsigned i=0;i<20;++i) seedFrame();require(seedTutorial->activeStep(15) && seedPanel->state().enabled,"BuySeeds did not open original repeated-action guide");
{auto& button=*seedPanel->cells()[0].button;const Point2 at{button.geometry().x,button.geometry().y};const auto before=seedSourceChoices.size();seedFrame(at,{}, {1,0,0});seedFrame(at,{0x208});seedFrame(at,{0x200});seedFrame({0,0},{},{1,0,0});require(seedSourceChoices.size()==before+1 && seedSourceChoices.back()==1,"BuySeeds guide input did not dispatch real first seed choice");++buySeedsChoices;}
for(unsigned i=0;i<16;++i) seedFrame();require(!seedTutorial->active() && profile.booleans["tip14"] && profile.booleans["tip15"] && seedTutorial->justProcessed==-1,"BuySeeds guide progression differs");
tutorialInputs.seed=false;field<unsigned char>(image,0x25f721)=0;
stage=383;const auto seedBefore=textureCounts();delete seedPanel;const auto seedMiddle=textureCounts();call(0x200f0,nativeSeedPanel);compareReferenceDeltas(seedBefore,seedMiddle);require(!BuySeeds::instance() && !field<void*>(image,0x25ff64),"BuySeeds destruction retains singleton");
world.overlays[1]=&selection;putPointer(0x25ff64,nativeSelection);
std::array<TutorialCommonTip*,5> keptTips{&seedTutorial->common(),&seedTutorial->arrow(),&seedTutorial->disabledAnchor(),&seedTutorial->action(),&seedTutorial->repeatedAction()};std::array<void*,5> nativeKeptTips{};for(unsigned i=0;i<5;++i) nativeKeptTips[i]=field<void*>(nativeSeedTutorial,0xbe8+i*4);
delete seedTutorial;world.tutorial=nullptr;call(0xd0490,nativeSeedTutorial);const auto tipBefore=textureCounts();views.deleteDiscarded();delete keptTips[4];const auto tipMiddle=textureCounts();for(auto* n:nativeKeptTips) call(0xcc770,n);compareReferenceDeltas(tipBefore,tipMiddle);nativeDiscards.clear();
putPointer(0x25f73c,contractTutorialObject.data());field<int>(contractTutorialObject.data(),0xbfc)=-1;redirect(0xd0a20,reinterpret_cast<void*>(&hudCan));redirect(0xcdef0,reinterpret_cast<void*>(&hudStep));redirect(0xcf3c0,reinterpret_cast<void*>(&hudShow));redirect(0xd0ac0,reinterpret_cast<void*>(&hudComplete));
