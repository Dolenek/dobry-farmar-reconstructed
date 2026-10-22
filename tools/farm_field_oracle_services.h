// Explicit remaining Farm indication/FieldTip observations. Original Field,
// action processor, all four field actions and dry-field query execute.
namespace {
using FieldEvent=std::array<unsigned,3>;
std::vector<FieldEvent> fieldSourceEvents,fieldNativeEvents;
std::map<void*,unsigned> fieldNativeIds;
void __fastcall fieldTipOpen(void*,void*,void* field) {fieldNativeEvents.push_back({1,fieldNativeIds.at(field),0});}
void __fastcall fieldTipHide(void*,void*,void* field) {fieldNativeEvents.push_back({2,fieldNativeIds.at(field),0});}
void __fastcall fieldIndicate(void*,void*,void* field) {fieldNativeEvents.push_back({4,fieldNativeIds.at(field),0});}
void __fastcall fieldClearIndication(void*,void*,void* field) {fieldNativeEvents.push_back({5,fieldNativeIds.at(field),0});}
void* fieldFirstNativeArea=nullptr;
void* __fastcall fieldFirstArea(void*,void*,const Point2* point,unsigned mask) {fieldNativeEvents.push_back({6,wordOf((*point)[0]),wordOf((*point)[1])});require(mask==64,"Field hover mask differs");return fieldFirstNativeArea;}
}
