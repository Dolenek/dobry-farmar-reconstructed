#include "profile_properties_oracle.h"
int wmain(int argc,wchar_t** argv) {
    if(argc!=2 && argc!=3) return 2;
    auto reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);if(!reference) return 2;
    try {
        initializePropertyOracle(reference,argc==3);
        ProfileProperties actual;std::array<unsigned char,88> profile{};
        const std::string keys[]={"","A","a","aaa",std::string(64,'x'),"same-key",std::string("\x80\xff",2),"z",std::string("A\0B",3)};
        for(scenario=0;scenario<2400;++scenario) {
            const auto& candidate=keys[(scenario*17+scenario/9)%9];
            // The unverified ACP adapter rejects high bytes in this CRT locale.
            // Keep serializer fixtures ASCII; map fixtures still test high bytes.
            const String key(scenario<1200 && candidate==keys[6] ? "ascii-key":candidate);const bool write=scenario%3!=0;
            kind=0;access(actual.floats,key,float(int(scenario%81)-40)/8,write);
            kind=1;access(actual.doubles,key,double(int(scenario%91)-45)/16,write);
            kind=2;access(actual.ints,key,std::int32_t(0x80000000U+scenario),write);
            kind=3;access(actual.uints,key,std::uint32_t(0xfffffff0U+scenario),write);
            kind=4;access(actual.longs,key,std::int32_t(scenario*123),write);
            kind=5;access(actual.ulongs,key,std::uint32_t(0x80000000U+scenario*333),write);
            kind=6;access(actual.longLongs,key,std::int64_t(0x8000000000000000ULL+scenario*555),write);
            kind=7;access(actual.colors,key,Color::fromARGB(scenario*134567),write);
            kind=8;access(actual.booleans,key,bool(scenario%2),write);
            kind=9;access(actual.uint16s,key,std::uint16_t(scenario*41),write);
            kind=10;access(actual.vectors,key,std::array<float,2>{float(scenario)/4,-float(scenario)/2},write);
            kind=11;access(actual.strings,key,String(std::string(scenario%61,'a'+scenario%26)),write);
            kind=12;access(actual.wideStrings,key,WString(std::wstring(scenario%51,L'A'+scenario%26)),write);
            checkAll(actual);
            const wchar_t* values[]={L"-1.25",L"0.03125",L"-2147483648",L"4294967295",L"2147483647",L"2147483648",L"-9223372036854775808",L"ff123456",L"1",L"65535",L"-2.5;0.25",L"ASCII long property text",L"\u0158epka \u65e5\u672c\u8a9e"};
            for(kind=0;kind<13;++kind) {
                const String type(stores[kind].name);const WString value(values[kind]);actual.setProperty(type,key,value);
                function<void(__thiscall*)(void*,const String*,const String*,const WString*)>(0xebd10)(profile.data(),&type,&key,&value);++dispatches;
            }
            const String unknown(scenario%2 ? "float":"unknown");const WString unused(L"123");actual.setProperty(unknown,key,unused);
            function<void(__thiscall*)(void*,const String*,const String*,const WString*)>(0xebd10)(profile.data(),&unknown,&key,&unused);++dispatches;checkAll(actual);
            if(scenario<1200 && scenario%10==0) {
                std::array<unsigned char,16> stream{};expectedStream=stream.data()+8;nativeOutput.str(L"");nativeOutput.clear();
                nativeOutput<<L"<?xml version=\"1.0\" encoding=\"UTF-16\"?><Props>";
                const unsigned writers[]={0xe4c70,0xe4e90,0xe50c0,0xe52e0,0xe5500,0xe5720,0xe5940,0xe3840,0xe5b60,0xe5d90,0xe30f0,0xe3320,0xe35a0};
                for(kind=0;kind<13;++kind) function<void(__stdcall*)(void*)>(writers[kind])(stream.data());
                nativeOutput<<L"</Props>";const auto text=actual.dumpXML();const auto expected=nativeOutput.str();
                require(text.view()==std::wstring_view(expected),"Property serialized XML differs");serializedUnits+=expected.size();++serializations;
            }
            if(scenario%17==16) { actual.clear();function<void(__cdecl*)()>(0xe9d80)();++clears;checkAll(actual); }
        }
        actual.clear();function<void(__cdecl*)()>(0xe9d80)();++clears;checkAll(actual);
        for(auto& store:stores) release(store.head);require(allocations.empty(),"Native property tree allocation leaked");
        require(registrations==13,"Original property type static initialization differs");
        std::cout<<"{\"scenarios\":2400,\"property_accesses\":"<<accesses<<",\"typed_dispatches\":"<<dispatches<<",\"map_comparisons\":"<<comparisons<<",\"clears\":"<<clears<<",\"stores\":13,\"serializations\":"<<serializations<<",\"serialized_units\":"<<serializedUnits<<"}\n";
        VirtualFree(image,0,MEM_RELEASE);FreeLibrary(reference);return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n';return 1; }
}
