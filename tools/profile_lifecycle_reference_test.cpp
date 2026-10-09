#define FARMAR_PROFILE_LIFECYCLE
#include "profile_properties_oracle.h"
#include "Profile.h"
#include "GameProfileDefaults.h"
#include "FileIndex.h"
#include "Settings.h"
#include "XmlDocument.h"
#include <iomanip>
#include <vector>
namespace {
FileIndex* nativeFiles=nullptr;Settings* nativeSettings=nullptr;
unsigned nativeInitializations=0,actualInitializations=0,nativeXmlDeletes=0,nativeXmlLoads=0,actualXmlLoads=0,writes=0,objects=0,totalInitializations=0;
unsigned removals=0,removedFiles=0;
struct Event { std::string name;std::wstring text;bool operator==(const Event& other) const { return name==other.name && text==other.text; } };
std::vector<Event> expectedWrites,actualWrites;std::vector<std::string> expectedInvalidations,actualInvalidations;
std::vector<std::string> expectedRemovals;
std::vector<std::shared_ptr<XmlDocument>> nativeDocuments;
struct XmlAdapter { void** table;const XmlNode* node; };
std::array<void*,35> xmlTable{};std::map<const XmlNode*,XmlAdapter> adapters;
XmlAdapter* wrap(const XmlNode* node) { auto& adapter=adapters[node];adapter={xmlTable.data(),node};return &adapter; }
WString* __fastcall attribute(XmlAdapter* self,void*,WString* result,const wchar_t* name) { return new(result) WString(self->node->attribute(WString(name))); }
unsigned __fastcall childCount(XmlAdapter* self,void*) { return unsigned(self->node->children().size()); }
bool __fastcall child(XmlAdapter* self,void*,unsigned index,XmlAdapter** result,String* name) { XmlNode* node=nullptr;if(!self->node->section(index,node,name)) return false;*result=wrap(node);return true; }
void __fastcall deleteXML(XmlAdapter*,void*) { ++nativeXmlDeletes; }
void* __fastcall loadXML(void*,void*,const String* filename) { ++nativeXmlLoads;auto doc=std::make_shared<XmlDocument>(*nativeFiles,*filename);nativeDocuments.push_back(doc);return wrap(doc.get()); }
bool __fastcall exists(void*,void*,const String* filename) { return nativeFiles->isExist(*filename); }
bool __fastcall removeNativeFile(void*,void*,const String* filename) { expectedRemovals.emplace_back(filename->view());return nativeFiles->removeCommon(*filename); }
void __fastcall enumerateNativeFiles(void*,void*,const String* prefix,void* output) {
    std::vector<String> names;nativeFiles->getFilesNames(*prefix,names);auto* head=field<void*>(output,4);
    for(const auto& name:names) {
        auto* node=allocate(36);auto* tail=field<void*>(head,4);field<void*>(node,0)=head;field<void*>(node,4)=tail;
        new(static_cast<unsigned char*>(node)+8) String(name);field<void*>(tail,0)=node;field<void*>(head,4)=node;++field<unsigned>(output,8);
    }
}
unsigned __cdecl wideLength(const wchar_t* value) { return unsigned(std::wcslen(value)); }
WString* __fastcall assignWideBytes(WString* self,void*,const wchar_t* value,unsigned size) { self->assign(value,size);return self; }
void __fastcall invalidate(void*,void*,const String* filename) { expectedInvalidations.emplace_back(filename->view()); }
String* __cdecl directory(String* result) {
    String text;nativeSettings->get(String("game"),String("profiles"),text);std::string path(text.view());if(!path.empty() && path.back()!='/') path+='/';return new(result) String(path);
}
String* __cdecl concatString(String* result,const String* left,const String* right) { return new(result) String(std::string(left->view())+std::string(right->view())); }
String* __cdecl concatLiteral(String* result,const String* left,const char* right) { return new(result) String(std::string(left->view())+right); }
int __fastcall compareWide(WString* self,void*,unsigned first,unsigned count,const wchar_t* text,unsigned size) { return self->compare(first,count,text,size); }
String* __cdecl formatID(String* output,unsigned id) { return new(output) String(formatProfileID(id)); }
void __cdecl initializeNative() {
    ++nativeInitializations;++totalInitializations;function<void(__cdecl*)()>(0x11390)();
    unsigned count=0;for(const auto& store:stores) count+=field<unsigned>(image,store.base+8);
    require(count==790,"Original game profile default count differs");
}
String indexedBuffer;
String* __cdecl indexedKey(const char* prefix,unsigned index) { indexedBuffer=String(std::string(prefix)+std::to_string(index));return &indexedBuffer; }
String* __cdecl tutorialKey(String* output,unsigned index) { return new(output) String(std::string("tip")+std::to_string(index)); }
void __fastcall streamConstruct(void* self,void*,int,int) { expectedStream=static_cast<unsigned char*>(self)+8;nativeOutput.str(L"");nativeOutput.clear(); }
void __fastcall streamDestroy(void*,void*) {}
WString* __fastcall streamContents(void*,void*,WString* output) { return new(output) WString(nativeOutput.str()); }
void* __cdecl writeNarrow(void* self,const char* literal) { require(self==expectedStream,"Header output stream differs");while(*literal) nativeOutput<<wchar_t(static_cast<unsigned char>(*literal++));return self; }
unsigned __cdecl writeFile(const String* filename,const WString* contents) {
    expectedWrites.push_back({std::string(filename->view()),std::wstring(contents->view())});auto file=nativeFiles->create(*filename);
    const wchar_t bom=0xfeff;file->write(&bom,2);file->write(contents->data(),contents->size()*2);return contents->size()*2+2;
}
void writeFixture(FileIndex& files,const String& name,const std::wstring& text) { auto file=files.create(name);const wchar_t bom=0xfeff;file->write(&bom,2);file->write(text.data(),unsigned(text.size()*2)); }
std::wstring readFixture(FileIndex& files,const String& name) {
    auto file=files.open(name);std::wstring text(file->size()/2,L'\0');require(file->read(text.data(),file->size())==file->size(),"Profile file read differs");require(!text.empty() && text[0]==0xfeff,"Profile file BOM differs");return text.substr(1);
}
void collectHeaders(void* node,ProfileHeaders::Names& result) {
    if(field<unsigned char>(node,45)) return;collectHeaders(field<void*>(node,0),result);result.emplace(field<WString>(node,12),field<unsigned>(node,40));collectHeaders(field<void*>(node,8),result);
}
void compareProfile(const Profile& actual,void* object) {
    const auto& a=actual.headers();
    if(a.nextID()!=field<unsigned>(object,68) || a.nowPlaying()!=bool(field<unsigned char>(object,4)) || a.dirty()!=bool(field<unsigned char>(object,84)))
        std::cerr<<"Scalars actual "<<a.nextID()<<','<<a.nowPlaying()<<','<<a.dirty()<<" native "<<field<unsigned>(object,68)<<','<<unsigned(field<unsigned char>(object,4))<<','<<unsigned(field<unsigned char>(object,84))<<'\n';
    require(a.nextID()==field<unsigned>(object,68) && a.nowPlaying()==bool(field<unsigned char>(object,4)) && a.dirty()==bool(field<unsigned char>(object,84)),"Profile lifecycle scalars differ");
    require(a.currentName().view()==field<WString>(object,36).view() && a.storedCurrentName().view()==field<WString>(object,8).view(),"Profile selected/stored names differ");
    ProfileHeaders::Names expected;auto* head=field<void*>(object,76);collectHeaders(field<void*>(head,4),expected);require(a.names().size()==expected.size(),"Profile lifecycle names count differs");
    auto at=expected.begin();for(const auto& row:a.names()) { require(row.first.view()==at->first.view() && row.second==at->second,"Profile lifecycle names/IDs differ");++at; }
    checkAll(profileProperties());require(actualInitializations==nativeInitializations,"Profile initializer order/count differs");require(actualXmlLoads==nativeXmlLoads && nativeXmlDeletes==nativeXmlLoads,"Profile XML ownership differs");
    require(actualInvalidations==expectedInvalidations,"Profile cache invalidation order differs");
}
void compareFiles(FileIndex& actualFiles) {
    for(const auto& write:expectedWrites) actualWrites.push_back({write.name,readFixture(actualFiles,String(write.name))});
    // Call after each phase and clear its write list, so files are compared at the
    // point of the write rather than against a later destructor's replacement.
    require(actualWrites==expectedWrites,"Profile exact UTF-16 file contents or write filename differs");writes+=unsigned(expectedWrites.size());expectedWrites.clear();actualWrites.clear();
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc!=3 && argc!=4) return 2;auto reference=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);if(!reference) return 2;
    try {
        initializePropertyOracle(reference,argc==4);
        redirect(0x1050,reinterpret_cast<void*>(&compareWide));redirect(0xe1010,reinterpret_cast<void*>(&directory));
        redirect(0x1cc0,reinterpret_cast<void*>(&concatString));redirect(0xa470,reinterpret_cast<void*>(&concatLiteral));redirect(0xe17e0,reinterpret_cast<void*>(&formatID));
        redirect(0x87d0,reinterpret_cast<void*>(&streamConstruct));redirect(0x5710,reinterpret_cast<void*>(&streamDestroy));redirect(0x17f6f0,reinterpret_cast<void*>(&streamDestroy));
        redirect(0xa430,reinterpret_cast<void*>(&streamContents));redirect(0x1d680,reinterpret_cast<void*>(&writeNarrow));redirect(0x10ce30,reinterpret_cast<void*>(&writeFile));
        redirect(0xa670,reinterpret_cast<void*>(&indexedKey));redirect(0xcfbe0,reinterpret_cast<void*>(&tutorialKey));
        redirect(0x1802fd,reinterpret_cast<void*>(&wideLength));redirect(0x1ab0,reinterpret_cast<void*>(&assignWideBytes));
        field<void*>(image,0x260290)=reinterpret_cast<void*>(&initializeNative);
        xmlTable[0]=reinterpret_cast<void*>(&deleteXML);xmlTable[5]=reinterpret_cast<void*>(&child);xmlTable[31]=reinterpret_cast<void*>(&attribute);xmlTable[34]=reinterpret_cast<void*>(&childCount);
        std::array<void*,9> fileTable{},resourceTable{};fileTable[5]=reinterpret_cast<void*>(&removeNativeFile);fileTable[6]=reinterpret_cast<void*>(&exists);fileTable[8]=reinterpret_cast<void*>(&enumerateNativeFiles);
        resourceTable[6]=reinterpret_cast<void*>(&loadXML);resourceTable[7]=reinterpret_cast<void*>(&invalidate);
        void* fileObject=fileTable.data();void* resourceObject=resourceTable.data();field<void*>(image,0x25f72c)=&fileObject;field<void*>(image,0x25f730)=&resourceObject;
        const auto root=toNarrow(WString(argv[2]));
        for(scenario=0;scenario<80;++scenario) {
            const auto sub=std::string(root.view())+"/case-"+std::to_string(scenario);const String actualRoot(sub+"/actual"),nativeRoot(sub+"/native");
            FileIndex::createDirectories(String(std::string(actualRoot.view())+"/"));FileIndex::createDirectories(String(std::string(nativeRoot.view())+"/"));
            // Remove named diagnostic fixtures before index construction. Removing
            // an indexed common override would restore its stale original entry.
            for(const auto* dir:{&actualRoot,&nativeRoot}) for(const auto* name:{"Profiles/profiles.xml","Profiles/00000101.xml","Profiles/00000102.xml","Profiles/00000005.xml","Profiles/00000001.xml"})
                DeleteFileA((std::string(dir->view())+"/"+name).c_str());
            for(const auto* dir:{&actualRoot,&nativeRoot}) for(unsigned id:{1,5,101,102}) for(const char* suffix:{".sav",".sav.bak","0.xml",".pack"})
                DeleteFileA((std::string(dir->view())+"/Profiles/"+std::string(formatProfileID(id).view())+suffix).c_str());
            FileIndex actualFiles(actualRoot,String("pack")),referenceFiles(nativeRoot,String("pack"));actualFiles.mountCommon(actualRoot);referenceFiles.mountCommon(nativeRoot);nativeFiles=&referenceFiles;
            const unsigned mode=scenario%8;
            for(auto* files:{&actualFiles,&referenceFiles}) {
                writeFixture(*files,String("sys/settings.xml"),L"<main><game profiles=\"Profiles\"/></main>");
                if(mode!=0 && mode!=7) {
                    const wchar_t* current=mode==1 ? L"Alice":mode==2 ? L"Bob":mode==3 ? L"Ghost":mode==4 ? L"":L"Alice";
                    std::wstring xml=L"<Profiles NextID=\"5\" CurrentUser=\"";xml+=current;xml+=L"\" NowPlaying=\"0\"><Profile Name=\"Alice\" ID=\"101\"/><Profile Name=\"Bob\" ID=\"102\"/></Profiles>";
                    writeFixture(*files,String("Profiles/profiles.xml"),xml);
                    const std::wstring props=L"<Props><prop Type=\"Bool\" Name=\"active\" Val=\"1\"/><prop Type=\"Int\" Name=\"money\" Val=\"123456\"/><prop Type=\"WString\" Name=\"name\" Val=\"Žofie 日本語\"/><unknown Type=\"String\" Name=\"note\" Val=\"ASCII\"/><prop Type=\"unknown\" Name=\"ignored\" Val=\"0\"/></Props>";
                    writeFixture(*files,String("Profiles/00000101.xml"),props);writeFixture(*files,String("Profiles/00000102.xml"),props);
                }
            }
            Settings actualSettings(actualFiles),referenceSettings(referenceFiles);nativeSettings=&referenceSettings;
            actualFiles.onResourceInvalidated([](const String& name) { actualInvalidations.emplace_back(name.view()); });
            referenceFiles.onResourceInvalidated([](const String& name) { expectedInvalidations.emplace_back(name.view()); });
            actualInitializations=nativeInitializations=actualXmlLoads=nativeXmlLoads=nativeXmlDeletes=0;expectedWrites.clear();actualWrites.clear();expectedInvalidations.clear();actualInvalidations.clear();expectedRemovals.clear();
            profileProperties().clear();std::array<unsigned char,88> object{};
            require(Profile::headersExist(actualFiles,actualSettings)==function<bool(__cdecl*)()>(0xe14e0)(),"Startup header existence differs");
            const WString startup=Profile::storedName(actualFiles,actualSettings,[&](const String& name)->std::shared_ptr<const XmlNode> { ++actualXmlLoads;return std::make_shared<XmlDocument>(actualFiles,name); });
            alignas(WString) unsigned char nameStorage[sizeof(WString)];auto* nativeName=function<WString*(__cdecl*)(void*)>(0xe15e0)(nameStorage);
            require(startup.view()==nativeName->view(),"Startup stored current user differs");nativeName->~WString();
            const WString selected(mode==7 ? L"":mode==0 ? L"Requested":mode==3 ? L"Ghost":mode==6 ? L"Bob":L"Alice");
            auto* actual=new Profile(actualFiles,actualSettings,selected,[] { ++actualInitializations;initializeGameProfile(profileProperties()); },
                [&](const String& name) -> std::shared_ptr<const XmlNode> { ++actualXmlLoads;return std::make_shared<XmlDocument>(actualFiles,name); },
                [](const String& name) { actualInvalidations.emplace_back(name.view()); });
            function<void*(__thiscall*)(void*,const WString*)>(0xecb20)(object.data(),&selected);++objects;
            compareProfile(*actual,object.data());compareFiles(actualFiles);
            actual->saveProperties();function<void(__thiscall*)(void*)>(0xe6cf0)(object.data());compareFiles(actualFiles);
            if(mode==5) { actual->create();function<void(__thiscall*)(void*)>(0xea5d0)(object.data());actual->saveHeaders();function<void(__thiscall*)(void*)>(0xe3a70)(object.data());compareProfile(*actual,object.data());compareFiles(actualFiles); }
            if(mode==6) { actual->load();function<void(__thiscall*)(void*)>(0xec310)(object.data());compareProfile(*actual,object.data());compareFiles(actualFiles); }
            if(scenario>=40) {
            // ID prefix matching includes a longer ID and multiple save suffixes;
            // pack files and directories must survive the original enumerator.
            for(auto* files:{&actualFiles,&referenceFiles}) for(unsigned id:{1,5,101,102}) {
                for(const char* suffix:{".sav",".sav.bak","0.xml",".pack"}) writeFixture(*files,String("Profiles/"+std::string(formatProfileID(id).view())+suffix),L"fixture");
                FileIndex::createDirectories(String(std::string(files->root().view())+"/Profiles/"+std::string(formatProfileID(id).view())+".dir/"));
            }
            auto removeProfile=[&](const WString& name) {
                actual->remove(name);function<void(__thiscall*)(void*,const WString*)>(0xe8d20)(object.data(),&name);++removals;
                compareProfile(*actual,object.data());compareFiles(actualFiles);
                require(actualFiles.files()==referenceFiles.files(),"Profile deletion file index differs");
                for(const auto& removed:expectedRemovals) { String key(removed);key.lowercase();require(!actualFiles.isCommonExist(key) && !referenceFiles.isCommonExist(key),"Deleted common profile file remains indexed"); }
            };
            removeProfile(WString(L"Missing"));
            if(scenario%5==1) removeProfile(selected);
            if(scenario%5==2) removeProfile(WString(L"Bob"));
            if(scenario%5>=3) while(!actual->headers().names().empty()) {
                const WString name(scenario%5==3 ? actual->headers().names().begin()->first:actual->headers().names().rbegin()->first);removeProfile(name);
            }
            for(const auto& removed:expectedRemovals) require(GetFileAttributesA((std::string(actualRoot.view())+"/"+removed).c_str())==INVALID_FILE_ATTRIBUTES &&
                GetFileAttributesA((std::string(nativeRoot.view())+"/"+removed).c_str())==INVALID_FILE_ATTRIBUTES,"Profile deletion physical file remains");
            removedFiles+=unsigned(expectedRemovals.size());
            for(auto* files:{&actualFiles,&referenceFiles}) for(unsigned id:{1,5,101,102}) {
                const String pack("Profiles/"+std::string(formatProfileID(id).view())+".pack");
                require(files->isExist(pack) && GetFileAttributesA((std::string(files->root().view())+"/"+std::string(pack.view())).c_str())!=INVALID_FILE_ATTRIBUTES,"Profile removal deleted excluded pack fixture");
                const auto attributes=GetFileAttributesA((std::string(files->root().view())+"/Profiles/"+std::string(formatProfileID(id).view())+".dir").c_str());
                require(attributes!=INVALID_FILE_ATTRIBUTES && (attributes&FILE_ATTRIBUTE_DIRECTORY),"Profile removal deleted directory fixture");
            }
            }
            delete actual;function<void(__thiscall*)(void*)>(0xead60)(object.data());
            checkAll(profileProperties());require(!Profile::instance() && !field<void*>(image,0x25f738),"Profile singleton destructor cleanup differs");
            require(actualInitializations==nativeInitializations && actualInvalidations==expectedInvalidations,"Profile destructor callback/cache ordering differs");compareFiles(actualFiles);
            nativeDocuments.clear();adapters.clear();
        }
        for(auto& store:stores) release(store.head);require(allocations.empty(),"Profile lifecycle original heap allocation leaked");
        for(unsigned i=0;i<21;++i) field<String>(image,0x25f7b8+i*28).~String();
        std::cout<<"{\"constructors\":"<<objects<<",\"destructors\":"<<objects<<",\"utf16_writes\":"<<writes<<",\"map_comparisons\":"<<comparisons<<",\"game_initializations\":"<<totalInitializations<<",\"game_default_entries\":790,\"remove_calls\":"<<removals<<",\"removed_files\":"<<removedFiles<<"}\n";
        VirtualFree(image,0,MEM_RELEASE);FreeLibrary(reference);return 0;
    } catch(const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }
}
