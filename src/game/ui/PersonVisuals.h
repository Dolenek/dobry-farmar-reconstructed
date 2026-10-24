#pragma once
#include "FieldObject.h"
#include "ProgressBar.h"
#include "ProgressDiscrete.h"
#include "AnimableAppearance.h"
#include "ProfileProperties.h"
#include "GameConfiguration.h"
#include <array>
namespace farmar::recovered {
struct PersonVisualWorld {
    virtual ~PersonVisualWorld()=default;
    virtual int digLevel()=0;virtual int mowLevel()=0;
    virtual unsigned carriedSeedKind()=0;virtual unsigned carriedSeedCount()=0;
};
struct PersonVisualContext {
    ResourceManager& resources;ButtonInteraction& interaction;ProfileProperties& profile;
    GameConfiguration& configuration;PersonVisualWorld& world;ViewRegistry& views;ControlRegistry& controls;
};
// Actual Person appearance/state data and owned controls. The Person actor
// composes this with PersonTasks and the walking/work action processor.
class PersonVisuals final {
public:
    PersonVisuals(FieldObject&,PersonVisualContext&);
    ~PersonVisuals();
    void loadXML(const XmlNode&); // 0044b600
    void releaseSounds(); // actor interleaves resource release and channel teardown
    void refreshHeldAppearance(); //00450f90, preserves activity and indicator positions
    void playIdle(); // 00450a30
    void playJob(unsigned); // 00449ea0
    void playWalk(const Point2& delta); // 0044fe20, selects motion appearance
    void playAxisWalk(const Point2& delta); // 004504d0, axis/vehicle appearance
    void playHose(const Point2& target); // 0044a330
    void showDiscrete(unsigned char enabled,unsigned current,unsigned maximum); // 0044ee70
    unsigned carriedProducts(); // 0044ed10, unsigned PCC0..7 sum
    Point2 hotSpot(); // 0044f1a0, world coordinates
    Point2 hotSpotOffset(unsigned); // 0044f250, mirrored local offset
    Point2 rawHotSpot(unsigned); // 0044f2e0, no mirroring
    unsigned direction=2,activity=12,hoseFrame=6;
    bool hoseAnimation=true;float speed;
    std::array<AnimableAppearance,72> moving;
    std::array<AnimableAppearance,66> working;
    AnimableAppearance hose;
    std::array<float,66> jobHeight;
    std::array<AnimableAppearance,72> idle;
    AnimableAppearance markerAppearance;
    Point2 markerSize{};
    std::array<Point2,64> waterEmitterOffsets{};
    std::array<float,64> waterAngles;
    std::array<Point2,6> productTips{},seedTips{};
    std::array<float,10> actionTimes;
    std::array<float,12> carryHeight;
    std::array<Point2,72> hotSpots{};
    std::array<cResource*,8> sounds{};
    ProgressBar& waterUse() noexcept {return *waterUse_;}
    ProgressDiscrete& discreteUse() noexcept {return *discreteUse_;}
    Picture& sign() noexcept {return *sign_;}Picture& substrate() noexcept {return *substrate_;}
private:
    FieldObject& owner_;PersonVisualContext& context_;
    ProgressBar* waterUse_;ProgressDiscrete* discreteUse_;Picture* sign_;Picture* substrate_;
    unsigned carryIndex();void applyHeight(float);void copyIfTextureChanged(const AnimableAppearance&);
};
}
