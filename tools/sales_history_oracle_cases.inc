// The Market scenario above has already inserted real sales into these two
// histories. Exercise the same live list across original clock boundaries.
const auto prune=[&](float time) {stage=542;saleHistory.prune(time);function<void(__thiscall*)(void*,float)>(0xc2b0)(nativeGame,time);++salesHistoryPrunes;saleHistoryCheck();};
const auto record=[&](unsigned amount,float time) {stage=543;saleHistory.record(amount,time);function<void(__thiscall*)(void*,unsigned,float)>(0x10f10)(nativeGame,amount,time);++salesHistoryRecords;saleHistoryCheck();};
for(unsigned i=0;i<16;++i) record(100+i,.25f*float(i));
for(float time:{59.f,60.f,60.25f,61.f,65.f,100.f,100.f,100.f,100.f,100.f}) prune(time);
for(unsigned i=0;i<7;++i) record(1,100.f);
prune(160.f); // strict boundary: time == cutoff remains
prune(std::nextafter(160.f,INFINITY));
prune(1000.f);prune(1000.f);prune(1000.f);
for(unsigned amount:{0U,1U,998U,999U,1000U,0xffffffffU,0x80000000U,0x7fffffffU}) record(amount,1000.f);
for(float time:{1000.f,1060.f,std::nextafter(1060.f,INFINITY),2000.f,2000.f,2000.f}) prune(time);
std::mt19937 historyRandom(0x410f10);
for(unsigned i=0;i<2000;++i) {
    const float time=float(int(historyRandom()%20000)-10000)*.125f;
    if(i%3) record(historyRandom(),time);else prune(time);
}
const unsigned specialTimes[]={0U,0x80000000U,0x7f800000U,0xff800000U,0x7fc00001U,0x7f800001U,0x7fffffffU,0xffc00001U,0x00000001U,0x80000001U,0x7f7fffffU,0xff7fffffU};
for(unsigned word:specialTimes) {float time;std::memcpy(&time,&word,4);record(historyRandom(),time);prune(time);}
// Clear reference storage explicitly without substituting either operation.
auto* head=field<void*>(nativeGame,0x9f8);auto* node=field<void*>(head,0);
while(node!=head) {auto* next=field<void*>(node,0);release(node);node=next;}release(head);
field<void*>(nativeGame,0x9f8)=nullptr;field<unsigned>(nativeGame,0x9fc)=0;
saleHistoryCheck={};
