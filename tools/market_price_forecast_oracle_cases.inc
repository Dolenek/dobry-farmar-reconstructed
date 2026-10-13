// Source/native Gameplay with actual and synthetic popularity timelines.
stage=226;
for(unsigned product=0;product<=19;++product) {
    const auto key=std::string("MEffPrice")+std::to_string(product);
    const std::int32_t prices[]{-2147483647,-1,0,1,5,6,79,100,2147483647};
    sourceProfile.ints[key]=nativeProfile.ints[key]=prices[(trial+product)%9];
    require(source->effectivePrice(product)==function<int(__stdcall*)(unsigned)>(0xb5ea0)(product),"Market effective price differs");++marketPriceStates;
    for(float interval:{0.125f,1.f,7.25f,10.f,59.75f,1000.f}) for(unsigned samples:{0U,1U,2U,3U,10U,11U,19U,37U}) {
        stage=227;
        auto forecast=source->priceForecast(product,interval,samples);std::array<unsigned char,16> nativeValues{};
        function<void(__thiscall*)(void*,unsigned,float,unsigned,void*)>(0xb62a0)(native.data(),product,interval,samples,nativeValues.data());
        const auto* begin=field<float*>(nativeValues.data(),4);const auto* end=field<float*>(nativeValues.data(),8);
        require(forecast.size()==(begin ? std::size_t(end-begin):0),"Market forecast count differs");
        if(samples && std::memcmp(forecast.data(),begin,samples*4)) {
            for(unsigned i=0;i<samples;++i) if(floatBits(forecast[i])!=floatBits(begin[i])) std::cerr<<"Forecast product "<<product<<" interval "<<interval<<" sample "<<i<<" count "<<samples<<" words "<<std::hex<<floatBits(forecast[i])<<'/'<<floatBits(begin[i])<<std::dec<<'\n';
            require(false,"Market future price samples differ");
        }
        require(sourceProfile.ints==nativeProfile.ints,"Market effective price profile changes differ");
        release(const_cast<float*>(begin));++marketForecasts;
    }
}
