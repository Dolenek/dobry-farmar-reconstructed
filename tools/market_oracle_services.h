namespace {
using MarketEffects=std::vector<std::array<unsigned,3>>;
MarketEffects marketSourceEffects,marketNativeEffects;
void __fastcall marketExpense(void* self,void*,unsigned product) {require(self==field<void*>(image,0x25f6ec),"Market expense receiver differs");marketNativeEffects.push_back({0,product,0});}
void __fastcall marketSale(void* self,void*,unsigned amount,float time) {require(self==field<void*>(image,0x25f6ec),"Market sale history receiver differs");marketNativeEffects.push_back({1,amount,floatBits(time)});}
}
