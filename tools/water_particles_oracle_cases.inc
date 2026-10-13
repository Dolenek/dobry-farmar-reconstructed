stage=650;redirect(0x185a13,reinterpret_cast<void*>(&gameplayGetThreadData));
const auto checkParticle=[&](Picture& particle,const WaterParticleMotion& m,void* n,bool hose) {
    stage=651;mainMenuCompareView(particle,n);compareControl(particle,n);compareFloat(m.velocity[0],field<float>(n,0x238),"WaterParticle velocity x");compareFloat(m.velocity[1],field<float>(n,0x23c),"WaterParticle velocity y");
    compareFloat(m.maximumScale,field<float>(n,0x244),"WaterParticle maximum scale");const unsigned size=hose ? 0x24c:0x248;compareFloat(m.originalSize[0],field<float>(n,size),"WaterParticle original width");compareFloat(m.originalSize[1],field<float>(n,size+4),"WaterParticle original height");
    compareFloat(m.lifetime,field<float>(n,hose ? 0x254:0x250),"WaterParticle lifetime");compareFloat(m.elapsed,field<float>(n,hose ? 0x258:0x254),"WaterParticle elapsed");require(m.finished==bool(field<unsigned char>(n,hose ? 0x248:0x240)),"WaterParticle finished flag differs");++waterParticleStates;
};
for(bool hose:{true,false}) for(unsigned seed:{1U,0xffffffffU,0xfeedbeefU}) {
    stage=652;trial=waterParticleObjects;ViewRegistry views;ControlRegistry controls;GameRandom particleRandom(seed);field<unsigned>(gameplayThreadData.data(),0x14)=seed;
    waterParticleGravity()={0,10};field<float>(image,0x25d288)=0;field<float>(image,0x25d28c)=10;
    std::unique_ptr<HoseParticle> hp;std::unique_ptr<SprinkleParticle> sp;
    if(hose) hp=std::make_unique<HoseParticle>(nullptr,manager,views,controls);else sp=std::make_unique<SprinkleParticle>(nullptr,manager,particleRandom,views,controls);
    Picture& particle=hose ? static_cast<Picture&>(*hp):static_cast<Picture&>(*sp);auto& m=hose ? hp->motion:sp->motion;
    auto* n=allocate(hose ? 604:600);std::memset(n,0,hose ? 604:600);function<void*(__thiscall*)(void*,void*)>(hose ? 0x416c0:0x60a90)(n,nullptr);checkParticle(particle,m,n,hose);
    const String path(hose ? "Effects/HoseEffect.xml":"Effects/SprinkleEffect.xml");XmlDocument xml(files,path);particle.loadXML(xml);function<void(__thiscall*)(void*,void*)>(hose ? 0x418f0:0x60bc0)(n,wrap(&xml));checkParticle(particle,m,n,hose);
    require(particleRandom.state()==field<unsigned>(gameplayThreadData.data(),0x14),"WaterParticle load random state differs");
    for(unsigned i=0;i<2;++i) compareFloat(waterParticleGravity()[i],field<float>(image,0x25d288+i*4),"WaterParticle shared gravity");
    if(hose) {hp->gravity=field<float>(n,0x240)=600;m.lifetime=field<float>(n,0x254)=1;m.velocity={80,-100};field<float>(n,0x238)=80;field<float>(n,0x23c)=-100;}
    {
        std::unique_ptr<HoseParticle> hc;std::unique_ptr<SprinkleParticle> sc;if(hose) hc=std::make_unique<HoseParticle>(*hp);else sc=std::make_unique<SprinkleParticle>(*sp);
        Picture& copied=hose ? static_cast<Picture&>(*hc):static_cast<Picture&>(*sc);auto& cm=hose ? hc->motion:sc->motion;auto* nc=allocate(hose ? 604:600);std::memset(nc,0,hose ? 604:600);function<void*(__thiscall*)(void*,void*)>(hose ? 0x41790:0x60b20)(nc,n);checkParticle(copied,cm,nc,hose);++waterParticleCopies;
        for(unsigned step=0;step<32;++step) {stage=653;action=step;elapsed=std::array<float,8>{0,0.001f,0.017f,0.05f,0.2f,0.4f,0.7f,1.2f}[step%8];copied.absoluteStep();call(hose ? 0x41e20:0x60e30,nc);checkParticle(copied,cm,nc,hose);}
        const auto before=textureCounts();hc.reset();sc.reset();const auto middle=textureCounts();function<void*(__thiscall*)(void*,unsigned)>(0x41770)(nc,0);compareReferenceDeltas(before,middle);
    }
    particle.setPosition({300,250});Point2 pos{300,250};function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(n,&pos);
    menuMilliseconds=0;GameTimer particleTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor particleCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&particleCursor;
    std::array<unsigned char,24> cursorFrame{};field<void*>(cursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=cursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    for(unsigned frame=0;frame<(waterEmittersMode ? 0U:24U);++frame) {stage=654;action=frame;menuMilliseconds+=17;menuPoint={790,590};menuMouseDelta={};menuInputEvents.clear();waterParticlePixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++waterParticleFrames;checkParticle(particle,m,n,hose);}
    menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    const auto before=textureCounts();hp.reset();sp.reset();const auto middle=textureCounts();function<void*(__thiscall*)(void*,unsigned)>(0x41770)(n,0);compareReferenceDeltas(before,middle);++waterParticleObjects;
}

if(waterEmittersMode) {
#include "water_emitters_oracle_cases.inc"
}
