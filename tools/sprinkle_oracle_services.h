// Diagnostic observations of the remaining actual Farm / cField dependencies.
// Sprinkle, ActionArea, effect, particle, WaterSource and rendering bodies execute.
namespace {
using SprinkleEvent=std::array<unsigned,5>;
std::vector<SprinkleEvent> sprinkleSourceEvents,sprinkleNativeEvents;
std::vector<void*> sprinkleNativeFields;
std::map<void*,unsigned> sprinkleNativeIds;
void* sprinkleNativeSelected=nullptr;
bool sprinklePlace=true,sprinkleBuilding=false;
void sprinkleVector(void* out,const std::vector<void*>& values) {
    if(field<void*>(out,4)) release(field<void*>(out,4));
    auto* items=values.empty() ? nullptr:static_cast<void**>(allocate(unsigned(values.size())*4));
    if(items) std::copy(values.begin(),values.end(),items);
    field<void*>(out,4)=items;field<void*>(out,8)=items ? items+values.size():nullptr;field<void*>(out,12)=items ? items+values.size():nullptr;
}
void __fastcall sprinkleCollect(void*,void*,const Point2* point,float radius,void* out) {
    sprinkleNativeEvents.push_back({1,wordOf((*point)[0]),wordOf((*point)[1]),wordOf(radius),0});sprinkleVector(out,sprinkleNativeFields);
}
void __fastcall sprinklePreviewClear(void*,void*) {sprinkleNativeEvents.push_back({2,0,0,0,0});}
template<bool Shown> void __fastcall sprinkleOverlay(void*,void*) {sprinkleNativeEvents.push_back({3,Shown,0,0,0});}
void* __fastcall sprinkleAreaAt(void*,void*,const Point2* point,unsigned mask) {sprinkleNativeEvents.push_back({4,wordOf((*point)[0]),wordOf((*point)[1]),mask,0});return sprinkleNativeSelected;}
void* __fastcall sprinkleBuildingAt(void*,void*,const Point2*,unsigned) {return sprinkleBuilding ? reinterpret_cast<void*>(1):nullptr;}
bool __fastcall sprinklePlaceAt(void*,void*,void*,const Point2* point) {sprinkleNativeEvents.push_back({5,wordOf((*point)[0]),wordOf((*point)[1]),0,0});return sprinklePlace;}
void __fastcall sprinkleRemove(void*,void*,void*) {sprinkleNativeEvents.push_back({6,0,0,0,0});}
void __fastcall sprinklePreview(void* self,void*,bool value) {sprinkleNativeEvents.push_back({7,sprinkleNativeIds.at(self),value,0,0});}
float __fastcall sprinkleWater(void* self,void*,float amount) {sprinkleNativeEvents.push_back({8,sprinkleNativeIds.at(self),wordOf(amount),0,0});return 0;}
bool __fastcall sprinkleAreasAt(void*,void*,const Point2*,unsigned,void* out) {sprinkleVector(out,sprinkleNativeFields);return !sprinkleNativeFields.empty();}
}
