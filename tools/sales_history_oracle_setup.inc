stage=540;write(0x10f10,saleHistoryEntry.data(),5);
field<void*>(nativeGame,0x9f8)=function<void*(__cdecl*)()>(0x122f00)();
field<unsigned>(nativeGame,0x9fc)=0;field<unsigned>(nativeGame,0xa00)=0;field<float>(nativeGame,0xa04)=0;
saleHistoryCheck=[&] {
    stage=541;const auto& records=saleHistory.records();auto* head=field<void*>(nativeGame,0x9f8);auto* current=field<void*>(head,0);auto* previous=head;
    require(records.size()==field<unsigned>(nativeGame,0x9fc),"Sale history count differs");require(saleHistory.total()==field<unsigned>(nativeGame,0xa00),"Sale history total differs");compareFloat(saleHistory.cutoff(),field<float>(nativeGame,0xa04),"Sale history cutoff");
    unsigned sum=0;
    for(const auto& record:records) {
        require(current!=head && field<void*>(current,4)==previous,"Sale history list links/order differ");
        compareFloat(record.time,field<float>(current,8),"Sale history event time");require(record.amount==field<unsigned>(current,12),"Sale history event amount differs");
        sum+=record.amount;previous=current;current=field<void*>(current,0);
    }
    require(current==head && field<void*>(head,4)==previous,"Sale history tail/sentinel differs");require(sum==saleHistory.total(),"Sale history total does not equal remaining amounts");
    require(profile.booleans==nativeProfile.booleans,"Sale history award stores differ");++salesHistoryStates;
};saleHistoryCheck();
