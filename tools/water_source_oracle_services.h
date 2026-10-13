namespace {
std::vector<unsigned> waterSourceEffects,waterNativeEffects;
std::vector<std::string> waterSourceOrigins,waterNativeOrigins;
void __fastcall waterNativePersonAppearance(void*,void*) {waterNativeEffects.push_back(1);}
void __cdecl waterNativeDiscardTask(void*,const String* origin) {waterNativeOrigins.emplace_back(origin->view());}
unsigned waterRequestedCost=0;void* waterCreatedNative=nullptr;
void* __fastcall waterNativeCreateSprinkle(void*,void*,unsigned cost) {waterRequestedCost=cost;waterNativeEffects.push_back(2);return waterCreatedNative;}
}
