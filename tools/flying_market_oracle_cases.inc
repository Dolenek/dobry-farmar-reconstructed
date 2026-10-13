stage=562;
// Money visual factories execute actual source/native cGame operations too.
// They do not replace the pending full setMoney world-refresh implementation.
for(unsigned amount:{0U,1U,99U,1000U,0xffffffffU,0x80000000U}) {
    marketFlyingSource.push_back(marketEffects.moneyExpense(amount));function<void(__thiscall*)(void*,unsigned)>(0xcf20)(nativeGame,amount);flyingMarketCheck();
    marketFlyingSource.push_back(marketEffects.moneyIncome(amount));function<void(__thiscall*)(void*,unsigned)>(0xd450)(nativeGame,amount);flyingMarketCheck();
}
flyingMarketEffects+=unsigned(marketFlyingSource.size());
// The connected fixture's game owner is intentionally hidden. Step the actual
// owned effects explicitly; standalone cFlyText frames prove visible rendering.
for(unsigned tick=0;tick<72;++tick) {
    stage=563;elapsed=tick%5==0 ? 0.f:.025f;
    for(unsigned i=0;i<marketFlyingSource.size();++i) {marketFlyingSource[i]->enabledStep();call(0x7d590,marketFlyingNative[i]);}
    flyingMarketCheck();
}
for(unsigned i=0;i<marketFlyingSource.size();++i) require(views.discarded().count(marketFlyingSource[i]) && nativeDiscards.count(marketFlyingNative[i]),"Market owned effect path did not complete");
stage=564;const auto beforeFlying=textureCounts();views.deleteDiscarded();const auto middleFlying=textureCounts();for(auto* native:marketFlyingNative) call(0x7ba00,native);compareReferenceDeltas(beforeFlying,middleFlying);
nativeDiscards.clear();nativeFlyingAllocation={};marketFlyingSource.clear();marketFlyingNative.clear();flyingMarketCheck={};
