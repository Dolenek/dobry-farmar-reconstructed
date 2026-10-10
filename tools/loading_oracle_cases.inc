LoadingDisabledAudio disabledBackend;SoundManager disabledSound(files,settings,disabledBackend,0);
MusicPlayback music;loadingSoundManager=&disabledSound;loadingFiles=&files;
const auto previousDestruction=resourceDestroyedCallback();
resourceDestroyedCallback()=[&](cResource* resource) {
    textureResources.erase(reinterpret_cast<cTexture*>(resource));previousDestruction(resource);
};
managerTable[3]=reinterpret_cast<void*>(&loadingSoundResource);managerTable[6]=reinterpret_cast<void*>(&loadingXML);
managerTable[15]=image+0x141700;managerTable[17]=reinterpret_cast<void*>(&loadingClear);
soundTable[3]=reinterpret_cast<void*>(&loadingMusicChannel);xmlTable[0]=reinterpret_cast<void*>(&loadingDeleteXML);xmlTable[34]=reinterpret_cast<void*>(&sectionCount);
std::array<void*,7> loadingFileTable{};loadingFileTable[6]=reinterpret_cast<void*>(&loadingExists);auto* loadingFilePointer=loadingFileTable.data();putPointer(0x25f72c,&loadingFilePointer);
redirect(0x2fe80,reinterpret_cast<void*>(&loadingOriginLine));redirect(0x1cc0,reinterpret_cast<void*>(&loadingConcat));
const unsigned batches[]={30,30,7,0xffffffffU,3,1,0,30};
for(trial=0;trial<8;++trial) {
    stage=130;ViewRegistry views;ControlRegistry controls;
    const String filename(trial==7 ? "missing-loading-oracle.xml":trial&1 ? "MenuResLog.xml":"GameResLog.xml");
    const unsigned beforeOwnCallbacks=loadingSourceCallbacks,beforeNativeCallbacks=loadingNativeCallbacks,beforeClears=loadingNativeClears,beforeDeletes=loadingNativeXMLDeletes;
    auto* screen=LoadingFromLog::create(files,manager,settings,strings,disabledSound,music,filename,&loadingSourceComplete,batches[trial],bool(trial&1),views,controls);
    auto* original=allocate(196);function<void*(__thiscall*)(void*,const String*,void(*)(),unsigned,bool)>(0x80770)(original,&filename,&loadingNativeComplete,batches[trial],bool(trial&1));
    require(loadingSourceCallbacks-beforeOwnCallbacks==loadingNativeCallbacks-beforeNativeCallbacks,"Loading constructor completion differs");
    require(music.sound()==field<cSound*>(image,0x25ffe0),"Loading selected music resource differs");require(!music.channel() && !field<void*>(image,0x25ffdc),"Disabled Loading retained a channel");
    if(trial==7) {
        require(!screen && views.enabled().empty() && controls.visible().empty(),"Missing Loading log retained screen controls");
        require(loadingSourceCallbacks-beforeOwnCallbacks==1 && loadingNativeXMLDeletes-beforeDeletes==1,"Missing Loading callback/XML destruction differs");++loadingMissing;continue;
    }
    require(screen!=nullptr,"Existing Loading log immediately completed");
    require(loadingNativeXMLDeletes-beforeDeletes==1,"Loading constructor did not release UI XML");
    const auto check=[&] {
        stage=131;require(screen->total()==field<unsigned>(original,0x7c) && screen->nextIndex()==field<unsigned>(original,0x78) && screen->batch()==field<unsigned>(original,0x80),"Loading range state differs");
        require(screen->firstStep()==bool(field<unsigned char>(original,0xc0)),"Loading first-step flag differs");
        require(screen->menuMusic().view()==field<String>(original,0x84).view() && screen->gameMusic().view()==field<String>(original,0xa0).view(),"Loading music XML differs");
        auto& bar=screen->progress();auto* nativeBar=field<void*>(original,0xbc);compareControl(bar,nativeBar);
        compareFloat(bar.maximum,field<float>(nativeBar,0x238),"Loading Progress maximum");compareFloat(bar.current,field<float>(nativeBar,0x23c),"Loading Progress current");
        require(bar.side==field<unsigned>(nativeBar,0x240) && bar.dirty()==bool(field<unsigned char>(nativeBar,0x1b0)),"Loading Progress side/dirty differs");
        bar.updateTransform();call(0xf9290,nativeBar);require(!std::memcmp(&bar.sprite(),static_cast<unsigned char*>(nativeBar)+0x70,76),"Loading Progress sprite differs");
        loadingDecorations+=compareDecorations(*screen,original);
        require(screen->state().shown==bool(field<unsigned char>(original,0x24)) && screen->state().enabled==bool(field<unsigned char>(original,0x26)),"Loading screen activation differs");
        require(loadingSourceCallbacks-beforeOwnCallbacks==loadingNativeCallbacks-beforeNativeCallbacks,"Loading completion order/count differs");++loadingStates;
    };check();
    // Native 0065ffe4 retains the FIRST screen's maximum across future screens.
    screen->progress().maximum=trial ? float(125+trial):27.25f;field<float>(field<void*>(original,0xbc),0x238)=screen->progress().maximum;
    const unsigned total=screen->total();require(total==(trial&1 ? 148U:539U),"Shipped resource log count differs");
    for(action=0;action<600;++action) {
        stage=132;screen->enabledStep();call(0x81130,original);++loadingRanges;check();
        nativeDiscards.clear();
        if(field<unsigned>(image,0x2604ec)) {
            require(field<unsigned>(image,0x2604ec)==1,"Loading native discard map has extra entries");
            auto* node=field<void*>(field<void*>(image,0x2604e8),0);
            nativeDiscards[field<void*>(node,12)]=std::string(field<String>(node,16).view());
        }
        require(loadingNativeClears-beforeClears==1,"Loading native cleared cache more than once");
        if(screen->nextIndex()>=total || (batches[trial]==0 && action==2)) break;
        if(trial<4 && action<4) {
            screen->disable();call(0xd3750,original);menuMilliseconds=0;
            GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
            std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
            menuMilliseconds+=17;menuInputEvents.clear();menuMouseDelta={0,0,0};menuPoint={0,0};
            stage=133;loadingPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++loadingFrames;
            menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
            screen->enable();call(0xd3bb0,original);check();
        }
    }
    stage=134;
    if(batches[trial]) {
        require(loadingSourceCallbacks-beforeOwnCallbacks==1 && views.discarded().size()==1 && nativeDiscards.size()==1,"Loading terminal discard/callback differs");
        require(views.discarded().at(screen).view()==nativeDiscards.at(original),"Loading discard origin differs");++loadingCompleted;
    } else require(loadingSourceCallbacks==beforeOwnCallbacks && views.discarded().empty() && nativeDiscards.empty(),"Zero-batch Loading completed");
    const auto before=textureCounts();delete screen;const auto middle=textureCounts();call(0x7fff0,original);compareReferenceDeltas(before,middle);++loadingObjects;nativeDiscards.clear();
    unsigned logXmlDeletes=0;
    if(batches[trial]) for(const auto& entry:loadingDocuments.at(std::string(filename.view()))->children())
        if(entry.second->attribute(WString(L"Type")).view()==L"1") ++logXmlDeletes;
    require(loadingNativeXMLDeletes-beforeDeletes==2+logXmlDeletes,"Loading destructor/log entries XML release count differs");require(views.enabled().empty() && controls.visible().empty(),"Loading destruction retained control registries");
}
music.clear();if(auto* resource=field<cSound*>(image,0x25ffe0)) resource->releaseReference();putPointer(0x25ffe0,nullptr);putPointer(0x25ffdc,nullptr);
manager.clearUseless();resourceDestroyedCallback()=previousDestruction;
loadingSoundManager=nullptr;loadingFiles=nullptr;loadingDocuments.clear();
managerTable[3]=reinterpret_cast<void*>(&getSound);managerTable[6]=reinterpret_cast<void*>(&dialogXML);soundTable[3]=nullptr;xmlTable[0]=reinterpret_cast<void*>(&releaseDialogXML);
