stage=370;const char* unionNames[]={"UI/gameUI/BuySeeds.xml","UI/gameUI/BuyBuilding.xml"};
std::array<std::unique_ptr<MovableUnion>,2> unions;std::array<void*,2> nativeUnions{};
std::array<std::vector<Point2>,2> initialChildPositions;
const auto compareUnion=[&](unsigned index) {
    stage=371;auto& u=*unions[index];auto* n=nativeUnions[index];mainMenuCompareView(u,n);compareControl(u,n);compareControl(u.origin(),field<void*>(n,0x26c));
    require(u.movement==field<unsigned>(n,0x248),"MovableUnion movement differs");compareFloat(u.moveSpeed,field<float>(n,0x254),"MovableUnion speed");compareFloat(u.remaining,field<float>(n,0x258),"MovableUnion remaining");
    for(unsigned d=0;d<2;++d) {compareFloat(u.from[d],field<float>(n,0x238+d*4),"MovableUnion from");compareFloat(u.to[d],field<float>(n,0x240+d*4),"MovableUnion to");compareFloat(u.velocity[d],field<float>(n,0x24c+d*4),"MovableUnion velocity");compareFloat(u.hiddenPlace[d],field<float>(n,0x25c+d*4),"MovableUnion hidden place");compareFloat(u.originalPosition[d],field<float>(n,0x264+d*4),"MovableUnion original position");}
    require(u.backgrounds().size()==field<unsigned>(n,0x278),"MovableUnion background count differs");auto* head=field<Link*>(n,0x274);auto* item=head->next;for(auto* picture:u.backgrounds()) {require(item!=head,"MovableUnion background list ended early");compareControl(*picture,item->control);item=item->next;}require(item==head,"MovableUnion background list ended late");++movableUnionStates;
};
for(unsigned i=0;i<2;++i) {
    unions[i]=std::make_unique<MovableUnion>(nullptr,manager,views,controls);nativeUnions[i]=allocate(636);std::memset(nativeUnions[i],0,636);function<void*(__thiscall*)(void*,void*)>(0x13780)(nativeUnions[i],nullptr);compareUnion(i);
    XmlDocument xml(files,String(unionNames[i]));unions[i]->loadXML(xml);function<void(__thiscall*)(void*,void*)>(0x14d60)(nativeUnions[i],wrap(&xml));compareUnion(i);
    // Repeated XML loads append and preserve owned Picture backgrounds.
    unions[i]->loadXML(xml);function<void(__thiscall*)(void*,void*)>(0x14d60)(nativeUnions[i],wrap(&xml));++movableUnionReloads;compareUnion(i);
    initialChildPositions[i].push_back({unions[i]->origin().geometry().x,unions[i]->origin().geometry().y});for(auto* picture:unions[i]->backgrounds()) initialChildPositions[i].push_back({picture->geometry().x,picture->geometry().y});
}
std::mt19937 unionRandom(0x4147f0);
explicitTipElapsed=true;
for(trial=0;trial<512;++trial) {
    stage=372;const unsigned index=trial%2;auto& u=*unions[index];auto* n=nativeUnions[index];
    u.moveSpeed=trial%5 ? 2400:0;field<float>(n,0x254)=u.moveSpeed;
    const Point2 from{float(int(unionRandom()%1800)-900),float(int(unionRandom()%1600)-800)};
    const Point2 to=trial%17 ? Point2{float(int(unionRandom()%1800)-900),float(int(unionRandom()%1600)-800)}:from;
    const unsigned movement=trial%4;u.move(movement,from,to);function<void(__thiscall*)(void*,unsigned,const Point2*,const Point2*)>(0x147f0)(n,movement,&from,&to);++movableUnionMoves;compareUnion(index);
    for(unsigned step=0;step<8;++step) {elapsed=float(step%3)*0.071f;u.absoluteStep();call(0x148e0,n);++movableUnionSteps;compareUnion(index);}
    u.riseControls();call(0x14070,n);compareUnion(index);u.unriseControls();call(0x14430,n);compareUnion(index);
}
explicitTipElapsed=false;
// The actual common bases replace the HUD's two overlay world objects for
// this scenario. Full BuySeeds/BuyBuilding controller behavior remains pending.
world.overlays[0]=unions[1].get();world.overlays[1]=unions[0].get();putPointer(0x260010,nativeUnions[1]);putPointer(0x25ff64,nativeUnions[0]);
for(unsigned i=0;i<2;++i) {
    auto& u=*unions[i];auto* n=nativeUnions[i];u.moveSpeed=2400;field<float>(n,0x254)=2400;u.movement=field<unsigned>(n,0x248)=0;u.setPosition(u.originalPosition);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(n,&u.originalPosition);
    // The preceding degenerate motion probes can leave NaN child geometry.
    // Explicit diagnostic restoration precedes visible valid-world frames.
    u.origin().setPosition(initialChildPositions[i][0]);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(field<void*>(n,0x26c),&initialChildPositions[i][0]);
    auto* head=field<Link*>(n,0x274);auto* item=head->next;for(unsigned j=0;j<u.backgrounds().size();++j) {u.backgrounds()[j]->setPosition(initialChildPositions[i][j+1]);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(item->control,&initialChildPositions[i][j+1]);item=item->next;require(std::isfinite(u.backgrounds()[j]->geometry().x) && std::isfinite(u.backgrounds()[j]->geometry().y),"MovableUnion visible frame contains invalid geometry");}
    u.hide();call(0xd3720,n);compareUnion(i);
}
for(trial=0;trial<6;++trial) {
    stage=373;const unsigned index=trial%2;auto& u=*unions[index];auto* n=nativeUnions[index];const unsigned state=trial%3 ? 2:1;
    const Point2 start=state==1 ? u.originalPosition:u.hiddenPlace,target=state==1 ? u.hiddenPlace:u.originalPosition;
    u.move(state,start,target);function<void(__thiscall*)(void*,unsigned,const Point2*,const Point2*)>(0x147f0)(n,state,&start,&target);++movableUnionMoves;compareUnion(index);
    for(unsigned step=0;step<12;++step) {frame();compareUnion(0);compareUnion(1);++movableUnionFrames;}
    // Actual scene callback touches the actual overlay enable flags.
    if(trial%2) {hud->switchFarm();call(0xbef70,original);}else {hud->switchMarket();call(0xbf4d0,original);}check();compareUnion(0);compareUnion(1);
}
world.overlays[0]=&overlay;world.overlays[1]=&selection;putPointer(0x260010,nativeOverlay);putPointer(0x25ff64,nativeSelection);
const auto unionBefore=textureCounts();unions[0].reset();unions[1].reset();const auto unionMiddle=textureCounts();for(auto* n:nativeUnions) call(0x13fa0,n);compareReferenceDeltas(unionBefore,unionMiddle);
