// Actual manager -> AwardEffect -> Awards -> AwardPanel -> return to game.
// Six coherent processes bound the existing monotonic diagnostic allocator.
// Predicates7/9/18 require the later complete tool/Manufacture/Market world;
// their existing owned flags are explicit inputs here, never substituted bodies.
if(awardManagerGroup==0) {
#include "ui_constructor_failure_cases.inc"
}
stage=940;redirect(0x7d0b0,reinterpret_cast<void*>(&nativeDiscard));redirect(0x180aed,reinterpret_cast<void*>(&awardsRegisterExit));redirect(0x2fe80,reinterpret_cast<void*>(&decimalString));redirect(0x1cc0,reinterpret_cast<void*>(&concatenateString));redirect(0x77290,reinterpret_cast<void*>(&nativeDiscard));
const std::array<std::array<unsigned,3>,6> awardGroups={{{0,1,2},{3,4,5},{6,8,10},{11,12,13},{14,15,16},{17,19,20}}};
const auto& awardGroup=awardGroups.at(awardManagerGroup);
for(unsigned i=0;i<21;++i) {const bool owned=std::find(awardGroup.begin(),awardGroup.end(),i)==awardGroup.end();const std::string key(Awards::conditionKey(i).view());profile.booleans[key]=nativeProfile.booleans[key]=owned;}
for(const char* key:{"awQCollect","awQProcess","awQTask","awQDestroy","awAdecvClick"}) profile.uints[key]=nativeProfile.uints[key]=0;
for(const char* key:{"awEarn","awHighSold"}) profile.booleans[key]=nativeProfile.booleans[key]=false;
profile.ints["awGrowType"]=nativeProfile.ints["awGrowType"]=0;profile.ints["Money"]=nativeProfile.ints["Money"]=0;
profile.floats["RunTime"]=nativeProfile.floats["RunTime"]=0;profile.floats["awNoDryTime"]=nativeProfile.floats["awNoDryTime"]=0;
for(unsigned i=0;i<11;++i) profile.booleans["awManDepotBuild"+std::to_string(i)]=nativeProfile.booleans["awManDepotBuild"+std::to_string(i)]=false;
hud->refreshMoney();call(0xbfee0,original);check();actualSounds.clear();expectedSounds.clear();
GameAwardContext awardContext{manager,settings,strings,audio,input,interaction,profile,game,world,[] {return tooltipDimensions;},views,controls};
auto awardManager=std::make_unique<GameAwardManager>(awardContext);stage=9401;auto* nativeManager=allocate(224);std::memset(nativeManager,0,224);function<void*(__thiscall*)(void*)>(0xc7d00)(nativeManager);stage=9402;
awardManager->disable();call(0xd3750,nativeManager);
const auto checkAwards=[&] {
    stage=941;require(GameAwardManager::instance()==awardManager.get() && field<void*>(image,0x25f6e8)==nativeManager,"Gameplay award singleton differs");mainMenuCompareView(*awardManager,nativeManager);
    for(unsigned i=0;i<21;++i) require(awardManager->attained[i]==bool(field<unsigned char>(nativeManager,0x34+i*8)),"Gameplay award cached condition differs");
    require(awardManager->requested==field<int>(nativeManager,0xdc),"Gameplay award request differs");auto* ne=field<void*>(nativeManager,0xd8);require(bool(awardManager->pending)==bool(ne),"Gameplay award pending lifetime differs");
    if(auto* effect=awardManager->pending) {mainMenuCompareView(*effect,ne);compareControl(*effect,ne);const auto& s=effect->effectState();require(effect->index()==field<unsigned>(ne,0x2bc) && s.phase==field<unsigned>(ne,0x238),"Gameplay award effect identity/phase differs");compareFloat(s.scale,field<float>(ne,0x250),"Gameplay award effect scale");require(!std::memcmp(s.destination.data(),static_cast<unsigned char*>(ne)+0x29c,8),"Gameplay award destination differs");}
    auto* screen=Awards::instance();auto* ns=field<void*>(image,0x25ffcc);require(bool(screen)==bool(ns),"Gameplay award screen lifetime differs");
    if(screen) {mainMenuCompareView(*screen,static_cast<unsigned char*>(ns)+4);compareDecorations(*screen,static_cast<unsigned char*>(ns)+4);for(unsigned i=0;i<21;++i) {auto& b=screen->button(i);auto* nb=field<void*>(ns,0x44+i*4);compareControl(b,nb);require(b.owned()==field<unsigned char>(nb,0x280),"Gameplay award screen ownership differs");}}
    require(actualSounds==expectedSounds,"Gameplay award UI sound requests differ");actualSounds.clear();expectedSounds.clear();check();++awardManagerStates;
};checkAwards();
menuMilliseconds=0;explicitTipElapsed=false;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
std::array<unsigned char,24> frameNativeCursor{};field<void*>(frameNativeCursor.data(),0)=cursorTable.data();nativeCursorFrame=frameNativeCursor.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);timerTable[3]=reinterpret_cast<void*>(&tooltipUnscaledTimer);graphicsTable[3]=reinterpret_cast<void*>(&tooltipHeight);graphicsTable[4]=reinterpret_cast<void*>(&tooltipWidth);
const auto frame=[&](Point2 point=Point2{0,0},std::vector<unsigned> events={},std::array<int,3> delta={}) {stage=942;menuMilliseconds+=50;menuPoint=point;menuInputEvents=std::move(events);menuMouseDelta=delta;awardManagerPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++awardManagerFrames;checkAwards();};
const auto discard=[&] {auto expected=nativeDiscards;mainMenuNativeDiscardTree(field<void*>(field<void*>(image,0x2604e8),4),expected);std::multiset<std::string> a,b;for(const auto& e:views.discarded()) a.emplace(e.second.view());for(const auto& e:expected) b.emplace(e.second);require(a==b,"Gameplay award discard origins differ");views.deleteDiscarded();for(const auto& e:expected) mainMenuDeleteNative(e.first);nativeDiscards.clear();};
const auto click=[&](ButtonPrototype& button) {Point2 point{button.geometry().x,button.geometry().y};frame(point,{}, {1,0,0});frame(point,{0x208});frame(point,{0x200});frame({0,0},{},{1,0,0});++awardManagerActions;};
const auto setInput=[&](unsigned index,bool positive) {
    const auto u=[&](const char* key,unsigned low,unsigned high) {profile.uints[key]=nativeProfile.uints[key]=positive ? high:low;};
    const auto f=[&](const char* key,float low,float high) {profile.floats[key]=nativeProfile.floats[key]=positive ? high:low;};
    const auto b=[&](const char* key) {profile.booleans[key]=nativeProfile.booleans[key]=positive;};
    switch(index) {
    case 0:u("awQCollect",99,100);break;case 1:u("awQProcess",99,100);break;case 2:u("awQCollect",999,1000);break;case 3:u("awQProcess",999,1000);break;
    case 4:u("awQTask",19,20);break;case 5:u("awQTask",39,40);break;case 6:b("awEarn");break;
    case 8:for(unsigned i=0;i<11;++i) b(("awManDepotBuild"+std::to_string(i)).c_str());break;
    case 10:u("awQDestroy",19,20);break;case 11:profile.ints["awGrowType"]=nativeProfile.ints["awGrowType"]=positive ? 255:254;break;
    case 12:f("awNoDryTime",1199,1200);break;case 13:f("RunTime",1799,1800);break;case 14:f("RunTime",10799,10800);break;case 15:b("awHighSold");break;
    case 16:profile.ints["Money"]=nativeProfile.ints["Money"]=positive ? 1000:999;break;case 17:profile.ints["Money"]=nativeProfile.ints["Money"]=positive ? 10000:9999;break;
    case 19:u("awAdecvClick",29,30);break;case 20:b(std::string(Awards::conditionKey(0).view()).c_str());break;
    default:throw std::runtime_error("Unexpected scoped award predicate");
    }
};
for(unsigned index:awardGroup) {
    trial=index;setInput(index,false);require(!awardManager->condition(index) && !reinterpret_cast<unsigned char(__cdecl*)()>(field<void*>(nativeManager,0x30+index*8))(),"Gameplay award below-threshold predicate differs");++awardManagerActions;checkAwards();
    setInput(index,true);require(awardManager->condition(index) && reinterpret_cast<unsigned char(__cdecl*)()>(field<void*>(nativeManager,0x30+index*8))(),"Gameplay award exact-threshold predicate differs");++awardManagerActions;checkAwards();
    awardManager->enable();call(0xd3bb0,nativeManager);
    AwardPanel* panel=nullptr;
    for(unsigned i=0;i<180 && !panel;++i) {frame();for(auto* view:views.enabled()) if(auto* candidate=dynamic_cast<AwardPanel*>(view)) panel=candidate;}
    require(panel && awardManager->pending && awardManager->pending->index()==index,"Gameplay award did not construct actual matching AwardPanel");
    for(unsigned i=0;i<24 && !views.transitions().empty();++i) frame();click(panel->okButton());discard();
    for(unsigned i=0;i<180 && awardManager->pending;++i) frame();require(!awardManager->pending,"Gameplay award effect did not arrive and dispose");
    awardManager->disable();call(0xd3750,nativeManager);checkAwards();
    require(profile.booleans[std::string(Awards::conditionKey(index).view())] && profile.booleans["awardShow"+std::to_string(index)],"Gameplay award did not commit condition/display flags");
}
auto* screen=Awards::instance();require(screen,"Gameplay award final screen missing");click(screen->returnButton());for(unsigned i=0;i<24 && !views.transitions().empty();++i) frame();discard();checkAwards();require(game.state().enabled && game.state().absoluteEnabled && !Awards::instance(),"Gameplay award return did not resume whole game");
menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);explicitTipElapsed=true;
const auto beforeManager=textureCounts();awardManager.reset();const auto middleManager=textureCounts();call(0xc8630,nativeManager);compareReferenceDeltas(beforeManager,middleManager);require(!GameAwardManager::instance() && !field<void*>(image,0x25f6e8),"Gameplay award singleton survived destruction");
