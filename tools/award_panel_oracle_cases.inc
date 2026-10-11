LoadingDisabledAudio disabledBackend;
class PanelAudioRecorder final:public SoundManager {
public:using SoundManager::SoundManager;
    void clickSound(cSound* sound) override {actualSounds.emplace_back(std::string(sound->resourceName().view()),1);SoundManager::clickSound(sound);}
} audio(files,settings,disabledBackend,0);
loadingFiles=&files;managerTable[6]=reinterpret_cast<void*>(&loadingXML);managerTable[3]=reinterpret_cast<void*>(&panelResourceFactory);xmlTable[0]=reinterpret_cast<void*>(&loadingDeleteXML);
panelSourceCounter=&panelSourceCallbacks;panelNativeCounter=&panelNativeCallbacks;
const auto directory=std::filesystem::absolute(std::filesystem::path(L".local")/L"award-panel-xml-oracle");std::filesystem::create_directories(directory);
std::ofstream(directory/L"sound.xml")<<"<AwardPanel Sound=\"Sounds/Award.wav\"/>";
{
    auto file=files.open(String("UI/AwardPanel.xml"));std::wstring text(file->size()/2,L'\0');file->read(text.data(),unsigned(text.size()*2));
    for(auto tag:{std::pair<const wchar_t*,const wchar_t*>{L"<Text0",L"<Text"},{L"</Text0>",L"</Text>"}}) {
        const auto at=text.find(tag.first);require(at!=std::wstring::npos,"AwardPanel shipped Text0 section missing");text.replace(at,std::wcslen(tag.first),tag.second);
    }
    std::ofstream output(directory/L"text.xml",std::ios::binary);output.write(reinterpret_cast<const char*>(text.data()),std::streamsize(text.size()*2));
}
FileIndex fixtureFiles(toNarrow(WString(directory.wstring())),String("pack"));XmlDocument soundDefinition(fixtureFiles,String("sound.xml")),textDefinition(fixtureFiles,String("text.xml"));
XmlDocument definition(files,String("UI/AwardPanel.xml"));menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
for(trial=0;trial<21;++trial) {
    stage=160;ViewRegistry views;ControlRegistry controls;interaction.active=nullptr;interaction.pending=0;putPointer(0x2604dc,nullptr);field<unsigned>(image,0x2604e0)=0;
    auto effect=std::make_unique<AwardEffect>(nullptr,trial,manager,audio,views,controls);auto* nativeEffect=allocate(704);function<void*(__thiscall*)(void*,void*,unsigned)>(0xc6a40)(nativeEffect,nullptr,trial);
    effect->hide();call(0xd3720,nativeEffect);effect->disable();call(0xd3750,nativeEffect);effect->setPhase(3);function<void(__thiscall*)(void*,unsigned)>(0xc5b70)(nativeEffect,3);
    const auto firstSound=sounds.size(),deletes=loadingNativeXMLDeletes;
    auto panel=std::make_unique<AwardPanel>(*effect,&panelAcceptedSource,manager,settings,strings,audio,input,interaction,views,controls);
    auto* original=allocate(596);function<void*(__thiscall*)(void*,void*,void(__cdecl*)(void*))>(0x761c0)(original,nativeEffect,&panelAcceptedNative);
    require(loadingNativeXMLDeletes==deletes+1,"AwardPanel constructor XML lifetime differs");
    require(sounds.size()==firstSound+4,"AwardPanel constructor button sound count differs");
    const auto compareMulti=[&](MultilineText& text,void* native) {
        text.enabledStep();call(0xf8e10,native);compareControl(text,native);
        require(text.string().view()==field<WString>(native,0x23c).view(),"AwardPanel label text differs");
        auto* head=field<Link*>(native,0x230);auto* node=head->next;require(text.lines().size()==field<unsigned>(native,0x234),"AwardPanel line count differs");
        for(auto* line:text.lines()) {require(node!=head,"AwardPanel lines ended early");compareControl(*line,node->control);panelLetters+=compare(*line,node->control);node=node->next;}require(node==head,"AwardPanel lines ended late");
    };
    const auto check=[&] {
        stage=161;compareControl(*panel,original);panel->updateTransform();call(0xf0b70,original);require(!std::memcmp(&panel->sprite(),static_cast<unsigned char*>(original)+0x70,76),"AwardPanel sprite differs");
        require(!std::memcmp(panel->awardPicturePosition().data(),static_cast<unsigned char*>(original)+0x244,8),"AwardPanel stored picture position differs");
        require(field<void*>(original,0x24c)==nativeEffect && &panel->effect()==effect.get(),"AwardPanel borrowed effect differs");
        auto* nativeButton=field<void*>(original,0x238);compareControl(panel->okButton(),nativeButton);panelLetters+=compare(panel->okButton().text(),field<void*>(nativeButton,0x27c));
        require(unsigned(panel->okButton().buttonState())==field<unsigned>(nativeButton,0x238),"AwardPanel OK input state differs");
        compareMulti(panel->text(),field<void*>(original,0x23c));compareMulti(panel->congratulation(),field<void*>(original,0x240));
        require(panel->sound()==field<cSound*>(original,0x250),"AwardPanel root sound differs");++panelStates;
    };check();
    const auto beforeReload=textureCounts();panel->loadXML(definition);const auto middleReload=textureCounts();function<void(__thiscall*)(void*,void*)>(0x75b00)(original,wrap(&definition));compareReferenceDeltas(beforeReload,middleReload);check();++panelReloads;
    for(action=0;action<12;++action) {
        stage=162;actualSounds.clear();expectedSounds.clear();
        switch(action%4) {case 0:panel->show();call(0xd37d0,original);break;case 1:panel->hide();call(0xd3720,original);break;case 2:panel->disable();call(0xd3750,original);break;case 3:panel->enable();call(0xd3bb0,original);break;}
        require(actualSounds==expectedSounds,"AwardPanel lifecycle sound request differs");check();
    }
    // Empty shipped Sound leaves the prior reference intact on reload.
    stage=163;panel->loadXML(soundDefinition);function<void(__thiscall*)(void*,void*)>(0x75b00)(original,wrap(&soundDefinition));check();
    const auto retainedSound=panel->sound()->referenceCount();panel->loadXML(soundDefinition);require(panel->sound()->referenceCount()==retainedSound+1,"AwardPanel source repeated root Sound differs");
    function<void(__thiscall*)(void*,void*)>(0x75b00)(original,wrap(&soundDefinition));require(panel->sound()->referenceCount()==retainedSound+2,"AwardPanel native repeated root Sound differs");
    panel->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0x75b00)(original,wrap(&definition));require(panel->sound()->referenceCount()==retainedSound+2,"AwardPanel empty root Sound cleared retained sound");check();
    panel->hide();call(0xd3720,original);actualSounds.clear();expectedSounds.clear();panel->show();call(0xd37d0,original);require(actualSounds==expectedSounds && actualSounds.size()==1,"AwardPanel root shown sound missing");
    actualSounds.clear();expectedSounds.clear();panel->enable();call(0xd3bb0,original);
    GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
    std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    for(unsigned frame=0;frame<4;++frame) {
        stage=164;action=frame;menuMilliseconds+=25;menuInputEvents.clear();menuMouseDelta={1,0,0};menuPoint=frame ? Point2{panel->okButton().geometry().x,panel->okButton().geometry().y}:Point2{790,590};
        if(frame==2) menuInputEvents={0x208};if(frame==3) menuInputEvents={0x200};
        panelPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++panelFrames;check();
    }
    require(panelSourceCallbacks==trial+1 && panelNativeCallbacks==trial+1,"AwardPanel actual input accepted callback differs");
    menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    // Additional explicit XML branch: Text instead of the actual Text0. This
    // runs after shipped GPU frames so it is not claimed as original artwork.
    effect->effectState().panelText=effect->effectState().congratulation;field<WString>(nativeEffect,0x264)=field<WString>(nativeEffect,0x280);
    const auto beforeText=textureCounts();panel->loadXML(textDefinition);const auto middleText=textureCounts();function<void(__thiscall*)(void*,void*)>(0x75b00)(original,wrap(&textDefinition));compareReferenceDeltas(beforeText,middleText);check();++panelReloads;
    stage=165;auto* rootSound=panel->sound();const auto beforeSound=rootSound->referenceCount();const auto beforeDelete=textureCounts();panel.reset();const auto middleDelete=textureCounts();require(rootSound->referenceCount()==beforeSound-1,"AwardPanel source destructor sound differs");
    call(0x75300,original);require(rootSound->referenceCount()==beforeSound-2,"AwardPanel native destructor sound differs");compareReferenceDeltas(beforeDelete,middleDelete);
    require(views.contains(effect.get()),"AwardPanel deleted its borrowed effect");effect.reset();call(0xc60e0,nativeEffect);++panelObjects;
    require(sounds.size()==firstSound+16,"AwardPanel reload sound fixture count differs");
    for(unsigned i=unsigned(firstSound);i<firstSound+12;++i) {require(sounds[i]->referenceCount()==1,"AwardPanel overwritten button sound retention differs");sounds[i]->releaseReference();++panelSoundResidues;}
    rootSound->releaseReference();rootSound->releaseReference(); // Diagnostic cleanup of confirmed earlier root Sound reload references.
}
panelSourceCounter=nullptr;panelNativeCounter=nullptr;loadingDocuments.clear();loadingFiles=nullptr;
