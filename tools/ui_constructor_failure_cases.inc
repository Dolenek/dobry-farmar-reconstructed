// Source-only failure injection. Remove an in-memory index entry; no data file
// or archive is changed. Native exceptional CRT/throw ABI remains unclaimed.
for(const char* path:{"ui/helpview.xml","ui/gameui/journal.xml"}) {
    auto& index=const_cast<FileIndex::Table&>(files.files());auto at=index.find(path);require(at!=index.end(),"Constructor failure input missing");
    const auto entry=*at;index.erase(at);
    struct RestoreIndex {FileIndex::Table& table;std::string key,value;~RestoreIndex() {table[key]=value;}} restore{index,entry.first,entry.second};
    for(unsigned repeat=0;repeat<2;++repeat) {
        const auto enabled=views.enabled(),absolute=views.absoluteEnabled(),transitions=views.transitions();const auto textures=textureCounts();const auto cached=manager.cachedCount();
        const auto press=input.controls(0x208).size(),release=input.controls(0x200).size(),mouse=input.mouse().size(),keyboard=input.keyboard().size();
        auto* active=interaction.active;const auto pending=interaction.pending;bool failed=false;std::string error;
        try {
            if(path[3]=='h') {HelpViewContext c{manager,settings,strings,input,interaction,views,controls};auto value=std::make_unique<HelpView>(nullptr,c);}
            else {JournalContext c{manager,settings,strings,input,interaction,views,controls};auto value=std::make_unique<Journal>(nullptr,c);}
        } catch(const std::exception& e) {failed=true;error=e.what();} catch(const char* e) {failed=true;error=e;}
        require(failed && error.find(path)!=std::string::npos && error.find("not found")!=std::string::npos,"Constructor did not reach expected missing XML failure");
        require(!HelpView::instance() && !Journal::instance(),"Failed UI constructor retained singleton");
        require(views.enabled()==enabled && views.absoluteEnabled()==absolute && views.transitions()==transitions,"Failed UI constructor retained active Views");
        require(input.controls(0x208).size()==press && input.controls(0x200).size()==release && input.mouse().size()==mouse && input.keyboard().size()==keyboard,"Failed UI constructor retained input");
        require(interaction.active==active && interaction.pending==pending && manager.cachedCount()==cached && textureCounts()==textures,"Failed UI constructor retained interaction/resources");
        ++uiConstructorFailureChecks;
    }
}
