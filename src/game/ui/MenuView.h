#pragma once
#include "View.h"
namespace farmar::recovered {
class ResourceManager;
class Settings;
class StringManager;
class ControlRegistry;
// Decoration-bearing cView (native 56 bytes). View owns the actual controls;
// the separate decoration set only remembers the objects created by loadXML.
class MenuView : public View {
public:
    MenuView(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,
        ViewRegistry& views,ControlRegistry& controls); // 004fb040
    void loadXML(const XmlNode& node) override; // 004fa8c0
    const ViewRegistry::Views& decorations() const noexcept { return decorations_; }
protected:
    void addDecoration(View* view) { decorations_.insert(view); } // owned by View, native set +2c
private:
    ResourceManager& resources_;Settings& settings_;StringManager& strings_;ControlRegistry& controls_;
    ViewRegistry::Views decorations_; // destructor 004fafc0 frees set, then View owns deletion
};
}
