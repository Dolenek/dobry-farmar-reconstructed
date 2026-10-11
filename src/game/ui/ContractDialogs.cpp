#include "ContractDialogs.h"
#include "ContractStatus.h"
#include "GameConfiguration.h"
#include "ResourceManager.h"
#include "XmlDocument.h"
#include "OriginalArithmetic.h"
#include <sstream>
#include <stdexcept>
namespace farmar::recovered {
namespace {
ContractAim* aimInstance=nullptr;ContractFail* failInstance=nullptr;ContractPassConfirm* confirmInstance=nullptr;
void loadChild(View& child,const XmlNode& node,const char* key) {XmlNode* section=nullptr;if(node.section(String(key),section)) child.loadXML(*section);}
WString number(unsigned value) {return WString(std::to_wstring(value));}
WString money(int value) {return WString(std::to_wstring(value));}
}
void ContractRequirementLine::loadXML(const XmlNode& node) {
    loadChild(*icon,node,"ProductIcon");loadChild(*label,node,"ReqText");loadChild(*number,node,"NumberText");
}
void ContractRequirementLine::setup(const ContractRequirement& requirement,GameConfiguration& configuration,unsigned count,bool available) {
    // The original hides only reqType 1 here. Other types use the product
    // appearance; the two navigation-state arguments are unused by 004aa890.
    if(requirement.reqType==1) {icon->hide();label->hide();number->hide();return;}
    icon->copyAppearance(configuration.products.at(requirement.productType).icon);
    if(count>requirement.count) count=requirement.count;
    number->setString(WString(std::to_wstring(available ? count:requirement.count)));
    icon->show();label->show();number->show();
}
ContractDialogBase::ContractDialogBase(View* parent,ContractDialogContext& context)
    :MenuView(parent,context.resources,context.settings,context.strings,context.views,context.controls),context_(context) {
    auto& c=context_;
    note_=new MultilineText(this,c.resources,c.settings,c.strings,c.views,c.controls);
    bonusLabel_=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
    penaltyLabel_=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
    bonusMoney_=new TextMoney(this,c.resources,c.settings,c.strings,c.views,c.controls);
    penaltyMoney_=new TextMoney(this,c.resources,c.settings,c.strings,c.views,c.controls);
    for(auto& line:lines_) createLine(line);
}
void ContractDialogBase::createLine(ContractRequirementLine& line) {
    auto& c=context_;line.icon=new Picture(this,c.resources,c.views,c.controls);
    line.label=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
    line.number=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
}
void ContractDialogBase::loadLines(const XmlNode& node,const char* prototype,std::array<ContractRequirementLine,2>& lines) {
    XmlNode* places=nullptr;if(!node.section(String("AimPlaces"),places)) return;
    const auto count=(std::min)(std::size_t(2),places->children().size());
    for(unsigned n=0;n<count;++n) {
        auto& line=lines[n];XmlNode* definition=nullptr;
        if(node.section(String(prototype),definition)) line.loadXML(*definition);
        XmlNode* position=nullptr;if(places->section(n,position)) {
            const auto value=position->attribute(WString(L"Place"));
            if(!value.empty()) {std::wistringstream stream(value.c_str());stream>>line.place[0]>>line.place[1];}
            for(Control* child:{static_cast<Control*>(line.icon),static_cast<Control*>(line.label),static_cast<Control*>(line.number)}) {
                const auto& p=child->geometry();child->setPosition({float(originalAdd(p.x,line.place[0])),float(originalAdd(p.y,line.place[1]))});
            }
        }
    }
}
void ContractDialogBase::loadXML(const XmlNode& node) {
    MenuView::loadXML(node);loadChild(*note_,node,"Note");
    loadChild(*bonusLabel_,node,"BonusText");loadChild(*penaltyLabel_,node,"PenaltyText");
    loadChild(*bonusMoney_,node,"BonusMoneyText");loadChild(*penaltyMoney_,node,"PenaltyMoneyText");
    loadLines(node,"AimProto",lines_);
    XmlNode* state=nullptr;if(node.section(String("ToMarketState"),state)) market_.loadXML(*state,context_.resources);
    if(node.section(String("ToWareState"),state)) warehouse_.loadXML(*state,context_.resources);
}
void ContractDialogBase::suspendGame() {context_.game.absoluteDisable();context_.game.disable();showTransparently();}
void ContractDialogBase::resumeGame() {context_.game.enable();context_.game.absoluteEnable();}
ContractAim* ContractAim::instance() noexcept {return aimInstance;}
ContractAim::ContractAim(View* parent,ContractDialogContext& context):ContractDialogBase(parent,context) {
    if(aimInstance) throw std::runtime_error("Object already exist.");aimInstance=this;auto& c=context_;
    ok_=new TextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);
    caption_=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
    for(auto& line:missing_) createLine(line);
    auto xml=c.resources.loadSettingsXML(String("Gameplay/ContractAim.xml"));loadXML(*xml);ok_->clickEvent=&clicked;
}
ContractAim::~ContractAim() {aimInstance=nullptr;}
void ContractAim::clicked(ButtonPrototype*) {instance()->accept();}
void ContractAim::loadXML(const XmlNode& node) {ContractDialogBase::loadXML(node);loadChild(*ok_,node,"OkButton");loadChild(*caption_,node,"CaptionText");loadLines(node,"AimProtoAdd",missing_);}
void ContractAim::start(ContractStatus& status) {
    status_=&status;const auto& contract=std::get<ContractEvent>(status.event->data);bonusMoney_->setString(money(contract.bonus));
    penaltyMoney_->hide();penaltyLabel_->hide();
    for(unsigned n=0;n<2;++n) {
        const auto& req=contract.requirements[n];const auto stock=req.reqType==0 ? status.stock(req.productType):0;
        auto& line=missing_[n];
        if(req.reqType==1) {line.icon->hide();line.label->hide();line.number->hide();}
        else {line.icon->copyAppearance(context_.configuration.products.at(req.productType).icon);line.number->setString(number(req.count<stock ? 0:req.count-stock));line.icon->show();line.label->show();line.number->show();}
        lines_[n].setup(req,context_.configuration,stock,true);
    }
    suspendGame();
}
void ContractAim::accept() {hideTransparently();resumeGame();}
ContractFail* ContractFail::instance() noexcept {return failInstance;}
ContractFail::ContractFail(View* parent,ContractDialogContext& context):ContractDialogBase(parent,context) {
    if(failInstance) throw std::runtime_error("Object already exist.");failInstance=this;auto& c=context_;
    ok_=new TextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);
    caption_=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
    result_=new MultilineText(this,c.resources,c.settings,c.strings,c.views,c.controls);
    auto xml=c.resources.loadSettingsXML(String("Gameplay/ContractFail.xml"));loadXML(*xml);ok_->clickEvent=&clicked;
}
ContractFail::~ContractFail() {failInstance=nullptr;}
void ContractFail::clicked(ButtonPrototype*) {instance()->accept();}
void ContractFail::loadXML(const XmlNode& node) {ContractDialogBase::loadXML(node);loadChild(*ok_,node,"OkButton");loadChild(*caption_,node,"CaptionText");loadChild(*result_,node,"Text");}
void ContractFail::start(ContractStatus& status) {
    status_=&status;const auto& contract=std::get<ContractEvent>(status.event->data);result_->setString(contract.lossNote,true);
    bonusMoney_->hide();bonusLabel_->hide();penaltyMoney_->hide();penaltyLabel_->hide();
    for(unsigned n=0;n<2;++n) {const auto& req=contract.requirements[n];lines_[n].setup(req,context_.configuration,req.reqType==0 ? status.stock(req.productType):0,true);}
    suspendGame();
}
void ContractFail::accept() {hideTransparently();status_=nullptr;context_.game.absoluteEnable();context_.game.enable();}
ContractPassConfirm* ContractPassConfirm::instance() noexcept {return confirmInstance;}
ContractPassConfirm::ContractPassConfirm(View* parent,ContractDialogContext& context):ContractDialogBase(parent,context) {
    if(confirmInstance) throw std::runtime_error("Object already exist.");confirmInstance=this;auto& c=context_;
    ok_=new TextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);
    cancel_=new TextAnimationButton(this,c.resources,c.settings,c.strings,c.input,c.interaction,c.views,c.controls);
    caption_=new TextLine(this,c.resources,c.settings,c.strings,c.views,c.controls);
    auto xml=c.resources.loadSettingsXML(String("Gameplay/ContractPassConfirm.xml"));loadXML(*xml);ok_->clickEvent=&accepted;cancel_->clickEvent=&cancelled;
}
ContractPassConfirm::~ContractPassConfirm() {confirmInstance=nullptr;}
void ContractPassConfirm::accepted(ButtonPrototype*) {instance()->accept();}
void ContractPassConfirm::cancelled(ButtonPrototype*) {instance()->cancel();}
void ContractPassConfirm::loadXML(const XmlNode& node) {ContractDialogBase::loadXML(node);loadChild(*ok_,node,"OkButton");loadChild(*cancel_,node,"CancelButton");loadChild(*caption_,node,"CaptionText");}
void ContractPassConfirm::start(ContractStatus& status) {
    status_=&status;const auto& contract=std::get<ContractEvent>(status.event->data);bonusMoney_->setString(money(contract.bonus));
    penaltyMoney_->hide();penaltyLabel_->hide();
    for(unsigned n=0;n<2;++n) {const auto& req=contract.requirements[n];lines_[n].setup(req,context_.configuration,req.reqType==0 ? status.stock(req.productType):0,true);}
    suspendGame();
}
void ContractPassConfirm::accept() {hideTransparently();resumeGame();status_->fail();status_=nullptr;}
void ContractPassConfirm::cancel() {hideTransparently();resumeGame();status_=nullptr;}
}
