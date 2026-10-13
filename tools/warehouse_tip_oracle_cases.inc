stage=502;detail->open();call(0x65050,nativeDetail);warehouseCheck();
for(unsigned tick=0;tick<16;++tick) {
    elapsed=float(tick%5)*0.0375f;detail->absoluteStep();call(0x65a70,nativeDetail);warehouseCheck();
}
const auto detailXmlDirectory=common/"waredetail-tip-reload";std::filesystem::create_directories(detailXmlDirectory);
std::ofstream(detailXmlDirectory/"missing.xml")<<"<WareDetail VisibleTime=\"0.2\"/>";
for(unsigned count:{0U,3U,21U}) {
    std::ofstream xml(detailXmlDirectory/("rows-"+std::to_string(count)+".xml"));xml<<"<WareDetail position=\"70 250\"><ProductInfo>";
    for(unsigned row=0;row<count;++row) xml<<"<Row IconType=\""<<row%2<<"\"><Icon Visible=\"1\" position=\"30 "<<112+row*7<<"\" size=\"32 32\" deep=\"1092\"/><Text Visible=\"1\" position=\"50 "<<112+row*7<<"\" size=\"122 32\" deep=\"1093\" Font=\"UI/menu_font.fnt\" Texture=\"UI/menu_font.dds\" Multiplier=\"0.5\" String=\"0\" RenderType=\"32\"/></Row>";
    xml<<"</ProductInfo></WareDetail>";
}
files.mountCommon(String(detailXmlDirectory.generic_string()));
for(const char* path:{"missing.xml","rows-3.xml","rows-0.xml","rows-21.xml","UI/gameUI/WareDetail.xml","missing.xml"}) {
    stage=505;XmlDocument definition(files,String(path));detail->loadXML(definition);function<void(__thiscall*)(void*,void*)>(0x645b0)(nativeDetail,wrap(&definition));warehouseCheck();++warehouseTipReloads;
    detail->open();call(0x65050,nativeDetail);warehouseCheck();
    detail->refresh();call(0x64b40,nativeDetail);warehouseCheck();
}
const unsigned detailTimerWords[]={0U,0x80000000U,0x3dcccccdU,0xbdcccccdU,0x7f800000U,0xff800000U,0x7fc00001U};
for(unsigned word:detailTimerWords) for(float tick:{0.f,0.0375f,0.1f}) {
    float timerValue;std::memcpy(&timerValue,&word,4);
    stage=506;detail->movement=field<unsigned>(nativeDetail,0x248)=0;detail->remainingVisible=field<float>(nativeDetail,0x36c)=timerValue;
    elapsed=tick;detail->absoluteStep();call(0x65a70,nativeDetail);warehouseCheck();
}
detail->armHide();call(0x65460,nativeDetail);warehouseCheck();
for(unsigned tick=0;tick<16;++tick) {elapsed=0.0375f;detail->absoluteStep();call(0x65a70,nativeDetail);warehouseCheck();}
detail->open();call(0x65050,nativeDetail);warehouseCheck();
for(unsigned product=0;product<19;++product) {
    warehouse->give(product,1,true,false);function<unsigned(__thiscall*)(void*,unsigned,unsigned,bool,bool)>(0x67ec0)(nw,product,1,true,false);warehouseCheck();
}
detail->close();call(0x654b0,nativeDetail);warehouseCheck();
// The real HUD hover hooks now invoke the real WareDetail controller.
hud->warehouseButton().hovered();call(0xbd8f0,field<void*>(original,0x88));warehouseCheck();
require(detail->opened,"HUD hover did not open actual WarehouseTip");
hud->warehouseButton().unhovered();call(0xbd920,field<void*>(original,0x88));warehouseCheck();
require(detail->opened && detail->remainingVisible==detail->visibleTime,"HUD leave did not arm actual WarehouseTip hide timer");
std::mt19937 detailRandom(0x464b40);
for(unsigned pattern=0;pattern<64;++pattern) {
    stage=504;trial=pattern;unsigned occupied=0;
    for(unsigned product=0;product<19;++product) {
        const auto value=pattern==0 ? 0U:pattern==1 ? 1U:detailRandom()%3 ? detailRandom():0U;
        profile.uints["WGoodCount"+std::to_string(product)]=nativeProfile.uints["WGoodCount"+std::to_string(product)]=value;occupied+=value;
        detail->rows[product].iconType=field<unsigned>(nativeDetail,0x288+product*12)=pattern%3==0 ? 1U:0U;
    }
    warehouse->occupied=field<unsigned>(nw,0x534)=occupied;warehouse->refresh();call(0x66530,static_cast<unsigned char*>(nw)+0x508);
    detail->open();call(0x65050,nativeDetail);warehouseCheck();
    detail->refresh();call(0x64b40,nativeDetail);warehouseCheck();
}
{
    menuMilliseconds=0;GameTimer detailTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});GameCursor detailCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&detailCursor;
    std::array<unsigned char,24> detailCursorFrame{};field<void*>(detailCursorFrame.data(),0)=cursorTable.data();nativeCursorFrame=detailCursorFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    detail->open();call(0x65050,nativeDetail);warehouseCheck();
    for(unsigned frame=0;frame<20;++frame) {
        stage=503;action=frame;menuMilliseconds+=50;menuPoint={790,590};menuMouseDelta={};menuInputEvents={};
        if(frame==9) {detail->armHide();call(0x65460,nativeDetail);}
        warehouseTipPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++warehouseTipFrames;warehouseCheck();
    }
    menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
}
