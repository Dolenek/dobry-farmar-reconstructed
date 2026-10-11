#pragma once
#include "ContractDialogs.h"
namespace farmar::recovered {
// The source Farm letter actor must supply its actual borrowed event (+37c).
// No fake actor or default event is supplied by this UI controller.
struct ContractOfferLetter {
    virtual ~ContractOfferLetter()=default;
    virtual GameplayEvent& contractEvent() const=0;
};
struct ContractOfferWorld {
    virtual ~ContractOfferWorld()=default;
    virtual int activeContracts() const=0; // GameMenu +60
    virtual int money() const=0; // 00410ce0
    virtual void setMoney(int)=0; // 00410d60, actual world refresh
    virtual void consumeLetter(ContractOfferLetter&)=0; // Farm 004341b0
    virtual void activateContract(GameplayEvent&)=0; // GameMenu 004c02c0
    virtual bool tutorialCanShow(unsigned) const=0; // 004d0a20
    virtual void showTutorial(unsigned,View&)=0; // 004cf3c0
};
class ContractProvide final:public ContractDialogBase {
public:
    ContractProvide(View*,ContractDialogContext&,ContractOfferWorld&); // 004ad4b0, native 220 bytes
    ~ContractProvide() override; // 004ada90
    void loadXML(const XmlNode&) override; // 004abef0
    void enabledStep() override; // 004abea0
    void start(ContractOfferLetter&); // 004ac570
    void accept(); // 004accc0, consumes letter, deducts penalty, activates contract
    void wait(); // 004ad0d0, retains offer
    TextAnimationButton& yesButton() noexcept {return *yes_;}
    TextAnimationButton& waitButton() noexcept {return *wait_;}
    TextLine& timeLabel() noexcept {return *timeLabel_;}
    TextLine& timeValue() noexcept {return *timeValue_;}
    ContractOfferLetter* letter() const noexcept {return letter_;}
    static ContractProvide* instance() noexcept;
private:
    ContractOfferWorld& world_;
    TextAnimationButton *yes_,*wait_;TextLine *timeLabel_,*timeValue_;
    ContractOfferLetter* letter_=nullptr; // borrowed; native ctor leaves unspecified
    static void accepted(ButtonPrototype*);static void waited(ButtonPrototype*);
};
}
