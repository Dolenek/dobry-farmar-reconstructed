stage=200;
LoadingDisabledAudio disabledBackend;SoundManager audio(files,settings,disabledBackend,0);
managerTable[3]=reinterpret_cast<void*>(&loadingSoundResource);
xmlTable[23]=reinterpret_cast<void*>(&gameConfigurationUnsigned);xmlTable[34]=reinterpret_cast<void*>(&sectionCount);
redirect(0xa670,reinterpret_cast<void*>(&gameConfigurationLevel));redirect(0xd9720,reinterpret_cast<void*>(&typed<int>));
XmlDocument definition(files,String("game.xml"));
const auto directory=std::filesystem::path(".local")/(std::string("game-configuration-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
std::filesystem::create_directories(directory);
{
    std::ofstream xml(directory/"partial.xml",std::ios::binary);
    xml<<"<Game UpgradeSound=\"Sounds/building_upgrade.wav\"><ProductionInfo>"
       <<"<Z Id=\"ignored\" CreationTime=\"12.75\" DieTime=\"3.125\" PriceRadius=\"-7\" GoodAttPriceRadius=\"-3\" MaxSellFrequency=\"0.03125\">"
       <<"<Icon Picture=\"\" anim_length=\"2.25\" frames_count=\"99\" anim_columns=\"3\" anim_rows=\"2\" anim_loop=\"0\" PlayAnimation=\"1\"/></Z>"
       <<"<A CreationTime=\"7.5\"><IconLarge Picture=\"Products/grass.jng\" frames_count=\"2\" anim_columns=\"2\" anim_rows=\"2\"/></A></ProductionInfo>"
       <<"<Seeds><Seed Price=\"4294967295\"><Icon Picture=\"\" frames_count=\"8\" anim_columns=\"2\" anim_rows=\"3\"/></Seed></Seeds>"
       <<"<Costs><Item><Up3>73</Up3></Item><Item><Up0>17</Up0><Other>4</Other><Up2>29</Up2></Item>";
    for(unsigned i=2;i<17;++i) xml<<"<Item><Up0>"<<i*101<<"</Up0><Up1>"<<i*102<<"</Up1><Up2>"<<i*103<<"</Up2><Up3>"<<i*104<<"</Up3><Extra>99</Extra></Item>";
    xml<<"</Costs><UpgradeStrings>";
    for(unsigned i=0;i<7;++i) xml<<"<S"<<i<<" string=\"upgrade-"<<i<<"\"/>";
    xml<<"</UpgradeStrings></Game>";
    std::ofstream(directory/"empty.xml")<<"<Game/>";
}
FileIndex fixtures(String(directory.generic_string()),String("pack"));XmlDocument partial(fixtures,String("partial.xml")),empty(fixtures,String("empty.xml"));
// Prime the actual resource cache so all per-operation reference deltas can be
// observed independently, including first XML assignment and empty replacement.
const auto prime=[&](const XmlNode& root,auto&& recurse)->void {
    const auto name=toNarrow(root.attribute(WString(L"Picture")));
    if(!name.empty()) {auto* texture=getTexture(nullptr,nullptr,&name);texture->releaseReference();}
    for(const auto& child:root.children()) recurse(*child.second,recurse);
};prime(definition,prime);prime(partial,prime);
XmlNode *group=nullptr;require(definition.section(String("ProductionInfo"),group) && group->children().size()==19,"Actual product count differs");
require(definition.section(String("Seeds"),group) && group->children().size()==8,"Actual seed count differs");
require(definition.section(String("Costs"),group) && group->children().size()==15,"Actual cost count differs");
for(trial=0;trial<8;++trial) {
    stage=201;ViewRegistry views;View sourceView(nullptr,views);
    auto config=std::make_unique<GameConfiguration>(manager);auto* native=allocate(2716);std::memset(native,0xcc,2716);
    function<void*(__thiscall*)(void*,void*)>(0xd55b0)(native,nullptr);
    // The actual product/seed constructors initialize only cAnimable members.
    for(unsigned i=0;i<19;++i) call(0x3610,static_cast<unsigned char*>(native)+0x30+i*100);
    for(unsigned i=0;i<8;++i) call(0x36b0,static_cast<unsigned char*>(native)+0x79c+i*44);
    for(unsigned i=0;i<5;++i) new(static_cast<unsigned char*>(native)+0xa10+i*28) WString();
    field<cResource*>(native,0xa08)=nullptr;
    for(unsigned i=0;i<19;++i) {
        auto& record=config->products[i];const auto offset=0x30+i*100;
        record.creationTime=field<float>(native,offset+0x60)=float(i+1)/3.f;
        record.dieTime=field<float>(native,offset)=float(i+2)/7.f;
        record.priceRadius=field<int>(native,offset+4)=100+i;
        record.goodAttPriceRadius=field<int>(native,offset+8)=200+i;
        record.maxSellFrequency=field<float>(native,offset+12)=float(i+3)/11.f;
    }
    for(unsigned i=0;i<8;++i) config->seeds[i].price=field<unsigned>(native,0x79c+i*44)=0x12340000+i;
    for(unsigned i=0;i<15;++i) for(unsigned j=0;j<4;++j) config->costs[i][j]=field<unsigned>(native,0x8fc+(i*4+j)*4)=0x45670000+i*4+j;
    const auto appearance=[&](const AnimableAppearance& source,void* original) {
        require(source.texture==field<cTexture*>(original,4),"Game appearance texture differs");
        require(source.frames==field<unsigned>(original,8) && source.columns==field<unsigned>(original,20) && source.rows==field<unsigned>(original,24),"Game appearance atlas differs");
        compareFloat(source.length,field<float>(original,12),"Game appearance length");
        require(source.loop==bool(field<unsigned char>(original,16)) && source.playing==bool(field<unsigned char>(original,28)),"Game appearance flags differ");
        require(!field<void*>(original,32) && !field<void*>(original,36),"Actual game.xml unexpectedly uses 3D appearance");++gameConfigurationAppearances;
    };
    const auto check=[&] {
        stage=202;
        require(sourceView.state().shown==bool(field<unsigned char>(native,0x24)) && sourceView.state().enabled==bool(field<unsigned char>(native,0x26)) && sourceView.state().absoluteEnabled==bool(field<unsigned char>(native,0x28)),"cGame base XML flags differ");
        for(unsigned i=0;i<19;++i) {
            const auto& record=config->products[i];const auto offset=0x30+i*100;
            compareFloat(record.creationTime,field<float>(native,offset+0x60),"Product CreationTime");
            compareFloat(record.dieTime,field<float>(native,offset),"Product DieTime");
            require(record.priceRadius==field<int>(native,offset+4) && record.goodAttPriceRadius==field<int>(native,offset+8),"Product price radii differ");
            compareFloat(record.maxSellFrequency,field<float>(native,offset+12),"Product sell frequency");
            appearance(record.icon,static_cast<unsigned char*>(native)+offset+16);appearance(record.iconLarge,static_cast<unsigned char*>(native)+offset+56);
        }
        for(unsigned i=0;i<8;++i) { require(config->seeds[i].price==field<unsigned>(native,0x79c+i*44),"Seed price differs");appearance(config->seeds[i].icon,static_cast<unsigned char*>(native)+0x7a0+i*44); }
        for(unsigned i=0;i<15;++i) for(unsigned j=0;j<4;++j) require(config->costs[i][j]==field<unsigned>(native,0x8fc+(i*4+j)*4),"Upgrade cost differs");
        for(unsigned i=0;i<5;++i) require(config->upgradeStrings[i].view()==field<WString>(native,0xa10+i*28).view(),"Upgrade label differs");
        require(config->upgradeSound()==field<cResource*>(native,0xa08),"Game upgrade sound cache identity differs");++gameConfigurationStates;
    };check();
    const auto load=[&](const XmlNode& node) {
        stage=203;const auto before=textureCounts();sourceView.loadXML(node);config->loadXML(node);const auto middle=textureCounts();
        function<void(__thiscall*)(void*,void*)>(0xe6e0)(native,wrap(&node));compareReferenceDeltas(before,middle);++gameConfigurationReloads;check();
    };
    load(trial&1 ? partial:definition);load(empty);load(partial);load(definition);load(definition);load(empty);
    // Direct record loaders keep missing fields and ignore XML Id/tag names.
    require(partial.section(String("ProductionInfo"),group),"Fixture product section missing");XmlNode* item=nullptr;require(group->section(0,item),"Fixture product missing");
    const auto beforeProduct=textureCounts();config->products[18].loadXML(*item,manager);const auto middleProduct=textureCounts();
    function<void(__thiscall*)(void*,void*)>(0xaf00)(static_cast<unsigned char*>(native)+0x30+18*100,wrap(item));compareReferenceDeltas(beforeProduct,middleProduct);check();
    require(partial.section(String("Seeds"),group) && group->section(0,item),"Fixture seed missing");
    const auto beforeSeed=textureCounts();config->seeds[7].loadXML(*item,manager);const auto middleSeed=textureCounts();
    function<void(__thiscall*)(void*,void*)>(0xb560)(static_cast<unsigned char*>(native)+0x79c+7*44,wrap(item));compareReferenceDeltas(beforeSeed,middleSeed);check();
    stage=204;auto* sound=config->upgradeSound();const auto soundBefore=sound->referenceCount();
    const auto beforeDelete=textureCounts();config.reset();const auto middleDelete=textureCounts();
    require(sound->referenceCount()+1==soundBefore,"Game current upgrade sound release differs");sound->releaseReference();
    for(unsigned i=8;i-->0;) call(0x36c0,static_cast<unsigned char*>(native)+0x79c+i*44);
    for(unsigned i=19;i-->0;) call(0x3660,static_cast<unsigned char*>(native)+0x30+i*100);
    for(unsigned i=5;i-->0;) field<WString>(native,0xa10+i*28).~WString();
    call(0xd58a0,native);compareReferenceDeltas(beforeDelete,middleDelete);++gameConfigurationObjects;
    // Cache ownership at ResourceManager shutdown reclaims retained prior sounds.
    gameConfigurationSoundResidues+=sound->referenceCount();
}
