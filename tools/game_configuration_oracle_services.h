// Explicit old-CRT formatting and typed XML boundary. Original cGame loaders
// execute; native full cGame constructor/world is not called by this diagnostic.
namespace {
String gameConfigurationKey;
String* __cdecl gameConfigurationLevel(const char* prefix,unsigned level) {
    gameConfigurationKey=String(std::string(prefix)+std::to_string(level));return &gameConfigurationKey;
}
unsigned __fastcall gameConfigurationUnsigned(XmlAdapter* self,void*) {
    require(!self->node->text().empty(),"Empty cost text is undefined in original typed getter");
    unsigned value=0;std::wistringstream stream(self->node->text().c_str());stream>>value;
    require(!stream.fail(),"Invalid cost text is outside this diagnostic scope");return value;
}
}
