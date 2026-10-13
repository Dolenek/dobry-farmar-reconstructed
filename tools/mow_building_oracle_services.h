namespace {
using MowEffects=std::vector<std::array<unsigned,3>>;
MowEffects mowSourceEffects,mowNativeEffects;
void __fastcall mowNativeHeldIndicator(void*,void*,bool flag,unsigned count,unsigned capacity) {mowNativeEffects.push_back({unsigned(flag),count,capacity});}
}
