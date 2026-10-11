stage=210;std::mt19937 gameplayRandom(0x4adac0);
const unsigned timeControl=_control87(0,0);
const auto timeCase=[&](float value) {
    const auto source=GameplayTime::fromSeconds(value);GameplayTime native;
    function<void(__thiscall*)(void*,float)>(0xadac0)(&native,value);
    if(!(source==native)) {std::cerr<<"GameplayTime mismatch input "<<std::hex<<field<unsigned>(&value,0)<<std::dec<<" source "<<source.hours<<':'<<source.minutes<<':'<<source.seconds<<" native "<<native.hours<<':'<<native.minutes<<':'<<native.seconds<<'\n';}
    require(source==native,"GameplayTime unsigned components differ");require(_control87(0,0)==timeControl,"GameplayTime changed x87 control");++gameplayTimeStates;
};
for(unsigned i=0;i<200000;++i) {
    const unsigned bits=gameplayRandom();float value;std::memcpy(&value,&bits,4);timeCase(value);
}
for(unsigned second=0;second<=12*3600;second+=15) {
    const float value=float(second);timeCase(value);timeCase(std::nextafter(value,-INFINITY));timeCase(std::nextafter(value,INFINITY));
}
stage=211;redirect(0xb0b70,reinterpret_cast<void*>(&gameplayTimeAttribute));redirect(0xd9720,reinterpret_cast<void*>(&typed<int>));
xmlTable[34]=reinterpret_cast<void*>(&sectionCount);
XmlDocument definition(files,String("Gameplay/Gameplay.xml"));XmlNode* group=nullptr;
require(definition.section(String("Events"),group) && group->children().size()==383,"Actual gameplay event population differs");
std::array<unsigned,10> population{};
for(const auto& row:group->children()) {unsigned type=99;const auto text=row.second->attribute(WString(L"Type"));std::wistringstream stream(text.c_str());stream>>type;require(type<10,"Actual gameplay type outside 0..9");++population[type];}
require(population==std::array<unsigned,10>{22,7,43,7,4,131,155,3,10,1},"Actual gameplay event types differ");
const auto directory=std::filesystem::path(".local")/(std::string("gameplay-definitions-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));std::filesystem::create_directories(directory);
{
    std::ofstream xml(directory/"definitions.xml",std::ios::binary);xml<<"<Gameplay><Events>";
    for(unsigned type=0;type<10;++type) {
        xml<<"<Event Type=\""<<type<<"\" Time=\"0:00:"<<(9-type)*5<<"\"><Note>kind-"<<type<<"</Note>";
        if(type==5) xml<<"<Data Range=\"2.75\" WinTime=\"0:02:05\" LetterTime=\"0:01:01\" Bonus=\"-19\" Penalty=\"23\" RandomFactor=\"0.375\"><Requirements><Criteria ProductType=\"3\" Count=\"7\" ReqType=\"0\"/><Criteria ProductType=\"12\" Count=\"9\" ReqType=\"1\"/><Criteria ProductType=\"18\" Count=\"99\" ReqType=\"0\"/></Requirements><WinNote>win</WinNote><LosNote>loss</LosNote></Data>";
        else if(type==6) {xml<<"<Data>";for(unsigned i=0;i<23;++i) xml<<"<Popularity ProductType=\""<<i%19<<"\" Value=\""<<int(i)-11<<"\"/>";xml<<"</Data>";}
        else xml<<"<Data Range=\"3.125\" Upgrade=\"2\" DepotType=\"4\" SeedType=\"7\" RandomFactor=\"0.75\" Value=\"-31\" UpgradeIndex=\"2\" Money=\"456\" WareLevel=\"3\"/>";
        xml<<"</Event>";
    }
    xml<<"<Event Type=\"1\" Time=\"0:00:05\"><Data SeedType=\"3\"/></Event><Event Type=\"33\" Time=\"0:00:00\"/></Events></Gameplay>";
    std::ofstream(directory/"empty.xml")<<"<Gameplay/>";
}
FileIndex fixtures(String(directory.generic_string()),String("pack"));XmlDocument alternate(fixtures,String("definitions.xml")),empty(fixtures,String("empty.xml"));
for(trial=0;trial<2;++trial) {
    stage=212;auto source=std::make_unique<GameplayEventDefinitions>();std::array<unsigned char,64> native{};
    native[4]=1;field<unsigned>(native.data(),0x3c)=0x12345678;source->endEventIndex=0x12345678;
    const auto check=[&] {
        stage=213;const auto count=field<unsigned>(native.data(),0x1c) ? (field<unsigned>(native.data(),0x20)-field<unsigned>(native.data(),0x1c))/4:0;
        require(count==source->events().size(),"Gameplay timeline count differs");
        require(source->endTime==field<GameplayTime>(native.data(),8) && source->noTimedEvents==bool(native[4]),"Gameplay end time/zero flag differs");
        require(source->endEventIndex==field<unsigned>(native.data(),0x3c),"Gameplay end-event index differs");
        for(unsigned i=0;i<count;++i) {
            const auto& event=*source->events()[i];auto* original=field<void**>(native.data(),0x1c)[i];
            require(unsigned(event.type)==field<unsigned>(original,4) && event.time==field<GameplayTime>(original,8) && event.index==field<unsigned>(original,0x34),"Gameplay sorted event type/time/index differs");
            require(event.note.view()==field<WString>(original,0x14).view(),"Gameplay note differs");compareFloat(event.range,field<float>(original,0x30),"Gameplay range");
            switch(event.type) {
            case GameplayEventType::DepotUpgrade: {const auto& value=std::get<DepotUpgradeEvent>(event.data);require(value.depotType==field<unsigned>(original,0x38) && value.upgrade==field<unsigned>(original,0x3c),"Depot unlock definition differs");break;}
            case GameplayEventType::SeedAvailable:require(std::get<SeedAvailableEvent>(event.data).seedType==field<unsigned>(original,0x38),"Seed unlock definition differs");break;
            case GameplayEventType::Money:case GameplayEventType::DestroyField:case GameplayEventType::DestroyDepot: {const auto& value=std::get<RandomValueEvent>(event.data);compareFloat(value.randomFactor,field<float>(original,0x38),"Gameplay random factor");require(value.value==field<int>(original,0x3c),"Gameplay random value differs");break;}
            case GameplayEventType::Contract: {
                const auto& value=std::get<ContractEvent>(event.data);require(value.winTime==field<GameplayTime>(original,0x38) && value.letterTime==field<GameplayTime>(original,0xa0),"Contract time definition differs");
                require(value.bonus==field<int>(original,0x44) && value.penalty==field<int>(original,0x48) && value.initialBonus==field<int>(original,0xac) && value.initialPenalty==field<int>(original,0xb0),"Contract bonus/penalty copies differ");compareFloat(value.randomFactor,field<float>(original,0x4c),"Contract random factor");
                require(value.winNote.view()==field<WString>(original,0x68).view() && value.lossNote.view()==field<WString>(original,0x84).view(),"Contract notes differ");
                require(!std::memcmp(value.requirements.data(),static_cast<unsigned char*>(original)+0x50,24) && !std::memcmp(value.initialRequirements.data(),static_cast<unsigned char*>(original)+0xb4,24),"Contract initial/current requirements differ");++gameplayContractStates;break;
            }
            case GameplayEventType::Popularity: {
                const auto& value=std::get<PopularityEvent>(event.data);
                for(const auto& pair:std::initializer_list<std::pair<const std::vector<PopularityValue>*,unsigned>>{{&value.values,0x38},{&value.initialValues,0x48}}) {
                    const auto first=field<unsigned>(original,pair.second+4),last=field<unsigned>(original,pair.second+8);const auto size=first ? (last-first)/8:0;
                    require(size==pair.first->size(),"Popularity vector count differs");if(size) require(!std::memcmp(pair.first->data(),reinterpret_cast<void*>(first),size*8),"Popularity initial/current values differ");
                }++gameplayPopularityStates;break;
            }
            case GameplayEventType::FieldUpgrade:require(std::get<FieldUpgradeEvent>(event.data).upgradeIndex==field<int>(original,0x38),"Field upgrade definition differs");break;
            case GameplayEventType::Trigger: {const auto& value=std::get<TriggerEvent>(event.data);require(value.money==field<int>(original,0x38) && value.wareLevel==field<int>(original,0x3c),"Trigger definition differs");break;}
            case GameplayEventType::End:break;
            }++gameplayEventStates;
        }++gameplayTimelineStates;
    };check();
    for(const XmlNode* node:{trial ? &alternate:&definition,&empty,&alternate,&definition}) {
        stage=214;source->loadXML(*node);function<void(__thiscall*)(void*,void*)>(0xb50d0)(native.data(),wrap(node));++gameplayReloads;check();
    }
    stage=215;source.reset();call(0xaeb10,native.data());require(!field<void*>(native.data(),0x1c) && !field<void*>(native.data(),0x20) && !field<void*>(native.data(),0x24),"Gameplay native timeline destruction did not clear storage");++gameplayObjects;
}
