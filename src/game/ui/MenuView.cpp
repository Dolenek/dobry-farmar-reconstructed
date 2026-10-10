#include "MenuView.h"
#include "Control.h"
#include "TextLine.h"
#include "MultilineText.h"
#include "XmlDocument.h"
namespace farmar::recovered {
MenuView::MenuView(View* parent,ResourceManager& resources,Settings& settings,StringManager& strings,ViewRegistry& views,ControlRegistry& controls)
    : View(parent,views),resources_(resources),settings_(settings),strings_(strings),controls_(controls) {}
void MenuView::loadXML(const XmlNode& node) {
    View::loadXML(node);
    for(const char* name:{"Background","BackText","BackMultitext"}) {
        XmlNode* section=nullptr;if(!node.section(String(name),section)) continue;
        for(unsigned index=0;index<section->children().size();++index) {
            XmlNode* entry=nullptr;if(!section->section(index,entry)) continue;
            View* decoration=nullptr;
            if(name[4]=='g') decoration=new Picture(this,resources_,registry_,controls_);
            else if(name[4]=='T') decoration=new TextLine(this,resources_,settings_,strings_,registry_,controls_);
            else decoration=new MultilineText(this,resources_,settings_,strings_,registry_,controls_);
            decoration->loadXML(*entry);decorations_.insert(decoration);
        }
    }
}
}
