LoadingDisabledAudio disabledBackend;SoundManager audio(files,settings,disabledBackend,0);loadingFiles=&files;
managerTable[6]=reinterpret_cast<void*>(&loadingXML);xmlTable[0]=reinterpret_cast<void*>(&loadingDeleteXML);xmlTable[34]=reinterpret_cast<void*>(&sectionCount);
redirect(0xa670,reinterpret_cast<void*>(&awardsProfileKey));redirect(0x105e0,reinterpret_cast<void*>(&awardsProfileBool));
redirect(0x2fe80,reinterpret_cast<void*>(&decimalString));redirect(0x1cc0,reinterpret_cast<void*>(&concatenateString));redirect(0x77290,reinterpret_cast<void*>(&nativeDiscard));
redirect(0x180aed,reinterpret_cast<void*>(&awardsRegisterExit));
graphicsTable[3]=reinterpret_cast<void*>(&tooltipHeight);graphicsTable[4]=reinterpret_cast<void*>(&tooltipWidth);timerTable[3]=reinterpret_cast<void*>(&tooltipUnscaledTimer);
tooltipDimensions={800,600};menuMilliseconds=0;GameTimer frameTimer(settings,TimerServices{&menuClock,&menuSleep,&menuPeriod,&menuPeriod});
XmlDocument definition(files,String("UI/Awards.xml"));
for(trial=0;trial<3;++trial) {
    stage=150;ViewRegistry views;ControlRegistry controls;profileProperties().booleans.clear();interaction.active=nullptr;interaction.pending=0;putPointer(0x2604dc,nullptr);field<unsigned>(image,0x2604e0)=0;
    for(unsigned i=0;i<21;++i) profileProperties().booleans["awardShow"+std::to_string(i)]=trial==0 ? false:trial==1 ? true:bool(i%3);
    auto parent=std::make_unique<MenuView>(nullptr,manager,settings,strings,views,controls);auto* nativeParent=allocate(56);function<void*(__thiscall*)(void*,void*)>(0xfb040)(nativeParent,nullptr);
    parent->show();call(0xd37d0,nativeParent);parent->enable();call(0xd3bb0,nativeParent);
    const auto deletes=loadingNativeXMLDeletes;const auto firstSound=sounds.size();
    auto screen=std::make_unique<Awards>(trial&1 ? parent.get():nullptr,manager,settings,strings,audio,input,interaction,[] { return tooltipDimensions; },views,controls);
    auto* original=allocate(748);function<void*(__thiscall*)(void*,void*)>(0x787e0)(original,trial&1 ? nativeParent:nullptr);auto* nativeView=static_cast<unsigned char*>(original)+4;
    require(loadingNativeXMLDeletes==deletes+43,"Awards constructor XML deletion count differs");
    require(Awards::instance()==screen.get() && field<void*>(image,0x25ffcc)==original,"Awards singleton constructor differs");
    screen->returnEvent=[&] {require(screen->state().transition==3,"Awards source callback preceded discard transition");++awardsSourceReturns;};field<void(*)()>(original,0x2e8)=&awardsReturned;
    const auto compareView=[&](View& view,void* native) {
        const auto& state=view.state();compareFloat(state.transparency,field<float>(native,8),"Awards transparency");compareFloat(state.transparencySpeed,field<float>(native,12),"Awards speed");
        require(state.transition==field<unsigned>(native,16) && state.shown==bool(field<unsigned char>(native,0x24)) && state.parentShown==bool(field<unsigned char>(native,0x25)) && state.enabled==bool(field<unsigned char>(native,0x26)) && state.parentEnabled==bool(field<unsigned char>(native,0x27)),"Awards View state differs");
    };
    const auto check=[&] {
        stage=151;compareView(*screen,nativeView);compareView(*parent,nativeParent);awardsDecorations+=compareDecorations(*screen,nativeView);compareControl(screen->returnButton(),field<void*>(original,0x3c));awardsLetters+=compare(screen->returnButton().text(),field<void*>(field<void*>(original,0x3c),0x27c));
        compareControl(screen->caption(),field<void*>(original,0x40));awardsLetters+=compare(screen->caption(),field<void*>(original,0x40));
        for(unsigned i=0;i<21;++i) {
            auto& button=screen->button(i);auto* nativeButton=field<void*>(original,0x44+i*4);compareControl(button,nativeButton);
            require(button.owned()==field<unsigned char>(nativeButton,0x280) && button.index()==field<unsigned>(nativeButton,0x26c),"AwardButton owned/index differs");
            require(unsigned(button.buttonState())==field<unsigned>(nativeButton,0x238),"AwardButton input state differs");
            require(button.normalTexture()==field<cTexture*>(nativeButton,0x270) && button.disabledTexture()==field<cTexture*>(nativeButton,0x274),"AwardButton retained textures differ");
            require(button.renderFlags()==field<unsigned>(nativeButton,0x6c),"AwardButton flags differ");
            require(button.dirty()==bool(field<unsigned char>(nativeButton,0x1b0)),"AwardButton dirty differs");
            button.updateTransform();call(0xf0b70,nativeButton);require(!std::memcmp(&button.sprite(),static_cast<unsigned char*>(nativeButton)+0x70,76),"AwardButton sprite differs");
            require(screen->description(i).view()==field<WString>(original,0x98+i*28).view(),"Awards description differs");
            auto& tip=button.tooltip();auto* nativeTip=field<void*>(nativeButton,0x278);compareControl(tip,nativeTip);compareFloat(tip.fadeDirection,field<float>(nativeTip,0x244),"AwardButton tooltip fade");
            tip.updateTransform();call(0xf0b70,nativeTip);require(!std::memcmp(&tip.sprite(),static_cast<unsigned char*>(nativeTip)+0x70,76),"Awards tooltip sprite differs");
            auto* nativeText=field<void*>(nativeTip,0x238);tip.text().enabledStep();call(0xf8e10,nativeText);compareControl(tip.text(),nativeText);
            require(tip.text().string().view()==field<WString>(nativeText,0x23c).view(),"AwardButton tooltip text differs");
            auto* head=field<Link*>(nativeText,0x230);auto* node=head->next;require(tip.text().lines().size()==field<unsigned>(nativeText,0x234),"Awards tooltip line count differs");
            for(auto* line:tip.text().lines()) {require(node!=head,"Awards tooltip line ended early");compareControl(*line,node->control);awardsLetters+=compare(*line,node->control);node=node->next;}require(node==head,"Awards tooltip line ended late");
            auto& effect=button.effect();auto* nativeEffect=field<void*>(nativeButton,0x27c);compareControl(effect,nativeEffect);
            require(effect.effectState().phase==field<unsigned>(nativeEffect,0x238) && effect.effectState().hoverAnimation==bool(field<unsigned char>(nativeEffect,0x260)),"Awards effect phase/hover differs");compareFloat(effect.effectState().scale,field<float>(nativeEffect,0x250),"Awards effect scale");
            effect.updateTransform();call(0xf0b70,nativeEffect);require(!std::memcmp(&effect.sprite(),static_cast<unsigned char*>(nativeEffect)+0x70,76),"Awards effect sprite differs");++awardsStates;
        }
    };check();
    for(action=0;action<84;++action) {
        stage=152;const unsigned index=action%21;auto& button=screen->button(index);auto* nativeButton=field<void*>(original,0x44+index*4);
        switch(action/21) {
        case 0: {const auto before=button.buttonState();const auto nativeBefore=field<unsigned>(nativeButton,0x238);const unsigned char owned=action%3 ? 1:252;button.setOwned(owned);function<void(__thiscall*)(void*,unsigned char)>(0x73150)(nativeButton,owned);require(button.buttonState()==before && field<unsigned>(nativeButton,0x238)==nativeBefore,"Award ownership overwrote input state");break;}
        case 1:button.hovered();call(0x73520,nativeButton);break;
        case 2:button.unhovered();call(0x73940,nativeButton);break;
        case 3:button.disable();call(0xd3750,nativeButton);button.enable();call(0xd3bb0,nativeButton);break;
        }check();
    }
    GameCursor frameCursor(window,CursorServices{&GetWindowInfo,&menuCursorPosition});menuCursor=&frameCursor;
    std::array<unsigned char,24> logicFrame{};field<void*>(logicFrame.data(),0)=cursorTable.data();nativeCursorFrame=logicFrame.data();putPointer(0x25ff60,nativeCursorFrame);inputTable[11]=reinterpret_cast<void*>(&menuNativeInput);
    for(unsigned frame=0;frame<24;++frame) {
        stage=153;action=frame;menuMilliseconds+=25;menuInputEvents.clear();menuMouseDelta={0,0,0};
        if(frame<21) {auto& button=screen->button(frame);menuPoint={button.geometry().x,button.geometry().y};menuMouseDelta={1,0,0};}
        else {menuPoint={790,590};menuMouseDelta={1,0,0};}
        if(frame==21) menuInputEvents={0x208};if(frame==22) menuInputEvents={0x200};
        awardsPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++awardsFrames;check();
    }
    std::vector<Sound*> overwrittenSounds;
    // Source ReturnButton loads two diagnostic sounds before native construction;
    // native construction loads 21 effect sounds, then its two ReturnButton sounds.
    // AwardButton XML reload reloads its tooltip, not its already-owned effect.
    require(sounds.size()==firstSound+25,"Awards constructor sound fixture count differs");
    for(auto i:{firstSound,firstSound+1,sounds.size()-2,sounds.size()-1}) overwrittenSounds.push_back(sounds[i].get());
    stage=154;const auto beforeReload=textureCounts();screen->loadXML(definition);const auto middleReload=textureCounts();function<void(__thiscall*)(void*,void*)>(0x763a0)(nativeView,wrap(&definition));compareReferenceDeltas(beforeReload,middleReload);check();++awardsReloads;
    // Show refresh uses the real profile map, independently of previous ownership.
    for(unsigned i=0;i<21;++i) profileProperties().booleans["awardShow"+std::to_string(i)]=bool((i+trial)&1);
    screen->hide();call(0xd3720,nativeView);screen->show();call(0xd37d0,nativeView);check();
    // Actual recovered/native Awards factories create the gameplay panel for
    // each borrowed effect, rather than substituting a generic View fixture.
    for(unsigned index=0;index<21;++index) {
        screen->enable();call(0xd3bb0,nativeView);
        auto* nativeEffect=field<void*>(field<void*>(original,0x44+index*4),0x27c);auto* panel=screen->showAwardPanel(screen->button(index).effect());
        lastNativeAwardPanel=nullptr;function<void(__thiscall*)(void*,void*,void(*)())>(0x783d0)(original,nativeEffect,nullptr);auto* nativePanel=lastNativeAwardPanel;require(nativePanel,"Awards factory did not allocate native panel");
        compareControl(*panel,nativePanel);require(panel->state().transition==field<unsigned>(nativePanel,0x10) && panel->state().transition==1,"Awards panel show transition differs");
        auto* nativePanelButton=field<void*>(nativePanel,0x238);require(screen->pendingEffect==&screen->button(index).effect() && field<void*>(original,0x2e4)==nativeEffect,"Awards factory pending effect differs");
        const auto* nativeKey=function<const String*(__cdecl*)(unsigned)>(0xab50)(index);require(Awards::conditionKey(index).view()==nativeKey->view(),"Awards condition key differs");
        profileProperties().booleans[std::string(nativeKey->view())]=false;
        panel->okButton().clickEvent(&panel->okButton());require(profileProperties().booleans.at(std::string(nativeKey->view())),"Awards source condition commit missing");
        profileProperties().booleans[std::string(nativeKey->view())]=false;function<void(__cdecl*)(void*)>(0x77e80)(nativePanelButton);
        require(profileProperties().booleans.at(std::string(nativeKey->view())) && !screen->pendingEffect && !field<void*>(original,0x2e4),"Awards native condition/pending clear differs");
        require(views.discarded().size()==1 && nativeDiscards.size()==1 && views.discarded().at(panel).view()==nativeDiscards.at(nativePanel),"Awards panel discard origin differs");
        require(!screen->state().enabled && !field<unsigned char>(nativeView,0x26),"Awards panel callback disable differs");check();++awardsPanelCallbacks;
        const auto beforePanelDelete=textureCounts();views.deleteDiscarded();const auto middlePanelDelete=textureCounts();call(0x75300,nativePanel);compareReferenceDeltas(beforePanelDelete,middlePanelDelete);nativeDiscards.clear();
        screen->markOwned(index);function<void(__thiscall*)(void*,unsigned)>(0x76ec0)(original,index);require(screen->button(index).owned()==1 && field<unsigned char>(field<void*>(original,0x44+index*4),0x280)==1,"Awards mark-owned helper differs");
    }
    screen->enable();call(0xd3bb0,nativeView);
    stage=155;
    for(unsigned frame=0;frame<2;++frame) {
        action=24+frame;menuMilliseconds+=25;menuPoint={screen->returnButton().geometry().x,screen->returnButton().geometry().y};menuMouseDelta={1,0,0};menuInputEvents={frame ? 0x200U:0x208U};
        awardsPixels+=compareMenuFrame(d3d,device,window,views,controls,input,0);++awardsFrames;
    }
    menuCursor=nullptr;nativeCursorFrame=nullptr;putPointer(0x25ff60,&cursorPointer);inputTable[11]=reinterpret_cast<void*>(&frameService);
    require(awardsSourceReturns==trial+1 && awardsNativeReturns==trial+1,"Awards return callback differs");
    require(screen->state().transition==field<unsigned>(nativeView,0x10) && !screen->state().enabled && !field<unsigned char>(nativeView,0x26),"Awards return transition/disable differs");
    const auto beforeDelete=textureCounts();screen.reset();const auto middleDelete=textureCounts();call(0x78dc0,original);compareReferenceDeltas(beforeDelete,middleDelete);require(!Awards::instance() && !field<void*>(image,0x25ffcc),"Awards singleton destruction differs");++awardsObjects;
    parent.reset();call(0xfafc0,nativeParent);
    for(auto* sound:overwrittenSounds) {
        if(sound->referenceCount()!=1) std::cerr<<"Awards retained sound "<<sound->resourceName().c_str()<<" references "<<sound->referenceCount()<<" prior resources "<<overwrittenSounds.size()<<'\n';
        require(sound->referenceCount()==1,"Awards XML sound overwrite does not match original retained reference");
        ++awardsSoundResidues;sound->releaseReference(); // Diagnostic cleanup after both retained references were verified.
    }
}
require(awardsExitRegistrations==1,"Awards indexed keys initialized repeatedly");
loadingDocuments.clear();loadingFiles=nullptr;profileProperties().booleans.clear();
