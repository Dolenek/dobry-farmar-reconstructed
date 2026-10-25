// The actual HelpView lives in the actual GamePauseContext. Both page changes
// and close execute through the regular input callbacks and whole UI frames.
const auto checkHelp=[&] {
    stage=920;require(HelpView::instance()==actualHelp.get() && field<void*>(image,0x25f778)==nativeHelp,"Help singleton differs");
    mainMenuCompareView(*actualHelp,nativeHelp);compareDecorations(*actualHelp,nativeHelp);
    require(actualHelp->currentPage()==field<unsigned>(nativeHelp,0x48),"Help current page differs");
    const auto first=field<void**>(nativeHelp,0x50),last=field<void**>(nativeHelp,0x54);
    require(actualHelp->pages().size()==std::size_t(last-first),"Help page count differs");
    for(auto entry:std::initializer_list<std::pair<TextAnimationButton*,unsigned>>{{&actualHelp->okButton(),0x3c},{&actualHelp->nextButton(),0x40},{&actualHelp->previousButton(),0x44}}) {
        auto* n=field<void*>(nativeHelp,entry.second);mainMenuCompareView(*entry.first,n);compareControl(*entry.first,n);compare(entry.first->text(),field<void*>(n,0x27c));
        require(unsigned(entry.first->buttonState())==field<unsigned>(n,0x238),"Help button state differs");
    }
    for(unsigned i=0;i<actualHelp->pages().size();++i) {
        auto& page=*actualHelp->pages()[i];auto* np=first[i];mainMenuCompareView(page,np);
        auto* head=field<Link*>(np,0x30);auto* link=head->next;require(page.pictures().size()==field<unsigned>(np,0x34),"Help picture count differs");
        for(auto* picture:page.pictures()) {require(link!=head,"Help pictures ended early");mainMenuCompareView(*picture,link->control);compareControl(*picture,link->control);link=link->next;}
        require(link==head,"Help pictures ended late");head=field<Link*>(np,0x3c);link=head->next;require(page.texts().size()==field<unsigned>(np,0x40),"Help text count differs");
        for(auto* text:page.texts()) {
            require(link!=head,"Help texts ended early");auto* nt=link->control;mainMenuCompareView(*text,nt);compareControl(*text,nt);
            require(text->string().view()==field<WString>(nt,0x23c).view(),"Help localized content differs");
            text->enabledStep();call(0xf8e10,nt);auto* th=field<Link*>(nt,0x230);auto* tl=th->next;
            require(text->lines().size()==field<unsigned>(nt,0x234),"Help generated line count differs");
            for(auto* line:text->lines()) {require(tl!=th,"Help lines ended early");compare(*line,tl->control);tl=tl->next;}
            require(tl==th,"Help lines ended late");link=link->next;
        }
        require(link==head,"Help texts ended late");
    }
    ++helpStates;
};
checkHelp();require(actualHelp->pages().size()==2,"Shipped Help should have two pages");
click(pause->helpButton());finishTransitions();check();checkHelp();
for(unsigned cycle=0;cycle<4;++cycle) {
    click(actualHelp->nextButton());++helpActions;checkHelp();require(actualHelp->currentPage()==1 && !actualHelp->nextButton().state().shown,"Help Next did not reach final page");
    click(actualHelp->previousButton());++helpActions;checkHelp();require(actualHelp->currentPage()==0 && !actualHelp->previousButton().state().shown,"Help Previous did not reach first page");
}
click(actualHelp->okButton());++helpActions;finishTransitions();check();checkHelp();
require(pause->state().shown && !game.state().enabled,"Help close did not return to actual paused menu");
// Real reload replaces the indexed page vector while the old View-owned pages
// remain children. This retained graph is deliberate original behavior.
XmlDocument helpDefinition(files,String("UI/HelpView.xml"));
actualHelp->loadXML(helpDefinition);function<void(__thiscall*)(void*,void*)>(0x7f370)(nativeHelp,wrap(&helpDefinition));++helpActions;checkHelp();
click(pause->helpButton());finishTransitions();checkHelp();click(actualHelp->nextButton());++helpActions;checkHelp();
click(actualHelp->okButton());++helpActions;finishTransitions();check();checkHelp();
