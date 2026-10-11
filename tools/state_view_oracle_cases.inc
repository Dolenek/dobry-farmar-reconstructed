stage=300;
ViewRegistry views;ControlRegistry controls;
View parent(nullptr,views);auto* nativeParent=allocate(44);function<void*(__thiscall*)(void*,void*)>(0xd55b0)(nativeParent,nullptr);
parent.show();parent.enable();parent.absoluteEnable();call(0xd37d0,nativeParent);call(0xd3bb0,nativeParent);call(0xd3fb0,nativeParent);
std::array<std::unique_ptr<StateView>,3> scenes;std::array<void*,3> originals{};
const char* definitions[]={"UI/gameUI/Farm.xml","UI/gameUI/Manufacture.xml","UI/gameUI/WareDetail.xml"};
const auto check=[&](unsigned index) {
    auto& scene=*scenes[index];auto* native=originals[index];mainMenuCompareView(scene,native);
    require(scene.movement==field<unsigned>(native,0x4c),"StateView movement differs");
    for(unsigned n=0;n<2;++n) {compareFloat(scene.from[n],field<float>(native,0x3c+n*4),"StateView from");compareFloat(scene.to[n],field<float>(native,0x44+n*4),"StateView to");compareFloat(scene.velocity[n],field<float>(native,0x50+n*4),"StateView velocity");}
    compareFloat(scene.moveSpeed,field<float>(native,0x58),"StateView speed");compareFloat(scene.remaining,field<float>(native,0x5c),"StateView remaining");
    auto* picture=field<void*>(native,0x38);auto* origin=field<void*>(native,0x60);compareControl(scene.movingPicture(),picture);compareControl(scene.origin(),origin);
    require(scene.movingPicture().parent()==&scene && scene.origin().parent()==&scene.movingPicture() && field<void*>(picture,0x20)==native && field<void*>(origin,0x20)==picture,"StateView owned parent differs");
    compareDecorations(scene,native);for(auto* decoration:scene.decorations()) require(decoration->parent()==&scene.movingPicture(),"StateView source decoration parent differs");
    std::vector<void*> nativeDecorations;collectControls(field<Node*>(native,0x30)->parent,nativeDecorations);for(auto* decoration:nativeDecorations) require(field<void*>(decoration,0x20)==picture,"StateView native decoration parent differs");++stateViewStates;
};
for(unsigned i=0;i<3;++i) {
    scenes[i]=std::make_unique<StateView>(&parent,manager,settings,strings,views,controls);originals[i]=allocate(100);std::memset(originals[i],0,100);
    function<void*(__thiscall*)(void*,void*)>(0xc48e0)(originals[i],nativeParent);check(i);
    XmlDocument definition(files,String(definitions[i]));scenes[i]->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0xc4a80)(originals[i],wrap(&definition));++stateViewReloads;check(i);
}
const auto move=[&](unsigned i,unsigned kind,Point2 from,Point2 to) {stage=301;scenes[i]->move(kind,from,to);function<void(__thiscall*)(void*,unsigned,const Point2*,const Point2*)>(0xc5630)(originals[i],kind,&from,&to);++stateViewMoves;check(i);};
const auto step=[&](unsigned i,float dt) {stage=302;elapsed=dt;scenes[i]->absoluteStep();call(0xc56f0,originals[i]);++stateViewSteps;check(i);};
std::mt19937 random(0x4c5630);std::uniform_real_distribution<float> coordinate(-10000,10000),speed(1,5000),time(0,3);
for(trial=0;trial<1200;++trial) {
    if(trial%2) {parent.disable();call(0xd3750,nativeParent);} else {parent.enable();call(0xd3bb0,nativeParent);}
    if(trial%3) {parent.hide();call(0xd3720,nativeParent);} else {parent.show();call(0xd37d0,nativeParent);}
    const unsigned i=trial%3;scenes[i]->moveSpeed=field<float>(originals[i],0x58)=speed(random);
    move(i,trial%5,{coordinate(random),coordinate(random)},{coordinate(random),coordinate(random)});
    for(action=0;action<5;++action) step(i,time(random));
    step(i,100000);scenes[i]->showOrigin();call(0xc4eb0,originals[i]);check(i);scenes[i]->hideOrigin();call(0xc5270,originals[i]);check(i);
}
// Zero-length/zero-speed motions preserve the original NaN comparison path.
for(unsigned i=0;i<3;++i) for(unsigned kind:{0U,1U,2U,7U}) {
    for(float value:{0.f,1400.f,-1400.f}) {scenes[i]->moveSpeed=field<float>(originals[i],0x58)=value;move(i,kind,{0,0},{0,0});step(i,0);step(i,1);}
    scenes[i]->moveSpeed=field<float>(originals[i],0x58)=2400;move(i,kind,{-1,0},{799,0});step(i,-0.01f);step(i,1);
}
// Reload reparents newly created real decorations into the owned moving picture.
for(unsigned i=0;i<3;++i) {XmlDocument definition(files,String(definitions[i]));scenes[i]->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0xc4a80)(originals[i],wrap(&definition));++stateViewReloads;check(i);}
parent.show();parent.enable();call(0xd37d0,nativeParent);call(0xd3bb0,nativeParent);
// Actual recovered timer/cursor/frame dispatch and original D3D8 rendering.
menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
const auto frame=[&] {stage=303;menuMilliseconds+=50;menuPoint={0,0};menuInputEvents.clear();menuMouseDelta={};stateViewPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++stateViewFrames;for(unsigned i=0;i<3;++i) check(i);};
for(unsigned i=0;i<3;++i) {scenes[i]->moveSpeed=field<float>(originals[i],0x58)=2400;scenes[i]->hide();call(0xd3720,originals[i]);}
for(unsigned i=0;i<3;++i) {move(i,2,{-801,0},{-1,0});for(unsigned n=0;n<9;++n) frame();move(i,1,{-1,0},{799,0});for(unsigned n=0;n<9;++n) frame();}
for(unsigned i=0;i<3;++i) {stage=304;const auto before=textureCounts();scenes[i].reset();const auto middle=textureCounts();call(0x2c7d0,originals[i]);compareReferenceDeltas(before,middle);}
call(0xd58a0,nativeParent);require(controls.visible().empty(),"StateView retained visible controls");
menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
