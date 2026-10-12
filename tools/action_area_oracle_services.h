// Explicit observations of the remaining Upgrade/Person boundaries. Farm
// registration still executes the real native vector append method.
namespace {
std::array<int,4> actionAreaLevels{};
std::map<void*,unsigned> actionAreaNativeLevels;
std::vector<unsigned> actionAreaSourceQueries,actionAreaNativeQueries;
using AreaTask=std::array<unsigned,3>; // float target words, index at callback
std::vector<AreaTask> actionAreaSourceTasks,actionAreaNativeTasks;
AreaTask areaTask(const Point2& p,unsigned index) {AreaTask result{};std::memcpy(result.data(),p.data(),8);result[2]=index;return result;}
int __fastcall actionAreaLevel(void* self,void*) {const auto id=actionAreaNativeLevels.at(self);actionAreaNativeQueries.push_back(id);return actionAreaLevels[id];}
void __fastcall actionAreaQueue(void*,void*,void* area,const Point2* point) {actionAreaNativeTasks.push_back(areaTask(*point,field<unsigned>(area,0x4fc)));}
struct AreaWorldFixture final:ActionAreaWorld {
    TutorialManager& tips;std::vector<ActionArea*> registered;
    explicit AreaWorldFixture(TutorialManager& t):tips(t) {}
    TutorialManager& tutorial() override {return tips;}
    void registerArea(ActionArea& area) override {registered.push_back(&area);}
    void queueAreaAction(ActionArea& area,const Point2& point) override {actionAreaSourceTasks.push_back(areaTask(point,area.clickIndex));}
    int warehouseLevel() override {actionAreaSourceQueries.push_back(0);return actionAreaLevels[0];}
    int digBuildingLevel() override {actionAreaSourceQueries.push_back(1);return actionAreaLevels[1];}
    int mowBuildingLevel() override {actionAreaSourceQueries.push_back(2);return actionAreaLevels[2];}
    int waterSourceLevel() override {actionAreaSourceQueries.push_back(3);return actionAreaLevels[3];}
};
}
