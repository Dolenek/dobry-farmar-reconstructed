#include "ProfileProperties.h"
#include "LegacyProfileParse.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdint>
#include <cstdarg>
#include <float.h>
#include <stdexcept>
#include <iostream>
using namespace farmar::recovered;
std::uint64_t systemLegacyParse(const std::string& text,std::uint64_t seed,bool single){
    std::wstring field(text.begin(),text.end());const auto hex=field.find_first_of(L"xX");if(hex!=field.npos) field.resize(hex);
    std::wistringstream gather{field};double check{};gather>>check;if(gather.fail()) return seed;
    const auto end=gather.eof() ? field.size():std::size_t(gather.tellg());std::string token(field.begin(),field.begin()+end);
    static auto module=LoadLibraryExW(L"msvcrt.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);if(!module)throw std::runtime_error("Load system CRT diagnostic");
    static auto convert=reinterpret_cast<double(__cdecl*)(const char*,char**)>(GetProcAddress(module,"strtod"));
    static auto error=reinterpret_cast<int*(__cdecl*)()>(GetProcAddress(module,"_errno"));if(!convert||!error)throw std::runtime_error("System CRT diagnostic exports");
    int* status=error();int prior=*status;*status=0;char* tail{};double value=convert(token.c_str(),&tail);int code=*status;*status=prior;
    if(code||tail==token.c_str()) return seed;
    if(single){float narrowed;__asm {fld value}
        __asm {fstp narrowed}
        std::uint32_t word{};std::memcpy(&word,&narrowed,4);return word;}
    std::uint64_t word{};std::memcpy(&word,&value,8);return word;
}
std::wstring legacy(double value){
    wchar_t buffer[128]{};
    const auto format=[](wchar_t* out,const wchar_t* spec,...){va_list args;va_start(args,spec);
        int n=__stdio_common_vswprintf(_CRT_INTERNAL_PRINTF_LEGACY_MSVCRT_COMPATIBILITY|_CRT_INTERNAL_PRINTF_LEGACY_THREE_DIGIT_EXPONENTS,out,128,spec,nullptr,args);va_end(args);return n;};
    if(format(buffer,L"%.6g",value)<0)throw std::runtime_error("Legacy formatter failed");return buffer;
}
int wmain(int argc,wchar_t** argv){
    if(argc<3||argc>4)return 2;
    try{
        _control87(argc==4 ? _PC_24:_PC_53,_MCW_PC);
        ProfileProperties profile,seeded;std::ifstream in{std::filesystem::path(argv[1])};std::ofstream values{std::filesystem::path(std::wstring(argv[2])+L".tsv")};
        if(!in||!values)throw std::runtime_error("Open probe files");
        std::string line;
        while(std::getline(in,line)){
            auto first=line.find('\t'),second=line.find('\t',first+1);if(first==line.npos||second==line.npos)throw std::runtime_error("Malformed fixture row");
            const auto third=line.find('\t',second+1);
            auto mode=line.substr(0,first),key=line.substr(first+1,second-first-1),payload=line.substr(second+1,third==line.npos ? line.npos:third-second-1);
            const auto seed=third==line.npos ? 0ULL:std::stoull(line.substr(third+1),nullptr,16);
            const bool single=mode=="F"||mode=="PF";const auto name=String(key);
            if(mode=="F"){auto bits=std::uint32_t(std::stoul(payload,nullptr,16));std::memcpy(&profile.floats[key],&bits,4);}
            else if(mode=="D"){auto bits=std::uint64_t(std::stoull(payload,nullptr,16));std::memcpy(&profile.doubles[key],&bits,8);}
            else profile.setProperty(String(single ? "Float":"Double"),name,WString(std::wstring(payload.begin(),payload.end())));
            if(mode=="PF") {auto word=parseLegacyProfileFloat(WString(std::wstring(payload.begin(),payload.end())),std::uint32_t(seed));std::memcpy(&seeded.floats[key],&word,4);}
            else if(mode=="PD") {auto word=parseLegacyProfileDouble(WString(std::wstring(payload.begin(),payload.end())),seed);std::memcpy(&seeded.doubles[key],&word,8);}
            else if(single) std::memcpy(&seeded.floats[key],&profile.floats[key],4);else std::memcpy(&seeded.doubles[key],&profile.doubles[key],8);
            std::uint64_t bits{};double number{};
            if(single){std::uint32_t word{};std::memcpy(&word,&profile.floats[key],4);bits=word;number=profile.floats[key];}
            else {std::memcpy(&bits,&profile.doubles[key],8);number=profile.doubles[key];}
            std::uint64_t seededBits{};
            if(single){std::uint32_t word{};std::memcpy(&word,&seeded.floats[key],4);seededBits=word;}else std::memcpy(&seededBits,&seeded.doubles[key],8);
            const auto systemBits=mode=="PF"||mode=="PD" ? systemLegacyParse(payload,seed,single):bits;
            auto text=legacy(number);values<<key<<'\t'<<std::hex<<std::setfill('0')<<std::setw(single ? 8:16)<<bits<<'\t'<<std::string(text.begin(),text.end())<<'\t'<<std::setw(single ? 8:16)<<seededBits<<'\t'<<std::setw(single ? 8:16)<<systemBits<<'\n';
        }
        const std::uint16_t bom=0xfeff;
        for(auto item:{std::make_pair(L".xml",&profile),std::make_pair(L".seeded.xml",&seeded)}) {
            auto xml=item.second->dumpXML();std::ofstream out{std::filesystem::path(std::wstring(argv[2])+item.first),std::ios::binary};out.write(reinterpret_cast<const char*>(&bom),2);out.write(reinterpret_cast<const char*>(xml.data()),xml.size()*2);if(!out)throw std::runtime_error("Write probe output");
        }
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
