#include "Texture.h"
#include "FileIndex.h"
#include <vector>
namespace farmar::recovered {
namespace { TextureEnvironment environment; }
TextureEnvironment& textureEnvironment() noexcept { return environment; }
cTexture::cTexture(const String& name) : cResource(name) { load(name); }
cTexture::~cTexture() { if(texture_) texture_->Release(); }
void cTexture::preResetGraphics() {
    if(texture_) { texture_->Release(); texture_=nullptr; }
}
void cTexture::restoreGraphics() { load(resourceName()); }
void cTexture::load(const String& name) {
    auto file=environment.files->open(name);
    unsigned long size=file->size();
    std::vector<unsigned char> bytes(size);
    file->read(bytes.data(),size);
    file.reset();
    TextureImageSize image;
    environment.load(bytes.data(),size,texture_,image);
    D3DSURFACE_DESC description;
    texture_->GetLevelDesc(0,&description);
    scaleU_=static_cast<float>(image.width)/static_cast<float>(description.Width);
    scaleV_=static_cast<float>(image.height)/static_cast<float>(description.Height);
    allocatedWidth_=description.Width; allocatedHeight_=description.Height;
    imageWidth_=image.width; imageHeight_=image.height;
    halfU_=0.5f/static_cast<float>(description.Width);
    halfV_=0.5f/static_cast<float>(description.Height);
}
}
