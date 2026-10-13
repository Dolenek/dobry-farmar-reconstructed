stage=220;redirect(0x185a13,reinterpret_cast<void*>(&gameplayGetThreadData));
redirect(0x106a0,reinterpret_cast<void*>(&gameplayFloatProperty));redirect(0x10770,reinterpret_cast<void*>(&gameplayIntProperty));
redirect(0xa670,reinterpret_cast<void*>(&gameConfigurationLevel));
redirect(0xb0b70,reinterpret_cast<void*>(&gameplayTimeAttribute));redirect(0xd9720,reinterpret_cast<void*>(&typed<int>));
xmlTable[34]=reinterpret_cast<void*>(&sectionCount);xmlTable[0]=reinterpret_cast<void*>(&gameplayReleaseXML);
managerTable[6]=reinterpret_cast<void*>(&gameplayRuntimeXML);
for(const unsigned seed:{1U,0xffffffffU,0xfeedbeefU}) {
    GameRandom source(seed);field<unsigned>(gameplayThreadData.data(),0x14)=seed;
    for(unsigned i=0;i<100000;++i) {
        require(source.next()==function<int(__cdecl*)()>(0x181021)(),"Original rand result differs");
        require(source.state()==field<unsigned>(gameplayThreadData.data(),0x14),"Original rand state differs");++gameplayRandomStates;
    }
}
XmlDocument actualDefinition(files,String("Gameplay/Gameplay.xml"));
const auto directory=std::filesystem::path(".local")/(std::string("gameplay-runtime-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));std::filesystem::create_directories(directory);
{
    std::ofstream xml(directory/"events.xml",std::ios::binary);xml<<"<Gameplay><Events>";
    for(unsigned type=0;type<10;++type) {
        xml<<"<Event Type=\""<<type<<"\" Time=\"0:00:"<<(type==9 ? 60:(type+1)*3)<<"\"><Note>execution-"<<type<<"</Note>";
        if(type==5) xml<<"<Data Range=\"1.875\" WinTime=\"0:02:05\" LetterTime=\"0:01:01\" Bonus=\"2147483600\" Penalty=\"1001\" RandomFactor=\"0.375\"><Requirements><Criteria ProductType=\"3\" Count=\"4294967295\" ReqType=\"0\"/><Criteria ProductType=\"12\" Count=\"9\" ReqType=\"1\"/></Requirements><WinNote>win</WinNote><LosNote>loss</LosNote></Data>";
        else if(type==6) {xml<<"<Data Range=\"0.8125\">";for(unsigned i=0;i<19;++i) xml<<"<Popularity ProductType=\""<<i<<"\" Value=\""<<int(i)-11<<"\"/>";xml<<"</Data>";}
        else xml<<"<Data Range=\"2.75\" Upgrade=\"2\" DepotType=\"4\" SeedType=\"7\" RandomFactor=\"0.75\" Value=\"-31\" UpgradeIndex=\"2\" Money=\"456\" WareLevel=\"3\"/>";
        xml<<"</Event>";
    }
    xml<<"<Event Type=\"6\" Time=\"0:01:30\"><Data Range=\"0.125\"><Popularity ProductType=\"18\" Value=\"2147483647\"/></Data></Event></Events></Gameplay>";
    std::ofstream(directory/"empty.xml")<<"<Gameplay/>";
    std::ofstream(directory/"zero.xml")<<"<Gameplay><Events><Event Type=\"9\" Time=\"0:00:00\"/></Events></Gameplay>";
}
FileIndex fixtures(String(directory.generic_string()),String("pack"));XmlDocument synthetic(fixtures,String("events.xml")),empty(fixtures,String("empty.xml")),zero(fixtures,String("zero.xml"));
std::array<unsigned char,160> contractController{};putPointer(0x25f744,contractController.data());
for(trial=0;trial<14;++trial) {
    stage=221;const auto* node=trial%2 ? static_cast<const XmlNode*>(&synthetic):static_cast<const XmlNode*>(&actualDefinition);
    if(trial==12) node=&empty;if(trial==13) node=&zero;
    const unsigned seed=std::array<unsigned,3>{1U,0xffffffffU,0xfeedbeefU}[trial%3];
    const float initialTime=trial<6 ? 0.f:17.5f;
    ProfileProperties sourceProfile,nativeProfile;sourceProfile.floats["GameTime"]=nativeProfile.floats["GameTime"]=initialTime;
    if(trial>=12) {sourceProfile.floats.clear();nativeProfile.floats.clear();}
    gameplayNativeProfile=&nativeProfile;gameplayRuntimeDefinition=node;gameplayXmlReleased=0;
    GameRandom random(seed);field<unsigned>(gameplayThreadData.data(),0x14)=seed;GameplayTrace executor(sourceProfile);
    auto source=trial==0 ? std::make_unique<Gameplay>(manager,sourceProfile,random,executor):std::make_unique<Gameplay>(*node,sourceProfile,random,executor);
    require(Gameplay::instance()==source.get(),"Source Gameplay singleton was not published");
    std::array<unsigned char,64> native{};function<void*(__thiscall*)(void*)>(0xb59c0)(native.data());
    require(field<void*>(image,0x25f75c)==native.data() && gameplayXmlReleased==1,"Native Gameplay singleton/XML ownership differs");
    if(trial>=12) require(sourceProfile.floats.empty() && nativeProfile.floats.empty(),"Zero/empty Gameplay unexpectedly accessed GameTime");
    const auto nativeEvents=field<void**>(native.data(),0x1c);const unsigned count=(field<unsigned>(native.data(),0x20)-field<unsigned>(native.data(),0x1c))/4;
    std::vector<std::array<void*,2>> tables(count);
    for(unsigned i=0;i<count;++i) {tables[i]={field<void**>(nativeEvents[i],0)[0],reinterpret_cast<void*>(&gameplayDispatch)};field<void*>(nativeEvents[i],0)=tables[i].data();}
    gameplayNativeCalls.clear();
    const auto check=[&] {
        stage=222;require(source->definitions().events().size()==count,"Gameplay runtime event count differs");
        compareFloat(source->endSeconds(),field<float>(native.data(),0x14),"Gameplay runtime end seconds");
        require(source->definitions().noTimedEvents==bool(native[4]),"Gameplay runtime zero flag differs");
        if(source->definitions().noTimedEvents) require(!field<void*>(native.data(),0x38) && source->currentEventIndex()==0,"Zero Gameplay has an active cursor");
        else {
            const auto current=(field<unsigned>(native.data(),0x38)-field<unsigned>(native.data(),0x1c))/4;
            require(source->currentEventIndex()==current,"Gameplay runtime event cursor differs");
        }
        compareFloat(sourceProfile.floats["GameTime"],nativeProfile.floats["GameTime"],"Gameplay runtime profile time");
        require(random.state()==field<unsigned>(gameplayThreadData.data(),0x14),"Gameplay runtime random sequence differs");
        require(executor.calls==gameplayNativeCalls,"Gameplay runtime dispatch order differs");
        const auto coefficients=source->coefficients();const auto first=field<float*>(native.data(),0x2c);
        const auto size=first ? (field<unsigned>(native.data(),0x30)-field<unsigned>(native.data(),0x2c))/4:0;
        require(coefficients.size()==size,"Gameplay coefficients count differs");
        if(size) require(!std::memcmp(coefficients.data(),first,size*4),"Gameplay coefficients bytes differ");
        for(unsigned i=0;i<count;++i) {
            const auto& event=*source->definitions().events()[i];void* expected=nativeEvents[i];
            if(event.type==GameplayEventType::Contract) {
                const auto& value=std::get<ContractEvent>(event.data);
                if(value.bonus!=field<int>(expected,0x44) || value.penalty!=field<int>(expected,0x48))
                    std::cerr<<"Contract "<<i<<" action "<<action<<" bonus "<<value.bonus<<'/'<<field<int>(expected,0x44)<<" penalty "<<value.penalty<<'/'<<field<int>(expected,0x48)<<'\n';
                require(value.bonus==field<int>(expected,0x44) && value.penalty==field<int>(expected,0x48),"Gameplay randomized contract amounts differ");
                require(!std::memcmp(value.requirements.data(),static_cast<unsigned char*>(expected)+0x50,24),"Gameplay randomized requirements differ");
            } else if(event.type==GameplayEventType::Popularity) {
                const auto& value=std::get<PopularityEvent>(event.data);
                if(!value.values.empty()) require(!std::memcmp(value.values.data(),field<void*>(expected,0x3c),value.values.size()*8),"Gameplay randomized popularity differs");
            }
            ++gameplayRuntimeEvents;
        }++gameplayRuntimeStates;
    };
    check();
    const auto restore=[&](const std::vector<float>& values) {
        stage=223;source->restoreCoefficients(values);std::array<void*,4> vector{nullptr,const_cast<float*>(values.data()),const_cast<float*>(values.data()+values.size()),const_cast<float*>(values.data()+values.size())};
        function<void(__thiscall*)(void*,void*)>(0xb4ad0)(native.data(),vector.data());++gameplayRuntimeRestores;check();
        std::array<unsigned char,16> copied{};function<void(__thiscall*)(void*,void*)>(0xb4ac0)(native.data(),copied.data());
        const auto copy=source->coefficients();require((field<unsigned>(copied.data(),8)-field<unsigned>(copied.data(),4))/4==copy.size(),"Gameplay save coefficient copy size differs");
        if(!copy.empty()) require(!std::memcmp(copy.data(),field<void*>(copied.data(),4),copy.size()*4),"Gameplay save coefficient copy bytes differ");
    };
    if(initialTime!=0 && trial<12) restore(std::vector<float>(count,0.375f));
    if(trial<12) {
#include "market_price_forecast_oracle_cases.inc"
    }
    for(action=0;action<(trial<12 ? 420U:6U);++action) {
        stage=224;const auto time=source->definitions().noTimedEvents ? GameplayTime{}:source->definitions().events()[source->currentEventIndex()]->time;
        const float seconds=float((time.hours*60U+time.minutes)*60U+time.seconds);
        float frame=action%4==0 ? std::nextafter(seconds,-INFINITY):action%4==1 ? seconds:action%4==2 ? seconds+0.125f:std::nextafter(seconds,INFINITY);
        if(action==390) frame=std::numeric_limits<float>::quiet_NaN();
        if(action>=400) frame=source->endSeconds()+float(action-400)*0.125f;
        sourceProfile.floats["GameTime"]=nativeProfile.floats["GameTime"]=frame;
        executor.contractState=action%3 ? 0:3;field<int>(contractController.data(),0x94)=executor.contractState;
        gameplayMutateTime=action%7==0;
        source->step();call(0xb5c60,native.data());++gameplayRuntimeSteps;check();
    }
    if(trial==1) {
        unsigned executedTypes=0;for(const auto& entry:executor.calls) executedTypes|=1U<<entry[0];
        require(executedTypes==0x3ff,"Gameplay runtime did not dispatch all ten original types");
    }
    gameplayMutateTime=false;
    // Reapplying saved coefficients is cumulative in the original. Exercise
    // signed quantization boundaries and float-to-int64 low DWORD conversion.
    for(float coefficient:{0.f,0.1f,-0.75f,1.f,3.75f,INFINITY,-INFINITY,std::numeric_limits<float>::quiet_NaN()}) restore(std::vector<float>(count,coefficient));
    if(trial==1) {
        std::mt19937 samples(0x4ae570);
        for(action=0;action<30000;++action) {
            std::vector<float> values(count);for(auto& value:values) {const unsigned bits=samples();std::memcpy(&value,&bits,4);}
            restore(values);
        }
    }
    stage=225;source.reset();require(Gameplay::instance()==nullptr,"Source Gameplay singleton survived destruction");call(0xaeb10,native.data());
    require(!field<void*>(image,0x25f75c) && !field<void*>(native.data(),0x2c) && !field<void*>(native.data(),0x1c),"Native Gameplay destruction did not clear singleton/vectors");++gameplayRuntimeObjects;
}
