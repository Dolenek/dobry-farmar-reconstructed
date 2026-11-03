// Call table initializers on owned diagnostic storage; no production code uses this.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <float.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
int wmain(int argc,wchar_t** argv){
    if(argc<7||argc>8)return 2;
    try{
        HMODULE native=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str()),source=LoadLibraryW(std::filesystem::absolute(argv[2]).c_str());
        if(!native||!source)throw std::runtime_error("DLL load");
        auto call=[](HMODULE h,unsigned rva,void* object){using Function=void(__thiscall*)(void*);reinterpret_cast<Function>(reinterpret_cast<std::uintptr_t>(h)+rva)(object);};
        std::vector<unsigned char>a(47744),b(47744);_control87(argc==8?_PC_24:_PC_53,_MCW_PC);
        call(native,std::stoul(argv[3],nullptr,16),b.data());call(source,std::stoul(argv[4],nullptr,16),a.data());
        unsigned offset=std::stoul(argv[5]),count=std::stoul(argv[6]),different=0;if(offset+count*4>a.size())throw std::runtime_error("range");
        std::cout<<"{\"offset\":"<<offset<<",\"words\":"<<count<<",\"differences\":[";
        for(unsigned i=0;i<count;++i){std::uint32_t x,y;std::memcpy(&x,a.data()+offset+4*i,4);std::memcpy(&y,b.data()+offset+4*i,4);if(x!=y){if(different++)std::cout<<',';std::cout<<'['<<i<<','<<x<<','<<y<<']';}}
        std::cout<<"],\"different_words\":"<<different<<"}\n";
        std::ofstream("source-math.bin",std::ios::binary).write(reinterpret_cast<char*>(a.data()),a.size());std::ofstream("native-math.bin",std::ios::binary).write(reinterpret_cast<char*>(b.data()),b.size());
        FreeLibrary(source);FreeLibrary(native);return different?1:0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
