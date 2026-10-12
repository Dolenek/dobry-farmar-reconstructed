// Actual native geometry and collision methods, using reconstructed Controls
// and explicit native-layout mask observations. This is a diagnostic adapter.
{
    stage=430;Control a(nullptr,manager,views,controls),b(nullptr,manager,views,controls);
    auto* na=allocate(556);auto* nb=allocate(556);function<void*(__thiscall*)(void*,void*)>(0xedc80)(na,nullptr);function<void*(__thiscall*)(void*,void*)>(0xedc80)(nb,nullptr);
    struct NativePlacementColumn {unsigned bitCount,allocator;unsigned* words;unsigned count,capacity;};
    struct NativePlacementMask {unsigned allocator=0;NativePlacementColumn *first=nullptr,*last=nullptr,*end=nullptr;};
    static_assert(sizeof(NativePlacementColumn)==20 && sizeof(NativePlacementMask)==16);
    std::vector<ClickMaskColumn> ma(13),mb(11);std::vector<NativePlacementColumn> nma(ma.size()),nmb(mb.size());
    for(auto* mask:{&ma,&mb}) for(auto& column:*mask) {column.bitCount=17;column.words.resize(1);}
    const auto nativeMask=[](std::vector<ClickMaskColumn>& source,std::vector<NativePlacementColumn>& storage) {
        for(unsigned i=0;i<source.size();++i) {auto& column=source[i];storage[i]={column.bitCount,0,column.words.data(),unsigned(column.words.size()),unsigned(column.words.size())};}
        return NativePlacementMask{0,storage.data(),storage.data()+storage.size(),storage.data()+storage.size()};
    };
    const auto nmA=nativeMask(ma,nma),nmB=nativeMask(mb,nmb);std::mt19937 random(0x4f0510);
    for(unsigned index=0;index<4096;++index) {
        stage=431;action=index;
        const Point2 sa{float(random()%79+1),float(random()%79+1)},sb{float(random()%79+1),float(random()%79+1)};
        const Point2 pa{float(int(random()%1600)-800)*0.0625f,float(int(random()%1600)-800)*0.0625f},pb{float(int(random()%1600)-800)*0.0625f,float(int(random()%1600)-800)*0.0625f};
        a.setSize(sa);b.setSize(sb);a.setPosition(pa);b.setPosition(pb);
        function<void(__thiscall*)(void*,const Point2*)>(0x13b80)(na,&sa);function<void(__thiscall*)(void*,const Point2*)>(0x13b80)(nb,&sb);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(na,&pa);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(nb,&pb);
        const ProductionRectangle rectangle{-30.f,-27.f,float(random()%75),float(random()%70)};const Point2 point{float(int(random()%128)-64),float(int(random()%128)-64)};
        require(productionPointInside(rectangle,point)==bool(function<unsigned char(__cdecl*)(const ProductionRectangle*,float,float)>(0x178c0)(&rectangle,point[0],point[1])),"Production point rectangle differs");++productionPlacementStates;
        require(productionRectangleContains(rectangle,&a)==bool(function<unsigned char(__cdecl*)(const ProductionRectangle*,void*)>(0x17d10)(&rectangle,na)),"Production rectangle containment differs");++productionPlacementStates;
        require(productionRectangleTouches(rectangle,&a)==bool(function<unsigned char(__cdecl*)(const ProductionRectangle*,void*)>(0x17b90)(&rectangle,na)),"Production rectangle corner overlap differs");++productionPlacementStates;
        for(auto* mask:{&ma,&mb}) for(auto& column:*mask) column.words[0]=index%4==0 ? 0:index%4==1 ? 0x1ffffU:random()&0x1ffffU;
        require(productionMasksOverlap(a,b,&ma,&mb)==bool(function<unsigned char(__cdecl*)(void*,void*,const NativePlacementMask*,const NativePlacementMask*)>(0xf0510)(na,nb,&nmA,&nmB)),"Production mask overlap differs");++productionPlacementStates;
        require(productionMasksOverlap(b,a,&mb,&ma)==bool(function<unsigned char(__cdecl*)(void*,void*,const NativePlacementMask*,const NativePlacementMask*)>(0xf0510)(nb,na,&nmB,&nmA)),"Production reversed mask overlap differs");++productionPlacementStates;
        require(productionMasksOverlap(a,b,nullptr,nullptr)==bool(function<unsigned char(__cdecl*)(void*,void*,void*,void*)>(0xf0510)(na,nb,nullptr,nullptr)),"Production null-mask shortcut differs");++productionPlacementStates;
    }
    const ProductionRectangle nullRectangle{0,0,1,1};require(!productionRectangleContains(nullRectangle,nullptr) && !function<unsigned char(__cdecl*)(const ProductionRectangle*,void*)>(0x17d10)(&nullRectangle,nullptr),"Production null control containment differs");
    call(0xf1560,na);call(0xf1560,nb);
}
