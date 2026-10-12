#include "ProductionBuilding.h"
#include "GameMenu.h"
#include "TutorialManager.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include "Sound.h"
#include "SoundManager.h"
#include "StringConversion.h"
#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <sstream>
#include <string>
namespace farmar::recovered {
namespace {
std::string property(const char* name,unsigned id) {return std::string(name)+std::to_string(id);}
bool shown(const View& view) {return view.state().shown && view.state().parentShown;}
void child(View& view,const XmlNode& node,const char* key) {XmlNode* section=nullptr;if(node.section(String(key),section)) view.loadXML(*section);}
template<class T> void attribute(const XmlNode& node,const wchar_t* key,T& value) {
    const auto text=node.attribute(WString(key));if(!text.empty()) {std::wistringstream stream(text.c_str());stream>>value;}
}
void colorAttribute(const XmlNode& node,const wchar_t* key,Color& value) {
    const auto text=node.attribute(WString(key));if(!text.empty()) {unsigned packed=0;std::wistringstream stream(text.c_str());stream>>std::hex>>packed;value=Color::fromARGB(packed);}
}
WString quantity(unsigned value) {return WString(std::wstring(L"x")+std::to_wstring(value));} // 0041db60, prefix 005f437c
unsigned truncatedWord(double value) {
    // 00580f98: signed 64-bit truncation; only low EAX is used. An
    // indefinite FISTP result is 80000000:00000000, hence low word zero.
    if(!std::isfinite(value) || value>=9223372036854775808.0 || value< -9223372036854775808.0) return 0;
    return static_cast<unsigned>(static_cast<std::int64_t>(value));
}
}
class ProductionSubstrateButton final:public Button {
public:
    ProductionSubstrateButton(ProductionBuilding& owner,ProductionBuildingContext& c):Button(&owner,c.resources,c.input,c.interaction,c.views,c.controls),owner_(owner) {}
    // Original vtable +150/+154 are genuine RET 004bd820, not missing methods.
    void clicked() override {} void unclicked() override {}
    void hovered() override {auto& w=owner_.context_.world;if(!w.movingBuilding()) {ButtonPrototype::hovered();if(!w.movingBuilding()) owner_.showControls();}}
    void unhovered() override {
        auto& w=owner_.context_.world;if(w.movingBuilding() || owner_.loaded_->dragging()) return;
        ButtonPrototype::unhovered();if(owner_.buttonState()!=ButtonState::Hovered) {if(&owner_!=w.focusedBuilding()) owner_.hideControls();owner_.onStateChanged(ButtonState::Normal);}
    }
private:ProductionBuilding& owner_;
};
ProductionBuilding::ProductionBuilding(unsigned type,unsigned slot,View* parent,ProductionBuildingContext& c)
    :ExtendedPictureButton(parent,c.resources,c.settings,c.strings,c.input,c.interaction,[&c]{return c.world.tutorial().active();},c.views,c.controls),
    Upgrade(c.upgrades,type,slot),buildingType(type),context_(c) {
    context_.profile.uints[property("ProdType",slot)]=type;
    take_=new ButtonPrototype(this,c.resources,c.input,c.interaction,c.views,c.controls);
    given_=new ButtonPrototype(this,c.resources,c.input,c.interaction,c.views,c.controls);
    startStop_=new ProductionStartStopButton(this,this,c.resources,c.input,c.interaction,c.views,c.controls);
    loadedFloat_=new ProgressBar(this,c.resources,c.views,c.controls);
    warehouseQty_=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
    loadedQty_=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
    loaded_=new Slider(this,c.resources,c.input,c.interaction,c.views,c.controls);
    addStock_=new AnimationButton(this,c.resources,c.input,c.interaction,c.views,c.controls);
    addDepot_=new AnimationButton(this,c.resources,c.input,c.interaction,c.views,c.controls);
    lighter_=new Picture(this,c.resources,c.views,c.controls);
    sensorPanel_=new ProductionSubstrateButton(*this,c);
    inverseSubstrate_=new Picture(this,c.resources,c.views,c.controls);
    loadedSubstrate_=new Picture(this,c.resources,c.views,c.controls);
    uiControls_={loaded_,startStop_,&loaded_->thumb(),addStock_,addDepot_};
    loaded_->dragChanged=&loadedDrag;
    loadXML(*c.resources.loadSettingsXML(String(productionKind(type).xml)));
}
ProductionBuilding::~ProductionBuilding() {
    if(context_.world.movingBuilding()==this) context_.world.movingBuilding()=nullptr;
    for(auto* sound:sounds_) if(sound) sound->releaseReference();
}
void ProductionBuilding::loadXML(const XmlNode& node) {
    ExtendedPictureButton::loadXML(node);Upgrade::loadXML(node);originalDepth=deep();normalColor=geometry().color;
    const wchar_t* keys[]={L"JobDoneSound",L"BuildSound",L"TakeHouseSound",L"DropHouseSound",L"DestroySound"};
    for(unsigned i=0;i<5;++i) {const auto name=toNarrow(node.attribute(WString(keys[i])));if(!name.empty()) sounds_[i]=context_.resources.getSound(name);} // native reload overwrites without releasing
    child(*take_,node,"TakeIcon");child(*given_,node,"GivenIcon");child(*startStop_,node,"StartStopButton");
    child(*warehouseQty_,node,"WarehouseQtyTag");child(*loadedQty_,node,"LoadedQtyTag");child(*loadedFloat_,node,"LoadedFloat");child(*loaded_,node,"Loaded");
    child(*addStock_,node,"AddStockButton");child(*addDepot_,node,"AddDepotButton");child(*loadedSubstrate_,node,"LoadedSubst");
    child(*lighter_,node,"Podlogka");child(*inverseSubstrate_,node,"IPodlogka");child(*sensorPanel_,node,"SensorPanel");
    attribute(node,L"IconType",iconType);colorAttribute(node,L"UnfitColor",unfitColor);colorAttribute(node,L"PodlogkaFitColor",fitSubstrate);colorAttribute(node,L"PodlogkaUnfitColor",unfitSubstrate);
    const auto& kind=productionKind(buildingType);const auto& input=context_.configuration.products.at(kind.inputProduct());const auto& output=context_.configuration.products.at(kind.outputProduct());
    take_->copyAppearance(iconType ? input.iconLarge:input.icon);given_->copyAppearance(iconType ? output.iconLarge:output.icon);
    const float x=float(originalSubtract(geometry().x,100));const float y=float(originalAdd(originalMultiply(geometry().height,0.5),geometry().y));
    for(Control* control:std::initializer_list<Control*>{take_,given_,startStop_,warehouseQty_,loadedQty_,loadedFloat_,loaded_,addStock_,addDepot_,loadedSubstrate_})
        control->setPosition({float(originalAdd(x,control->geometry().x)),float(originalAdd(y,control->geometry().y))});
    addDepot_->clickEvent=&addStock;addStock_->clickEvent=&returnStock;loaded_->valueChanged=&loadedValue;startStop_->clickEvent=&toggleStop;
    auto& world=context_.world;int currentLevel=level();
    if(currentLevel==-1) {
        currentLevel=0;copyAppearance(levels[0].appearance);markTransformDirty();const auto all=world.buildings();for(auto* building:all) building->unhovered();
        flags=1;world.movingBuilding()=this;setDeep(float(originalAdd(originalDepth,1730)));built=false;
        positionProduction(context_.interaction.cursor(),false);dragOffset={0,0};setProductionStopped(false);startStop_->setStoppedSign(false);setProductionCount(0);hideControls();
        for(auto* building:all) {building->lighter_->show();building->lighter_->setColor(building->fitSubstrate);}
        lighter_->show();lighter_->setColor(fitSubstrate);world.beginManipulation();
    } else {
        setPosition({context_.profile.floats[property("XProdPos",propertyId)],context_.profile.floats[property("YProdPos",propertyId)]});
        setLevel(currentLevel,false);built=true;const bool stopped=productionStopped();startStop_->setStoppedSign(stopped);
        const auto count=productionCount();setProductionCount(count);if(count) flags|=2;if(stopped) flags|=4;setDeep(originalDepth);
    }
    productionSpeed=float(originalDivide(100,levels.at(static_cast<unsigned>(currentLevel)).maximumProductionTime));unhovered();
    overlay().setTextureName(toNarrow(node.attribute(WString(L"lighter_mask")))); // 004a18f5 addresses +27c, not Podlogka +620
}
unsigned ProductionBuilding::productionCount() {return context_.profile.uints[property("ProdCount",propertyId)];}
float ProductionBuilding::productionProgress() {return context_.profile.floats[property("ProdPct",propertyId)];}
bool ProductionBuilding::productionStopped() {return context_.profile.uints[property("ProdStop",propertyId)]!=0;}
void ProductionBuilding::setProductionProgress(float value) {
    const auto key=property("ProdPct",propertyId);if(value!=context_.profile.floats[key]) context_.profile.floats[key]=value;
}
void ProductionBuilding::setProductionStopped(bool value) {
    const auto key=property("ProdStop",propertyId);const auto old=context_.profile.uints[key];
    if(value) flags|=4;else flags&=~4U;
    if((old!=0)!=value) context_.profile.uints[key]=value ? 1:0;
}
void ProductionBuilding::positionProduction(const Point2& position,bool save) {
    setPosition(position);if(save) {context_.profile.floats[property("XProdPos",propertyId)]=position[0];context_.profile.floats[property("YProdPos",propertyId)]=position[1];}
}
void ProductionBuilding::loadLevel(unsigned index,const XmlNode& node) {levels.at(index).loadXML(node,context_.resources);}
unsigned ProductionBuilding::capacity() {
    const int current=level();if(current==-1) return tipAppearances()[2].loopStorageWord();
    return levels.at(static_cast<unsigned>(current)).productionCount;
}
void ProductionBuilding::setProductionCount(unsigned value) {
    const auto key=property("ProdCount",propertyId);context_.profile.uints[key];if(!value) setProductionProgress(0);
    const auto maximum=capacity();value=(std::min)(value,maximum);
    const float percent=float(originalDivide(originalMultiply(originalUnsigned(value),100),originalUnsigned(maximum)));
    loaded_->setValue(percent);loaded_->updateThumb();loadedFloat_->current=percent;loadedFloat_->updateLid();
    loadedQty_->setString(quantity(value));warehouseQty_->setString(quantity(context_.world.warehouseGoods(productionKind(buildingType).inputProduct())));context_.profile.uints[key]=value;
    if(!value) {flags&=~2U;setFrame(0);setState(ButtonState::Normal);refreshMouseInput();}
    else {flags|=2;const auto& appearance=levels.at(static_cast<unsigned>(level())).appearance;if(texture()!=appearance.texture) {copyAppearance(levels.at(static_cast<unsigned>(level())).appearance);markTransformDirty();}}
}
void ProductionBuilding::refresh() {
    const int current=level();if(current==-1) return;auto& record=levels.at(static_cast<unsigned>(current));copyAppearance(record.appearance);
    if(record.maskName.empty()) setMaskEnabled(false);else setMask(loadClickMask(record.maskName));
    markTransformDirty();productionSpeed=float(originalDivide(100,record.maximumProductionTime));setProductionCount(productionCount());
    if(current==3) context_.profile.booleans[property("awDepotUpg",itemType)]=true;
}
void ProductionBuilding::onEnabled() {
    ExtendedPictureButton::onEnabled();if(!context_.world.movingBuilding()) {
        const auto position=context_.interaction.cursor();
        // Original 00499e30 compares +27c (PictureButton overlay), not this.
        if(static_cast<View*>(&overlay())!=static_cast<View*>(context_.world.focusedBuilding())) {if(pointInRect(position)) hovered();else unhovered();}
    }
}
void ProductionBuilding::onDisabled() {ExtendedPictureButton::onDisabled();}
void ProductionBuilding::hovered() {
    auto* text=dynamic_cast<TextLine*>(tips().at(1));if(!text) throw std::logic_error("Production house requires first tooltip TextLine");
    text->setString(context_.configuration.upgradeStrings.at(static_cast<unsigned>(level()+1)));
    if(loaded_->dragging()) return;auto& world=context_.world;if(!world.movingBuilding()) showControls();
    int current=level();if(current==-1) current=0;auto& record=levels.at(static_cast<unsigned>(current));copyAppearance((flags&2) ? record.appearance:record.hovered);markTransformDirty();
    if(!world.movingBuilding()) ExtendedPictureButton::hovered();else if(world.movingBuilding()==this) ButtonPrototype::hovered();
}
void ProductionBuilding::unhovered() {
    if(sensorPanel_->buttonState()==ButtonState::Hovered || loaded_->dragging()) {tipsDirection=1;return;}
    auto& world=context_.world;if(world.focusedBuilding()!=this) hideControls();
    if(!(flags&2)) {int current=level();if(current==-1) current=0;copyAppearance(levels.at(static_cast<unsigned>(current)).appearance);markTransformDirty();}
    if(!world.movingBuilding()) ExtendedPictureButton::unhovered();else if(world.movingBuilding()==this) ButtonPrototype::unhovered();
}
void ProductionBuilding::absoluteStep() {
    auto& world=context_.world;
    if(flags&1) {
        if(world.tutorial().canShow(37)) world.tutorial().showAction(37,nullptr);
        const auto cursor=context_.interaction.cursor();positionProduction({float(originalSubtract(cursor[0],dragOffset[0])),float(originalSubtract(cursor[1],dragOffset[1]))},false);setFrame(0);
        const bool valid=world.validPlacement(*this);setColor(valid ? normalColor:unfitColor);lighter_->setColor(valid ? fitSubstrate:unfitSubstrate);
    } else if(flags&8) {
        const auto cursor=context_.interaction.cursor();const auto dx=originalSubtract(pressCursor[0],cursor[0]),dy=originalSubtract(pressCursor[1],cursor[1]);
        if(originalAdd(originalMultiply(dy,dy),originalMultiply(dx,dx))>32) {
            flags=(flags&~8U)|1;world.movingBuilding()=this;setDeep(float(originalAdd(originalDepth,1730)));if(sounds_[2]) context_.sound.clickSound(sounds_[2]);
            hideControls();world.beginManipulation();for(auto* building:world.buildings()) {building->lighter_->show();building->lighter_->setColor(building->fitSubstrate);}
        }
    } else if((flags&6)==2) {
        if(!uiElapsedTime()) throw std::logic_error("Production house timer is not bound");float elapsed=uiElapsedTime()();
        if(world.tutorial().active()) elapsed=0;if(world.scenesMoving()) elapsed=0;
        const auto count=productionCount();if(!count) {setFrame(0);return;}
        const float previous=productionProgress();const auto unspilled=originalAdd(originalMultiply(elapsed,productionSpeed),previous);float progress=float(unspilled);if(unspilled>=100) progress=100;
        setProductionProgress(progress);
        if(progress==100) {
            if(!world.warehouseFree()) {setFrame(0);return;}
            setProductionCount(count-1);world.giveWarehouse(productionKind(buildingType).outputProduct(),1,true,false);setProductionProgress(0);markTransformDirty();animation_.time=float(originalAdd(elapsed,animation_.time));
            if(count-1==0 && sounds_[0]) {context_.sound.clickSound(sounds_[0]);context_.profile.booleans["TProdCompl"]=true;}
            const auto completed=context_.profile.uints["awQProcess"];context_.profile.uints["awQProcess"]=completed+1;
        } else {markTransformDirty();animation_.time=float(originalAdd(elapsed,animation_.time));}
    }
}
void ProductionBuilding::clicked() {
    auto& world=context_.world;if(world.movingBuilding() && world.movingBuilding()!=this) return;if(world.manipulationBlocked()) return;
    if(flags&1) {
        if(world.validPlacement(*this)) {
            flags&=~1U;setDeep(originalDepth);positionProduction({geometry().x,geometry().y},true);world.movingBuilding()=nullptr;world.endManipulation();
            for(auto* building:world.buildings()) building->lighter_->hide();
            if(built) {if(sounds_[3]) context_.sound.clickSound(sounds_[3]);}
            else {
                built=true;context_.profile.booleans[property("awManDepotBuild",itemType)]=true;setLevel(0,false);if(sounds_[1]) context_.sound.clickSound(sounds_[1]);
                unsigned price=context_.configuration.upgradeCost(itemType,0);if(world.tutorial().activeStep(37)) price=0;
                const unsigned remaining=unsigned(world.money())-price;int signedMoney;std::memcpy(&signedMoney,&remaining,4);world.setMoney(signedMoney);
                if(auto* focused=world.focusedBuilding()) focused->hideControls();world.focus(this,false);
            }
            showControls();
        } else if(!built) {
            if(world.tutorial().activeStep(37)) return;
            world.movingBuilding()=nullptr;world.endManipulation();world.remove(*this);for(auto* building:world.buildings()) building->lighter_->hide();return;
        }
    } else if(!world.movingBuilding()) {
        const bool controlActive=std::any_of(uiControls_.begin(),uiControls_.end(),[](ButtonPrototype* control){return control->buttonState()==ButtonState::Hovered || control->buttonState()==ButtonState::Pressed;});
        if(!controlActive) {flags|=8;const Point2 position{geometry().x,geometry().y};pressCursor=context_.interaction.cursor();dragOffset={float(originalSubtract(pressCursor[0],position[0])),float(originalSubtract(pressCursor[1],position[1]))};pressPosition=position;}
    }
    ButtonPrototype::clicked();tipsDirection=1;
}
void ProductionBuilding::unclicked() {
    auto& world=context_.world;if(world.movingBuilding() && world.movingBuilding()!=this) return;
    if(flags&1) {
        if(world.validPlacement(*this)) {
            flags&=~1U;setDeep(originalDepth);positionProduction({geometry().x,geometry().y},true);world.movingBuilding()=nullptr;world.endManipulation();
            for(auto* building:world.buildings()) building->lighter_->hide();if(sounds_[3]) context_.sound.clickSound(sounds_[3]);showControls();
        }
    } else if(flags&8) {
        flags&=~8U;auto* focused=world.focusedBuilding();if(focused && focused!=this) focused->hideControls();world.focus(this,true);
    }
    ButtonPrototype::unclicked();tipsDirection=1;
}
void ProductionBuilding::hideControls() {
    // Exact 00499c00 order; StartStop and LoadedSubst are absent from this list.
    for(View* view:std::initializer_list<View*>{take_,given_,warehouseQty_,loadedQty_,loadedFloat_,loaded_,addStock_,addDepot_,inverseSubstrate_,loadedSubstrate_}) view->hide();
}
void ProductionBuilding::showControls() {
    take_->show();given_->show();warehouseQty_->show();loadedQty_->show();
    for(View* view:std::initializer_list<View*>{loaded_,loadedFloat_,addStock_,addDepot_}) if(!shown(*view)) view->show();
    loadedSubstrate_->show();inverseSubstrate_->show();if(buttonState()!=ButtonState::Hovered) setState(ButtonState::Hovered);
}
bool ProductionBuilding::cursorInControls() {
    const auto position=context_.interaction.cursor();for(auto* control:uiControls_) if(control->pointInRect(position)) return true;return false;
}
bool ProductionBuilding::refreshHover() {
    const auto position=context_.interaction.cursor();if(this==context_.world.focusedBuilding()) return false;
    if(pointInRect(position)) hovered();else unhovered();return true;
}
void ProductionBuilding::toggleStop(ButtonPrototype* button) {
    auto& self=*static_cast<ProductionStartStopButton*>(button)->owner();const bool stopped=!self.productionStopped();
    self.startStop_->setStoppedSign(stopped);self.setProductionStopped(stopped);
}
void ProductionBuilding::addStock(ButtonPrototype* button) {
    auto& self=*static_cast<ProductionBuilding*>(button->parent());const auto capacity=self.capacity();
    const auto count=self.productionCount();const auto input=productionKind(self.buildingType).inputProduct();
    if(self.context_.world.warehouseGoods(input) && capacity-count) {self.context_.world.takeWarehouse(input,1,true,false);self.setProductionCount(count+1);}
}
void ProductionBuilding::returnStock(ButtonPrototype* button) {
    auto& self=*static_cast<ProductionBuilding*>(button->parent());self.level(); // original discarded level query
    const auto count=self.productionCount();if(self.context_.world.warehouseFree() && count) {
        self.context_.world.giveWarehouse(productionKind(self.buildingType).inputProduct(),1,true,false);self.setProductionCount(count-1);
    }
}
void ProductionBuilding::loadedDrag(Slider* slider,bool dragging) {
    if(!dragging) {auto& self=*static_cast<ProductionBuilding*>(slider->parent());self.refreshHover();self.context_.world.hud().stockChanged(false);}
}
void ProductionBuilding::loadedValue(Slider* slider,float value) {
    auto& self=*static_cast<ProductionBuilding*>(slider->parent());auto& world=self.context_.world;
    self.loadedFloat_->current=value;self.loadedFloat_->updateLid();const unsigned maximum=self.capacity();unsigned count=self.productionCount();
    const float maximumFloat=float(originalUnsigned(maximum));const auto converted=truncatedWord(originalMultiply(originalMultiply(originalUnsigned(maximum),value),-0.01f));
    const unsigned deltaWord=0U-count-converted;int delta;std::memcpy(&delta,&deltaWord,4);const auto input=productionKind(self.buildingType).inputProduct();
    if(delta>0) {
        const unsigned transfer=(std::min)(unsigned(delta),world.warehouseGoods(input));if(transfer) {world.takeWarehouse(input,transfer,true,true);self.setProductionCount(count+transfer);}
    } else if(delta<0) {
        unsigned transfer=(std::min)(0U-deltaWord,world.warehouseFree());transfer=(std::min)(transfer,count);
        if(transfer) {world.giveWarehouse(input,transfer,true,true);self.setProductionCount(count-transfer);}
    }
    const unsigned free=world.warehouseFree();count=self.productionCount();float minimum=0;
    if(free<count) minimum=float(originalDivide(originalMultiply(originalUnsigned(count-free),100),maximumFloat));
    const unsigned available=world.warehouseGoods(input)+count;
    const auto upper=maximum>available ? originalDivide(originalMultiply(originalUnsigned(available),100),maximumFloat):100;
    if(value<minimum) value=minimum;if(value>upper) value=float(upper);
    slider->setValue(value);slider->updateThumb();self.loadedFloat_->current=value;self.loadedFloat_->updateLid();
}
}
