// Pure original matrix oracle; no original entry point or import initialization.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "GameMatrix.h"
#include <cstring>
#include <float.h>
#include <iostream>
#include <random>
#include <stdexcept>
using namespace farmar::recovered;
int wmain(int argc,wchar_t** argv) {
    if(argc!=2 && argc!=3) return 2;
    auto module=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);
    if(!module) return 2;
    try {
        if(argc==3) _control87(_PC_24,_MCW_PC);
        auto* image=reinterpret_cast<unsigned char*>(module);
        // Only two known projection constant operands need relocation; matrix
        // multiplication/transposition are position-independent pure routines.
        DWORD old; if(!VirtualProtect(image+0x19221a,0xa2,PAGE_EXECUTE_READWRITE,&old)) throw std::runtime_error("Oracle protection failed");
        for(auto operand:{0x192220U,0x19223cU}) {
            std::uint32_t address; std::memcpy(&address,image+operand,4);
            if(address!=0x5ef1b4U && address!=0x5f2400U) throw std::runtime_error("Projection constant changed");
            address=reinterpret_cast<std::uintptr_t>(image)+(address-0x400000U);
            std::memcpy(image+operand,&address,4);
        }
        DWORD restored; VirtualProtect(image+0x19221a,0xa2,old,&restored); FlushInstructionCache(GetCurrentProcess(),image+0x19221a,0xa2);
        using Multiply=void(__stdcall*)(Matrix4*,const Matrix4*,const Matrix4*);
        using Transpose=void(__stdcall*)(Matrix4*,const Matrix4*);
        using Ortho=void(__stdcall*)(Matrix4*,float,float,float,float,float,float);
        const auto multiply=reinterpret_cast<Multiply>(image+0x191fe8);
        const auto transpose=reinterpret_cast<Transpose>(image+0x1920cb);
        const auto ortho=reinterpret_cast<Ortho>(image+0x19221a);
        std::mt19937 random(0x4d415458); unsigned checks=0;
        auto equal=[&](const Matrix4& actual,const Matrix4& expected,const char* operation) {
            if(std::memcmp(actual.values,expected.values,sizeof(Matrix4))) {
                for(unsigned i=0;i<16;++i) if(std::memcmp(&actual.values[i],&expected.values[i],4))
                    std::cerr << operation << " element " << i << " actual " << std::hexfloat << actual.values[i] << " expected " << expected.values[i] << '\n';
                throw std::runtime_error("Original matrix differs");
            }
            ++checks;
        };
        auto number=[&] { return static_cast<float>(static_cast<int>(random()%2000001)-1000000)/997.f; };
        for(unsigned trial=0;trial<10000;++trial) {
            Matrix4 left,right,actual,expected;
            for(auto& value:left.values) value=number(); for(auto& value:right.values) value=number();
            multiplyMatrix(actual,left,right); multiply(&expected,&left,&right); equal(actual,expected,"multiply");
            actual=left; expected=left; multiplyMatrix(actual,actual,right); multiply(&expected,&expected,&right); equal(actual,expected,"left alias");
            actual=right; expected=right; multiplyMatrix(actual,left,actual); multiply(&expected,&left,&expected); equal(actual,expected,"right alias");
            actual=left; expected=left; multiplyMatrix(actual,actual,actual); multiply(&expected,&expected,&expected); equal(actual,expected,"square alias");
            transposeMatrix(actual,left); transpose(&expected,&left); equal(actual,expected,"transpose");
            actual=left; expected=left; transposeMatrix(actual,actual); transpose(&expected,&expected); equal(actual,expected,"transpose alias");
            const float l=number(),r=l+1.f+static_cast<float>(random()%100000)/17.f;
            const float b=number(),t=b+1.f+static_cast<float>(random()%100000)/19.f;
            const float n=number(),f=n+1.f+static_cast<float>(random()%100000)/23.f;
            orthographicMatrix(actual,l,r,b,t,n,f); ortho(&expected,l,r,b,t,n,f); equal(actual,expected,"orthographic");
        }
        for(auto dimensions:{800.f,1920.f}) {
            Matrix4 actual,expected; orthographicMatrix(actual,0,dimensions,600,0,-3000,3000); ortho(&expected,0,dimensions,600,0,-3000,3000);
            equal(actual,expected,"game projection");
        }
        FreeLibrary(module); std::cout << checks << '\n';
    } catch(const std::exception& error) { std::cerr << error.what() << '\n'; FreeLibrary(module); return 1; }
}
