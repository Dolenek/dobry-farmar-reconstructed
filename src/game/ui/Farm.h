#pragma once
#include "StateView.h"
#include "FarmAreas.h"
#include "FarmField.h"
#include "SeedPackage.h"
#include "Sprinkle.h"
#include "Person.h"
#include "ContractLetterSlot.h"
#include "DigBuilding.h"
#include "MowBuilding.h"
#include "WaterSource.h"
namespace farmar::recovered {
class cSound;class Farm;
// These are actual cGame operations, with no substitute implementation.
struct FarmGameServices {
    virtual ~FarmGameServices()=default;
    virtual int money()=0;virtual void setMoney(int)=0;
    virtual GameplayEvent& savedContract(int)=0; //0042cde0, Gameplay indexed list
    virtual void playUpgradeSound()=0; //00404170
};
struct FarmContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;
    InputBindings& input;ButtonInteraction& interaction;ProfileProperties& profile;
    GameConfiguration& configuration;GameRandom& random;SoundManager& sound;
    TutorialManager& tips;FarmGameServices& game;ViewRegistry& views;ControlRegistry& controls;
};
class FarmLifetime {protected:explicit FarmLifetime(Farm*);~FarmLifetime();void releaseSingleton();};
// Actual Farm scene and concrete bindings of the recovered actor/field/tool
// interfaces. UI children are explicitly deleted while these bindings live.
class Farm final:public StateView,private FarmLifetime,public FarmFieldWorld,
    public SeedPackageWorld,public SprinkleWorld,public PersonWorld,public ContractLetterWorld,
    public DigBuildingWorld,public MowBuildingWorld,public WaterSourceWorld,public ContractOfferWorld {
public:
    Farm(View*,FarmContext&); //00435c40
    ~Farm() override; //00434b80
    static Farm* instance() noexcept;
    void loadXML(const XmlNode&) override; //004305c0
    void absoluteStep() override; //004331b0
    void onShown() override;void onHidden() override; //00434cc0/004350d0
    void onDisabled() override; //0042c830
    TutorialManager& tutorial() override;StateView& farmScene() override {return *this;}
    int money() override;void setMoney(int) override;
    int money() const override;int activeContracts() const override;
    void consumeLetter(ContractOfferLetter&) override;void activateContract(GameplayEvent&) override;
    bool tutorialCanShow(unsigned) const override;void showTutorial(unsigned,View&) override;
    FarmAreas& areas() override {return graph;}
    void registerArea(ActionArea&) override;
    void queueAreaAction(ActionArea&,const Point2&) override;
    int warehouseLevel() override;int digBuildingLevel() override;int mowBuildingLevel() override;int waterSourceLevel() override;
    bool hasDryField() override;bool farmClickHeld() override {return clickHeld;}
    void setFarmClickHeld(bool v) override {clickHeld=v;}
    bool areaGate(unsigned) override;
    bool cursorOverAreaType(unsigned,const ActionArea*) override;
    ActionArea* firstAreaAt(const Point2&,unsigned) override;
    void indicateField(FarmField&) override;void clearFieldIndication(FarmField&) override;
    std::vector<ActionArea*> seedPackages() override;
    bool seedMoveActive() override {return seedMoving;}void setSeedMoveActive(bool v) override {seedMoving=v;}
    SeedPackage* selectedSeed() override {return selected;}void setSelectedSeed(SeedPackage* p) override {selected=p;}
    bool canPlaceSeed(SeedPackage&,const Point2&) override;ActionArea* topAt(const Point2&,unsigned) override;
    std::vector<ActionArea*> areasAt(const Point2&,unsigned) override;
    void setFarmActionButtonEnabled(bool) override;
    void removeSeed(SeedPackage&) override;FarmField* fieldForSeedTutorial() override;
    Control& digBuilding() override;
    bool sprinkleMoveActive() override {return sprinkleMoving;}void setSprinkleMoveActive(bool v) override {sprinkleMoving=v;}
    ActionArea* areaAt(const Point2&,unsigned) override;bool buildingAt(const Point2&) override;
    bool canPlaceSprinkle(Sprinkle&,const Point2&) override;
    std::vector<ActionArea*> wateredFields(const Point2&,float) override;
    void clearWaterPreview() override;void previewWater(ActionArea&,bool) override;double addWater(ActionArea&,float) override;
    void removeSprinkle(Sprinkle&) override;
    HoseEmitter& hoseEmitter() override {return *hose_;}
    void removeLetter(ContractLetterSlot&) override; //004341b0
    void addLetter(GameplayEvent&); //0042ec20
    bool hasFreeLetter() const {return letterCount!=3;} //0042f030
    int heldType() override;void setHeldType(int) override;
    bool personPresent() override;unsigned heldProductCount() override;
    void hidePersonHeldIndicator() override;void updatePersonHeldIndicator(bool,unsigned,unsigned) override;
    Point2 characterPosition() override;void updatePersonAppearance() override;
    void refreshHeldAppearance() override;void cancelPersonAction(const String&) override;
    void animateHeldDigUpgrade(const Point2&,const Point2&,int,bool) override;
    void animateHeldUpgrade(const Point2&,const Point2&,int,bool) override;
    unsigned sprinkleCount() override {return sprinklers;}
    Sprinkle* createSprinkle(unsigned) override;
    SeedPackage* createSeed(unsigned);
    SeedPackage* restoreSeed(unsigned kind,unsigned count,const Point2&,unsigned status,bool unpaid); //00412afe
    void configureHose(const Point2&,const Point2&,float); //0042f5a0
    void stopHose(); //0042ca30
    void refreshSeedButton(); //00434b30
    void refreshUpgradeButton(); //004345d0
    void setAvailableUpgrade(int); //004354d0
    void setCurrentUpgrade(int); //00435660
    unsigned upgradeCost(unsigned index) const {return upgradePrices.at(index);} //0042fa40
    void buyFields(); //004357f0
    void destroyFields(int,float); //0042f080, original biased selection and distinct random factors
    FarmAreas graph;
    bool clickHeld=false,inputHeld=false,seedMoving=false,sprinkleMoving=false;
    SeedPackage* selected=nullptr;unsigned sprinklers=0,letterCount=0;
    std::array<unsigned,4> upgradePrices{};std::array<Point2,4> buyPlaces{};
    FarmField* indicated=nullptr;
    ExtendedAnimationButton& seedsButton() {return *seeds_;}
    ExtendedTextAnimationButton& fieldsButton() {return *fields_;}
    TextMoney& fieldsPrice() {return *price_;}
    Picture& lighter() {return *lighter_;}Picture& postBox() {return *postBox_;}
    TextLine& seedText() {return *seedText_;}TextLine& mailText() {return *mailText_;}
    ContractLetterSlot& letter(unsigned i) {return *letters_.at(i);}
private:
    FarmContext& context_;
    FarmFieldContext fieldContext_;SeedPackageContext seedContext_;SprinkleContext sprinkleContext_;ContractLetterContext letterContext_;
    ExtendedAnimationButton* seeds_;ExtendedTextAnimationButton* fields_;TextMoney* price_;
    Picture *lighter_,*postBox_;TextLine *seedText_,*mailText_;
    std::array<ContractLetterSlot*,3> letters_{};HoseEmitter* hose_;cSound* letterSound_=nullptr;
    static void openSeeds(ButtonPrototype*);static void purchasedFields(ButtonPrototype*);
    static void rightClick(void*);static void leftPress(void*);static void leftRelease(void*);
    void unbindInput();
};
}
