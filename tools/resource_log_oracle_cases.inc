stage=180;
LoadingDisabledAudio disabledBackend;SoundManager disabledSound(files,settings,disabledBackend,0);
const auto common=std::filesystem::path(".local")/(std::string(preserve ? "resource-log-pc53-":"resource-log-pc24-")+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
std::filesystem::create_directories(common/"source");std::filesystem::create_directories(common/"native");
files.mountCommon(String((common/"source").generic_string()));
FileIndex nativeFiles(toNarrow(WString(argv[2])),String("pack"));nativeFiles.mountCommon(String((common/"native").generic_string()));
resourceLogFiles=&nativeFiles;
resourceLogFileTable[0]=reinterpret_cast<void*>(&resourceLogDelete);resourceLogFileTable[4]=reinterpret_cast<void*>(&resourceLogWrite);
std::array<void*,5> writerFileTable{};writerFileTable[4]=reinterpret_cast<void*>(&resourceLogCreate);
auto* writerFilePointer=writerFileTable.data();putPointer(0x25f72c,&writerFilePointer);
redirect(0x87d0,reinterpret_cast<void*>(&resourceLogConstruct));redirect(0xae240,reinterpret_cast<void*>(&resourceLogLiteral));
redirect(0x1d680,reinterpret_cast<void*>(&resourceLogNarrow));redirect(0xa8be0,reinterpret_cast<void*>(&resourceLogNumber));
redirect(0xa430,reinterpret_cast<void*>(&resourceLogContents));
std::map<std::string,cResource*> cache;
const auto previousDestroyed=resourceDestroyedCallback();
resourceDestroyedCallback()=[&](cResource* resource) {
    cache.erase(std::string(resource->resourceName().view()));textureResources.erase(reinterpret_cast<cTexture*>(resource));previousDestroyed(resource);
};
const auto add=[&](const String& name,unsigned type) {
    cResource* resource=nullptr;
    switch(type) {
    case 0:resource=manager.getTexture(name);break;
    case 1:resource=manager.getXML(name);break;
    case 2:resource=manager.getFont(name);break;
    case 3:resource=manager.getShader(name);break;
    case 4:resource=manager.getSound(name);break;
    default:throw std::runtime_error("Unexpected shipped resource log type");
    }
    cache[std::string(manager.getResourceName(resource).view())]=resource;return resource;
};
shaderEnvironment().files=&files;
shaderEnvironment().create=[&](const DWORD* code,const unsigned long&,DWORD& shader) {
    createSpriteVertexShader(device,code,shader);require(shader!=0,"Resource log real shader creation failed");
};
shaderEnvironment().remove=[&](const DWORD& shader) { require(SUCCEEDED(device->DeleteVertexShader(shader)),"Resource log shader deletion failed"); };
const auto read=[&](FileIndex& index,const String& name) {
    auto file=index.open(name);std::vector<unsigned char> bytes(file->size());require(file->read(bytes.data(),unsigned(bytes.size()))==bytes.size(),"Resource log file read differs");return bytes;
};
const auto check=[&](const char* output) {
    stage=181;require(cache.size()==manager.cachedCount(),"Resource log cache fixture incomplete");
    std::vector<ResourceLogNode> nodes(cache.size());unsigned i=0;
    for(const auto& entry:cache) { nodes[i].name=String(entry.first);nodes[i++].resource=entry.second; }
    ResourceLogNode head;head.nil=head.color=1;head.parent=resourceLogTree(nodes,0,unsigned(nodes.size()),&head,&head);
    head.left=nodes.empty() ? &head:&nodes.front();head.right=nodes.empty() ? &head:&nodes.back();
    std::array<unsigned char,40> original{};field<void*>(original.data(),8)=&head;
    const String name(output);manager.logResources(name);
    function<void(__thiscall*)(void*,const String*)>(0x13fdf0)(original.data(),&name);
    const auto actual=read(files,name),expected=read(nativeFiles,name);
    require(actual==expected && actual.size()%2==0 && actual[0]=='<' && actual[1]==0,"Resource log UTF-16 bytes/BOM differ");
    resourceLogUnits+=actual.size()/2;resourceLogEntries+=nodes.size();++resourceLogStates;
    // Creation removes the matching name from the source cache AFTER the
    // serialized snapshot. The detached resource is separately destroyed.
    cache.erase(std::string(name.view()));require(cache.size()==manager.cachedCount(),"Resource log output invalidation differs");
};
check("empty.xml");
for(const char* log:{"MenuResLog.xml","GameResLog.xml"}) {
    XmlDocument document(files,String(log));unsigned row=0;
    require(document.children().size()==(std::string(log)=="MenuResLog.xml" ? 148U:539U),"Resource log shipped population differs");
    for(const auto& child:document.children()) {
        const auto name=toNarrow(child.second->attribute(WString(L"Path")));
        unsigned type=0;std::wistringstream(child.second->attribute(WString(L"Type")).c_str())>>type;
        auto* resource=add(name,type);if(type==1) delete resource;else resource->releaseReference();
        if(++row%30==0) check("batch.xml");
    }
    check(log);manager.clearUseless();check("after-clear.xml");
}
// All five real resource implementations. The shipped logs omit shaders;
// use the already verified diagnostic VS1.1 grammar fixture on the real D3D8
// device, rather than inventing an original packed shader filename.
const DWORD code[]={0xfffe0101,20,0xc00f0000,0x90e40000,0xa0e40000,
    5,0xd00f0000,0x90e40005,0xa0e40004,1,0xe00f0000,0x90e40007,0x0000ffff};
{auto file=files.create(String("sprite-fixture.vs"));file->write(code,sizeof(code));}
add(String("sprite-fixture.vs"),3);++resourceLogShaders;
auto* xml=add(String("UI/MainMenu.xml"),1);
XmlDocument menuLog(files,String("MenuResLog.xml"));std::array<bool,5> found{};found[1]=found[3]=true;
for(const auto& child:menuLog.children()) {
    unsigned type=0;std::wistringstream(child.second->attribute(WString(L"Type")).c_str())>>type;
    if(!found[type]) { add(toNarrow(child.second->attribute(WString(L"Path"))),type);found[type]=true; }
}
for(bool type:found) require(type,"Resource log real resource type absent");
check("five-types.xml");
check("ui/mainmenu.xml");delete xml;check("after-output-invalidation.xml");
// Original raw XML writer does not escape markup in filenames. Retain that
// behavior for an actual custom XML resource and verify it byte for byte.
const String special("fixture&.xml");const wchar_t xmlText[]=L"<Fixture/>";
{ auto file=files.create(special);file->write(xmlText,sizeof(xmlText)-2); }
auto* markup=add(special,1);check("unescaped.xml");delete markup;
for(const auto& entry:cache) if(entry.second->resourceType()==3) entry.second->preResetGraphics();
std::vector<cResource*> remaining;for(const auto& entry:cache) remaining.push_back(entry.second);
for(auto* resource:remaining) delete resource;check("final-empty.xml");
require(resourceLogCreates==resourceLogStates && resourceLogWrites==resourceLogStates && resourceLogDeletes==resourceLogStates,"Resource log file lifetime differs");
resourceDestroyedCallback()=previousDestroyed;resourceLogFiles=nullptr;shaderEnvironment()={};
