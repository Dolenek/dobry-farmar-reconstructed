// Actual Journal and actual HUD-owned GameMessage objects. The cGame/other
// scene services retain the documented HUD adapters; Journal has no body stubs.
stage=930;write(0x46d50,journalOriginalOpen.data(),5);
JournalContext journalContext{manager,settings,strings,input,interaction,views,controls};auto journal=std::make_unique<Journal>(nullptr,journalContext);hudActualJournal=journal.get();
auto* nativeJournal=allocate(156);std::memset(nativeJournal,0,156);function<void*(__thiscall*)(void*,void*)>(0x46aa0)(nativeJournal,nullptr);
const auto checkJournal=[&] {
    stage=931;require(Journal::instance()==journal.get() && field<void*>(image,0x25f764)==nativeJournal,"Journal singleton differs");mainMenuCompareView(*journal,nativeJournal);compareDecorations(*journal,nativeJournal);
    require(!std::memcmp(journal->fit.data(),static_cast<unsigned char*>(nativeJournal)+0x48,16),"Journal fit rect differs");compareFloat(journal->minDistance,field<float>(nativeJournal,0x58),"Journal min distance");
    require(!std::memcmp(&journal->readColor,static_cast<unsigned char*>(nativeJournal)+0x7c,16) && !std::memcmp(&journal->unreadColor,static_cast<unsigned char*>(nativeJournal)+0x8c,16),"Journal colors differ");
    for(auto entry:std::initializer_list<std::pair<Control*,unsigned>>{{&journal->upButton(),0x3c},{&journal->downButton(),0x40},{&journal->okButton(),0x44},{&journal->prototype(),0x5c}}) {mainMenuCompareView(*entry.first,field<void*>(nativeJournal,entry.second));compareControl(*entry.first,field<void*>(nativeJournal,entry.second));}
    if(journal->state().shown) require(journal->count==field<int>(nativeJournal,0x70) && journal->first==field<int>(nativeJournal,0x74) && journal->last==field<int>(nativeJournal,0x78),"Journal message indices differ");
    auto* begin=field<void**>(nativeJournal,0x64);auto* end=field<void**>(nativeJournal,0x68);require(journal->visibleMessages().size()==std::size_t(end-begin),"Journal displayed message count differs");
    for(unsigned i=0;i<journal->visibleMessages().size();++i) {
        auto& text=*journal->visibleMessages()[i];auto* nt=begin[i];mainMenuCompareView(text,nt);compareControl(text,nt);compareControl(const_cast<TextLine&>(text.prototype()),field<void*>(nt,0x238));
        require(text.string().view()==field<WString>(nt,0x23c).view(),"Journal message text differs");require(!std::memcmp(&text.textColor(),static_cast<unsigned char*>(nt)+0x268,16),"Journal text color differs");
        auto size=text.measuredSize();Point2 ns{};function<void(__thiscall*)(void*,Point2*)>(0xf5db0)(nt,&ns);require(!std::memcmp(size.data(),ns.data(),8),"Journal measured text size differs");
        auto* head=field<Link*>(nt,0x230);auto* node=head->next;require(text.lines().size()==field<unsigned>(nt,0x234),"Journal multiline line count differs");
        for(auto* line:text.lines()) {require(node!=head,"Journal lines ended early");compare(*line,node->control);node=node->next;}require(node==head,"Journal lines ended late");
    }
    check();++journalStates;
};checkJournal();
menuMilliseconds=0;explicitTipElapsed=false;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
std::array<unsigned char,24> frameNativeCursor{};field<void*>(frameNativeCursor.data(),0)=cursorTable.data();nativeCursorFrame=frameNativeCursor.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);timerTable[3]=reinterpret_cast<void*>(&tooltipUnscaledTimer);graphicsTable[3]=reinterpret_cast<void*>(&tooltipHeight);graphicsTable[4]=reinterpret_cast<void*>(&tooltipWidth);
const auto frame=[&](Point2 point=Point2{0,0},std::vector<unsigned> events={},std::array<int,3> delta={}) {stage=932;menuMilliseconds+=50;menuPoint=point;menuInputEvents=std::move(events);menuMouseDelta=delta;journalPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++journalFrames;checkJournal();};
const auto finishTransitions=[&] {for(unsigned i=0;i<24 && !views.transitions().empty();++i) frame();require(views.transitions().empty(),"Journal transition did not finish");};
const auto click=[&](ButtonPrototype& button) {Point2 point{button.geometry().x,button.geometry().y};frame(point,{}, {1,0,0});frame(point,{0x208});frame(point,{0x200});frame({0,0},{},{1,0,0});++journalActions;};
hudInputs.can=0;hudInputs.active=false;hud->show();hud->enable();hud->absoluteEnable();call(0xd37d0,original);call(0xd3bb0,original);call(0xd3fb0,original);scenes[0]->show();call(0xd37d0,nativeScenes[0]);
struct JournalMessages {unsigned allocator=0;GameMessage *begin=nullptr,*end=nullptr,*capacity=nullptr;};
for(unsigned count:{1U,2U,6U,12U,28U}) {
    std::vector<GameMessage> messages;for(unsigned i=0;i<count;++i) {std::wstring text=L"Journal message "+std::to_wstring(i)+L" ";for(unsigned n=0;n<1+i%5;++n) text+=L"Field harvest and warehouse. ";if(i%3==0) text+=L"/nSecond line.";messages.push_back({WString(text),bool(i%2)});}
    JournalMessages nativeMessages;nativeMessages.begin=messages.data();nativeMessages.end=nativeMessages.capacity=nativeMessages.begin+messages.size();hud->assignMessages(messages);function<void(__thiscall*)(void*,JournalMessages*)>(0xc4390)(original,&nativeMessages);
    // The real HUD journal button dispatches the real Journal open method.
    click(hud->journalButton());finishTransitions();checkJournal();require(journal->state().shown && !hud->state().enabled && !hud->state().absoluteEnabled,"Journal mouse open did not suspend actual HUD");
    if(count==6 || count==28) for(int index:{-7,0,int(count/2),int(count-1),int(count+7)}) {
        journal->setFirst(index);function<void(__thiscall*)(void*,int)>(0x46420)(nativeJournal,index);++journalActions;checkJournal();
        journal->setLast(index);function<void(__thiscall*)(void*,int)>(0x46740)(nativeJournal,index);++journalActions;checkJournal();
    }
    for(unsigned round=0;round<3;++round) {if(journal->upButton().state().shown) click(journal->upButton());if(journal->downButton().state().shown) click(journal->downButton());}
    click(journal->okButton());finishTransitions();checkJournal();require(hud->state().enabled && hud->state().absoluteEnabled && !hud->hasUnread,"Journal close did not resume HUD and mark messages read");
}
XmlDocument journalDefinition(files,String("UI/gameUI/Journal.xml"));journal->loadXML(journalDefinition);function<void(__thiscall*)(void*,void*)>(0x45880)(nativeJournal,wrap(&journalDefinition));++journalActions;checkJournal();
click(hud->journalButton());finishTransitions();checkJournal();click(journal->okButton());finishTransitions();checkJournal();
menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);explicitTipElapsed=true;
hudActualJournal=nullptr;const auto beforeJournal=textureCounts();journal.reset();const auto middleJournal=textureCounts();call(0x46d10,nativeJournal);compareReferenceDeltas(beforeJournal,middleJournal);require(!Journal::instance() && !field<void*>(image,0x25f764),"Journal singleton survived destruction");
