#pragma once
#include "MovableUnion.h"
#include "TextLine.h"
#include <array>
namespace farmar::recovered {
class GameConfiguration;class WarehouseTip;
class WarehouseTipLifetime {
protected:explicit WarehouseTipLifetime(WarehouseTip*);~WarehouseTipLifetime();
};
struct WarehouseTipContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;
    GameConfiguration& configuration;ViewRegistry& views;ControlRegistry& controls;
};
// Actual native 12-byte ProductInfo UI record, not a warehouse product model.
// XML row order supplies display slots; refresh compacts nonzero stocks into them.
struct WarehouseTipRow {Picture* icon;TextLine* quantity;unsigned iconType;};
// cWareDetail, native 884 bytes, singleton 0065ffc4. The separate market
// scene at 0065ffc8 is still reconstructed independently.
class WarehouseTip final:public MovableUnion,private WarehouseTipLifetime {
public:
    WarehouseTip(View*,WarehouseTipContext&); // 004658b0
    ~WarehouseTip() override=default; // 00465a10, singleton before MovableUnion teardown
    void loadXML(const XmlNode&) override; // 004645b0
    void absoluteStep() override; // 00465a70
    void refresh(); // 00464b40: actual Warehouse/Profile -> actual quantity/icon controls
    void open(); // 00465050: 1e9 timer, refresh, incoming movement
    void armHide(); // 00465460: copies VisibleTime, does not initiate movement
    void close(); // 004654b0: opened=false, outgoing movement
    std::array<WarehouseTipRow,19> rows; // native +280; pointers/types assigned by real XML
    unsigned productCount; // +364, ctor leaves unspecified until ProductInfo
    float visibleTime=2,remainingVisible; // +368/+36c; remaining unspecified until open/armHide
    bool opened=false; // +370
    static WarehouseTip* instance() noexcept;
private:
    WarehouseTipContext& tipContext_;
    void loadRow(WarehouseTipRow&,const XmlNode&); // 00464410
};
}
