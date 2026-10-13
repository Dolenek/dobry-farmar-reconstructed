namespace {
int digHeldType=0;
using DigEffects=std::vector<std::array<unsigned,7>>;
DigEffects digSourceEffects,digNativeEffects;
int __fastcall digNativeHeld(void*,void*) {return digHeldType;}
void __fastcall digNativeAnimation(void*,void*,const Point2* from,const Point2* to,int level,bool flag) {digNativeEffects.push_back({0,floatBits((*from)[0]),floatBits((*from)[1]),floatBits((*to)[0]),floatBits((*to)[1]),unsigned(level),unsigned(flag)});}
void __fastcall digNativeRefreshHeld(void*,void*) {digNativeEffects.push_back({1,0,0,0,0,0,0});}
}
