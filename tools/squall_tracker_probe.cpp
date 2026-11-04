// Bounded actual tracker decoder methods through loaded DLLs; diagnostic only.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <float.h>
#include <cstdint>
#include <stdexcept>
#include "Squall.h"
void need(bool ok,const char* what){if(!ok)throw std::runtime_error(what);}
void stage(const char* what){std::ofstream("operations.log",std::ios::app)<<what<<'\n';}
int wmain(int argc,wchar_t**argv){if(argc<8||argc>9)return 2;SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::cout.setf(std::ios::unitbuf);
    try{
        _control87(argc==9?_PC_24:_PC_53,_MCW_PC);auto h=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());need(h!=nullptr,"DLL load");
#define API(name) auto name=reinterpret_cast<decltype(&SQUALL_##name)>(GetProcAddress(h,"SQUALL_" #name));need(name!=nullptr,#name)
        API(Sample_LoadFile);API(Sample_UnloadAll);API(Sample_GetFileLength);API(Sample_GetFileFrequency);API(Sample_GetFileLengthMs);
#undef API
        auto filename=std::filesystem::path(argv[3]).string();auto mode=std::filesystem::path(argv[4]).string();unsigned start=std::stoul(argv[5]),count=std::stoul(argv[6]);int memory=std::stoi(argv[7]);need(count<1048576,"Count bound");
        wchar_t readyPath[32768]{};
        if(GetEnvironmentVariableW(L"TRACKER_DEBUG_READY_FILE",readyPath,32768)){
            std::filesystem::path ready(readyPath);std::ofstream(ready)<<"{\"pid\":"<<GetCurrentProcessId()<<",\"module_base\":"<<reinterpret_cast<std::uintptr_t>(h)<<"}\n";
            auto go=ready;go+=L".go";auto deadline=GetTickCount64()+60000;
            while(!std::filesystem::exists(go)){need(GetTickCount64()<deadline,"Diagnostic attach deadline");Sleep(20);}
        }
        stage("load");auto id=Sample_LoadFile(filename.data(),memory,nullptr);need(id>0,"Tracker rejected");stage("load_returned");auto length=Sample_GetFileLength(id),frequency=Sample_GetFileFrequency(id),ms=Sample_GetFileLengthMs(id);std::cout<<"{\"sample_id\":"<<id<<",\"frames\":"<<length<<",\"frequency\":"<<frequency<<",\"milliseconds\":"<<ms;
        if(mode!="metadata"){
            need(mode=="mono"||mode=="stereo","Mode");using File=void*(__cdecl*)(int);using Pcm=unsigned(__thiscall*)(void*,void*,unsigned,unsigned,bool);auto file=reinterpret_cast<File>(reinterpret_cast<std::uintptr_t>(h)+std::stoul(argv[2],nullptr,16));auto* object=file(id);need(object!=nullptr,"Actual file");auto* decoder=*reinterpret_cast<void**>(static_cast<unsigned char*>(object)+12);need(decoder!=nullptr,"Actual decoder");auto method=reinterpret_cast<Pcm>((*reinterpret_cast<void***>(decoder))[mode=="mono"?1:2]);unsigned channels=mode=="mono"?1:2;std::vector<unsigned char>data(32+count*channels*2,0x5a);
            stage("decode");auto returned=method(decoder,data.data()+16,start,count,false);stage("decode_returned");std::ofstream("decoded-pcm.bin",std::ios::binary).write(reinterpret_cast<const char*>(data.data()),data.size());bool guards=true;unsigned nonzero=0;for(unsigned i=0;i<16;++i)guards&=data[i]==0x5a&&data[data.size()-1-i]==0x5a;for(unsigned i=16;i<data.size()-16;++i)nonzero+=data[i]!=0;std::cout<<",\"returned_bytes\":"<<returned<<",\"guards_intact\":"<<(guards?"true":"false")<<",\"nonzero_bytes\":"<<nonzero;
        }
        stage("unload");Sample_UnloadAll();stage("unload_returned");FreeLibrary(h);stage("complete");std::cout<<"}\n";return 0;
    }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
