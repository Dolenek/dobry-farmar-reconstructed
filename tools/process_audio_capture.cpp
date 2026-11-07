// Test-only Windows process loopback. Includes only the requested process tree;
// never captures a microphone or unrelated application/endpoint mix.
// API contract: Microsoft ApplicationLoopback sample and SDK26100 headers.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NTDDI_VERSION 0x0A00000A
#include <windows.h>
#include <audioclient.h>
#include <audioclientactivationparams.h>
#include <mmdeviceapi.h>
#include <wrl.h>
#include <wrl/implements.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using Microsoft::WRL::ComPtr;
void checked(HRESULT value) {if(FAILED(value)) throw value;}
class Completion final:public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
    IActivateAudioInterfaceCompletionHandler,Microsoft::WRL::FtmBase> {
public:
    HANDLE event=CreateEventW(nullptr,FALSE,FALSE,nullptr);
    HRESULT result=E_PENDING;
    ComPtr<IAudioClient> client;
    ~Completion() {if(event) CloseHandle(event);}
    STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation* operation) override {
        ComPtr<IUnknown> value;HRESULT activation=E_UNEXPECTED;
        result=operation->GetActivateResult(&activation,&value);
        if(SUCCEEDED(result)) result=activation;
        if(SUCCEEDED(result)) result=value.As(&client);
        SetEvent(event);return S_OK;
    }
};
int wmain(int argc,wchar_t** argv) {
    try {
        if(argc!=4) throw std::runtime_error("Expected owned-game PID, seconds (0.2..30), raw PCM output");
        const DWORD pid=std::stoul(argv[1]);const double seconds=std::stod(argv[2]);
        if(!pid || seconds<.2 || seconds>30) throw std::runtime_error("Invalid bounded capture arguments");
        checked(CoInitializeEx(nullptr,COINIT_MULTITHREADED));
        auto completion=Microsoft::WRL::Make<Completion>();
        if(!completion || !completion->event) throw std::runtime_error("Completion event unavailable");
        AUDIOCLIENT_ACTIVATION_PARAMS parameters{};
        parameters.ActivationType=AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
        parameters.ProcessLoopbackParams.TargetProcessId=pid;
        parameters.ProcessLoopbackParams.ProcessLoopbackMode=PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;
        PROPVARIANT variant{};variant.vt=VT_BLOB;variant.blob.cbSize=sizeof(parameters);
        variant.blob.pBlobData=reinterpret_cast<BYTE*>(&parameters);
        ComPtr<IActivateAudioInterfaceAsyncOperation> operation;
        checked(ActivateAudioInterfaceAsync(VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK,__uuidof(IAudioClient),
            &variant,completion.Get(),&operation));
        if(WaitForSingleObject(completion->event,10000)!=WAIT_OBJECT_0) throw std::runtime_error("Process loopback activation timeout");
        checked(completion->result);auto client=completion->client;
        WAVEFORMATEX format{};format.wFormatTag=WAVE_FORMAT_PCM;format.nChannels=2;
        format.nSamplesPerSec=44100;format.wBitsPerSample=16;format.nBlockAlign=4;
        format.nAvgBytesPerSec=176400;
        checked(client->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_LOOPBACK|
            AUDCLNT_STREAMFLAGS_EVENTCALLBACK|AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM,0,0,&format,nullptr));
        HANDLE ready=CreateEventW(nullptr,FALSE,FALSE,nullptr);
        if(!ready) throw std::runtime_error("Capture event unavailable");
        checked(client->SetEventHandle(ready));ComPtr<IAudioCaptureClient> capture;
        checked(client->GetService(IID_PPV_ARGS(&capture)));
        std::ofstream output(std::filesystem::path(argv[3]),std::ios::binary);
        if(!output) throw std::runtime_error("Capture output unavailable");
        checked(client->Start());const ULONGLONG start=GetTickCount64();
        std::cout<<"{\"event\":\"capturing\",\"pid\":"<<pid
            <<",\"process_tree_only\":true,\"microphone\":false,\"rate\":44100,\"channels\":2,\"bits\":16}"<<std::endl;
        std::uint64_t frames=0,nonzero=0,packets=0,silentPackets=0,discontinuities=0;int peak=0;long double square=0;
        while(GetTickCount64()-start<seconds*1000) {
            UINT32 available=0;checked(capture->GetNextPacketSize(&available));
            while(available) {
                BYTE* data=nullptr;UINT32 count=0;DWORD flags=0;
                checked(capture->GetBuffer(&data,&count,&flags,nullptr,nullptr));
                std::vector<std::int16_t> silence;
                if(flags&AUDCLNT_BUFFERFLAGS_SILENT) {silence.resize(std::size_t(count)*2);data=reinterpret_cast<BYTE*>(silence.data());++silentPackets;}
                if(flags&AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY) ++discontinuities;
                const auto* samples=reinterpret_cast<const std::int16_t*>(data);
                for(std::size_t i=0;i<std::size_t(count)*2;++i) {
                    const int value=samples[i];if(value) ++nonzero;
                    peak=std::max(peak,std::abs(value));square+=static_cast<long double>(value)*value;
                }
                output.write(reinterpret_cast<const char*>(data),std::streamsize(count)*4);
                checked(capture->ReleaseBuffer(count));frames+=count;++packets;
                checked(capture->GetNextPacketSize(&available));
            }
            WaitForSingleObject(ready,20);
        }
        checked(client->Stop());CloseHandle(ready);output.close();
        std::cout<<"{\"event\":\"completed\",\"pid\":"<<pid<<",\"frames\":"<<frames
            <<",\"packets\":"<<packets<<",\"silent_packets\":"<<silentPackets
            <<",\"discontinuities\":"<<discontinuities<<",\"nonzero_samples\":"<<nonzero
            <<",\"peak\":"<<peak<<",\"rms\":"<<(frames ? std::sqrt(double(square/(frames*2))):0)
            <<",\"elapsed_ms\":"<<GetTickCount64()-start<<"}"<<std::endl;
        return 0;
    } catch(HRESULT error) {std::cerr<<"{\"error_hresult\":"<<static_cast<unsigned long>(error)<<"}\n";return 1;}
      catch(const std::exception& error) {std::cerr<<"{\"error\":\""<<error.what()<<"\"}\n";return 1;}
}
