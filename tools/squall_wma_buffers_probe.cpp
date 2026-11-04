// Actual WMA Sample_Play exports and actual DirectSound buffers; isolated per case.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <float.h>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include "Squall.h"
#include "SoundEngine/Context.h"
void need(bool ok,const char* what){if(!ok)throw std::runtime_error(what);}
void stage(const char* name){std::ofstream("operations.log",std::ios::app)<<name<<'\n';}
int wmain(int argc,wchar_t**argv){if(argc<7||argc>8)return 2;SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::cout.setf(std::ios::unitbuf);HWND window{};
    try{
        _control87(argc==8?_PC_24:_PC_53,_MCW_PC);auto module=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());need(module!=nullptr,"Load DLL");
#define API(name) auto name=reinterpret_cast<decltype(&SQUALL_##name)>(GetProcAddress(module,"SQUALL_" #name));need(name!=nullptr,#name)
        API(Init);API(Free);API(SetBufferSize);API(Sample_LoadFile);API(Sample_UnloadAll);API(Sample_GetFileLength);API(Sample_GetFileFrequency);API(Sample_PlayEx);API(Sample_Play3DEx);API(Channel_Stop);API(Channel_Status);API(Channel_Start);API(Channel_Pause);API(Channel_GetPlayPosition);
#undef API
        auto* context=reinterpret_cast<context_t*>(reinterpret_cast<std::uintptr_t>(module)+std::stoul(argv[2],nullptr,16));auto file=std::filesystem::path(argv[3]).string();bool spatial=std::stoi(argv[4])!=0;int memory=std::stoi(argv[5]),bufferMs=std::stoi(argv[6]);
        window=CreateWindowExW(0,L"STATIC",L"Owned WMA buffer test",0,0,0,64,64,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);need(window!=nullptr,"Owned window");squall_parameters_t p{};p.Window=window;p.SampleRate=44100;p.BitPerSample=16;p.Channels=4;p.UseAlg=1;p.BufferSize=200;p.DistanceFactor=1;p.RolloffFactor=.001f;p.DopplerFactor=1;
        stage("init");auto initialized=Init(&p);need(initialized>=0,"Init");auto changed=SetBufferSize(bufferMs);need(changed>=0&&context->_bufferSize==bufferMs,"Actual public buffer-size setting");
        stage("sample_load");auto sample=Sample_LoadFile(file.data(),memory,nullptr);need(sample>0,"WMA load");auto frames=Sample_GetFileLength(sample),frequency=Sample_GetFileFrequency(sample);
        stage("sample_play");std::array<float,3>zero{};auto channel=spatial?Sample_Play3DEx(sample,1,17,0,zero.data(),zero.data(),7,0,0,1,1000):Sample_PlayEx(sample,1,17,0,7,0,0,50);stage("sample_play_returned");
        std::cout<<"{\"init\":"<<initialized<<",\"buffer_set_result\":"<<changed<<",\"buffer_milliseconds\":"<<context->_bufferSize<<",\"sample\":"<<sample<<",\"frames\":"<<frames<<",\"frequency\":"<<frequency<<",\"channel\":"<<channel;
        if(channel>=0){
            need(context->_channelsArray&&unsigned(channel&255)<unsigned(context->_channels),"Actual channel");auto* buffer=context->_channelsArray[channel&255].DS_Buffer;need(buffer!=nullptr,"Real DirectSound buffer");DSBCAPS caps{};caps.dwSize=sizeof(caps);need(buffer->GetCaps(&caps)==DS_OK&&caps.dwBufferBytes>0&&caps.dwBufferBytes<1048576,"Actual caps");
            void* first{};void* second{};DWORD n{},m{};need(buffer->Lock(0,caps.dwBufferBytes,&first,&n,&second,&m,0)==DS_OK,"Actual Lock");std::vector<unsigned char>data(n+m);std::memcpy(data.data(),first,n);if(m)std::memcpy(data.data()+n,second,m);need(buffer->Unlock(first,n,second,m)==DS_OK,"Actual Unlock");std::ofstream("physical-pcm.bin",std::ios::binary).write(reinterpret_cast<const char*>(data.data()),data.size());unsigned nonzero=0;for(auto x:data)nonzero+=x!=0;
            std::cout<<",\"physical_bytes\":"<<data.size()<<",\"nonzero_bytes\":"<<nonzero;stage("buffer_snapshotted");
            DWORD before{},writeCursor{};need(buffer->GetCurrentPosition(&before,&writeCursor)==DS_OK,"Before cursor");stage("start_actual_playback");need(Channel_Start(channel)>=0,"Start");DWORD after=before;const auto progressDeadline=GetTickCount64()+180;int logicalBefore=Channel_GetPlayPosition(channel),logicalAfter=logicalBefore;
            do{Sleep(2);need(buffer->GetCurrentPosition(&after,&writeCursor)==DS_OK,"Playing cursor");logicalAfter=Channel_GetPlayPosition(channel);}while(after==before&&logicalAfter==logicalBefore&&GetTickCount64()<progressDeadline);
            const bool progress=(after!=before||logicalAfter!=logicalBefore)&&logicalAfter>=0;DWORD playbackStatus{};need(buffer->GetStatus(&playbackStatus)==DS_OK,"Actual buffer status");const int channelStatus=Channel_Status(channel);
            need(Channel_Pause(channel,1)>=0,"Pause");DWORD paused{},pausedAfter{};need(buffer->GetCurrentPosition(&paused,&writeCursor)==DS_OK,"Paused cursor");Sleep(25);need(buffer->GetCurrentPosition(&pausedAfter,&writeCursor)==DS_OK&&pausedAfter==paused,"Paused cursor changed");stage(progress?"playback_and_pause_verified":"no_cursor_progress_pause_observed");
            std::cout<<",\"actual_playback_progress\":"<<(progress?"true":"false")<<",\"directsound_playing_flag\":"<<((playbackStatus&DSBSTATUS_PLAYING)?"true":"false")<<",\"raw_channel_status\":"<<channelStatus<<",\"actual_pause_stable\":true,\"raw_cursors\":["<<before<<','<<after<<','<<paused<<','<<pausedAfter<<"],\"raw_logical_positions\":["<<logicalBefore<<','<<logicalAfter<<']';need(Channel_Stop(channel)>=0,"Stop");auto deadline=GetTickCount64()+3000;while(Channel_Status(channel)!=0){need(GetTickCount64()<deadline,"Worker retirement");Sleep(1);}
        }
        stage("unload");Sample_UnloadAll();stage("unload_returned");auto exclusive=CreateFileW(std::filesystem::path(file).c_str(),GENERIC_READ,0,nullptr,OPEN_EXISTING,0,nullptr);bool closed=exclusive!=INVALID_HANDLE_VALUE;if(closed)CloseHandle(exclusive);std::cout<<",\"file_exclusive_after_unload\":"<<(closed?"true":"false");Free();DestroyWindow(window);window=nullptr;FreeLibrary(module);stage("complete");std::cout<<"}\n";return 0;
    }catch(const std::exception&e){if(window)DestroyWindow(window);std::cerr<<e.what()<<'\n';return 1;}
}
