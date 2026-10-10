// Source LoadingFromLog over the two ACTUAL logs and both physical DLLs.
// Completion is an explicitly counted diagnostic callback, never a fake cGame.
ViewRegistry views;ControlRegistry controls;MusicPlayback loadingMusic;
loadingAudioCompletions=0;sound.setPause(0);sound.setMusicVolume(0);sound.setSoundVolume(0);
loadingMusic.replace(resources,sound,String("Sounds/Menu_music.ogg"));
record("loading old music status",backend.channelStatus(loadingMusic.channel()->channelID()));
auto loadingProgress=[&] {
    const int channel=loadingMusic.channel()->channelID();
    require(dll.SQUALL_Channel_GetVolume(channel)==0,"Loading music is not silent");
    const int before=dll.SQUALL_Channel_GetPlayPosition(channel);require(before>=0,"Loading music initial cursor failed");
    int after=before;const auto deadline=GetTickCount64()+1500;
    while(after==before && GetTickCount64()<deadline) {Sleep(10);after=dll.SQUALL_Channel_GetPlayPosition(channel);}
    require(after>=0 && after!=before,"Loading music did not physically play");++result.cursorProgressChecks;result.positionProgress+=unsigned(after-before);
};
for(unsigned menu=0;menu<2;++menu) {
    auto* screen=LoadingFromLog::create(files,resources,settings,strings,sound,loadingMusic,String(menu ? "MenuResLog.xml":"GameResLog.xml"),&loadingAudioComplete,30,menu!=0,views,controls);
    require(screen && screen->total()==(menu ? 148U:539U),"Loading actual shipped log missing");
    record("loading total",int(screen->total()));record("loading initial callback",int(loadingAudioCompletions));
    require(loadingMusic.sound()->resourceName().view()==(menu ? "sounds/menu_music.ogg":"sounds/game_music.ogg"),"Loading wrong physical music resource");
    const int channel=loadingMusic.channel()->channelID();
    record("loading music status",backend.channelStatus(channel));record("loading music loop",dll.SQUALL_Channel_GetLoop(channel));record("loading music group",int(loadingMusic.channel()->group()));
    loadingProgress();loadingMusic.channel()->pause();record("loading paused status",backend.channelStatus(channel));
    const int paused=dll.SQUALL_Channel_GetPlayPosition(channel);require(paused>=0,"Loading paused cursor failed");Sleep(40);
    require(dll.SQUALL_Channel_GetPlayPosition(channel)==paused,"Loading paused cursor moved");++result.pausedCursorChecks;
    loadingMusic.channel()->start();record("loading resumed status",backend.channelStatus(channel));loadingProgress();
    for(unsigned batch=0;screen->nextIndex()<screen->total();++batch) {
        require(batch<30,"Loading failed to finish its real log");screen->enabledStep();
        record("loading next index",int(screen->nextIndex()));unsigned progressBits;const float value=screen->progress().current;std::memcpy(&progressBits,&value,4);record("loading progress",int(progressBits));
        record("loading callbacks",int(loadingAudioCompletions));
    }
    require(loadingAudioCompletions==menu+1 && views.discarded().size()==1,"Loading physical completion/discard missing");
    record("loading cache count",int(resources.cachedCount()));record("loading screen shown",screen->state().shown);
    record("loading music after completion",backend.channelStatus(channel));
    views.deleteDiscarded();require(controls.visible().empty() && views.enabled().empty(),"Loading frame deletion retained controls");
    record("loading XML deleted",int(resources.cachedCount()));
}
require(!files.isExist(String("missing-loading-audio-oracle.xml")),"Missing Loading test file unexpectedly exists");
auto* absent=LoadingFromLog::create(files,resources,settings,strings,sound,loadingMusic,String("missing-loading-audio-oracle.xml"),&loadingAudioComplete,30,true,views,controls);
require(!absent && loadingAudioCompletions==3 && controls.visible().empty(),"Missing Loading log lifecycle differs");
record("missing loading callbacks",int(loadingAudioCompletions));record("missing loading music",backend.channelStatus(loadingMusic.channel()->channelID()));loadingProgress();
loadingMusic.clear();
