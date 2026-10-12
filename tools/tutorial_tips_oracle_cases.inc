stage=330;redirect(0xd0c30,reinterpret_cast<void*>(&tipNativeFinish));
const auto compareMultiline=[&](MultilineText& text,void* native) {
    compareControl(text,native);require(text.string().view()==field<WString>(native,0x23c).view() && text.textDirty()==bool(field<unsigned char>(native,0x258)),"Tip multiline text differs");
    compareFloat(text.multiplier(),field<float>(field<void*>(native,0x238),0x298),"Tip text multiplier");compareFloat(text.interlinear(),field<float>(native,0x25c),"Tip text interlinear");
    auto* head=field<Link*>(native,0x230);auto* item=head->next;require(text.lines().size()==field<unsigned>(native,0x234),"Tip multiline line count differs");unsigned letters=0;for(auto* line:text.lines()) {require(item!=head,"Tip multiline list ended early");compareControl(*line,item->control);letters+=compare(*line,item->control);item=item->next;}require(item==head,"Tip multiline list ended late");return letters;
};
TipDiagnosticHost tipHost(*hud);TutorialTipContext tipContext{manager,settings,strings,input,interaction,tipHost,views,controls};
std::vector<WString> texts;XmlDocument tutorialDefinition(files,String("Tutorial/Tutorial.xml"));XmlNode* tipDefinitions=nullptr;require(tutorialDefinition.section(String("Tips"),tipDefinitions),"Tutorial actual tips missing");
for(unsigned n=0;n<tipDefinitions->children().size();++n) {XmlNode* item=nullptr;XmlNode* text=nullptr;require(tipDefinitions->section(n,item) && item->section(String("Text"),text),"Tutorial actual text missing");texts.push_back(text->text());}
Control tipAnchor(nullptr,manager,views,controls);auto* nativeAnchor=allocate(556);function<void*(__thiscall*)(void*,void*)>(0xedc80)(nativeAnchor,nullptr);tipSourceAnchor=&tipAnchor;tipNativeAnchor=nativeAnchor;
const unsigned tipCtors[]={0xcdde0,0xcc150,0xcc700,0xca6e0,0xcac90};const unsigned tipSizes[]={84,124,128,128,136};
for(unsigned kind=0;kind<5;++kind) {
    stage=331;std::unique_ptr<TutorialCommonTip> tip;
    switch(kind) {case 0:tip=std::make_unique<TutorialCommonTip>(nullptr,tipContext);break;case 1:tip=std::make_unique<TutorialArrowTip>(nullptr,tipContext);break;case 2:tip=std::make_unique<TutorialDisabledAnchorTip>(nullptr,tipContext);break;case 3:tip=std::make_unique<TutorialActionTip>(nullptr,tipContext);break;default:tip=std::make_unique<TutorialRepeatedActionTip>(nullptr,tipContext);}
    auto* nativeTip=allocate(tipSizes[kind]);std::memset(nativeTip,0,tipSizes[kind]);function<void*(__thiscall*)(void*,void*)>(tipCtors[kind])(nativeTip,nullptr);++tutorialTipObjects;
    const char* names[]={"Tutorial/CommonHelpTip.xml","Tutorial/ArrowHelpTip.xml","Tutorial/ArrowHelpTip.xml","Tutorial/ActionTip.xml","Tutorial/ActionTip.xml"};XmlDocument xml(files,String(names[kind]));tip->loadXML(xml);function<void(__thiscall*)(void*,void*)>(kind ? 0xcb8f0:0xcd400)(nativeTip,wrap(&xml));
    const auto compareTip=[&] {stage=332;mainMenuCompareView(*tip,nativeTip);compareControl(tip->image(),field<void*>(nativeTip,0x40));tutorialTipLetters+=compareMultiline(tip->text(),field<void*>(nativeTip,0x44));compareControl(tip->okButton(),field<void*>(nativeTip,0x48));tutorialTipLetters+=compare(tip->okButton().text(),field<void*>(field<void*>(nativeTip,0x48),0x27c));
        compareFloat(tip->originalDepth,field<float>(nativeTip,0x34),"Tip original depth");compareFloat(tip->defaultMultiplier,field<float>(nativeTip,0x4c),"Tip XML multiplier");compareFloat(tip->defaultInterlinear,field<float>(nativeTip,0x50),"Tip XML interlinear");
        for(unsigned n=0;n<2;++n) {compareFloat(tip->originalPosition[n],field<float>(nativeTip,0x2c+n*4),"Tip original position");compareFloat(tip->offset[n],field<float>(nativeTip,0x38+n*4),"Tip offset");}
        require(tip->children().size()==(kind ? 2:1),"Tip retained owned subtrees differ");
        if(kind) {auto& arrow=static_cast<TutorialArrowTip&>(*tip);compareControl(arrow.arrow(),field<void*>(nativeTip,0x60));for(unsigned n=0;n<2;++n) {compareFloat(arrow.arrowOffset[n],field<float>(nativeTip,0x58+n*4),"Tip arrow offset");compareFloat(arrow.arrowOrigin[n],field<float>(nativeTip,0x64+n*4),"Tip arrow origin");compareFloat(arrow.velocity[n],field<float>(nativeTip,0x6c+n*4),"Tip arrow velocity");}compareFloat(arrow.phase,field<float>(nativeTip,0x74),"Tip phase");require(arrow.arrow().geometry().invertX==bool(field<unsigned char>(field<void*>(nativeTip,0x60),0x1c4)) && arrow.arrow().geometry().invertY==bool(field<unsigned char>(field<void*>(nativeTip,0x60),0x1c5)),"Tip arrow flips differ");}
        require(tipSourceFinishes==tipNativeFinishes && tipSourcePredicates==tipNativePredicates,"Tip manager/predicate calls differ");if(tipHost.cancel!=bool(field<unsigned char>(contractTutorialObject.data(),0xc08))) std::cerr<<"Tip kind "<<kind<<" source cancel "<<tipHost.cancel<<" native "<<unsigned(field<unsigned char>(contractTutorialObject.data(),0xc08))<<" actual global "<<(field<void*>(image,0x25f73c)==contractTutorialObject.data())<<" states "<<tutorialTipStates<<"\n";require(tipHost.cancel==bool(field<unsigned char>(contractTutorialObject.data(),0xc08)),"Tip cancellation differs");++tutorialTipStates;check();};compareTip();
    for(trial=0;trial<50;++trial) {
        stage=333;tipHost.cancel=false;field<unsigned char>(contractTutorialObject.data(),0xc08)=0;tipPredicateResult=false;
        const Point2 offset{float(60+trial*3),float(75+trial)},arrowOffset{float(10+trial),float(17+trial)};const Point2 at{float(trial%2 ? 500:200),float(trial%3 ? 500:200)};
        tipAnchor.setPosition(at);function<void(__thiscall*)(void*,const Point2*)>(0xf0110)(nativeAnchor,&at);tipAnchor.enable();call(0xd3bb0,nativeAnchor);
        if(kind==0) {tip->start(offset,texts[trial%texts.size()]);function<void(__thiscall*)(void*,const Point2*,const WString*)>(0xcc7c0)(nativeTip,&offset,&texts[trial%texts.size()]);}
        else {Control* anchor=trial%5==0 && kind!=2 ? nullptr:&tipAnchor;void* counterpart=anchor ? nativeAnchor:nullptr;tipSourceAnchor=anchor;tipNativeAnchor=counterpart;
            if(kind==1) {static_cast<TutorialArrowTip&>(*tip).start(offset,arrowOffset,texts[trial%texts.size()],anchor);function<void(__thiscall*)(void*,const Point2*,const Point2*,const WString*,void*)>(0xcb7e0)(nativeTip,&offset,&arrowOffset,&texts[trial%texts.size()],counterpart);}
            else if(kind==2) {static_cast<TutorialDisabledAnchorTip&>(*tip).start(offset,arrowOffset,texts[trial%texts.size()],anchor);function<void(__thiscall*)(void*,const Point2*,const Point2*,const WString*,void*)>(0xcc290)(nativeTip,&offset,&arrowOffset,&texts[trial%texts.size()],counterpart);}
            else {auto test=&tipSourcePredicate;if(kind==3) static_cast<TutorialActionTip&>(*tip).start(offset,arrowOffset,texts[trial%texts.size()],anchor,test);else static_cast<TutorialRepeatedActionTip&>(*tip).start(offset,arrowOffset,texts[trial%texts.size()],anchor,test);function<void(__thiscall*)(void*,const Point2*,const Point2*,const WString*,void*,void*)>(kind==3 ? 0xca7d0:0xcaf70)(nativeTip,&offset,&arrowOffset,&texts[trial%texts.size()],counterpart,reinterpret_cast<void*>(&tipNativePredicate));}
        }++tutorialTipStarts;compareTip();
        explicitTipElapsed=true;
        if(kind) for(unsigned n=0;n<6;++n) {elapsed=0.117f+n*0.013f;tip->enabledStep();call(kind<3 ? 0xcbd60:kind==3 ? 0xca690:0xcac10,nativeTip);++tutorialTipSteps;compareTip();}
        explicitTipElapsed=false;
        if(trial==0) for(unsigned n=0;n<4;++n) {frame();compareTip();++tutorialTipFrames;}
        if(kind<3 && trial==0) {const Point2 at{tip->okButton().geometry().x,tip->okButton().geometry().y};frame(at,{}, {1,0,0});compareTip();frame(at,{0x208});compareTip();frame(at,{0x200});compareTip();frame({0,0},{},{1,0,0});compareTip();tutorialTipFrames+=4;}
        else if(kind<3) {tip->okButton().clickEvent(&tip->okButton());function<void(__cdecl*)(void*)>(kind==2 ? 0xcc320:0xcda00)(field<void*>(nativeTip,0x48));}
        else {explicitTipElapsed=true;tipPredicateResult=trial%2;tipHost.cancel=!(trial%2);field<unsigned char>(contractTutorialObject.data(),0xc08)=tipHost.cancel;elapsed=0.02f;tip->enabledStep();call(kind==3 ? 0xca690:0xcac10,nativeTip);++tutorialTipSteps;}
        compareTip();
        explicitTipElapsed=false;
    }
    const auto before=textureCounts();tip.reset();const auto middle=textureCounts();call(0xcc770,nativeTip);compareReferenceDeltas(before,middle);
}
call(0xf1560,nativeAnchor);tipSourceAnchor=nullptr;tipNativeAnchor=nullptr;
