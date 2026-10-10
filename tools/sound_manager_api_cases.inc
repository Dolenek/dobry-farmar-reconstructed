// Included inside the native sound oracle after file/volume comparisons.
const auto empty=[](const String&) { return std::make_shared<XmlNode>(); };
std::array<void*,19> settingsTable{};settingsTable[8]=reinterpret_cast<void*>(&getUnsigned);settingsTable[18]=reinterpret_cast<void*>(&setUnsigned);
SettingsAdapter settingsAdapter{settingsTable.data(),nullptr};pointer(0x25f728,&settingsAdapter);
for(trial=0;trial<24;++trial) {
    Settings settings(files,empty),nativeSettings(files,empty);settingsAdapter.settings=&nativeSettings;
    const bool disabled=trial%4==0;const unsigned char initialPause=static_cast<unsigned char>(trial%2);
    if(trial%3!=0) { const unsigned value=trial%2 ? 73:0;settings.set(String("Sound"),String("Music"),value);nativeSettings.set(String("Sound"),String("Music"),value); }
    if(trial%5!=0) { const unsigned value=trial%2 ? 19:100;settings.set(String("Sound"),String("Sound"),value);nativeSettings.set(String("Sound"),String("Sound"),value); }
    ApiFixture actualApi,expectedApi;actualApi.initResult=expectedApi.initResult=disabled ? -1:0;
    activeApi=&actualApi;auto manager=std::make_unique<SoundManager>(files,settings,actualApi,initialPause);
    std::array<unsigned char,40> native{};field<unsigned char>(native.data(),4)=initialPause;
    activeApi=&expectedApi;function<void*(__thiscall*)(void*)>(0x135d50)(native.data());
    const auto check=[&]() {
        require(actualApi.events==expectedApi.events,"SoundManager API call sequence differs");
        require(manager->disabled()==bool(field<unsigned char>(native.data(),0x24)) && manager->pauseState()==field<unsigned char>(native.data(),4),"SoundManager flags differ");
        require(manager->soundVolume()==field<unsigned>(native.data(),0x18) && manager->musicVolume()==field<unsigned>(native.data(),0x14),"SoundManager user volumes differ");
        require(manager->transformedSoundVolume()==field<unsigned>(native.data(),0x20) && manager->transformedMusicVolume()==field<unsigned>(native.data(),0x1c),"SoundManager transformed volumes differ");
        require(sameSettings(settings,nativeSettings),"SoundManager Settings side effects differ");++managerStates;
    };
    check();
    const String name(names[(trial*7)%names.size()]);
    activeApi=&actualApi;auto sound=std::make_unique<cSound>(name);
    std::array<unsigned char,40> nativeSound{};activeApi=&expectedApi;function<void*(__thiscall*)(void*,const String*)>(0x159570)(nativeSound.data(),&name);++sampleObjects;
    const auto checkSound=[&]() {
        require(sound->sampleID()==field<int>(nativeSound.data(),0x24) && sound->referenceCount()==field<int>(nativeSound.data(),4),"Sound sample/refcount differs");
        require(sound->resourceName().view()==field<String>(nativeSound.data(),8).view(),"Sound resource name differs");
        require(sound->resourceType()==function<unsigned(__thiscall*)(void*)>(0x159960)(nativeSound.data()),"Sound resource type differs");check();
    };
    checkSound();
    if(disabled) {
        activeApi=&actualApi;auto channel=std::make_unique<cChannel>(123,sound.get());
        std::array<unsigned char,16> nativeChannel{};activeApi=&expectedApi;function<void*(__thiscall*)(void*,int,void*)>(0xfb8b0)(nativeChannel.data(),123,nativeSound.data());checkSound();
        activeApi=&actualApi;channel->pause();channel->start();activeApi=&expectedApi;
        function<void(__thiscall*)(void*)>(0xfbda0)(nativeChannel.data());function<void(__thiscall*)(void*)>(0xfc1d0)(nativeChannel.data());checkSound();
        activeApi=&actualApi;channel.reset();activeApi=&expectedApi;function<void*(__thiscall*)(void*,unsigned)>(0xfb700)(nativeChannel.data(),0);checkSound();++channelObjects;
    }
    for(step=0;step<80;++step) {
        const unsigned value=step%9==0 ? 0xffffffffU:random();
        activeApi=&actualApi;manager->setSoundVolume(value);activeApi=&expectedApi;function<void(__thiscall*)(void*,unsigned)>(0x132f40)(native.data(),value);check();
        const unsigned musicValue=step%7==0 ? step%101:random();
        activeApi=&actualApi;manager->setMusicVolume(musicValue);activeApi=&expectedApi;function<void(__thiscall*)(void*,unsigned)>(0x132f90)(native.data(),musicValue);check();
        const unsigned char paused=static_cast<unsigned char>((step/3)%2);
        activeApi=&actualApi;manager->setPause(paused);activeApi=&expectedApi;function<void(__thiscall*)(void*,unsigned char)>(0x132e00)(native.data(),paused);check();
        activeApi=&actualApi;manager->clickSound(sound.get());activeApi=&expectedApi;function<void(__thiscall*)(void*,void*)>(0x134b70)(native.data(),nativeSound.data());checkSound();
        for(unsigned music=0;music<2;++music) {
            actualApi.statuses=expectedApi.statuses={int(step%6)-1,int((step+2)%6)-1};actualApi.statusIndex=expectedApi.statusIndex=0;
            const bool loop=step%2!=0,start=step%3!=0;
            activeApi=&actualApi;std::unique_ptr<cChannel> channel(music ? manager->playMusic(sound.get(),loop,start):manager->playSound(sound.get(),loop,start));
            activeApi=&expectedApi;void* nativeChannel=function<void*(__thiscall*)(void*,void*,bool,bool)>(music ? 0x134be0:0x134730)(native.data(),nativeSound.data(),loop,start);
            require(bool(channel)==bool(nativeChannel),"Sound disabled play return differs");checkSound();
            if(channel) {
                require(channel->channelID()==field<int>(nativeChannel,4) && channel->group()==field<unsigned>(nativeChannel,12),"Sound channel ID/group differs");
                actualApi.statuses=expectedApi.statuses={int(step%6)-1,int((step+1)%6)-1};actualApi.statusIndex=expectedApi.statusIndex=0;
                activeApi=&actualApi;channel->pause();activeApi=&expectedApi;function<void(__thiscall*)(void*)>(0xfbda0)(nativeChannel);checkSound();
                actualApi.statuses=expectedApi.statuses={int(step%6)-1,int((step+3)%6)-1};actualApi.statusIndex=expectedApi.statusIndex=0;
                activeApi=&actualApi;channel->start();activeApi=&expectedApi;function<void(__thiscall*)(void*)>(0xfc1d0)(nativeChannel);checkSound();
                activeApi=&actualApi;channel.reset();activeApi=&expectedApi;function<void*(__thiscall*)(void*,unsigned)>(0xfb700)(nativeChannel,1);checkSound();++channelObjects;
            }
        }
    }
    activeApi=&actualApi;sound.reset();activeApi=&expectedApi;function<void(__thiscall*)(void*)>(0x159970)(nativeSound.data());check();
    // Squall Free may invoke installed close while SoundFiles is still alive.
    if(!disabled) {
        activeApi=&actualApi;actualApi.openHandle=actualApi.openCallback(name.c_str());
        activeApi=&expectedApi;expectedApi.openHandle=expectedApi.openCallback(name.c_str());
        require(actualApi.openHandle==expectedApi.openHandle && manager->files().openCount()==1 && field<unsigned>(native.data(),0x10)==1,"Sound callbacks were not bound to real file maps");
        std::array<unsigned char,64> actualBytes{},nativeBytes{};
        require(actualApi.readCallback(actualApi.openHandle,actualBytes.data(),64)==expectedApi.readCallback(expectedApi.openHandle,nativeBytes.data(),64) && actualBytes==nativeBytes,"Installed callbacks read different file bytes");
        const auto leftover=actualApi.openCallback(name.c_str()),nativeLeftover=expectedApi.openCallback(name.c_str());
        require(leftover==nativeLeftover && manager->files().openCount()==2 && field<unsigned>(native.data(),0x10)==2,"Additional destructor file cleanup fixture differs");
    }
    activeApi=&actualApi;manager.reset();activeApi=&expectedApi;function<void(__thiscall*)(void*)>(0x135280)(native.data());
    require(actualApi.events==expectedApi.events && sameSettings(settings,nativeSettings),"SoundManager destructor API/settings sequence differs");
    require(!SoundManager::instance() && !field<void*>(image,0x25f734) && allocations.empty(),"SoundManager owned resource/file map leaked");
    apiEvents+=unsigned(actualApi.events.size());++managerLifetimes;activeApi=nullptr;
}
// Source integration uses the real recovered cache and UI bindings. The native
// manager/resource routines above are the independent fidelity evidence; this
// integration case is not a native full ResourceManager map comparison.
{
    Settings settings(files,empty);ApiFixture api;activeApi=&api;
    SoundManager manager(files,settings,api,0);
    ResourceManager resources(files);ButtonInteraction interaction;
    bindButtonSound(interaction,resources,manager);
    String name(names.front()),upper(name);for(unsigned i=0;i<upper.size();++i) if(upper.data()[i]>='a' && upper.data()[i]<='z') upper.data()[i]=char(upper.data()[i]-'a'+'A');
    auto* first=resources.getSound(upper);auto* second=static_cast<cSound*>(interaction.loadSound(name));
    require(first==second && first->referenceCount()==2 && first->resourceName().view()==name.view() && resources.cachedCount()==1,"Integrated sound cache normalization/reuse differs");
    const auto before=api.events.size();interaction.playClick(first);require(api.events.size()==before+2 && api.events.back().kind==10,"Button click did not route through recovered SoundManager");
    unsigned resetRequests=0;auto options=bindOptionsSound(manager,[&](bool force) { require(force,"Options reset flag changed");++resetRequests; });
    options.setSoundVolume(27);options.setMusicVolume(63);options.resetGraphics(true);
    require(options.soundVolume()==27 && options.musicVolume()==63 && !options.soundDisabled() && resetRequests==1,"Options services did not route through recovered SoundManager");
    bool wrongType=false;try { resources.getFont(name); } catch(const char* message) { wrongType=std::strcmp(message,"Wrong resource type.")==0; }
    require(wrongType,"Sound cache accepted an incompatible resource type");
    resources.deleteAllSound();require(resources.cachedCount()==0 && api.events.back().kind==8,"Sound cache forced deletion/unload did not run");
}
activeApi=nullptr;
