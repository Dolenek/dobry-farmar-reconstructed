#include "ClickMask.h"
#include "Texture.h"
#include "FileIndex.h"
#include "NativeImage.h"
#include "OriginalArithmetic.h"
#include <cstring>
#include <stdexcept>
namespace farmar::recovered {
std::vector<ClickMaskColumn> loadClickMask(const String& name) {
    auto& environment=textureEnvironment();
    if(!environment.files || !environment.load) throw std::logic_error("Click-mask texture environment is not bound");
    auto file=environment.files->open(name); const auto count=file->size(); std::vector<unsigned char> bytes(count); file->read(bytes.data(),count); file.reset();
    unsigned magic=0; if(count>=4) std::memcpy(&magic,bytes.data(),4);
    if(magic!=0x474e4a8bU) {
        // 0051c040 requests A8R8G8B8 from D3DX for native images. Shipped
        // 32-bit TGA masks already have this format, so the reconstructed
        // native reader/upload needs no format conversion. Other formats
        // remain an explicit unsupported path until conversion is recovered.
        const auto native=decodeNativeImage(bytes.data(),bytes.size());
        if(native.planes.empty() || native.planes.front().format!=D3DFMT_A8R8G8B8)
            throw std::runtime_error("Click-mask D3DX conversion to BGRA is not reconstructed yet");
    }
    IDirect3DTexture8* texture=nullptr; TextureImageSize dimensions{}; const unsigned long size=count;
    environment.load(bytes.data(),size,texture,dimensions);
    if(!texture) throw std::runtime_error("Click-mask texture is unavailable");
    try {
        D3DSURFACE_DESC description{}; texture->GetLevelDesc(0,&description);
        if(description.Format!=D3DFMT_A8R8G8B8) throw std::runtime_error("Click-mask requires original BGRA format");
        std::vector<ClickMaskColumn> columns(dimensions.width);
        for(auto& column:columns) { column.bitCount=dimensions.height;column.words.resize((dimensions.height+31)/32); }
        D3DLOCKED_RECT locked{}; texture->LockRect(0,&locked,nullptr,D3DLOCK_READONLY);
        const auto* pixels=static_cast<const unsigned char*>(locked.pBits);
        for(unsigned y=0;y<dimensions.height;++y) for(unsigned x=0;x<dimensions.width;++x) {
            // Original uses level Width for stride, despite querying LockRect.
            const auto alpha=pixels[(std::size_t(y)*description.Width+x)*4+3];
            if(originalMultiply(alpha,0.003921568859368563F)>0.0010000000474974513F) columns[x].words[y>>5]|=1U<<(y&31);
        }
        texture->UnlockRect(0);texture->Release();return columns;
    } catch(...) { texture->Release();throw; }
}
}
