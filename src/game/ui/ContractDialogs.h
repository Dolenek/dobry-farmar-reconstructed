#pragma once
#include "MenuView.h"
#include "MultilineText.h"
#include "PriceText.h"
#include "ButtonText.h"
#include "AnimableAppearance.h"
#include "GameplayEvents.h"
namespace farmar::recovered {
class ContractStatus;class GameConfiguration;
struct ContractDialogContext {
    ResourceManager& resources;Settings& settings;StringManager& strings;
    InputBindings& input;ButtonInteraction& interaction;GameConfiguration& configuration;
    View& game; // suspended GameMenu singleton 0065f740, not cGame
    ViewRegistry& views=globalViewRegistry();ControlRegistry& controls=globalControlRegistry();
};
struct ContractRequirementLine {
    Picture* icon=nullptr;TextLine* label=nullptr;TextLine* number=nullptr;Point2 place{};
    void loadXML(const XmlNode&); // 004aa690
    void setup(const ContractRequirement&,GameConfiguration&,unsigned count,bool available); // 004aa890
};
class ContractDialogBase:public MenuView {
public:
    ContractDialogBase(View*,ContractDialogContext&); // 004aadb0, native 196 bytes
    void loadXML(const XmlNode&) override; // 004ab380
    MultilineText& note() noexcept {return *note_;}
    TextLine& bonusLabel() noexcept {return *bonusLabel_;}
    TextLine& penaltyLabel() noexcept {return *penaltyLabel_;}
    TextMoney& bonusMoney() noexcept {return *bonusMoney_;}
    TextMoney& penaltyMoney() noexcept {return *penaltyMoney_;}
    ContractRequirementLine& requirement(unsigned n) {return lines_.at(n);}
    AnimableAppearance& marketState() noexcept {return market_;}
    AnimableAppearance& warehouseState() noexcept {return warehouse_;}
protected:
    ContractDialogContext& context_;
    MultilineText* note_;std::array<ContractRequirementLine,2> lines_;
    AnimableAppearance market_,warehouse_;
    TextLine *bonusLabel_,*penaltyLabel_;TextMoney *bonusMoney_,*penaltyMoney_;
    void createLine(ContractRequirementLine&);
    void loadLines(const XmlNode&,const char* prototype,std::array<ContractRequirementLine,2>&);
    void suspendGame();void resumeGame();
};
class ContractAim final:public ContractDialogBase {
public:
    ContractAim(View*,ContractDialogContext&); // 004aa040, native 252 bytes
    ~ContractAim() override; // 004aa660
    void loadXML(const XmlNode&) override; // 004a8d50
    void start(ContractStatus&); // 004a94e0
    void accept(); // 004a9c60, retains status pointer
    TextLine& caption() noexcept {return *caption_;}
    TextAnimationButton& okButton() noexcept {return *ok_;}
    ContractRequirementLine& missing(unsigned n) {return missing_.at(n);}
    ContractStatus* status() noexcept {return status_;}
    static ContractAim* instance() noexcept;
private:
    TextLine* caption_;TextAnimationButton* ok_;std::array<ContractRequirementLine,2> missing_;
    ContractStatus* status_=nullptr;
    static void clicked(ButtonPrototype*);
};
class ContractFail final:public ContractDialogBase {
public:
    ContractFail(View*,ContractDialogContext&); // 004b73f0, native 216 bytes
    ~ContractFail() override; // 004b7980
    void loadXML(const XmlNode&) override; // 004b65b0
    void start(ContractStatus&); // 004b6b90
    void accept(); // 004b7010
    TextLine& caption() noexcept {return *caption_;}
    MultilineText& resultText() noexcept {return *result_;}
    TextAnimationButton& okButton() noexcept {return *ok_;}
    ContractStatus* status() noexcept {return status_;}
    static ContractFail* instance() noexcept;
private:
    TextLine* caption_;MultilineText* result_;TextAnimationButton* ok_;ContractStatus* status_=nullptr;
    static void clicked(ButtonPrototype*);
};
class ContractPassConfirm final:public ContractDialogBase {
public:
    ContractPassConfirm(View*,ContractDialogContext&); // 004b8ce0, native 216 bytes
    ~ContractPassConfirm() override; // 004b9290
    void loadXML(const XmlNode&) override; // 004b7a00
    void start(ContractStatus&); // 004b7fe0
    void accept(); // 004b8510, invokes ContractStatus::fail
    void cancel(); // 004b8900
    TextLine& caption() noexcept {return *caption_;}
    TextAnimationButton& okButton() noexcept {return *ok_;}
    TextAnimationButton& cancelButton() noexcept {return *cancel_;}
    ContractStatus* status() noexcept {return status_;}
    static ContractPassConfirm* instance() noexcept;
private:
    TextLine* caption_;TextAnimationButton *ok_,*cancel_;ContractStatus* status_=nullptr;
    static void accepted(ButtonPrototype*);static void cancelled(ButtonPrototype*);
};
}
