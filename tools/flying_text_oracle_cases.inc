stage=550;loadingFiles=&files;managerTable[6]=reinterpret_cast<void*>(&loadingXML);xmlTable[0]=reinterpret_cast<void*>(&loadingDeleteXML);xmlTable[34]=reinterpret_cast<void*>(&sectionCount);
redirect(0x7d0b0,reinterpret_cast<void*>(&nativeDiscard));
const char* definitions[]{"UI/FlyProductExpence.xml","UI/FlyTextExpence.xml","UI/FlyTextIncome.xml"};
for(trial=0;trial<6;++trial) {
    ViewRegistry views;ControlRegistry controls;FlyingTextContext context{manager,settings,strings,views,controls};
    const String path(definitions[trial%3]);auto effect=std::make_unique<FlyingText>(nullptr,context,path);auto* native=allocate(760);std::memset(native,0,760);
    function<void*(__thiscall*)(void*,void*,const String*)>(0x7cc50)(native,nullptr,&path);
    effect->setString(WString(trial%3==0 ? L"1":trial%3==1 ? L"123":L"456"));WString number(effect->string());function<void(__thiscall*)(void*,const WString*)>(0xdd030)(native,&number);
    unsigned sourceCallbacks=0,nativeCallbacks=0;
    const auto completed=+[](void* count) {++*static_cast<unsigned*>(count);};
    if(trial&1) {effect->completed=completed;effect->completionArgument=&sourceCallbacks;field<void*>(native,0x2cc)=reinterpret_cast<void*>(completed);field<void*>(native,0x2d0)=&nativeCallbacks;}
    const auto check=[&] {
        stage=551;mainMenuCompareView(*effect,native);compareControl(*effect,native);if(effect->textDirty()) effect->makeLetters();compare(*effect,native);
        compareFloat(effect->speed,field<float>(native,0x2b8),"FlyingText speed");compareFloat(effect->totalDistance,field<float>(native,0x2bc),"FlyingText total distance");compareFloat(effect->distance,field<float>(native,0x2c0),"FlyingText distance");
        require(effect->positionIndex==field<unsigned>(native,0x2c4) && effect->alphaIndex==field<unsigned>(native,0x2c8),"FlyingText indices differ");
        require(sourceCallbacks==nativeCallbacks && bool(effect->completed)==bool(field<void*>(native,0x2cc)),"FlyingText completion callback differs");
        const auto* positions=field<FlyingPosition*>(native,0x2d8);const auto* positionEnd=field<FlyingPosition*>(native,0x2dc);require(effect->positions().size()==std::size_t(positionEnd-positions),"FlyingText position count differs");
        require(!std::memcmp(effect->positions().data(),positions,effect->positions().size()*20),"FlyingText position records differ");
        const auto* alphas=field<FlyingAlpha*>(native,0x2e8);const auto* alphaEnd=field<FlyingAlpha*>(native,0x2ec);require(effect->alphas().size()==std::size_t(alphaEnd-alphas),"FlyingText alpha count differs");
        require(!std::memcmp(effect->alphas().data(),alphas,effect->alphas().size()*12),"FlyingText alpha records differ");
        require(effect->icon()!=nullptr && field<void*>(native,0x2f4),"FlyingText icon missing");compareControl(*effect->icon(),field<void*>(native,0x2f4));
        const auto* head=field<Node*>(native,0x18);std::vector<void*> children;collectControls(head->parent,children);require(effect->children().size()==children.size(),"FlyingText retained children differ");
        require(views.discarded().count(effect.get())==nativeDiscards.count(native),"FlyingText deferred discard differs");
        if(views.discarded().count(effect.get())) require(views.discarded().at(effect.get()).view()==nativeDiscards.at(native),"FlyingText discard origin differs");++flyingTextStates;
    };check();
    if(trial>=3) {
        for(unsigned reload=0;reload<2;++reload) {XmlDocument xml(files,path);effect->loadXML(xml);function<void(__thiscall*)(void*,void*)>(0x7c250)(native,wrap(&xml));++flyingTextReloads;check();}
    }
    for(action=0;action<32;++action) {stage=552;elapsed=float(action%5)*.0137f;effect->enabledStep();call(0x7d590,native);check();}
    // Actual frame dispatch/rendering follows the remainder of the path and
    // observes deferred disposal; deletion is explicit after the comparison.
    menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
    std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    for(unsigned frame=0;frame<24;++frame) {stage=553;action=frame;menuMilliseconds+=17;menuPoint={790,590};menuMouseDelta={0,0,0};menuInputEvents.clear();flyingTextPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++flyingTextFrames;check();}
    menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    require(!views.discarded().empty(),"FlyingText path never completed");
    require(sourceCallbacks==unsigned(bool(trial&1)),"FlyingText completion callback count differs");flyingTextCallbacks+=sourceCallbacks;
    stage=554;const auto before=textureCounts();auto* pointer=effect.release();views.deleteDiscarded();require(!views.contains(pointer),"FlyingText discard did not remove object");const auto middle=textureCounts();call(0x7ba00,native);compareReferenceDeltas(before,middle);nativeDiscards.clear();++flyingTextObjects;
}
