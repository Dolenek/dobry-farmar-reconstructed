stage=500;for(const auto& entry:warehouseTipEntries) write(entry.first,entry.second.data(),5);
putPointer(0x25ffc4,nullptr);detail=std::make_unique<WarehouseTip>(nullptr,detailContext);
nativeDetail=allocate(884);std::memset(nativeDetail,0,884);function<void*(__thiscall*)(void*,void*)>(0x658b0)(nativeDetail,nullptr);
// Original +36c is not initialized by the ctor. Seed both diagnostic words
// explicitly before any absolute dispatch; this does not claim allocator parity.
detail->remainingVisible=field<float>(nativeDetail,0x36c)=0;
hudUseActualWarehouseTip=true;
warehouseTipCheck=[&] {
    stage=501;require(WarehouseTip::instance()==detail.get() && field<void*>(image,0x25ffc4)==nativeDetail,"WarehouseTip singleton differs");
    mainMenuCompareView(*detail,nativeDetail);compareControl(*detail,nativeDetail);compareControl(detail->origin(),field<void*>(nativeDetail,0x26c));
    require(detail->productCount==field<unsigned>(nativeDetail,0x364) && detail->opened==bool(field<unsigned char>(nativeDetail,0x370)),"WarehouseTip count/open differs");
    compareFloat(detail->visibleTime,field<float>(nativeDetail,0x368),"WarehouseTip VisibleTime");compareFloat(detail->remainingVisible,field<float>(nativeDetail,0x36c),"WarehouseTip timer");
    require(detail->movement==field<unsigned>(nativeDetail,0x248),"WarehouseTip movement differs");compareFloat(detail->moveSpeed,field<float>(nativeDetail,0x254),"WarehouseTip movement speed");compareFloat(detail->remaining,field<float>(nativeDetail,0x258),"WarehouseTip movement remaining");
    require(!std::memcmp(detail->from.data(),static_cast<unsigned char*>(nativeDetail)+0x238,8) && !std::memcmp(detail->to.data(),static_cast<unsigned char*>(nativeDetail)+0x240,8) && !std::memcmp(detail->velocity.data(),static_cast<unsigned char*>(nativeDetail)+0x24c,8),"WarehouseTip movement vectors differ");
    for(unsigned row=0;row<detail->productCount;++row) {
        const auto& actual=detail->rows[row];auto* icon=field<void*>(nativeDetail,0x280+row*12);auto* quantity=field<void*>(nativeDetail,0x284+row*12);
        require(actual.iconType==field<unsigned>(nativeDetail,0x288+row*12),"WarehouseTip row icon type differs");compareControl(*actual.icon,icon);
        if(actual.quantity->textDirty()) actual.quantity->makeLetters();compare(*actual.quantity,quantity);mainMenuCompareView(*actual.icon,icon);mainMenuCompareView(*actual.quantity,quantity);
    }
    ++warehouseTipStates;
};warehouseTipCheck();
