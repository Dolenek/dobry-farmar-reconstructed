// Test-only 88-byte Profile service fixture. UI methods remain native instructions.
// Profile persistence itself executes the separately verified recovered lifecycle
// on the source side; this boundary independently records UI open/delete actions.
ProfileHeaders::Names selectorNames;
unsigned selectorNextID=1;
std::vector<std::pair<unsigned,std::wstring>> selectorEvents;
unsigned selectorStates=0,selectorLetters=0,selectorObjects=0,selectorCallbacks=0,selectorFrames=0,selectorDeletions=0,selectorServiceEvents=0;
unsigned long long selectorPixels=0;
std::vector<unsigned> actualSelectorCallbacks,nativeSelectorCallbacks;
void selectorAccepted() { actualSelectorCallbacks.push_back(1); }
void selectorCancelled() { actualSelectorCallbacks.push_back(2); }
void nativeSelectorAccepted() { nativeSelectorCallbacks.push_back(1); }
void nativeSelectorCancelled() { nativeSelectorCallbacks.push_back(2); }
void* __fastcall selectorProfileDestroy(void* self,void*,unsigned flags) {
    selectorEvents.emplace_back(2,std::wstring(field<WString>(self,0x24).view()));
    field<WString>(self,0x24).~WString();putPointer(0x25f738,nullptr);if(flags&1) release(self);return self;
}
std::array<void*,1> selectorProfileTable={reinterpret_cast<void*>(&selectorProfileDestroy)};
void* __fastcall selectorProfileConstruct(void* self,void*,const WString* name) {
    std::memset(self,0,88);field<void*>(self,0)=selectorProfileTable.data();new(static_cast<unsigned char*>(self)+0x24) WString(*name);
    selectorEvents.emplace_back(1,std::wstring(name->view()));
    if(selectorNames.empty()) selectorNextID=1; // Missing/removed profiles.xml starts fresh headers.
    if(!name->empty() && !selectorNames.count(*name)) selectorNames.emplace(*name,selectorNextID++);
    putPointer(0x25f738,self);return self;
}
void __fastcall selectorProfileNames(void*,void*,void* output) {
    auto* head=field<Link*>(output,4);
    for(const auto& item:selectorNames) {
        auto* node=static_cast<Link*>(allocate(36));node->next=head;node->previous=head->previous;
        new(reinterpret_cast<unsigned char*>(node)+8) WString(item.first);
        head->previous->next=node;head->previous=node;++field<unsigned>(output,8);
    }
}
void __fastcall selectorProfileRemove(void* self,void*,const WString* name) {
    selectorEvents.emplace_back(3,std::wstring(name->view()));
    if(selectorNames.erase(*name)) field<WString>(self,0x24)=selectorNames.empty() ? WString():selectorNames.begin()->first;
}
