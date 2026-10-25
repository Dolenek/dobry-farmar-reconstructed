stage=290;
const auto common=std::filesystem::path(".local")/(std::string("game-pause-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
std::filesystem::create_directories(common);files.mountCommon(String(common.generic_string()));
String previousProfiles;settings.get(String("game"),String("profiles"),previousProfiles);settings.set(String("game"),String("profiles"),String("PauseMenuProfiles"));
LoadingDisabledAudio disabledBackend;SoundManager audio(files,settings,disabledBackend,0);MusicPlayback music;
ProfileSession profiles(files,settings,manager);profiles.open(WString(L"Alpha"));profiles.open(WString(L"Zebra"));profiles.open(WString(L"Alpha"));
selectorNames=profiles.current()->headers().names();selectorNextID=profiles.current()->headers().nextID();selectorEvents.clear();
auto* nativeProfile=allocate(88);selectorProfileConstruct(nativeProfile,nullptr,&profiles.current()->headers().currentName());selectorEvents.clear();
loadingFiles=&files;loadingSoundManager=&audio;
managerTable[6]=reinterpret_cast<void*>(&loadingXML);managerTable[14]=reinterpret_cast<void*>(&mainMenuNativeLog);
managerTable[15]=image+0x141700;managerTable[17]=reinterpret_cast<void*>(&loadingClear);
xmlTable[0]=reinterpret_cast<void*>(&loadingDeleteXML);xmlTable[34]=reinterpret_cast<void*>(&sectionCount);
settingsTable[10]=reinterpret_cast<void*>(&mainMenuSettingsBool);
redirect(0xecb20,reinterpret_cast<void*>(&selectorProfileConstruct));redirect(0xe8900,reinterpret_cast<void*>(&selectorProfileNames));redirect(0xe8d20,reinterpret_cast<void*>(&selectorProfileRemove));
redirect(0x2fe80,reinterpret_cast<void*>(&decimalString));redirect(0x1cc0,reinterpret_cast<void*>(&concatenateString));redirect(0x7d0b0,reinterpret_cast<void*>(&nativeDiscard));
redirect(0x842d0,reinterpret_cast<void*>(&mainMenuNativeDiscard));redirect(0x832f0,reinterpret_cast<void*>(&mainMenuNativeComplete));
redirect(0xa670,reinterpret_cast<void*>(&awardsProfileKey));redirect(0x105e0,reinterpret_cast<void*>(&awardsProfileBool));
graphicsTable[3]=reinterpret_cast<void*>(&tooltipHeight);graphicsTable[4]=reinterpret_cast<void*>(&tooltipWidth);timerTable[3]=reinterpret_cast<void*>(&tooltipUnscaledTimer);
nativeOptionsAudio={audio.disabled(),audio.soundVolume(),audio.musicVolume(),{}};nativeOptionsFull=false;settings.set(String("graphics"),String("fullscreen"),false);
std::array<void*,7> loadingFileTable{};loadingFileTable[6]=reinterpret_cast<void*>(&loadingExists);auto* loadingFilePointer=loadingFileTable.data();putPointer(0x25f72c,&loadingFilePointer);
soundTable[3]=reinterpret_cast<void*>(&loadingMusicChannel);graphicsTable[5]=reinterpret_cast<void*>(&optionsReset);
void* quitFunction=reinterpret_cast<void*>(&PostQuitMessage);write(0x1ee268,&quitFunction,4);
ViewRegistry views;ControlRegistry controls;unsigned resets=0;
MainMenuContext context(files,manager,settings,strings,audio,music,profiles,input,interaction,[] {return tooltipDimensions;},
    [&](bool) {++resets;},&mainMenuSourceComplete,views,controls);
View game(nullptr,views),placeholderHelp(nullptr,views);auto* nativeGame=allocate(44);auto* nativeHelp=allocate(helpMode ? 92:44);
function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeGame,nullptr);putPointer(0x25f6ec,nativeGame);
HelpViewContext helpContext{manager,settings,strings,input,interaction,views,controls};std::unique_ptr<HelpView> actualHelp;
if(helpMode) {actualHelp=std::make_unique<HelpView>(nullptr,helpContext);std::memset(nativeHelp,0,92);function<void*(__thiscall*)(void*,void*)>(0x7fa60)(nativeHelp,nullptr);}
else {function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeHelp,nullptr);putPointer(0x25f778,nativeHelp);}
View& help=helpMode ? static_cast<View&>(*actualHelp):placeholderHelp;
game.show();game.enable();game.absoluteEnable();call(0xd37d0,nativeGame);call(0xd3bb0,nativeGame);call(0xd3fb0,nativeGame);
redirect(0x86970,reinterpret_cast<void*>(&pauseNativeGameDiscard));pauseNativeGameDiscards.clear();pauseSourceGameDiscards.clear();
GamePauseContext pauseContext{files,manager,settings,strings,audio,music,input,interaction,game,[&]() -> View& {return help;},context,[] {return tooltipDimensions;},[&](bool){++resets;},[&](const String& origin){pauseSourceGameDiscards.emplace_back(origin.view());},views,controls};
auto* pause=new GamePauseMenu(nullptr,pauseContext);auto* original=allocate(80);function<void*(__thiscall*)(void*,void*)>(0x87330)(original,nullptr);auto* nativeView=static_cast<unsigned char*>(original)+4;
const auto check=[&] {
    stage=291;require(GamePauseMenu::instance()==pause && field<void*>(image,0x25ffd8)==original,"Pause singleton differs");
    for(auto pair:std::initializer_list<std::pair<View*,void*>>{{pause,nativeView},{&game,nativeGame},{&help,nativeHelp}}) if(pair.first->state().transparency!=field<float>(pair.second,8)) std::cerr<<"Pause checkpoint "<<pauseStates<<" object "<<(pair.first==pause ? "pause":pair.first==&game ? "game":"help")<<" alpha "<<pair.first->state().transparency<<"/"<<field<float>(pair.second,8)<<"\n";
    mainMenuCompareView(*pause,nativeView);mainMenuCompareView(game,nativeGame);mainMenuCompareView(help,nativeHelp);compareDecorations(*pause,nativeView);
    for(const auto& pair:std::initializer_list<std::pair<TextAnimationButton*,unsigned>>{{&pause->continueButton(),0x3c},{&pause->awardsButton(),0x44},{&pause->optionsButton(),0x40},{&pause->mainMenuButton(),0x48},{&pause->helpButton(),0x4c}}) {
        auto* native=field<void*>(original,pair.second);compareControl(*pair.first,native);require(unsigned(pair.first->buttonState())==field<unsigned>(native,0x238),"Pause owned button state differs");pauseLetters+=compare(pair.first->text(),field<void*>(native,0x27c));
    }
    for(unsigned control:{1U,0x1cU,0x39U}) require(input.controls(control).size()==nativeInput.controls(control).size(),"Pause input binding count differs");
    require(pauseNativeGameDiscards==pauseSourceGameDiscards,"Pause game discard request/origin differs");++pauseStates;
};check();
menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
const auto frame=[&](Point2 cursor,std::vector<unsigned> events={},std::array<int,3> delta={}) {stage=292;menuMilliseconds+=50;menuPoint=cursor;menuInputEvents=std::move(events);menuMouseDelta=delta;pausePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++pauseFrames;};
const auto finishTransitions=[&] {for(unsigned n=0;n<24 && !views.transitions().empty();++n) frame({0,0});require(views.transitions().empty() && field<unsigned>(image,0x260528)==0,"Pause transition did not finish");};
const auto deleteDiscarded=[&] {
    auto expected=nativeDiscards;mainMenuNativeDiscardTree(field<void*>(field<void*>(image,0x2604e8),4),expected);
    require(expected.size()==views.discarded().size(),"Pause discard queue size differs");std::multiset<std::string> sourceOrigins,nativeOrigins;
    for(const auto& entry:views.discarded()) sourceOrigins.emplace(entry.second.view());for(const auto& entry:expected) nativeOrigins.emplace(entry.second);
    require(sourceOrigins==nativeOrigins,"Pause discard queue origins differ");views.deleteDiscarded();for(const auto& entry:expected) mainMenuDeleteNative(entry.first);nativeDiscards.clear();require(field<unsigned>(image,0x2604ec)==0,"Pause destruction retained native discard tree");
};
const auto click=[&](TextAnimationButton& button) {const Point2 point{button.geometry().x,button.geometry().y};frame(point,{}, {1,0,0});frame(point,{0x208});frame(point,{0x200});frame({0,0},{},{1,0,0});++pauseActions;};
const auto open=[&] {
    game.disable();game.absoluteDisable();call(0xd3750,nativeGame);call(0xd3790,nativeGame);
    pause->showTransparently();pause->enable();call(0xd4590,nativeView);call(0xd3bb0,nativeView);finishTransitions();check();
};
// Exact resume order and both actual keyboard bindings.
for(trial=0;trial<3;++trial) {open();if(trial==0) click(pause->continueButton());else {frame({0,0},{trial==1 ? 1U:0x1cU});++pauseActions;}finishTransitions();check();require(game.state().enabled && game.state().absoluteEnabled && !pause->state().shown,"Pause resume did not restore game");}
// Native XML reload uses the MenuView subobject at +4.
const String partialName("pause-partial.xml");const wchar_t partialText[]=L"<Menu Visible=\"0\" Enable=\"1\"/>";
{auto file=files.create(partialName);file->write(partialText,sizeof(partialText)-2);}XmlDocument partial(files,partialName);
pause->loadXML(partial);function<void(__thiscall*)(void*,void*)>(0x85680)(nativeView,wrap(&partial));++pauseReloads;check();
const auto overwrittenSoundCount=sounds.size();
XmlDocument definition(files,String("UI/Menu.xml"));pause->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0x85680)(nativeView,wrap(&definition));++pauseReloads;check();
// Complete actual Options navigation, including reuse of an existing singleton.
open();click(pause->optionsButton());auto* options=Options::instance();auto* nativeOptions=field<void*>(image,0x26000c);require(options && nativeOptions,"Pause did not construct Options");mainMenuCompareView(*options,static_cast<unsigned char*>(nativeOptions)+4);check();
require(pause->openOptions()==options,"Pause did not reuse existing Options");function<void(__cdecl*)()>(0x85590)();++pauseActions;mainMenuCompareView(*options,static_cast<unsigned char*>(nativeOptions)+4);check();
finishTransitions();click(options->okButton());finishTransitions();deleteDiscarded();check();
// Awards hides the game; return restores pause/game visibility, keeping game paused.
click(pause->awardsButton());auto* awards=Awards::instance();auto* nativeAwards=field<void*>(image,0x25ffcc);require(awards && nativeAwards,"Pause did not construct Awards");mainMenuCompareView(*awards,static_cast<unsigned char*>(nativeAwards)+4);check();
require(!pause->openAwards(),"Pause duplicate Awards guard differs");function<void(__cdecl*)()>(0x86550)();++pauseActions;check();
finishTransitions();click(awards->returnButton());finishTransitions();deleteDiscarded();check();require(game.state().shown && !game.state().enabled,"Pause Awards return resumed gameplay");
if(helpMode) {
#include "help_view_oracle_cases.inc"
} else {
// The older pause-only comparison retains its explicit Help boundary.
click(pause->helpButton());finishTransitions();check();help.hide();help.disable();call(0xd3720,nativeHelp);call(0xd3750,nativeHelp);open();
}
const auto findDialog=[&]() -> YesNoDialog* {for(auto* view:views.enabled()) if(auto* dialog=dynamic_cast<YesNoDialog*>(view)) return dialog;return nullptr;};
click(pause->mainMenuButton());auto* dialog=findDialog();auto* nativeDialog=pauseLastNativeDialog;require(dialog && nativeDialog,"Pause did not construct YesNo dialog");
mainMenuCompareView(*dialog,nativeDialog);require(dialog->text().string().view()==field<WString>(field<void*>(nativeDialog,0x240),0x23c).view(),"Pause confirmation raw Russian text differs");check();
click(dialog->noButton());finishTransitions();deleteDiscarded();check();require(pause->state().shown && pause->state().enabled,"Pause No did not restore pause menu");
click(pause->mainMenuButton());dialog=findDialog();nativeDialog=pauseLastNativeDialog;require(dialog && nativeDialog,"Pause second YesNo missing");check();
managerTable[3]=reinterpret_cast<void*>(&loadingSoundResource);click(dialog->yesButton());deleteDiscarded();check();
LoadingFromLog* loading=nullptr;for(auto* view:views.enabled()) if(auto* candidate=dynamic_cast<LoadingFromLog*>(view)) {loading=candidate;break;}
auto* nativeLoading=mainMenuLastNativeLoading;require(loading && nativeLoading,"Pause Yes did not construct real Loading");
require(pauseSourceGameDiscards==std::vector<std::string>{".\\Game\\Ui\\Menu.cpp185"},"Pause Yes game discard origin differs");
for(unsigned n=0;n<60 && !MainMenu::instance();++n) {
    loading->enabledStep();call(0x81130,nativeLoading);require(loading->nextIndex()==field<unsigned>(nativeLoading,0x78) && loading->total()==field<unsigned>(nativeLoading,0x7c) && loading->batch()==3 && field<unsigned>(nativeLoading,0x80)==3,"Pause menu Loading range differs");
    require(bool(MainMenu::instance())==bool(field<void*>(image,0x25ffd0)),"Pause actual MainMenu completion order differs");++pauseLoadingSteps;
}
require(MainMenu::instance() && field<void*>(image,0x25ffd0),"Pause real Loading did not construct actual MainMenu");deleteDiscarded();
auto* menu=MainMenu::instance();auto* nativeMain=static_cast<unsigned char*>(field<void*>(image,0x25ffd0))+4;
mainMenuCompareView(*menu,nativeMain);compareDecorations(*menu,nativeMain);mainMenuCompareView(*context.fixedBack(),field<void*>(image,0x25ffec));
for(const auto& pair:std::initializer_list<std::pair<TextAnimationButton*,unsigned>>{{menu->content().start,0x3c},{menu->content().exit,0x40},{menu->content().changeUser,0x44},{menu->content().options,0x4c},{menu->content().awards,0x54},{menu->content().credits,0x50}}) {auto* native=field<void*>(static_cast<unsigned char*>(nativeMain)-4,pair.second);compareControl(*pair.first,native);pauseLetters+=compare(pair.first->text(),field<void*>(native,0x27c));}
finishTransitions();check();delete menu;mainMenuDeleteNative(nativeMain);
// Compare native MenuView resource destruction and singleton order.
pause->disable();call(0xd3750,nativeView);const auto before=textureCounts();delete pause;const auto middle=textureCounts();call(0x87980,original);compareReferenceDeltas(before,middle);
require(!GamePauseMenu::instance() && !field<void*>(image,0x25ffd8),"Pause destruction retains singleton");
for(unsigned n=0;n<overwrittenSoundCount;++n) {require(sounds[n]->referenceCount()==1,"Pause native/source XML sound overwrite retention differs");sounds[n]->releaseReference();++pauseSoundResidues;}
call(0xd58a0,nativeGame);if(helpMode) {const auto hb=textureCounts();actualHelp.reset();const auto hm=textureCounts();call(0x7dde0,nativeHelp);compareReferenceDeltas(hb,hm);require(!HelpView::instance() && !field<void*>(image,0x25f778),"Help singleton survived destruction");} else call(0xd58a0,nativeHelp);putPointer(0x25f6ec,nullptr);putPointer(0x25f778,nullptr);
music.clear();if(auto* resource=field<cSound*>(image,0x25ffe0)) resource->releaseReference();putPointer(0x25ffe0,nullptr);putPointer(0x25ffdc,nullptr);
profiles.close();selectorProfileDestroy(field<void*>(image,0x25f738),nullptr,1);settings.set(String("game"),String("profiles"),previousProfiles);
require(controls.visible().empty() && input.controls(1).empty() && nativeInput.controls(1).empty(),"Pause retained controls/input");
menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);loadingDocuments.clear();loadingFiles=nullptr;loadingSoundManager=nullptr;profileProperties().booleans.clear();
