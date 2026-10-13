#include "Market.h"
#include "Warehouse.h"
#include "Histogram.h"
#include "PriceText.h"
#include "TutorialManager.h"
#include "Gameplay.h"
#include "GameRandom.h"
#include "GameConfiguration.h"
#include "ProductionKinds.h"
#include "ProfileProperties.h"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "Sound.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include "StringConversion.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
Market* currentMarket=nullptr;
std::string key(const char* prefix,unsigned product) {return std::string(prefix)+std::to_string(product);}
int signedWord(unsigned word) {int result;std::memcpy(&result,&word,4);return result;}
int wrapAdd(int left,int right) {return signedWord(unsigned(left)+unsigned(right));}
int wrapSubtract(int left,int right) {return signedWord(unsigned(left)-unsigned(right));}
template<class T> void attribute(const XmlNode& node,const wchar_t* name,T& value) {const auto text=node.attribute(WString(name));if(!text.empty()) {std::wistringstream input(text.c_str());input>>value;}}
void position(const XmlNode& node,Point2& value) {const auto text=node.attribute(WString(L"pos"));if(!text.empty()) {std::wistringstream input(text.c_str());input>>value[0];input.ignore(1);input>>value[1];}}
void child(View* item,const XmlNode& node,const char* name) {XmlNode* entry=nullptr;if(item && node.section(String(name),entry)) item->loadXML(*entry);}
std::array<Control*,12> controls(const MarketRow& r) {return {r.background,r.icon,r.warehouseQuantity,r.addOne,r.addTen,r.addAll,r.sendingQuantity,r.decreasePrice,r.price,r.increasePrice,r.attitude,r.histogram};}
bool shown(const View& v) {return v.state().shown && v.state().parentShown;}
bool enabled(const View& v) {return v.state().enabled && v.state().parentEnabled;}

}
MarketLifetime::MarketLifetime(Market* self) {if(currentMarket) throw "Object already exist.";currentMarket=self;}
MarketLifetime::~MarketLifetime() {currentMarket=nullptr;}
Market* Market::instance() noexcept {return currentMarket;}
Market::Market(View* parent,MarketContext& c):StateView(parent,c.resources,c.settings,c.strings,c.views,c.controls),MarketLifetime(this),context_(c) {
    for(unsigned i=0;i<19;++i) {
        auto& r=rows_[i];auto* owner=&movingPicture();
        const auto picture=[&] {return new Picture(owner,c.resources,c.views,c.controls);};
        const auto text=[&] {return new TextLine(owner,c.resources,c.settings,c.strings,c.views,c.controls);};
        const auto button=[&] {auto* b=new MarketNumberButton(owner,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);b->product=i;return b;};
        r.background=picture();r.icon=picture();r.warehouseQuantity=text();r.addOne=button();r.addTen=button();r.addAll=button();r.sendingQuantity=text();
        r.decreasePrice=button();r.price=new TextMoney(owner,c.resources,c.settings,c.strings,c.views,c.controls);r.increasePrice=button();
        r.attitude=picture();r.histogram=new Histogram(owner,c.resources,c.views,c.controls);
        r.addOne->clickEvent=&addOne;r.addTen->clickEvent=&addTen;r.addAll->clickEvent=&addAll;r.decreasePrice->clickEvent=&decreasePrice;r.increasePrice->clickEvent=&increasePrice;
    }
    loadXML(*c.resources.loadSettingsXML(String("UI/gameUI/Market.xml")));
}
Market::~Market() {if(saleSound_) saleSound_->releaseReference();} // appearances, singleton, then View-owned UI
void Market::loadXML(const XmlNode& node) {
    StateView::loadXML(node);attribute(node,L"IconType",iconType);attribute(node,L"PriceCheckPeriod",priceCheckPeriod);attribute(node,L"PriceUpdatePeriod",priceUpdatePeriod);
    remainingPriceUpdate=priceUpdatePeriod;attribute(node,L"ChangePricePeriod",changePricePeriod);
    const auto sound=toNarrow(node.attribute(WString(L"SellSound")));if(!sound.empty()) saleSound_=context_.resources.getSound(sound); // original reload retains previous resource reference
    XmlNode* list=nullptr;
    if(node.section(String("ProductPlaces"),list)) for(unsigned i=0;i<(std::min)(list->children().size(),std::size_t(19));++i) {XmlNode* entry=nullptr;if(list->section(i,entry)) position(*entry,places[i]);}
    if(node.section(String("Texts"),list)) for(unsigned i=0;i<list->children().size();++i) {XmlNode* entry=nullptr;if(list->section(i,entry)) {auto* text=new TextLine(&movingPicture(),context_.resources,context_.settings,context_.strings,context_.views,context_.controls);text->loadXML(*entry);}}
    XmlNode* line=nullptr;if(!node.section(String("ProductLine"),line)) throw std::runtime_error("Market requires ProductLine XML section");
    for(unsigned i=0;i<19;++i) {
        auto& r=rows_[i];child(r.background,*line,"Back");child(r.icon,*line,"ProductIcon");child(r.warehouseQuantity,*line,"WareQtyLabel");
        child(r.addOne,*line,"IncQtyButton");child(r.addTen,*line,"Inc10QtyButton");child(r.addAll,*line,"IncAllQtyButton");child(r.sendingQuantity,*line,"MarketQtyLabel");
        child(r.decreasePrice,*line,"DecPriceButton");child(r.price,*line,"PriceLabel");child(r.increasePrice,*line,"IncPriceButton");child(r.attitude,*line,"PriceAttitudeIcon");child(r.histogram,*line,"PriceGrowHistogram");
        for(auto* control:controls(r)) control->setPosition({float(originalAdd(places[i][0],control->geometry().x)),float(originalAdd(places[i][1],control->geometry().y))});
    }
    if(node.section(String("AttitudeIconStates"),list)) for(unsigned i=0;i<(std::min)(list->children().size(),std::size_t(3));++i) {XmlNode* entry=nullptr;if(list->section(i,entry)) attitudes[i].loadXML(*entry,context_.resources);}
}
unsigned Market::goods(unsigned product) {if(product>18) throw "_type>=OVERALL_PROD_COUNT";return context_.profile.uints[key("MGoodCount",product)];}
unsigned Market::price(unsigned product) {if(product>18) throw "_type>=OVERALL_PROD_COUNT";return context_.profile.uints[key("MGoodPrice",product)];}
void Market::setGoods(unsigned product,unsigned value) {if(product>18) throw "_type>=OVERALL_PROD_COUNT";context_.profile.uints[key("MGoodCount",product)]=value;refresh();context_.profile.floats[key("MSellTime",product)]=-1;}
void Market::setPrice(unsigned product,unsigned value) {context_.profile.uints[key("MGoodPrice",product)]=value;refresh();context_.profile.floats[key("MSellTime",product)]=-1;}
bool Market::available(unsigned product) {
    if(!product) return true;
    if(product<8 && context_.profile.uints[key("SeedAvail",product)]) return true;
    if(Warehouse::instance()->goods(product) || goods(product)) return true;
    bool result=false;
    for(unsigned type=0;type<11;++type) if(context_.profile.uints[key("UpAvail",type)]!=0xffffffffU) {
        const auto& kind=productionKinds[type];result|=product==kind.outputProduct() || product==kind.inputProduct();
    }
    return result;
}
unsigned Market::priceAttitude(unsigned product,unsigned amount) {
    const auto delta=wrapSubtract(signedWord(amount),context_.gameplay.effectivePrice(product));const auto radius=context_.configuration.products.at(product).goodAttPriceRadius;
    return radius<delta ? 0:unsigned(delta<wrapSubtract(0,radius))+1;
}
void Market::updateForecast(MarketRow& row,unsigned product) {
    const auto count=row.histogram->bars().size();const auto samples=context_.gameplay.priceForecast(product,priceCheckPeriod,unsigned(count+1));
    std::vector<float> values(count);for(unsigned i=0;i<count;++i) values[i]=float(originalMultiply(originalAdd(samples[i+1],samples[i]),.5f));
    row.histogram->applyValues(values);row.histogram->setMaximum(100);
}
void Market::refreshForecasts() {unsigned row=0;for(unsigned product=0;product<19;++product) if(available(product)) updateForecast(rows_[row++],product);}
void Market::refresh() {
    unsigned visible=0;
    for(unsigned product=0;product<19;++product) {
        if(!available(product)) continue;auto& r=rows_[visible];const auto ware=Warehouse::instance()->goods(product),count=goods(product),amount=price(product);
        r.warehouseQuantity->setString(WString(std::wstring(L"x")+std::to_wstring(ware)));r.sendingQuantity->setString(WString(std::to_wstring(count)));r.price->setString(WString(std::to_wstring(amount)));
        r.attitude->copyAppearance(attitudes[priceAttitude(product,amount)]);r.attitude->markTransformDirty();
        for(auto* b:{r.addOne,r.addTen,r.addAll,r.decreasePrice,r.increasePrice}) b->product=product;
        const auto& info=context_.configuration.products.at(product);r.icon->copyAppearance(iconType ? info.iconLarge:info.icon);r.icon->markTransformDirty();
        for(auto* b:{r.addOne,r.addTen,r.addAll}) if(!shown(*b)) b->show();
        if(!ware) {r.addAll->disable();r.addOne->disable();}else {if(!enabled(*r.addAll)) r.addAll->enable();if(!enabled(*r.addOne)) r.addOne->enable();}
        if(ware<10) r.addTen->disable();else if(!enabled(*r.addTen)) r.addTen->enable();
        r.background->show();r.icon->show();r.warehouseQuantity->show();r.sendingQuantity->show();r.price->show();
        if(!shown(*r.decreasePrice)) r.decreasePrice->show();if(!shown(*r.increasePrice)) r.increasePrice->show();r.attitude->show();
        updateForecast(r,product);r.histogram->show();++visible;
    }
    for(;visible<19;++visible) for(auto* control:controls(rows_[visible])) control->hide();
}
void Market::onShown() {refresh();View::onShown();}
void Market::enterState() {const auto value=context_.profile.ints["marketIt"];context_.profile.ints["marketIt"]=wrapAdd(value,1);}
void Market::transfer(unsigned product,unsigned maximum) {const auto count=goods(product),amount=(std::min)(maximum,Warehouse::instance()->goods(product));if(amount) {Warehouse::instance()->take(product,amount,false,false);setGoods(product,count+amount);}}
void Market::changePrice(unsigned product,bool increase) {
    const auto amount=price(product);if(priceClickElapsed<=changePricePeriod) ++priceIncrement;else priceIncrement=1;priceClickElapsed=0;
    if(increase) {const auto next=amount+priceIncrement;setPrice(product,next>99 ? 99:next);}else {const auto next=signedWord(amount-priceIncrement);setPrice(product,next<1 ? 1:unsigned(next));}
}
void Market::addOne(ButtonPrototype* b) {instance()->transfer(static_cast<MarketNumberButton*>(b)->product,1);}
void Market::addTen(ButtonPrototype* b) {instance()->transfer(static_cast<MarketNumberButton*>(b)->product,10);}
void Market::addAll(ButtonPrototype* b) {instance()->transfer(static_cast<MarketNumberButton*>(b)->product,0xffffffffU);}
void Market::increasePrice(ButtonPrototype* b) {instance()->changePrice(static_cast<MarketNumberButton*>(b)->product,true);}
void Market::decreasePrice(ButtonPrototype* b) {instance()->changePrice(static_cast<MarketNumberButton*>(b)->product,false);}
void Market::enabledStep() {
    View::enabledStep();auto& tips=context_.tutorial;const auto iteration=context_.profile.ints["marketIt"];
    if(iteration==1) {
        if(tips.canShow(8)) {if(!movement && shown(*this)) tips.showCommon(8);return;}
        if(tips.canShow(9)) {if(!movement && shown(*this)) tips.showArrow(9,rows_[0].icon);return;}
        if(tips.canShow(45) && !movement && shown(*this)) tips.showAction(45,rows_[0].addTen);
    }else if(iteration==2) {if(tips.canShow(11) && !movement && shown(*this)) tips.showArrow(11,rows_[0].price);}
    else if(iteration==3 && tips.canShow(12) && !movement && shown(*this)) {
        rows_[0].histogram->setMaximum(100);rows_[0].histogram->applyValues({100,50,200,25,50,75,100,150,100,10});rows_[0].histogram->markTransformDirty();tips.showArrow(12,rows_[0].histogram);
    }
}
void Market::absoluteStep() {
    StateView::absoluteStep();if(!uiElapsedTime()) throw std::logic_error("Market timer is not bound");const float elapsed=uiElapsedTime()();
    for(unsigned product=0;product<19;++product) {
        if(!goods(product)) continue;float time=context_.profile.floats[key("MSellTime",product)];double next=time;
        if(time<=0) {
            const auto amount=signedWord(price(product)),effective=context_.gameplay.effectivePrice(product);const auto& info=context_.configuration.products.at(product);const auto radius=info.priceRadius;
            double frequency=0;
            if(amount<wrapAdd(radius,effective)) {
                if(wrapSubtract(effective,radius)<amount) frequency=originalMultiply(originalSubtract(1,originalDivide(wrapAdd(wrapSubtract(radius,effective),amount),wrapAdd(radius,radius))),info.maxSellFrequency);
                else frequency=info.maxSellFrequency;
            }
            next=originalDivide(1,frequency);
        }
        time=float(originalSubtract(next,elapsed));
        if(time<=0) {
            const auto amount=signedWord(price(product)),effective=context_.gameplay.effectivePrice(product);const auto radius=context_.configuration.products.at(product).priceRadius;
            float probability=0;
            if(amount<wrapAdd(radius,effective)) probability=wrapSubtract(effective,radius)<amount ? float(originalDivide(wrapAdd(wrapSubtract(radius,effective),amount),wrapAdd(radius,radius))):1;
            const auto sample=context_.random.next();
            if(originalAdd(originalMultiply(sample,3.0515457183355466e-05f),.0001f)<=probability) {
                setGoods(product,goods(product)-1);const auto sold=price(product);context_.world.setMoney(wrapAdd(context_.world.money(),signedWord(sold)));
                context_.world.productExpense(product);if(signedWord(sold)>79) context_.profile.booleans["awHighSold"]=true;
                if(saleSound_) context_.sound.clickSound(saleSound_);time=-1;const float gameTime=context_.profile.floats["GameTime"];context_.world.recordSale(sold,gameTime);
            }
        }
        context_.profile.floats[key("MSellTime",product)]=time;
    }
    const auto remaining=originalSubtract(remainingPriceUpdate,elapsed);remainingPriceUpdate=float(remaining);
    if(remaining<=0) {remainingPriceUpdate=priceUpdatePeriod;refreshForecasts();}
    priceClickElapsed=float(originalAdd(elapsed,priceClickElapsed));
}
}
