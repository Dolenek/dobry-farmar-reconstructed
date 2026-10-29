stage=230;redirect(0x10770,reinterpret_cast<void*>(&upgradeProperty));
redirect(0xa670,reinterpret_cast<void*>(&gameConfigurationLevel));
redirect(0x6b00,reinterpret_cast<void*>(&upgradeStreamConstruct));redirect(0x5060,reinterpret_cast<void*>(&listStreamNoop));
redirect(0x6700,reinterpret_cast<void*>(&upgradeWriteLiteral));redirect(0x6570,reinterpret_cast<void*>(&upgradeWriteIndex));
redirect(0xa450,reinterpret_cast<void*>(&upgradeStreamContents));redirect(0x59e0,reinterpret_cast<void*>(&listStreamNoop));
xmlTable[34]=reinterpret_cast<void*>(&sectionCount);xmlTable[23]=reinterpret_cast<void*>(&gameConfigurationUnsigned);
LoadingDisabledAudio backend;UpgradeAudio audio(files,settings,backend);upgradeAudio=&audio;
GameConfiguration configuration(manager);XmlDocument game(files,String("game.xml"));configuration.loadXML(game);
std::array<unsigned char,2716> nativeGame{};std::memcpy(nativeGame.data()+0x8fc,configuration.costs.data(),240);
field<cResource*>(nativeGame.data(),0xa08)=configuration.upgradeSound();putPointer(0x25f6ec,nativeGame.data());
soundTable[1]=reinterpret_cast<void*>(&upgradePlaySound);
const auto directory=std::filesystem::path(".local")/(std::string("upgrade-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));std::filesystem::create_directories(directory);
{
    std::ofstream(directory/"all.xml")<<"<Object><Upgrades><Up0/><Up1/><Up2/><Up3/><Up4/><Up5/></Upgrades></Object>";
    std::ofstream(directory/"sparse.xml")<<"<Object><Upgrades><Up0/><Other/><Up3/></Upgrades></Object>";
    std::ofstream(directory/"last.xml")<<"<Object><Upgrades><Up3/></Upgrades></Object>";
    std::ofstream(directory/"empty.xml")<<"<Object/>";
}
FileIndex fixtures(String(directory.generic_string()),String("pack"));XmlDocument all(fixtures,String("all.xml")),sparse(fixtures,String("sparse.xml")),last(fixtures,String("last.xml")),empty(fixtures,String("empty.xml"));
std::mt19937 random(0x4174b0);
for(trial=0;trial<16;++trial) {
    stage=231;const unsigned type=trial<15 ? trial:0xffffffffU,id=trial<15 ? trial:0x80000001U;
    ProfileProperties sourceProfile,nativeProfile;upgradeNativeProfile=&nativeProfile;
    UpgradeContext context(configuration,sourceProfile,audio);auto source=std::make_unique<UpgradeTrace>(context,type,id);
    std::array<unsigned char,44> native{};function<void(__thiscall*)(void*,unsigned,unsigned)>(0x169c0)(native.data(),type,id);
    std::array<void*,3> table{image+0x169a0,reinterpret_cast<void*>(&upgradeRefresh),reinterpret_cast<void*>(&upgradeLoadLevel)};field<void*>(native.data(),0)=table.data();
    upgradeNativeLoads.clear();upgradeNativeRefreshes.clear();upgradeActualSounds.clear();upgradeNativeSounds.clear();
    const auto check=[&] {
        stage=232;require(source->itemType==field<unsigned>(native.data(),4) && source->propertyId==field<unsigned>(native.data(),0x24),"Upgrade identifiers differ");
        require(source->levelCount==field<int>(native.data(),0x28) && source->name.view()==field<String>(native.data(),8).view(),"Upgrade scalar/name state differs");
        require(sourceProfile.ints==nativeProfile.ints && sourceProfile.uints==nativeProfile.uints,"Upgrade profile maps differ");
        require(source->loads==upgradeNativeLoads && source->refreshes==upgradeNativeRefreshes,"Upgrade level/refresh dispatch differs");
        require(upgradeActualSounds==upgradeNativeSounds && !context.channel() && !field<void*>(nativeGame.data(),0xa0c),"Upgrade disabled sound dispatch differs");++upgradeStates;
    };check();
    for(const XmlNode* node:{&all,&sparse,&last,&empty,&all}) {
        stage=233;source->loadXML(*node);function<void(__thiscall*)(void*,void*)>(0x16a30)(native.data(),wrap(node));++upgradeReloads;check();
    }
    upgradeMutateCount=true;source->loadXML(all);function<void(__thiscall*)(void*,void*)>(0x16a30)(native.data(),wrap(&all));upgradeMutateCount=false;++upgradeReloads;check();
    for(action=0;action<2048;++action) {
        stage=234;const auto key="Upgrade"+std::to_string(id),availableKey="UpAvail"+std::to_string(type);
        const std::array<int,10> levels{-1,0,1,2,3,4,-17,INT_MAX,INT_MIN,99};
        const int current=levels[random()%levels.size()];sourceProfile.ints[key]=nativeProfile.ints[key]=current;
        const unsigned moneyBits=random();int money;std::memcpy(&money,&moneyBits,4);sourceProfile.ints["Money"]=nativeProfile.ints["Money"]=money;
        const unsigned available=random();sourceProfile.uints[availableKey]=nativeProfile.uints[availableKey]=available;
        const int count=action%7==0 ? INT_MIN:action%7==1 ? INT_MAX:action%7==2 ? -1:int(random()%6);
        source->levelCount=field<int>(native.data(),0x28)=count;
        const bool checkMoney=type<15 && current>=-1 && current<3 && action%2,checkAvailable=action%3!=0;
        if(type<15) for(unsigned level=0;level<4;++level) {
            configuration.costs[type][level]=random();field<unsigned>(nativeGame.data(),0x8fc+(type*4+level)*4)=configuration.costs[type][level];
            require(configuration.upgradeCost(type,level)==function<unsigned(__thiscall*)(void*,unsigned,unsigned)>(0xba00)(nativeGame.data(),type,level),"Upgrade cost lookup differs");++upgradeCosts;
        }
        const auto actual=source->canUpgrade(checkMoney,checkAvailable),expected=function<bool(__thiscall*)(void*,bool,bool)>(0x17430)(native.data(),checkMoney,checkAvailable);
        require(actual==expected,"Upgrade availability/money result differs");++upgradeQueries;check();
        const int requested=levels[random()%levels.size()];const bool play=action%4!=0;
        source->setLevel(requested,play);function<void(__thiscall*)(void*,int,bool)>(0x174b0)(native.data(),requested,play);++upgradeChanges;check();
        require(source->level()==function<int(__thiscall*)(void*)>(0x17050)(native.data()),"Upgrade current level getter differs");
    }
    stage=235;upgradeLevelLoads+=unsigned(source->loads.size());upgradeSoundCalls+=unsigned(upgradeActualSounds.size());source.reset();call(0x16970,native.data());
    require(field<String>(native.data(),8).empty(),"Upgrade native name did not clear");++upgradeObjects;
}
// Same bytes as the unadjusted focused View passed by original0040be10.
// All booleans remain valid; only the two original padding bytes vary.
for(unsigned flags=0;flags<16;++flags) for(unsigned absolute=0;absolute<4;++absolute)
    for(unsigned padding:{0U,1U,0x7fffU,0x8000U,0xffffU,0xbaadU,0xcdcdU,0xfeeU})
    for(unsigned initial=0;initial<6;++initial) {
        stage=236;unsigned id=0;
        for(unsigned bit=0;bit<4;++bit) if(flags&(1U<<bit)) id|=1U<<(bit*8);
        const unsigned countBits=(padding<<16)|((absolute&2U) ? 256U:0U)|(absolute&1U);
        ProfileProperties sourceProfile,nativeProfile;upgradeNativeProfile=&nativeProfile;
        const auto key="Upgrade"+std::to_string(id);const std::array<unsigned,5> values{0U,0xffffffffU,0x80000000U,0x7fffffffU,0xbaad0100U};
        if(initial){int value;const unsigned bits=values[initial-1];std::memcpy(&value,&bits,4);sourceProfile.ints[key]=nativeProfile.ints[key]=value;}
        UpgradeAliasView view;view.words(id,countBits);const auto words=viewUpgradeAliasWords(view);
        require(words.propertyId==id && unsigned(words.levelCount)==countBits,"View alias flag/padding words differ");
        std::array<unsigned char,44> native{};std::array<void*,3> table{image+0x169a0,reinterpret_cast<void*>(&upgradeAliasType),reinterpret_cast<void*>(&upgradeLoadLevel)};
        field<void*>(native.data(),0)=table.data();field<unsigned>(native.data(),0x24)=id;field<unsigned>(native.data(),0x28)=countBits;
        upgradeAliasNativeTypes=0;
        const auto current=function<int(__thiscall*)(void*)>(0x17050)(native.data());
        function<void(__thiscall*)(void*,int,bool)>(0x174b0)(native.data(),current,false);
        refreshViewUpgradeAlias(sourceProfile,view);
        require(sourceProfile.ints==nativeProfile.ints && sourceProfile.uints==nativeProfile.uints,"Focused View alias profile map differs");
        require(view.typeCalls==1 && upgradeAliasNativeTypes==1,"Focused View alias virtual type dispatch differs");
        ++upgradeAliasStates;
    }
