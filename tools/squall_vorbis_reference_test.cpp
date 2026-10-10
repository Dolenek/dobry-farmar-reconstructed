// Original Squall routines are loaded only as independent test oracles.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <filesystem>
#include <float.h>
#include <iostream>
#include <cstring>
#include <vector>
extern "C" {
#include "mdct.h"
void farmar_mdct_butterfly_32(float*);
void farmar_mdct_butterfly_8(float*);
void farmar_mdct_butterfly_generic(float*,float*,int,int);
}
namespace {
float inputValue(unsigned& seed) {
    seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
    unsigned bits=(seed&0x807fffffU)|((120U+(seed>>24)%15U)<<23);
    float value;std::memcpy(&value,&bits,4);return value;
}
}
int wmain(int argc,wchar_t** argv) {
    if(argc<2 || argc>3) return 2;
    auto dll=LoadLibraryW(std::filesystem::absolute(argv[1]).c_str());if(!dll) return 2;
    _control87(argc==3 ? _PC_24 : _PC_53,_MCW_PC);
    unsigned short cw;__asm fnstcw cw
    std::cerr<<"Control before lookup: "<<std::hex<<cw<<std::dec<<'\n';
    const auto init=reinterpret_cast<void(__cdecl*)(mdct_lookup*,int)>(reinterpret_cast<unsigned char*>(dll)+0x37110);
    const auto clear=reinterpret_cast<void(__cdecl*)(mdct_lookup*)>(reinterpret_cast<unsigned char*>(dll)+0x372f0);
    const auto backward=reinterpret_cast<void(__cdecl*)(mdct_lookup*,float*,float*)>(reinterpret_cast<unsigned char*>(dll)+0x37330);
    unsigned differences=0,compared=0,transforms=0,transformValues=0,butterflyValues=0;unsigned seed=0x5a73d102;
    for(auto routine:{0x37840U,0x37d40U}) {
        const unsigned count=routine==0x37840 ? 32:8;
        const auto native=reinterpret_cast<void(__cdecl*)(float*)>(reinterpret_cast<unsigned char*>(dll)+routine);
        const auto source=routine==0x37840 ? &farmar_mdct_butterfly_32:&farmar_mdct_butterfly_8;
        unsigned mismatches=0;
        for(unsigned trial=0;trial<10000;++trial) {
            std::vector<float> a(count),b;
            for(auto& value:a) value=inputValue(seed);
            b=a;source(a.data());native(b.data());
            for(unsigned i=0;i<count;++i) if(std::memcmp(&a[i],&b[i],4)) {
                if(!mismatches) {unsigned x,y;std::memcpy(&x,&a[i],4);std::memcpy(&y,&b[i],4);std::cerr<<"Butterfly "<<count<<" trial "<<trial<<" i "<<i<<" source "<<std::hex<<x<<" native "<<y<<std::dec<<'\n';}
                ++mismatches;
            }
        }
        std::cerr<<"Butterfly "<<count<<" differences "<<mismatches<<'\n';
        differences+=mismatches;butterflyValues+=10000*count;
    }
    for(int n=64;n<=8192;n*=2) {
        mdct_lookup actual{},expected{};mdct_init(&actual,n);init(&expected,n);
        __asm fnstcw cw
        std::cerr<<"Control after lookup: "<<std::hex<<cw<<std::dec<<'\n';
        unsigned mismatch=0;
        for(int i=0;i<n+n/4;++i) {
            if(std::memcmp(&actual.trig[i],&expected.trig[i],4)) {
                if(!mismatch) {
                    unsigned a,b;std::memcpy(&a,&actual.trig[i],4);std::memcpy(&b,&expected.trig[i],4);
                    std::cerr<<"MDCT lookup n="<<n<<" index="<<i<<" source="<<std::hex<<a<<" native="<<b<<std::dec<<'\n';
                }
                ++mismatch;
            }
            ++compared;
        }
        if(std::memcmp(actual.bitrev,expected.bitrev,n) || actual.n!=expected.n || actual.log2n!=expected.log2n || actual.scale!=expected.scale) ++mismatch;
        std::cerr<<"n="<<n<<" differences="<<mismatch<<'\n';differences+=mismatch;
        unsigned transformDifferences=0;
        for(unsigned trial=0;trial<128;++trial) {
            std::vector<float> input(n/2),decoded(n),wanted(n);
            for(auto& value:input) value=inputValue(seed);
            mdct_backward(&actual,input.data(),decoded.data());backward(&expected,input.data(),wanted.data());
            for(int i=0;i<n;++i) {
                if(std::memcmp(&decoded[i],&wanted[i],4)) {
                    if(!transformDifferences) {
                        unsigned a,b;std::memcpy(&a,&decoded[i],4);std::memcpy(&b,&wanted[i],4);
                        std::cerr<<"MDCT transform n="<<n<<" trial="<<trial<<" index="<<i<<" source="<<std::hex<<a<<" native="<<b<<std::dec<<'\n';
                    }
                    ++transformDifferences;
                }
                ++transformValues;
            }
            ++transforms;
        }
        std::cerr<<"n="<<n<<" transform differences="<<transformDifferences<<'\n';differences+=transformDifferences;
        mdct_clear(&actual);clear(&expected);
    }
    if((_control87(0,0)&_MCW_PC)!=(argc==3 ? _PC_24:_PC_53)) return 1;
    FreeLibrary(dll);std::cout<<"{\"lookup_values\":"<<compared<<",\"butterfly_values\":"<<butterflyValues<<",\"transforms\":"<<transforms<<",\"transform_values\":"<<transformValues<<",\"differences\":"<<differences<<"}\n";
    return differences?1:0;
}
