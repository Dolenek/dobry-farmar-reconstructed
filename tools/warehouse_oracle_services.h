// Diagnostic remaining market, WareDetail, Farm gate and cGame-money boundaries.
// Warehouse stock/progress, Manufacture labels and HUD execute actual controllers.
namespace {
using WarehouseEvent=std::array<unsigned,5>;
std::vector<WarehouseEvent> warehouseSourceEvents,warehouseNativeEvents;
std::array<bool,2> warehouseCursorGates{};
GameMenu* warehouseSourceHud=nullptr;
unsigned wordOf(float value) {unsigned word;std::memcpy(&word,&value,4);return word;}
WarehouseEvent warehouseSourceSnapshot(unsigned kind,unsigned argument=0) {
    auto& w=*Warehouse::instance();return {kind,argument,w.occupied,wordOf(w.loadingProgress().current),wordOf(warehouseSourceHud->warehouseProgress().current)};
}
WarehouseEvent warehouseNativeSnapshot(unsigned kind,unsigned argument=0) {
    auto* w=field<void*>(image,0x25f748);auto* hud=field<void*>(image,0x25f740);
    return {kind,argument,field<unsigned>(w,0x534),wordOf(field<float>(field<void*>(w,0x6fc),0x23c)),wordOf(field<float>(field<void*>(hud,0x8c),0x23c))};
}
void __fastcall warehouseMarket(void* self,void*) {require(self==field<void*>(image,0x25ffc8),"Warehouse market notification receiver differs");warehouseNativeEvents.push_back(warehouseNativeSnapshot(1));}
void __fastcall warehouseTip(void* self,void*) {require(self==field<void*>(image,0x25ffc4),"Warehouse tooltip notification receiver differs");warehouseNativeEvents.push_back(warehouseNativeSnapshot(2));}
template<unsigned Index> bool __fastcall warehouseGate(void* self,void*,void* excluded) {
    require(self==field<void*>(image,0x25f744) && !excluded,"Warehouse Farm gate arguments differ");
    warehouseNativeEvents.push_back(warehouseNativeSnapshot(3,Index ? 32:64));return warehouseCursorGates[Index];
}
void __cdecl warehouseSetMoney(int value) {
    warehouseNativeEvents.push_back(warehouseNativeSnapshot(4,unsigned(value)));contractNativeProfile->ints["Money"]=(std::max)(value,0);
}
}
