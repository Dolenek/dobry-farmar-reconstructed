namespace {
// Time string/old-CRT boundary is separate from the native fromSeconds oracle.
// This parses original data independently; it does not call GameplayTime code.
void __fastcall gameplayTimeAttribute(void* output,void*,XmlAdapter* node,const wchar_t* name) {
    const auto text=toNarrow(node->node->attribute(WString(name)));std::istringstream stream(std::string(text.view()));
    std::array<unsigned,3> value{};stream>>value[0];require(stream.get()==':',"Native time service first colon missing");
    stream>>value[1];require(stream.get()==':',"Native time service second colon missing");stream>>value[2];
    std::memcpy(output,value.data(),12);
}
}
