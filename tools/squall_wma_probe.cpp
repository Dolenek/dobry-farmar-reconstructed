// Observe actual legacy WMA COM/file/decode paths in isolated diagnostic processes.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <amstream.h>
#include <float.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include "Squall.h"
void need(bool ok,const char* what){if(!ok)throw std::runtime_error(what);}
void stage(const char* name){std::ofstream("operations.log",std::ios::app)<<name<<'\n';}
int wmain(int argc,wchar_t**argv){
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    if(argc==2&&std::wstring(argv[1])==L"com"){
        auto init=CoInitialize(nullptr);IAMMultiMediaStream* stream{};auto create=CoCreateInstance(CLSID_AMMultiMediaStream,nullptr,CLSCTX_INPROC_SERVER,IID_IAMMultiMediaStream,reinterpret_cast<void**>(&stream));
        std::cout<<"{\"co_initialize\":"<<init<<",\"multimedia_stream_create\":"<<create<<",\"multimedia_stream_present\":"<<(stream?"true":"false")<<"}\n";if(stream)stream->Release();if(SUCCEEDED(init))CoUninitialize();return 0;
    }
    if(argc<8||argc>9)return 2;
    try{
        _control87(argc==9?_PC_24:_PC_53,_MCW_PC);stage("load_dll");auto module=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());need(module!=nullptr,"Load DLL");
#define API(name) auto name=reinterpret_cast<decltype(&SQUALL_##name)>(GetProcAddress(module,"SQUALL_" #name));need(name!=nullptr,#name)
        API(Sample_LoadFile);API(Sample_UnloadAll);API(Sample_GetFileLength);API(Sample_GetFileLengthMs);API(Sample_GetFileFrequency);
#undef API
        auto filename=std::filesystem::path(argv[3]).string();auto mode=std::filesystem::path(argv[4]).string();auto start=std::stoul(argv[5]),count=std::stoul(argv[6]);int memory=std::stoi(argv[7]);need(count<65536,"Count bound");
        stage("sample_load");int id=Sample_LoadFile(filename.data(),memory,nullptr);stage("sample_load_returned");need(id>0,"Sample rejected WMA");
        auto length=Sample_GetFileLength(id),ms=Sample_GetFileLengthMs(id),frequency=Sample_GetFileFrequency(id);stage("metadata_returned");
        std::cout<<"{\"sample_id\":"<<id<<",\"frames\":"<<length<<",\"milliseconds\":"<<ms<<",\"frequency\":"<<frequency;
        if(mode!="metadata"){
            need(mode=="mono"||mode=="stereo","Read mode");using File=void*(__cdecl*)(int);auto file=reinterpret_cast<File>(reinterpret_cast<std::uintptr_t>(module)+std::stoul(argv[2],nullptr,16));auto* object=file(id);need(object!=nullptr,"Sample reader");auto* decoder=*reinterpret_cast<void**>(static_cast<unsigned char*>(object)+12);need(decoder!=nullptr,"Actual decoder");
            using Pcm=unsigned(__thiscall*)(void*,void*,unsigned,unsigned,bool);auto method=reinterpret_cast<Pcm>((*reinterpret_cast<void***>(decoder))[mode=="mono"?1:2]);const unsigned channels=mode=="mono"?1:2;std::vector<unsigned char>data(16+count*channels*2+16,0x5a);
            std::ofstream("decoder-state-before.json")<<"{\"module_base\":"<<reinterpret_cast<std::uintptr_t>(module)<<",\"decoder\":"<<reinterpret_cast<std::uintptr_t>(decoder)<<",\"temporary_buffer\":"<<*reinterpret_cast<std::uintptr_t*>(static_cast<unsigned char*>(decoder)+24)<<",\"stereo_input_channels\":"<<reinterpret_cast<WAVEFORMATEX*>(static_cast<unsigned char*>(decoder)+28)->nChannels<<",\"output_buffer\":"<<reinterpret_cast<std::uintptr_t>(data.data()+16)<<",\"declared_output_bytes\":"<<count*channels*2<<"}\n";
            wchar_t traceGate[8]{};
            if(GetEnvironmentVariableW(L"FARMAR_WMA_TRACE_GATE",traceGate,8)){
                stage("diagnostic_trace_ready");auto deadline=GetTickCount64()+20000;
                while(GetFileAttributesW(L"continue.decode")==INVALID_FILE_ATTRIBUTES){need(GetTickCount64()<deadline,"Trace gate timeout");Sleep(10);}
            }
            stage("decode");auto returned=method(decoder,data.data()+16,start,count,false);stage("decode_returned");std::ofstream("decoded-pcm.bin",std::ios::binary).write(reinterpret_cast<const char*>(data.data()),data.size());
            unsigned changed=0;for(unsigned i=16;i<data.size()-16;++i)changed+=data[i]!=0x5a;bool guards=true;for(unsigned i=0;i<16;++i)guards&=data[i]==0x5a&&data[data.size()-1-i]==0x5a;
            std::cout<<",\"returned_bytes\":"<<returned<<",\"changed_bytes\":"<<changed<<",\"guards_intact\":"<<(guards?"true":"false");
        }
        stage("unload");Sample_UnloadAll();stage("unload_returned");FreeLibrary(module);stage("free_library_returned");std::cout<<"}\n";return 0;
    }catch(const std::exception&e){stage("cpp_exception");std::cerr<<e.what()<<'\n';return 1;}
}
