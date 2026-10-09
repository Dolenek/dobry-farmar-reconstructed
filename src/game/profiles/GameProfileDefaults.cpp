#include "GameProfileDefaults.h"
#include "ProfileProperties.h"
#include <array>
#include <string>
namespace farmar::recovered {
namespace {
std::string key(const char* prefix,unsigned index) { return prefix+std::to_string(index); }
constexpr std::array<const char*,21> awards={"Award00Grow100","Award01Produce100","Award02Grow1000","Award03Produce1000",
    "Award04Make20Task","Award05Make40Task","Award06Earn1000perMinute","Award07UpgradeAllFarmDepots",
    "Award08BuildAllManufactureDepots","Award09UpgradeAllManufactureDepots","Award10Destroy100Depots",
    "Award11GrowAllAtOneTime","Award12NoWaterNeedIn20Minute","Award13Play30Minute","Award14Play3Hour",
    "Award15SellItemPer40","Award16Getting1000Gold","Award17Getting10000Gold","Award19MaxMarketAssortiment",
    "Award20Megaplanning","Award21CollectAllAwards"};
}
const char* awardPropertyName(unsigned index) { return awards[index]; }
void initializeGameProfile(ProfileProperties& v) {
    // The shared 00410770 template targets Int or UInt according to ECX. Preserve
    // the instruction-confirmed map for every field, including uint -1 markers.
    v.ints["CarUp"]=0;v.ints["YachtUp"]=0;v.floats["XPersPos"]=256;v.floats["YPersPos"]=256;
    v.uints["PCarry"]=0;for(unsigned i=0;i<8;++i) v.uints[key("PCC",i)]=0;
    for(unsigned i=0;i<20;++i) {
        v.ints[key("Upgrade",i)]=-1;v.uints[key("ProdType",i)]=0xffffffffU;
        v.floats[key("XProdPos",i)]=v.floats[key("YProdPos",i)]=0;
        v.uints[key("ProdCount",i)]=0;v.floats[key("ProdPct",i)]=0;v.uints[key("ProdStop",i)]=0;
    }
    for(unsigned i=31;i<=34;++i) v.ints[key("Upgrade",i)]=0;
    for(unsigned i=0;i<15;++i) v.uints[key("UpAvail",i)]=i>=11 ? 0:0xffffffffU;
    for(unsigned i=0;i<65;++i) {
        v.floats[key("FFW",i)]=0;v.floats[key("FFF",i)]=100;
        v.uints[key("FFS",i)]=0;v.uints[key("FFC",i)]=0;v.floats[key("FFP",i)]=0;
    }
    v.ints["fcu"]=v.ints["fua"]=0;for(unsigned i=0;i<8;++i) v.uints[key("SeedAvail",i)]=0;
    for(unsigned i=0;i<3;++i) {
        v.floats[key("SPXPos",i)]=v.floats[key("SPYPos",i)]=0;v.uints[key("SPStatus",i)]=0;v.booleans[key("SPExist",i)]=false;
    }
    for(unsigned i=0;i<19;++i) {
        v.uints[key("WGoodCount",i)]=v.uints[key("WGoodPrice",i)]=0;
        v.uints[key("MGoodPrice",i)]=2;v.uints[key("MGoodCount",i)]=0;v.floats[key("MSellTime",i)]=-1;v.ints[key("MEffPrice",i)]=10;
    }
    for(unsigned i=0;i<3;++i) {
        v.ints[key("task",i)]=-1;v.floats[key("taskTime",i)]=0;v.booleans[key("taskPassCancel",i)]=false;
        v.ints[key("letter",i)]=-1;v.floats[key("letTime",i)]=0;v.ints[key("letState",i)]=0;
    }
    for(unsigned i=0;i<21;++i) { v.booleans[awardPropertyName(i)]=false;v.booleans[key("awardShow",i)]=false; }
    v.ints["marketIt"]=0;v.ints["tutJustProc"]=-1;for(unsigned i=0;i<50;++i) v.booleans[key("tip",i)]=false;
    for(const auto* name:{"awQCollect","awQProcess","awQTask","awQDestroy","awAdecvClick"}) v.uints[name]=0;
    for(const auto* name:{"awEarn","awNoDry","awHighSold","awAdecvStart"}) v.booleans[name]=false;
    v.floats["awNoDryTime"]=0;
    for(unsigned i=0;i<11;++i) v.booleans[key("awManDepotBuild",i)]=false;
    for(unsigned i=0;i<15;++i) v.booleans[key("awDepotUpg",i)]=false;
    v.ints["awGrowType"]=v.ints["mTrigger"]=0;v.ints["wlTrigger"]=-1;
    v.booleans["TFastSpeed"]=v.booleans["TProdCompl"]=false;v.ints["Money"]=0;
    v.floats["GameTime"]=v.floats["RunTime"]=0;v.booleans["MainGame"]=true;
}
}
