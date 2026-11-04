// Compare actual tracker constructors on sufficiently large owned storage.
// Production factories and originals remain untouched; this is not a normal-load proof.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include "Squall.h"
void need(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
int wmain(int argc,wchar_t**argv){if(argc!=9)return 2;SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::cout.setf(std::ios::unitbuf);
    try{
        auto native=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str()),source=LoadLibraryW(std::filesystem::absolute(argv[2]).c_str());need(native&&source,"DLL load");
        auto load=reinterpret_cast<decltype(&SQUALL_Sample_LoadFile)>(GetProcAddress(source,"SQUALL_Sample_LoadFile"));auto unload=reinterpret_cast<decltype(&SQUALL_Sample_UnloadAll)>(GetProcAddress(source,"SQUALL_Sample_UnloadAll"));need(load&&unload,"Exports");auto filename=std::filesystem::path(argv[3]).string();auto id=load(filename.data(),1,nullptr);need(id>0,"Source file load");
        using File=void*(__cdecl*)(int);auto file=reinterpret_cast<File>(reinterpret_cast<std::uintptr_t>(source)+std::stoul(argv[4],nullptr,16));auto* owner=file(id);need(owner!=nullptr,"Owned source reader");auto* data=*reinterpret_cast<unsigned char**>(static_cast<unsigned char*>(owner)+8);need(data!=nullptr,"Owned SourceData");auto* reader=*reinterpret_cast<void**>(data+4);
        using Seek=unsigned(__thiscall*)(void*,unsigned);auto seek=reinterpret_cast<Seek>(reinterpret_cast<std::uintptr_t>(source)+std::stoul(argv[5],nullptr,16));need(seek(reader,0)==0,"Reset owned memory reader");*reinterpret_cast<unsigned*>(data+16)=0;
        auto which=std::filesystem::path(argv[8]).string();auto target=which=="native"?native:source;unsigned ctor=std::stoul(argv[6],nullptr,16);std::vector<unsigned char> object(65536,0);WAVEFORMATEX format{};bool initialized=false;
        using Ctor=void*(__thiscall*)(void*,WAVEFORMATEX*,bool*,void*);auto construct=reinterpret_cast<Ctor>(reinterpret_cast<std::uintptr_t>(target)+ctor);construct(object.data(),&format,&initialized,data);need(initialized,"Actual constructor rejected");
        std::ofstream("constructed-object.bin",std::ios::binary).write(reinterpret_cast<char*>(object.data()),object.size());using Length=unsigned(__thiscall*)(void*);auto* table=*reinterpret_cast<void***>(object.data());auto length=reinterpret_cast<Length>(table[5])(object.data());
        unsigned changedTail=0;for(unsigned i=0xeef8;i<object.size();++i)changedTail+=object[i]!=0;std::cout<<"{\"initialized\":true,\"constructor_backend\":\""<<which<<"\",\"frequency\":"<<format.nSamplesPerSec<<",\"channels\":"<<format.nChannels<<",\"bits\":"<<format.wBitsPerSample<<",\"samples\":"<<length<<",\"nonzero_bytes_past_native_factory_allocation\":"<<changedTail;
        using Pcm=unsigned(__thiscall*)(void*,void*,unsigned,unsigned,bool);auto method=reinterpret_cast<Pcm>(table[std::stoi(argv[7])]);std::vector<unsigned char>pcm(32+16384,0x5a);auto returned=method(object.data(),pcm.data()+16,0,4096,false);std::ofstream("decoded-pcm.bin",std::ios::binary).write(reinterpret_cast<char*>(pcm.data()),pcm.size());bool guards=true;unsigned nonzero=0;for(unsigned i=0;i<16;++i)guards&=pcm[i]==0x5a&&pcm[pcm.size()-1-i]==0x5a;for(unsigned i=16;i<pcm.size()-16;++i)nonzero+=pcm[i]!=0;
        std::cout<<",\"returned_bytes\":"<<returned<<",\"nonzero_destination_bytes\":"<<nonzero<<",\"wide_guards_intact\":"<<(guards?"true":"false");using Destroy=void*(__thiscall*)(void*,unsigned);reinterpret_cast<Destroy>(table[0])(object.data(),0);unload();FreeLibrary(source);FreeLibrary(native);std::cout<<"}\n";return 0;
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
