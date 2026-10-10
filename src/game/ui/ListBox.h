#pragma once
#include "ButtonText.h"
namespace farmar::recovered {
class ListBox final : public Picture {
public:
    using Selected=void(*)(unsigned,ListBox*);
    ListBox(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,InputBindings& input,ButtonInteraction& interaction,
        ViewRegistry& views=globalViewRegistry(),ControlRegistry& controls=globalControlRegistry()); // 00521100
    void loadXML(const XmlNode& node) override; // 005206e0
    void append(const WString& name); // 005212a0
    void clear(); // 00521080
    void updateList(); // 0051e6d0
    void scrollUp(); // 00521060
    void scrollDown(); // 00521020
    Selected selected=nullptr; // original +270
    const std::vector<TextButton*>& rows() const noexcept { return rows_; }
    const Button& up() const noexcept { return *up_; }
    const Button& down() const noexcept { return *down_; }
    const TextButton& prototype() const noexcept { return *prototype_; }
    unsigned firstRow() const noexcept { return firstRow_; }
    unsigned maxRows() const noexcept { return maxRows_; }
    float interlinear() const noexcept { return interlinear_; }
    const Point2& topLeft() const noexcept { return topLeft_; }
    const Point2& bottomRight() const noexcept { return bottomRight_; }
private:
    std::vector<TextButton*> rows_;
    Button *up_,*down_;TextButton* prototype_;
    unsigned maxRows_=10,firstRow_=0;
    Point2 topLeft_{},bottomRight_{};float interlinear_=50;
    static void onUp(ButtonPrototype* button);
    static void onDown(ButtonPrototype* button);
    static void onSelect(ButtonPrototype* button); // 0051ec50
};
}
