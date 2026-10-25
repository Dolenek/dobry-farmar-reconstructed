// Source-only whole-cGame composition probe. Native whole-world comparison
// and interactive executable coverage are separate, explicitly pending work.
#include "Game.h"
#include "GamePersistence.h"
#include "GameConfiguration.h"
#include "GameRandom.h"
#include "GameTimer.h"
#include "GameWindow.h"
#include "GameMenu.h"
#include "MainMenu.h"
#include "HelpView.h"
#include "Farm.h"
#include "Market.h"
#include "Manufacture.h"
#include "GameAwardManager.h"
#include "TutorialManager.h"
#include "Journal.h"
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
#include "GraphicsRenderState.h"
#include "SpriteBatches.h"
#include "SalesHistory.h"
#include <filesystem>
#include <cstring>
#include <iostream>
#include <fstream>
using namespace farmar::recovered;
namespace {
GameContext* gameContext=nullptr;
DWORD probeMilliseconds=0;
DWORD WINAPI probeClock() {return probeMilliseconds;}
void WINAPI probeSleep(DWORD delay) {probeMilliseconds+=delay;}
void constructGame() {try {new Game(*gameContext);}catch(const std::exception& error) {std::cerr<<"construct: "<<error.what()<<'\n';throw;}catch(const char* error) {std::cerr<<"construct: "<<error<<'\n';throw;}}
void require(bool value,const char* message) {if(!value) {std::cerr<<"check: "<<message<<'\n';throw std::runtime_error(message);}}
String ansi(const wchar_t* text) {int n=WideCharToMultiByte(CP_ACP,0,text,-1,nullptr,0,nullptr,nullptr);std::string result(n,0);WideCharToMultiByte(CP_ACP,0,text,-1,result.data(),n,nullptr,nullptr);return String(result.c_str());}
struct RuntimeModule {HMODULE handle=nullptr;~RuntimeModule() {if(handle) FreeLibrary(handle);}};
std::vector<char> bytes(FileIndex& files,const String& name) {auto file=files.open(name);std::vector<char> result(file->size());file->read(result.data(),unsigned(result.size()));return result;}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3 && argc!=4) return 2;
    try {
        RuntimeModule runtimeModule; // outlives the resource cache's COM objects
        const auto common=std::filesystem::absolute(argv[2])/std::to_string(GetCurrentProcessId());std::filesystem::create_directories(common);
        FileIndex files(ansi(argv[1]),String("pack"));files.mountCommon(String(common.generic_string()));
        ResourceManager resources(files);Settings settings(files,[&](const String& name){return resources.loadSettingsXML(name);});
        settings.set(String("graphics"),String("fullscreen"),false);settings.set(String("game"),String("profiles"),String("GameWorldProfiles"));
        auto window=createGameWindow(GetModuleHandleW(nullptr),settings,WString(L"Farmar source composition"));require(window,"source probe window failed");
        wchar_t system[MAX_PATH];GetSystemDirectoryW(system,MAX_PATH);HMODULE module=runtimeModule.handle=LoadLibraryW((std::wstring(system)+L"/d3d8.dll").c_str());require(module,"D3D8 runtime failed");
        auto factory=reinterpret_cast<IDirect3D8*(WINAPI*)(UINT)>(GetProcAddress(module,"Direct3DCreate8"));auto* d3d=factory(120);require(d3d,"D3D8 SDK failed");
        PrimaryGraphicsSettings graphics;graphics.load(settings);GraphicsPresentation presentation(d3d,window);presentation.prepare(graphics);auto parameters=presentation.parameters();IDirect3DDevice8* device=nullptr;
        const DWORD deviceFlags=D3DCREATE_SOFTWARE_VERTEXPROCESSING|(argc==4 ? D3DCREATE_FPU_PRESERVE:0);
        require(SUCCEEDED(d3d->CreateDevice(0,D3DDEVTYPE_HAL,window,deviceFlags,&parameters,&device)),"source probe device failed");
        textureEnvironment().files=&files;textureEnvironment().load=[&](const void* data,const unsigned long& size,IDirect3DTexture8*& texture,TextureImageSize& image){getImageTexture(device,data,size,texture,image);};
        fontFiles()=&files;shaderEnvironment().files=&files;
        shaderEnvironment().create=[&](const DWORD* code,const unsigned long&,DWORD& handle){createSpriteVertexShader(device,code,handle);};
        shaderEnvironment().remove=[&](const DWORD& handle){device->DeleteVertexShader(handle);};
        {
        TimerServices timerServices;timerServices.milliseconds=&probeClock;timerServices.sleep=&probeSleep;
        GameTimer timer(settings,timerServices);uiElapsedTime()=[&]{return timer.elapsed();};
        StringManager strings(files,settings,resources);SquallSoundApi api;SoundManager sound(files,settings,api,0);MusicPlayback music;
        InputBindings input;ButtonInteraction interaction;Point2 cursor{799,599};interaction.cursor=[&] {return cursor;};interaction.loadSound=[&](const String& name){return resources.getSound(name);};
        interaction.playClick=[&](cResource* resource){sound.clickSound(static_cast<cSound*>(resource));};interaction.playHover=[&](cResource* resource,bool loop,unsigned start){delete sound.playSound(static_cast<cSound*>(resource),loop,start!=0);};
        ViewRegistry views;ControlRegistry controls;ProfileSession profiles(files,settings,resources);profiles.open(WString(L"WorldProbe"));GameRandom random;
        const auto screen=[] {return std::array<unsigned,2>{800,600};};
        const auto reset=[&](bool){resources.preResetGraphics();require(SUCCEEDED(device->Reset(&parameters)),"source probe graphics reset failed");resources.restoreGraphics();};
        MainMenuContext menu(files,resources,settings,strings,sound,music,profiles,input,interaction,screen,reset,&constructGame,views,controls);
        std::unique_ptr<GamePauseContext> pause;
        GameContext context{files,resources,settings,strings,sound,*profiles.current(),random,input,interaction,screen,
            [&](Game& game)->GamePauseContext& {pause=std::make_unique<GamePauseContext>(GamePauseContext{files,resources,settings,strings,sound,music,input,interaction,game,[]()->View&{return *HelpView::instance();},menu,screen,reset,[&](const String& origin){views.requestDiscard(Game::instance(),origin);},views,controls});return *pause;},views,controls};
        gameContext=&context;
        GraphicsRenderState render(device);SpriteBuffers buffers(device);buffers.resize(12000);SpriteBatches batches(buffers);
        unsigned frames=0,submitted=0,triangles=0;
        const auto frame=[&] {
            probeMilliseconds+=10;timer.process();views.deleteDiscarded();views.dispatchSteps();views.updateTransitions([&]{return timer.elapsed();});
            batches.begin();controls.forEachVisible([&](Control& control){control.submitSprites(batches,false,presentation.imagePlacement());++submitted;});batches.finish();
            render.setup(graphics,presentation);device->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,graphics.background,1,0);device->BeginScene();
            triangles+=drawSpriteBatches(device,buffers,batches.batches(),render.transposedViewProjection(),false);device->EndScene();device->Present(nullptr,nullptr,nullptr,nullptr);++frames;
        };
        unsigned clicks=0;
        const auto settle=[&] {for(unsigned i=0;!views.transitions().empty() && i<500;++i) frame();require(views.transitions().empty(),"source UI transition did not settle");};
        const auto click=[&](ButtonPrototype& button) {
            cursor={button.geometry().x,button.geometry().y};
            if(auto* field=dynamic_cast<FieldObject*>(&button)) cursor[1]-=field->heightOffset;
            if(!button.pointInRect(cursor)) {
                const auto center=cursor;float best=1e30f;bool found=false;
                for(unsigned y=0;y<600;y+=2) for(unsigned x=0;x<800;x+=2) {
                    const Point2 point{float(x),float(y)};const float dx=point[0]-center[0],dy=point[1]-center[1],distance=dx*dx+dy*dy;
                    if(distance<best && button.pointInRect(point)) {best=distance;cursor=point;found=true;}
                }
                require(found,"actual widget has no hittable point");
            }
            input.dispatchMouse({1,0,0});frame();input.dispatchControls({0x208});frame();input.dispatchControls({0x200});frame();++clicks;settle();
        };
        std::cerr<<"construct first cGame\n";constructGame();auto& game=*Game::instance();
        require(game.farm().graph.fields.size()==60,"actual Game must own sixty fields");require(GameAwardManager::instance(),"actual award manager missing");
        for(unsigned i=0;i<12;++i) frame();
        game.farm().restoreSeed(2,7,{123,245},0,true);game.hud().notify(WString(L"Whole world save/load probe"),false);game.sales().record(17,5);
        frame();require(game.tutorial().activeStep(20),"actual seed did not start tutorial20");
        click(*DigBuilding::instance());for(unsigned i=0;i<1000 && !game.tutorial().isProcessed(20);++i) frame();
        require(game.person().heldType()==2 && game.tutorial().isProcessed(20),"actual tool pickup did not complete tutorial20");
        if(game.tutorial().activeStep(21)) {
            FarmField* plot=nullptr;for(auto* field:game.farm().graph.fields) if(field->active) {plot=field;break;}require(plot,"actual active field missing");
            click(*plot);for(unsigned i=0;i<1000 && !game.tutorial().isProcessed(21);++i) frame();require(game.tutorial().isProcessed(21),"actual digging did not complete tutorial21");
        }
        click(game.hud().journalButton());require(Journal::instance()->state().shown,"actual HUD input did not open Journal");
        click(Journal::instance()->okButton());require(!Journal::instance()->state().shown && game.hud().messages().back().read,"actual Journal input did not return/read messages");
        click(game.hud().mainButton());require(game.hud().pauseMenu().state().shown,"actual HUD input did not open pause");
        click(game.hud().pauseMenu().helpButton());require(HelpView::instance()->state().shown,"actual pause input did not open Help");
        click(HelpView::instance()->okButton());require(game.hud().pauseMenu().state().shown,"actual Help input did not return to pause");
        click(game.hud().pauseMenu().continueButton());require(game.state().enabled && game.state().absoluteEnabled,"actual pause input did not resume whole Game");
        game.save();const auto path=GamePersistence(files,*profiles.current(),game.farm(),game.hud(),game.gameplay(),game.sales()).filename();
        const auto saved=bytes(files,path);require(!saved.empty(),"Game failed to save");
        std::cerr<<"destroy first cGame\n";delete Game::instance();views.deleteDiscarded();
        std::cerr<<"construct restored cGame\n";constructGame();auto& restored=*Game::instance();
        std::cerr<<"restored cGame constructed\n";
        const auto seeds=restored.farm().seedPackages();require(seeds.size()==1,"Game did not restore actual seed");auto& seed=static_cast<SeedPackage&>(*seeds[0]);
        require(seed.kind()==2 && seed.count()==7 && seed.status()==0 && seed.unpaid,"restored seed fields differ");
        require(seed.geometry().x==124 && seed.geometry().y==245,"restored seed coordinates differ from native relative-input behavior");
        require(!restored.hud().messages().empty() && restored.hud().messages().back().text.view()==L"Whole world save/load probe","restored message differs");
        std::cerr<<"restored cGame checked\n";for(unsigned i=0;i<12;++i) frame();restored.save();
        //0040fbb0 subtracts parent X=-1;00412850 passes that relative point
        //unchanged to0045e130/004192a0. Preserve the resulting +1 native quirk.
        auto expectedSaved=saved;const float nextRelativeX=125;std::memcpy(expectedSaved.data()+4,&nextRelativeX,4);
        require(expectedSaved==bytes(files,path),"source save/restore/save differs beyond native seed x drift");std::cerr<<"restored frames complete\n";
        std::cout<<"{\"source_game_lifetimes\":2,\"device_flags\":"<<deviceFlags<<",\"input_clicks\":"<<clicks<<",\"frames\":"<<frames<<",\"submitted_controls\":"<<submitted<<",\"triangles\":"<<triangles<<",\"save_bytes\":"<<saved.size()<<",\"native_comparison\":false}\n";
        delete Game::instance();views.deleteDiscarded();profiles.close();music.clear();pause.reset();gameContext=nullptr;std::cerr<<"second world destroyed\n";
        // The native Tutorial repeated tip remains an independent registry root.
        // Do not treat this source-only probe as a complete teardown comparison.
        }
        std::cerr<<"release graphics\n";resources.clearUseless();textureEnvironment()={};shaderEnvironment()={};fontFiles()=nullptr;device->Release();d3d->Release();DestroyWindow(window);
        return 0;
    }catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}catch(const char* error) {std::cerr<<error<<'\n';return 1;}
}
