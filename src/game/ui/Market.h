#pragma once
#include "StateView.h"
#include "ButtonText.h"
#include "AnimableAppearance.h"
#include <array>
namespace farmar::recovered {
class ProfileProperties;class GameConfiguration;class Gameplay;class GameRandom;class SoundManager;class TutorialManager;class TextMoney;class Histogram;class cSound;
struct MarketWorld {
    virtual ~MarketWorld()=default;
    virtual std::int32_t money()=0; // 00410ce0
    virtual void setMoney(std::int32_t)=0; // 00410d60
    virtual void productExpense(unsigned)=0; // 0040ef00, real FlyProductExpence effect
    virtual void recordSale(std::uint32_t amount,float time)=0; // 00410f10, real cGame sale history
};
struct MarketContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;InputBindings& input;ButtonInteraction& interaction;
    ProfileProperties& profile;GameConfiguration& configuration;Gameplay& gameplay;GameRandom& random;SoundManager& sound;
    TutorialManager& tutorial;MarketWorld& world;ViewRegistry& views;ControlRegistry& controls;
};
class MarketNumberButton final:public TextAnimationButton {
public:
    using TextAnimationButton::TextAnimationButton;
    unsigned product=0; // original +280, assigned by actual Market constructor/refresh
};
struct MarketRow {
    Picture *background=nullptr,*icon=nullptr;TextLine* warehouseQuantity=nullptr;
    MarketNumberButton *addOne=nullptr,*addTen=nullptr,*addAll=nullptr;
    TextLine* sendingQuantity=nullptr;MarketNumberButton* increasePrice=nullptr;TextMoney* price=nullptr;
    MarketNumberButton* decreasePrice=nullptr;Picture* attitude=nullptr;Histogram* histogram=nullptr;
};
class Market;
class MarketLifetime {protected:explicit MarketLifetime(Market*);~MarketLifetime();};
// Actual cMarket, native 1320 bytes. Remaining cGame services are mandatory.
class Market final:public StateView,private MarketLifetime {
public:
    Market(View*,MarketContext&); // 00471d40
    ~Market() override; // 0046b630
    void loadXML(const XmlNode&) override; // 0046bec0
    void onShown() override; // 00471d30
    void enabledStep() override; // 0046e760
    void absoluteStep() override; // 004721d0
    void enterState() override; // 0046eaa0
    unsigned goods(unsigned); // 0046eb70
    unsigned price(unsigned); // 0046ef70
    void setGoods(unsigned,unsigned); // 004707e0
    void setPrice(unsigned,unsigned); // 00470c00, no index gate in native setter
    bool available(unsigned); // 0046f370
    unsigned priceAttitude(unsigned,unsigned); // 0046b850
    void refresh(); // 0046f9e0
    void refreshForecasts(); // 00470270
    static Market* instance() noexcept;
    const std::array<MarketRow,19>& rows() const noexcept {return rows_;}
    std::array<Point2,19> places{};std::array<AnimableAppearance,3> attitudes;
    int iconType;float priceCheckPeriod,priceUpdatePeriod,remainingPriceUpdate;
    float priceClickElapsed=0,changePricePeriod=.3f;unsigned priceIncrement=1;
private:
    MarketContext& context_;std::array<MarketRow,19> rows_{};cSound* saleSound_=nullptr;
    void updateForecast(MarketRow&,unsigned);
    void transfer(unsigned,unsigned);void changePrice(unsigned,bool);
    static void addOne(ButtonPrototype*);static void addTen(ButtonPrototype*);static void addAll(ButtonPrototype*);
    static void increasePrice(ButtonPrototype*);static void decreasePrice(ButtonPrototype*);
};
}
