// All original cAwardEffect states, all 21 real definitions and complete GPU frames.
LoadingDisabledAudio disabledBackend;
class AudioRecorder final:public SoundManager {
public:
    using SoundManager::SoundManager;
    void clickSound(cSound* sound) override {actualSounds.emplace_back(std::string(sound->resourceName().view()),1);SoundManager::clickSound(sound);}
} audio(files,settings,disabledBackend,0);
loadingFiles=&files;managerTable[6]=reinterpret_cast<void*>(&loadingXML);managerTable[3]=reinterpret_cast<void*>(&loadingSoundResource);xmlTable[0]=reinterpret_cast<void*>(&loadingDeleteXML);
const auto directory=std::filesystem::absolute(std::filesystem::path(L".local")/L"award-effect-xml-oracle");std::filesystem::create_directories(directory);
std::ofstream(directory/L"sound.xml")<<"<Awards Sound=\"Sounds/menu_click.wav\"/>";
FileIndex fixtureFiles(toNarrow(WString(directory.wstring())),String("pack"));XmlDocument soundDefinition(fixtureFiles,String("sound.xml"));
XmlDocument definition(files,String("UI/Awards.xml"));std::mt19937 random(0x4c5bc0);menuMilliseconds=0;
GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
const auto setNativePoint=[](void* original,unsigned offset,const Point2& point) {std::memcpy(static_cast<unsigned char*>(original)+offset,point.data(),8);};
for(trial=0;trial<21;++trial) {
    stage=140;ViewRegistry views;ControlRegistry controls;
    auto parent=std::make_unique<Picture>(nullptr,manager,views,controls);auto* nativeParent=allocate(568);function<void*(__thiscall*)(void*,void*)>(0xf16f0)(nativeParent,nullptr);
    const auto deletes=loadingNativeXMLDeletes;
    auto effect=std::make_unique<AwardEffect>(trial&1 ? parent.get():nullptr,trial,manager,audio,views,controls);auto* original=allocate(704);
    function<void*(__thiscall*)(void*,void*,unsigned)>(0xc6a40)(original,trial&1 ? nativeParent:nullptr,trial);
    require(loadingNativeXMLDeletes==deletes+1,"AwardEffect constructor XML destruction differs");
    require(definition.attribute(WString(L"Sound")).view()==L"Sounds/Award.wav","AwardEffect shipped root Sound attribute differs");
    auto* shippedSound=effect->sound();require(shippedSound && shippedSound->resourceName().view()=="sounds/award.wav","AwardEffect normalized resource sound differs");
    const auto check=[&] {
        stage=141;compareControl(*effect,original);const auto& state=effect->effectState();
        require(effect->index()==field<unsigned>(original,0x2bc) && state.phase==field<unsigned>(original,0x238),"AwardEffect index/phase differs");
        compareFloat(state.originalDeep,field<float>(original,0x23c),"AwardEffect initial deep");compareFloat(state.scale,field<float>(original,0x250),"AwardEffect scale");
        compareFloat(state.appearSpeed,field<float>(original,0x254),"AwardEffect appear speed");compareFloat(state.hoverScale,field<float>(original,0x258),"AwardEffect hover scale");compareFloat(state.hoverSpeed,field<float>(original,0x25c),"AwardEffect hover speed");
        compareFloat(state.moveSpeed,field<float>(original,0x2b4),"AwardEffect move speed");
        for(auto pair:{std::pair<const Point2*,unsigned>{&state.effectSize,0x240},{&state.buttonSize,0x248},{&state.destination,0x29c},{&state.panelPosition,0x2a4},{&state.initialPosition,0x2ac}})
            require(!std::memcmp(pair.first->data(),static_cast<unsigned char*>(original)+pair.second,8),"AwardEffect stored Point2 differs");
        require(state.panelText.view()==field<WString>(original,0x264).view() && state.congratulation.view()==field<WString>(original,0x280).view(),"AwardEffect labels differ");
        require(effect->sound()==field<cSound*>(original,0x2b8) && state.hoverAnimation==bool(field<unsigned char>(original,0x260)),"AwardEffect sound/hover flag differs");
        require(effect->animation().loop==bool(field<unsigned char>(original,0x3c)) && effect->renderFlags()==field<unsigned>(original,0x6c),"AwardEffect loop/render flags differ");
        require(effect->dirty()==bool(field<unsigned char>(original,0x1b0)),"AwardEffect dirty differs");
        effect->updateTransform();call(0xf0b70,original);require(!std::memcmp(&effect->sprite(),static_cast<unsigned char*>(original)+0x70,76),"AwardEffect full sprite differs");++effectStates;
    };check();
    effect->effectState().destination={float(80+trial*30),float(70+trial*19)};setNativePoint(original,0x29c,effect->effectState().destination);
    for(action=0;action<240;++action) {
        stage=142;
        if(action%20==0) {
            const unsigned phase=(action/20)%12;effect->setPhase(phase);function<void(__thiscall*)(void*,unsigned)>(0xc5b70)(original,phase);check();
            auto& state=effect->effectState();
            state.scale=float(random()%2000)/1000.f;field<float>(original,0x250)=state.scale;
            state.hoverAnimation=bool(action&0x20);field<unsigned char>(original,0x260)=state.hoverAnimation;
            const float time=float(int(random()%1400)-100)/1000.f;effect->setTime(time);function<void(__thiscall*)(void*,const float*)>(0x13bc0)(original,&time);
        }
        if(action%40==7) {
            const auto soundBefore=shippedSound->referenceCount();const auto before=textureCounts();effect->loadXML(definition);const auto middle=textureCounts();
            require(effect->sound()==shippedSound && shippedSound->referenceCount()==soundBefore+1,"AwardEffect source sound reload retained reference differs");
            function<void(__thiscall*)(void*,void*)>(0xc61e0)(original,wrap(&definition));
            require(shippedSound->referenceCount()==soundBefore+2,"AwardEffect native sound reload retained reference differs");
            compareReferenceDeltas(before,middle);++effectReloads;++effectSoundReloads;
        }
        menuMilliseconds+=random()%30+1;frameTimer.process();actualSounds.clear();expectedSounds.clear();
        effect->enabledStep();call(0xc5bc0,original);require(actualSounds==expectedSounds,"AwardEffect click sound dispatch differs");check();++effectSteps;
    }
    // Replace the actual Sounds/Award.wav with another shipped sound. Native
    // XML keeps the prior reference; the replacement callback is recorded too.
    const auto retainedShipped=shippedSound->referenceCount();
    stage=143;effect->loadXML(soundDefinition);function<void(__thiscall*)(void*,void*)>(0xc61e0)(original,wrap(&soundDefinition));check();
    require(shippedSound->referenceCount()==retainedShipped,"AwardEffect replacement released original sound");++effectSoundReloads;
    effect->setPhase(0);function<void(__thiscall*)(void*,unsigned)>(0xc5b70)(original,0);effect->effectState().scale=1;field<float>(original,0x250)=1;
    actualSounds.clear();expectedSounds.clear();effect->enabledStep();call(0xc5bc0,original);
    require(actualSounds==expectedSounds && actualSounds.size()==1,"AwardEffect completed appearance sound missing");++effectSounds;check();
    auto* replacedSound=effect->sound();const auto retainedReplacement=replacedSound->referenceCount();
    stage=144;effect->loadXML(definition);require(shippedSound->referenceCount()==retainedShipped+1,"AwardEffect source sound restore differs");
    function<void(__thiscall*)(void*,void*)>(0xc61e0)(original,wrap(&definition));
    require(shippedSound->referenceCount()==retainedShipped+2 && replacedSound->referenceCount()==retainedReplacement,"AwardEffect native restore/reference retention differs");++effectSoundReloads;
    parent->show();call(0xd37d0,nativeParent);effect->show();call(0xd37d0,original);effect->enable();call(0xd3bb0,original);
    GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
    std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    for(unsigned frame=0;frame<4;++frame) {
        stage=145;action=frame;menuMilliseconds+=25;menuInputEvents.clear();menuMouseDelta={0,0,0};menuPoint={0,0};
        const unsigned phases[]={0,2,4,5,6,7,8,3};effect->setPhase(phases[(frame+trial)%8]);function<void(__thiscall*)(void*,unsigned)>(0xc5b70)(original,phases[(frame+trial)%8]);
        effect->effectState().scale=frame ? 1.f:.5f;field<float>(original,0x250)=effect->effectState().scale;
        effectPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++effectFrames;check();
    }
    menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    stage=146;const auto before=textureCounts();auto* sound=effect->sound();const auto soundBefore=sound->referenceCount();effect.reset();const auto middle=textureCounts();const auto soundMiddle=sound->referenceCount();call(0xc60e0,original);compareReferenceDeltas(before,middle);
    require(soundBefore-soundMiddle==1 && soundMiddle-sound->referenceCount()==1,"AwardEffect Sound destruction differs");++effectObjects;
    parent.reset();call(0xf1780,nativeParent);
}
loadingDocuments.clear();loadingFiles=nullptr;
