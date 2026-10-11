// Original writer instructions, with explicit old-CRT stream and file adapters.
// The cache nodes are independently seeded ABI fixtures; this does not validate
// the original manager's container allocation or the original stream locale.
#include "Shader.h"
#include "XmlResource.h"
namespace {
std::wostringstream resourceLogStream;void* resourceLogStreamPointer=nullptr;
FileIndex* resourceLogFiles=nullptr;
unsigned resourceLogWrites=0,resourceLogCreates=0,resourceLogDeletes=0;
void __fastcall resourceLogConstruct(void* self,void*,int mode,int initialize) {
    require(mode==3 && initialize==1,"Resource log stream construction differs");
    resourceLogStreamPointer=static_cast<unsigned char*>(self)+8;
    resourceLogStream.str(L"");resourceLogStream.clear();
}
void* __cdecl resourceLogLiteral(void* self,const wchar_t* text) {
    require(self==resourceLogStreamPointer,"Resource log wide stream differs");resourceLogStream<<text;return self;
}
void* __cdecl resourceLogNarrow(void* self,const char* text) {
    require(self==resourceLogStreamPointer,"Resource log narrow stream differs");resourceLogStream<<text;return self;
}
void* __fastcall resourceLogNumber(void* self,void*,int value) {
    require(self==resourceLogStreamPointer,"Resource log signed number stream differs");resourceLogStream<<value;return self;
}
WString* __fastcall resourceLogContents(void* self,void*,WString* output) {
    require(static_cast<unsigned char*>(self)+8==resourceLogStreamPointer,"Resource log contents stream differs");
    return new(output) WString(resourceLogStream.str());
}
struct ResourceLogFile { void** table;std::unique_ptr<iFile> file; };
std::array<void*,6> resourceLogFileTable{};
void* __fastcall resourceLogDelete(ResourceLogFile* self,void*,unsigned flags) {
    require(flags==1,"Resource log deleting flags differ");++resourceLogDeletes;delete self;return nullptr;
}
unsigned __fastcall resourceLogWrite(ResourceLogFile* self,void*,const void* data,unsigned count) {
    require(count==resourceLogStream.str().size()*2 && !std::memcmp(data,resourceLogStream.str().data(),count),"Resource log raw write differs");
    ++resourceLogWrites;return self->file->write(data,count);
}
void* __fastcall resourceLogCreate(void*,void*,const String* name) {
    ++resourceLogCreates;return new ResourceLogFile{resourceLogFileTable.data(),resourceLogFiles->create(*name)};
}
struct ResourceLogNode {
    ResourceLogNode *left,*parent,*right;String name;cResource* resource=nullptr;
    unsigned char color=0,nil=0,padding[2]{};
};
static_assert(sizeof(ResourceLogNode)==48 && offsetof(ResourceLogNode,name)==12 && offsetof(ResourceLogNode,resource)==40);
ResourceLogNode* resourceLogTree(std::vector<ResourceLogNode>& nodes,unsigned first,unsigned end,ResourceLogNode* parent,ResourceLogNode* head) {
    if(first==end) return head;auto* node=&nodes[(first+end)/2];node->parent=parent;
    node->left=resourceLogTree(nodes,first,(first+end)/2,node,head);
    node->right=resourceLogTree(nodes,(first+end)/2+1,end,node,head);return node;
}
}
