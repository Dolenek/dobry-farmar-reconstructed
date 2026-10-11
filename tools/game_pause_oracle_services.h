namespace {
std::vector<std::string> pauseNativeGameDiscards,pauseSourceGameDiscards;
void __cdecl pauseNativeGameDiscard(void* game,const String* origin) {
    require(game==field<void*>(image,0x25f6ec),"Pause game discard pointer differs");pauseNativeGameDiscards.emplace_back(origin->view());
}
}
