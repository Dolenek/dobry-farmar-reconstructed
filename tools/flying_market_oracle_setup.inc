stage=560;write(0xef00,flyingExpenseEntry.data(),5);redirect(0x7d0b0,reinterpret_cast<void*>(&nativeDiscard));
marketFlyingSource.clear();marketFlyingNative.clear();nativeFlyingAllocation=[](void* pointer) {marketFlyingNative.push_back(pointer);};
flyingMarketCheck=[&] {
    stage=561;require(marketFlyingSource.size()==marketFlyingNative.size(),"Market actual expense object counts differ");
    for(unsigned i=0;i<marketFlyingSource.size();++i) {
        auto& effect=*marketFlyingSource[i];auto* native=marketFlyingNative[i];mainMenuCompareView(effect,native);compareControl(effect,native);
        require(effect.parent()==&game && field<void*>(native,0x20)==nativeGame,"Market effect owner differs");
        if(effect.textDirty()) effect.makeLetters();compare(effect,native);compareControl(*effect.icon(),field<void*>(native,0x2f4));
        compareFloat(effect.distance,field<float>(native,0x2c0),"Market expense distance");compareFloat(effect.totalDistance,field<float>(native,0x2bc),"Market expense path");
        require(effect.positionIndex==field<unsigned>(native,0x2c4) && effect.alphaIndex==field<unsigned>(native,0x2c8),"Market expense indices differ");
        require(!std::memcmp(effect.positions().data(),field<void*>(native,0x2d8),effect.positions().size()*20),"Market expense position records differ");
        require(!std::memcmp(effect.alphas().data(),field<void*>(native,0x2e8),effect.alphas().size()*12),"Market expense alpha records differ");
        require(views.discarded().count(&effect)==nativeDiscards.count(native),"Market expense deferred deletion differs");
    }++flyingMarketStates;
};
