// Actual recovered game audio flow. Original DLL is an independent oracle;
// production SquallSoundApi imports only the source-built Squall.dll.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "SquallSoundApi.h"
#include "SoundManager.h"
#include "Sound.h"
#include "Channel.h"
#include "UiSound.h"
#include "ResourceManager.h"
#include "Settings.h"
#include "StringConversion.h"
#include "MainMenuContent.h"
#include "StringManager.h"
#include "Texture.h"
#include "Font.h"
#include "ImageTexture.h"
#include "XmlDocument.h"
#include "GameCursor.h"
#include "../src/squall/runtime/Squall.h"
#include <array>
#include <cstring>
#include <filesystem>
#include <float.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
using namespace farmar::recovered;
namespace {
void require(bool condition,const char* message) {if(!condition) throw std::runtime_error(message);}
struct LoadedApi {
    HMODULE module{};
#define SQUALL_ENTRY(name) decltype(&::name) name{};
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    explicit LoadedApi(const std::filesystem::path& path) {
        module=LoadLibraryW(path.c_str());require(module!=nullptr,"LoadLibrary failed");
#define SQUALL_ENTRY(name) name=reinterpret_cast<decltype(name)>(GetProcAddress(module,#name));require(name!=nullptr,#name);
#include "squall_exports.inc"
#undef SQUALL_ENTRY
    }
    ~LoadedApi() {if(module) FreeLibrary(module);}
};
class NativeApi final : public SoundApi {
    LoadedApi& api_;
public:
    explicit NativeApi(LoadedApi& api):api_(api) {}
    int initialize(const SoundSetup& setup) override {squall_parameters_t p;std::memcpy(&p,&setup,sizeof(p));return api_.SQUALL_Init(&p);}
    void free() override {api_.SQUALL_Free();}
    int setFileCallbacks(SoundOpen a,SoundSeek b,SoundRead c,SoundClose d) override {return api_.SQUALL_SetFileCallbacks(a,b,c,d);}
    int setEaxPreset(int value) override {return api_.SQUALL_Listener_EAX_SetPreset(value);}
    int pause(int value) override {return api_.SQUALL_Pause(value);}
    int stop() override {return api_.SQUALL_Stop();}
    int setGroupVolume(int group,int value) override {return api_.SQUALL_ChannelGroup_SetVolume(group,value);}
    int loadSample(const char* name,int memory,const void* defaults) override {return api_.SQUALL_Sample_LoadFile(const_cast<char*>(name),memory,const_cast<squall_sample_default_t*>(static_cast<const squall_sample_default_t*>(defaults)));}
    void unloadSample(int sample) override {api_.SQUALL_Sample_Unload(sample);}
    int sampleFrequency(int sample) override {return api_.SQUALL_Sample_GetFileFrequency(sample);}
    int playSample(int a,int b,int c,int d,int e,int f,int g,int h) override {return api_.SQUALL_Sample_PlayEx(a,b,c,d,e,f,g,h);}
    int stopChannel(int channel) override {return api_.SQUALL_Channel_Stop(channel);}
    int channelStatus(int channel) override {return api_.SQUALL_Channel_Status(channel);}
    int pauseChannel(int channel,int value) override {return api_.SQUALL_Channel_Pause(channel,value);}
    int startChannel(int channel) override {return api_.SQUALL_Channel_Start(channel);}
};
struct Observation {std::string name;int value;};
struct Result {
    std::vector<Observation> states;
    unsigned cursorProgressChecks=0,pausedCursorChecks=0;
    std::uint64_t positionProgress=0;
};
struct Graphics {
    HWND window{};HMODULE module{};IDirect3D8* d3d{};IDirect3DDevice8* device{};
    Graphics() {
        window=CreateWindowExW(0,L"STATIC",L"Recovered menu audio integration",WS_OVERLAPPEDWINDOW,0,0,800,600,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
        require(window!=nullptr,"Menu diagnostic window failed");
        wchar_t system[MAX_PATH];GetSystemDirectoryW(system,MAX_PATH);
        module=LoadLibraryW((std::wstring(system)+L"\\d3d8.dll").c_str());require(module!=nullptr,"Windows D3D8 unavailable");
        auto factory=reinterpret_cast<IDirect3D8*(WINAPI*)(UINT)>(GetProcAddress(module,"Direct3DCreate8"));
        require(factory!=nullptr,"Windows D3D8 factory missing");d3d=factory(120);require(d3d!=nullptr,"D3D8 SDK120 failed");
        D3DPRESENT_PARAMETERS p{};p.BackBufferWidth=800;p.BackBufferHeight=600;p.BackBufferFormat=D3DFMT_X8R8G8B8;
        p.BackBufferCount=1;p.SwapEffect=D3DSWAPEFFECT_DISCARD;p.Windowed=TRUE;p.hDeviceWindow=window;
        require(SUCCEEDED(d3d->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_FPU_PRESERVE,&p,&device)),"Menu D3D8 device failed");
    }
    ~Graphics() {if(device) device->Release();if(d3d) d3d->Release();if(module) FreeLibrary(module);if(window) DestroyWindow(window);}
};
void importedCandidateIs(LoadedApi& candidate) {
    // Verify the actual executable IAT, rather than assuming DLL search order.
    auto* base=reinterpret_cast<unsigned char*>(GetModuleHandleW(nullptr));
    auto* pe=reinterpret_cast<IMAGE_NT_HEADERS32*>(base+reinterpret_cast<IMAGE_DOS_HEADER*>(base)->e_lfanew);
    auto* imports=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+pe->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    unsigned matched=0;
    for(;imports->Name;++imports) {
        if(_stricmp(reinterpret_cast<char*>(base+imports->Name),"Squall.dll")!=0) continue;
        auto* names=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+imports->OriginalFirstThunk);
        auto* addresses=reinterpret_cast<IMAGE_THUNK_DATA32*>(base+imports->FirstThunk);
        for(;names->u1.AddressOfData;++names,++addresses) {
            const char* symbol;
            if(IMAGE_SNAP_BY_ORDINAL32(names->u1.Ordinal)) symbol=MAKEINTRESOURCEA(IMAGE_ORDINAL32(names->u1.Ordinal));
            else symbol=reinterpret_cast<char*>(reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+names->u1.AddressOfData)->Name);
            require(addresses->u1.Function==reinterpret_cast<unsigned>(GetProcAddress(candidate.module,symbol)),"Production adapter is not bound to the source candidate DLL");
            ++matched;
        }
    }
    require(matched==15,"Production fifteen-import contract changed");
}
Result run(SoundApi& backend,LoadedApi& dll,const wchar_t* data,IDirect3DDevice8* device,HWND window) {
    Result result;
    auto record=[&](const char* name,int value) {result.states.push_back({name,value});};
    FileIndex files(toNarrow(WString(data)),String("pack"));files.mountCommon(toNarrow(WString(data)));
    Settings settings(files);
    // Exercise actual Windows playback silently. Audible/loopback validation is separate.
    settings.set(String("Sound"),String("Sound"),0U);settings.set(String("Sound"),String("Music"),0U);
    {
        SoundManager sound(files,settings,backend,0);
        require(!sound.disabled(),"Actual game SoundSetup initialization failed");
        record("sound volume",int(sound.soundVolume()));record("music volume",int(sound.musicVolume()));
        ResourceManager resources(files);
        textureEnvironment().files=&files;
        textureEnvironment().load=[device](const void* bytes,const unsigned long& size,IDirect3DTexture8*& texture,TextureImageSize& dimensions) {getImageTexture(device,bytes,size,texture,dimensions);};
        fontFiles()=&files;
        StringManager strings(files,settings,resources);
        auto* music=resources.getSound(String("sounds/menu_music.ogg"));
        auto* reused=resources.getSound(String("SOUNDS/MENU_MUSIC.OGG"));
        require(reused==music,"Real music cache lookup failed");reused->releaseReference();
        record("sample frequency",backend.sampleFrequency(music->sampleID()));
        std::unique_ptr<cChannel> channel(sound.playMusic(music,true,false));
        require(channel!=nullptr,"Prepared actual game channel missing");
        const int id=channel->channelID();
        auto status=[&](const char* name) {record(name,backend.channelStatus(id));};
        auto counts=[&](const char* name) {
            squall_channels_t channels{};record(name,dll.SQUALL_GetChannelsInfo(&channels));
            const int* fields=reinterpret_cast<const int*>(&channels);
            for(unsigned i=0;i<6;++i) record(name,fields[i]);
            return channels;
        };
        status("prepared");counts("prepared counts");
        record("loop",dll.SQUALL_Channel_GetLoop(id));record("priority",dll.SQUALL_Channel_GetPriority(id));
        record("frequency",dll.SQUALL_Channel_GetFrequency(id));record("pan",dll.SQUALL_Channel_GetPan(id));
        auto progress=[&]() {
            require(dll.SQUALL_Channel_GetVolume(id)==0,"Cursor playback scenario is not silent");
            const int before=dll.SQUALL_Channel_GetPlayPosition(id);
            require(before>=0,"Initial playback position failed");
            const auto deadline=GetTickCount64()+1500;int after=before;
            while(after==before && GetTickCount64()<deadline) {Sleep(10);after=dll.SQUALL_Channel_GetPlayPosition(id);}
            require(after>=0 && after!=before,"Actual DirectSound playback did not advance");
            ++result.cursorProgressChecks;result.positionProgress+=unsigned(after-before);
        };
        auto frozen=[&]() {
            const int before=dll.SQUALL_Channel_GetPlayPosition(id);require(before>=0,"Paused position failed");
            Sleep(40);require(dll.SQUALL_Channel_GetPlayPosition(id)==before,"Paused channel advanced");++result.pausedCursorChecks;
        };
        channel->start();status("playing");counts("playing counts");progress();
        channel->pause();status("channel paused");counts("paused counts");frozen();
        channel->start();status("channel resumed");progress();
        sound.setPause(1);status("manager paused");frozen();
        sound.setPause(1);status("unchanged manager pause");
        sound.setPause(0);status("manager resumed");progress();
        // Both original group curves and controller call order are exercised.
        sound.setPause(1);
        auto options=bindOptionsSound(sound,{});
        require(!options.soundDisabled(),"Options sound binding disabled unexpectedly");
        for(unsigned volume:{0U,1U,24U,50U,99U,100U}) {
            options.setMusicVolume(volume);record("options music",int(options.musicVolume()));
            record("physical music volume",dll.SQUALL_Channel_GetVolume(id));
            options.setSoundVolume(volume);record("options sound",int(options.soundVolume()));
        }
        options.setMusicVolume(0);options.setSoundVolume(0);
        sound.setPause(0);status("after options");progress();
        // Physical channel group 1 through the same button binding used by menu controls.
        GameCursor cursor(window);
        ButtonInteraction buttons;bindButtonSound(buttons,resources,sound);
        buttons.cursor=[&] {cursor.update(false,{});return cursor.position();};
        auto* click=buttons.loadSound(String("sounds/menu_click.wav"));require(click!=nullptr,"UI click sample failed");
        buttons.playClick(click);counts("button click counts");
        sound.setPause(1);
        {
            // Real menu XML, actual controls, resources/textures/fonts and real
            // audio services. MainMenu action controller is still separate.
            InputBindings input;ViewRegistry views;ControlRegistry controls;
            MenuView menu(nullptr,resources,settings,strings,views,controls),fixed(nullptr,resources,settings,strings,views,controls);
            MainMenuContent content(menu,resources,settings,strings,input,buttons,views,controls);
            const auto menuXML=resources.loadSettingsXML(String("UI/MainMenu.xml"));
            const auto fixedXML=resources.loadSettingsXML(String("UI/FixedBack.xml"));
            content.loadXML(*menuXML,[&] {fixed.loadXML(*fixedXML);fixed.show();},nullptr);
            menu.show();menu.enable();
            auto services=bindOptionsSound(sound,[](bool) {throw std::runtime_error("Fullscreen reset is outside the audio comparison");});
            Options options(resources,settings,strings,input,buttons,std::move(services),views,controls);
            for(float volume:{0.0f,24.5f,50.75f,99.0f,100.0f}) {
                options.musicSlider().setValue(volume);
                options.musicSlider().valueChanged(&options.musicSlider(),options.musicSlider().value());
                record("actual options music",int(sound.musicVolume()));record("actual options channel volume",dll.SQUALL_Channel_GetVolume(id));
                options.soundSlider().setValue(volume);
                options.soundSlider().valueChanged(&options.soundSlider(),options.soundSlider().value());record("actual options sound",int(sound.soundVolume()));
            }
            sound.setMusicVolume(0);sound.setSoundVolume(0);
            squall_channels_t before{};require(dll.SQUALL_GetChannelsInfo(&before)>=0,"Menu channel counts failed");
            content.start->clicked();const auto after=counts("actual menu button counts");
            require(after.Play==before.Play+1,"Actual menu button did not start its ambient click channel");
        }
        record("stop all",backend.stop());
        const auto deadline=GetTickCount64()+2000;
        for(;;) {
            squall_channels_t channels{};require(dll.SQUALL_GetChannelsInfo(&channels)>=0,"GetChannelsInfo after Stop failed");
            const int* fields=reinterpret_cast<const int*>(&channels);bool empty=true;for(unsigned i=0;i<6;++i) empty &= fields[i]==0;
            if(empty && backend.channelStatus(id)==0) break;
            require(GetTickCount64()<deadline,"Game channel workers did not retire");Sleep(1);
        }
        status("retired");counts("retired counts");
        channel.reset();music->releaseReference();click->releaseReference();
        resources.clearUseless();record("cache after unload",int(resources.cachedCount()));
        require(sound.files().openCount()==0,"Actual game callback handles leaked");
    }
    unsigned music{},effects{};require(settings.get(String("Sound"),String("Music"),music) && settings.get(String("Sound"),String("Sound"),effects),"Settings lifetime missing");
    record("saved music",int(music));record("saved sound",int(effects));
    textureEnvironment()={};fontFiles()=nullptr;
    return result;
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc<3 || argc>4) return 2;
    try {
        wchar_t executable[MAX_PATH];require(GetModuleFileNameW(nullptr,executable,MAX_PATH)>0,"Executable path missing");
        LoadedApi source(std::filesystem::path(executable).parent_path()/L"Squall.dll"),native(std::filesystem::absolute(argv[1]));
        importedCandidateIs(source);
        _control87(argc==4 ? _PC_24:_PC_53,_MCW_PC);
        Graphics graphics;
        NativeApi nativeBackend(native);SquallSoundApi sourceBackend;
        const auto expected=run(nativeBackend,native,argv[2],graphics.device,graphics.window),actual=run(sourceBackend,source,argv[2],graphics.device,graphics.window);
        require(actual.states.size()==expected.states.size(),"Game audio state count differs");
        for(unsigned i=0;i<actual.states.size();++i) {
            if(actual.states[i].name!=expected.states[i].name || actual.states[i].value!=expected.states[i].value) {
                std::cerr<<i<<' '<<actual.states[i].name<<" source="<<actual.states[i].value<<" native="<<expected.states[i].value<<'\n';throw std::runtime_error("Actual game audio behavior differs");
            }
        }
        require(actual.cursorProgressChecks==4 && expected.cursorProgressChecks==4 && actual.pausedCursorChecks==2 && expected.pausedCursorChecks==2,"Playback checks incomplete");
        require((_control87(0,0)&_MCW_PC)==(argc==4 ? _PC_24:_PC_53),"Playback changed x87 precision");
        std::cout<<"{\"state_comparisons\":"<<actual.states.size()<<",\"production_imports\":15,\"cursor_progress_checks\":"<<actual.cursorProgressChecks+expected.cursorProgressChecks<<",\"paused_cursor_checks\":"<<actual.pausedCursorChecks+expected.pausedCursorChecks<<",\"source_position_progress\":"<<actual.positionProgress<<",\"native_position_progress\":"<<expected.positionProgress<<",\"playback_tested\":true,\"silent_playback\":true,\"audible_output_verified\":false}\n";
        return 0;
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
