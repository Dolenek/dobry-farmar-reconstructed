#pragma once
#include "Direct3D8.h"
namespace farmar::recovered {
// Original00512c40 acquires/caches the back buffer and prepares an unused
// timestamp.dds name. Its complete instruction body never writes an image.
class GraphicsScreenshot {
public:
    explicit GraphicsScreenshot(IDirect3DDevice8* device):device_(device) {}
    ~GraphicsScreenshot() {release();}
    void prepare();
    void release();
private:
    IDirect3DDevice8* device_;
    IDirect3DSurface8* backBuffer_=nullptr;
};
}
