#include "GameApplication.h"
#include "GameFileSystem.h"
#include "GameWindow.h"
#include "GameTimer.h"
#include "GameCursor.h"
#include "GameRandom.h"
#include "Game.h"
#include "MainMenu.h"
#include "StartupView.h"
#include "GamePauseMenu.h"
#include "HelpView.h"
#include "FileIndex.h"
#include "Settings.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "Font.h"
#include "Shader.h"
#include "ImageTexture.h"
#include "StringManager.h"
#include "ProfileSession.h"
#include "SoundManager.h"
#include "Sound.h"
#include "SquallSoundApi.h"
#include "MusicPlayback.h"
#include "InputDevices.h"
#include "GraphicsRenderState.h"
#include "SpriteBatches.h"
#include "StringConversion.h"
#include <shlwapi.h>
#include <stdexcept>
namespace farmar::recovered {
namespace {
struct Module {HMODULE value=nullptr;~Module(){if(value) FreeLibrary(value);}};
template<class T> struct Com {
    T* value=nullptr;~Com(){reset();}void reset(){if(value)value->Release();value=nullptr;}
};
String applicationClass(HINSTANCE instance) {
    char shortPath[MAX_PATH]{},longPath[MAX_PATH]{};
    if(!GetModuleFileNameA(instance,shortPath,MAX_PATH) || !GetLongPathNameA(shortPath,longPath,MAX_PATH)) return String();
    const auto* name=PathFindFileNameA(longPath);if(name==longPath) return String();
    String result(std::string("Melesta_")+name);result.lowercase();return result;
}
}
struct GameApplication::Runtime {
    static Runtime* active;
    Module module;
    std::unique_ptr<FileIndex> files;
    std::unique_ptr<ResourceManager> resources;
    std::unique_ptr<Settings> settings;
    HWND window=nullptr;Com<IDirect3D8> d3d;Com<IDirect3DDevice8> device;
    PrimaryGraphicsSettings graphics;std::unique_ptr<GraphicsPresentation> presentation;
    std::unique_ptr<GraphicsRenderState> render;std::unique_ptr<SpriteBuffers> buffers;std::unique_ptr<SpriteBatches> batches;
    std::unique_ptr<InputDevices> devices;std::unique_ptr<GameTimer> timer;SquallSoundApi api;
    std::unique_ptr<SoundManager> sound;MusicPlayback music;std::unique_ptr<GameCursor> cursor;
    std::unique_ptr<StringManager> strings;ViewRegistry views;ControlRegistry controls;ButtonInteraction interaction;
    std::unique_ptr<ProfileSession> profiles;std::unique_ptr<MainMenuContext> mainMenu;std::unique_ptr<StartupContext> startup;
    GameRandom random;std::unique_ptr<GameContext> gameContext;std::unique_ptr<GamePauseContext> pauseContext;
    bool paused=false,minimized=false,finished=false;
    Runtime(HINSTANCE instance,const GameApplicationOptions& options) {
        if(active) throw "Object already exist.";
        files=std::make_unique<FileIndex>(options.dataDirectory,String("pack"));mountGameCommonFiles(*files,options.commonDirectory);
        resources=std::make_unique<ResourceManager>(*files);
        settings=std::make_unique<Settings>(*files,[this](const String& name){return resources->loadSettingsXML(name);});
        window=createGameWindow(instance,*settings,toWide(applicationClass(instance)));if(!window) throw std::runtime_error("Cannot create game window");
        wchar_t system[MAX_PATH]{};GetSystemDirectoryW(system,MAX_PATH);module.value=LoadLibraryW((std::wstring(system)+L"/d3d8.dll").c_str());
        if(!module.value) throw "Can't create Direct3D.";
        const auto factory=reinterpret_cast<IDirect3D8*(WINAPI*)(UINT)>(GetProcAddress(module.value,"Direct3DCreate8"));
        d3d.value=factory ? factory(120):nullptr;if(!d3d.value) throw "Can't create Direct3D.";
        graphics.load(*settings);presentation=std::make_unique<GraphicsPresentation>(d3d.value,window);presentation->prepare(graphics);
        ShowWindow(window,SW_SHOW);UpdateWindow(window);SetActiveWindow(window);
        auto parameters=presentation->parameters();
        if(FAILED(d3d.value->CreateDevice(0,D3DDEVTYPE_HAL,window,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&parameters,&device.value))) throw std::runtime_error("Cannot create original Direct3D8 device");
        createBuffers();
        textureEnvironment().files=files.get();textureEnvironment().load=[this](const void* data,const unsigned long& size,IDirect3DTexture8*& texture,TextureImageSize& image){getImageTexture(device.value,data,size,texture,image);};
        fontFiles()=files.get();shaderEnvironment().files=files.get();
        shaderEnvironment().create=[this](const DWORD* code,const unsigned long&,DWORD& handle){createSpriteVertexShader(device.value,code,handle);};
        shaderEnvironment().remove=[this](const DWORD& handle){device.value->DeleteVertexShader(handle);};
        devices=std::make_unique<InputDevices>(instance,window);devices->bindWindowTargets();
        timer=std::make_unique<GameTimer>(*settings);uiElapsedTime()=[this]{return timer->elapsed();};
        // The original SoundManager leaves its pause byte unspecified. The
        // application's defined initial state is unpaused; WM activation owns
        // subsequent real backend pause/resume calls. Heap-byte ABI is pending.
        sound=std::make_unique<SoundManager>(*files,*settings,api,0);
        cursor=std::make_unique<GameCursor>(window);
        windowTargets().pauseMain=[this](bool value){paused=value;};windowTargets().minimizeMain=[this](bool value){minimized=value;};
        windowTargets().pauseSound=[this](bool value){sound->setPause(static_cast<unsigned char>(value));};
        strings=std::make_unique<StringManager>(*files,*settings,*resources);
        interaction.cursor=[this]{return cursor->position();};interaction.loadSound=[this](const String& name){return resources->getSound(name);};
        interaction.playClick=[this](cResource* resource){sound->clickSound(static_cast<cSound*>(resource));};
        interaction.playHover=[this](cResource* resource,bool loop,unsigned start){delete sound->playSound(static_cast<cSound*>(resource),loop,start!=0);};
        profiles=std::make_unique<ProfileSession>(*files,*settings,*resources);
        const auto screen=[this]{return std::array<unsigned,2>{unsigned(graphics.width),unsigned(graphics.height)};};
        const auto reset=[this](bool reload){resetGraphics(reload);};
        mainMenu=std::make_unique<MainMenuContext>(*files,*resources,*settings,*strings,*sound,music,*profiles,devices->bindings(),interaction,screen,reset,&constructGame,views,controls);
        startup=std::make_unique<StartupContext>(*files,*resources,*settings,*strings,*sound,music,*profiles,devices->bindings(),interaction,*mainMenu,views,controls);
        active=this;auto* screenView=new StartupView(*startup);screenView->hide();screenView->showTransparently();
    }
    void createBuffers() {
        buffers=std::make_unique<SpriteBuffers>(device.value);buffers->resize(graphics.reservedSprites);
        render=std::make_unique<GraphicsRenderState>(device.value);render->setup(graphics,*presentation);
        batches=std::make_unique<SpriteBatches>(*buffers);
    }
    void resetGraphics(bool reload) {
        if(reload) graphics.load(*settings);
        resources->preResetGraphics();batches.reset();render.reset();buffers.reset();
        presentation->prepare(graphics);auto parameters=presentation->parameters();
        const auto result=device.value->Reset(&parameters);
        if(FAILED(result)) throw std::runtime_error("Direct3D8 reset failed");
        createBuffers();resources->restoreGraphics();
    }
    static void constructGame() {
        auto& r=*active;if(Game::instance()) throw "Object already exist.";
        auto* profile=r.profiles->current();if(!profile) throw std::logic_error("Actual game construction requires the selected profile");
        const auto screen=[&r]{return std::array<unsigned,2>{unsigned(r.graphics.width),unsigned(r.graphics.height)};};
        const auto reset=[&r](bool reload){r.resetGraphics(reload);};
        r.gameContext=std::make_unique<GameContext>(GameContext{*r.files,*r.resources,*r.settings,*r.strings,*r.sound,*profile,r.random,r.devices->bindings(),r.interaction,screen,
            [&r,screen,reset](Game& game)->GamePauseContext& {
                r.pauseContext=std::make_unique<GamePauseContext>(GamePauseContext{*r.files,*r.resources,*r.settings,*r.strings,*r.sound,r.music,r.devices->bindings(),r.interaction,game,
                    []()->View&{return *HelpView::instance();},*r.mainMenu,screen,reset,[&r](const String& origin){r.views.requestDiscard(Game::instance(),origin);},r.views,r.controls});
                return *r.pauseContext;
            },r.views,r.controls});
        new Game(*r.gameContext);
    }
    void frame() {
        if(minimized) {views.deleteDiscarded();return;}
        cursor->update(graphics.fullscreen,presentation->imagePlacement());timer->process();devices->process();
        if(!paused) {views.dispatchSteps();views.updateTransitions([this]{return timer->elapsed();});}
        const auto presented=device.value->Present(nullptr,nullptr,nullptr,nullptr);
        if(presented==D3DERR_DEVICELOST) {
            const auto state=device.value->TestCooperativeLevel();
            if(state==D3DERR_DEVICENOTRESET) resetGraphics(false);else Sleep(10);
            views.deleteDiscarded();return;
        }
        batches->begin();controls.forEachVisible([this](Control& control){control.submitSprites(*batches,graphics.fullscreen,presentation->imagePlacement());});batches->finish();
        device.value->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,graphics.showTriangles ? 0xff000000U:graphics.background,0,0);
        if(FAILED(device.value->BeginScene())) throw std::runtime_error("Direct3D8 BeginScene failed");
        drawSpriteBatches(device.value,*buffers,batches->batches(),render->transposedViewProjection(),graphics.fullscreen);
        DWORD lighting=0;device.value->GetRenderState(D3DRS_LIGHTING,&lighting);if(lighting) device.value->SetRenderState(D3DRS_LIGHTING,0);
        if(graphics.fullscreen) {device.value->SetTexture(0,nullptr);device.value->SetStreamSource(0,render->borderVertices(),24);device.value->DrawPrimitive(D3DPT_TRIANGLELIST,0,4);}
        device.value->EndScene();views.deleteDiscarded();
    }
    void shutdown() {
        if(finished) return;finished=true;
        if(Game::instance()) delete Game::instance();views.deleteDiscarded();
        if(profiles) profiles->close();
        windowTargets()={};startup.reset();mainMenu.reset();pauseContext.reset();gameContext.reset();profiles.reset();
        strings.reset();music.clear();sound.reset();timer.reset();uiElapsedTime()={};devices.reset();cursor.reset();
        batches.reset();render.reset();buffers.reset();device.reset();d3d.reset();presentation.reset();
        if(settings) settings->save();settings.reset();resources.reset();
        textureEnvironment()={};shaderEnvironment()={};fontFiles()=nullptr;files.reset();
        if(window) {DestroyWindow(window);window=nullptr;}active=nullptr;
    }
    ~Runtime() {if(!finished) {try {shutdown();}catch(...) {OutputDebugStringW(L"Game shutdown failed\n");}}}
};
GameApplication::Runtime* GameApplication::Runtime::active=nullptr;
GameApplication::GameApplication(HINSTANCE instance,const GameApplicationOptions& options):runtime_(std::make_unique<Runtime>(instance,options)) {}
GameApplication::~GameApplication()=default;
void GameApplication::run() {processGameMessages([this]{runtime_->frame();});runtime_->shutdown();}
bool GameApplication::activateExisting(HINSTANCE instance) {
    const auto name=applicationClass(instance);if(name.empty()) return false;
    const auto window=FindWindowA(name.c_str(),nullptr);if(!window) return false;
    if(IsIconic(window)) SendMessageW(window,WM_SYSCOMMAND,SC_RESTORE,0);else SetForegroundWindow(window);return true;
}
}
