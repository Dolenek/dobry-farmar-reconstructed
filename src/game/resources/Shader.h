#pragma once
#include "Resource.h"
#include "Direct3D8.h"
#include <functional>
namespace farmar::recovered {
class FileIndex;
struct ShaderEnvironment {
    FileIndex* files=nullptr;
    std::function<void(const DWORD*,const unsigned long&,DWORD&)> create;
    std::function<void(const DWORD&)> remove;
};
ShaderEnvironment& shaderEnvironment() noexcept;
// Declaration from 0065d318, used by graphics API 0050d240.
void createSpriteVertexShader(IDirect3DDevice8* device,const DWORD* function,DWORD& handle);
#pragma pack(push,4)
class cShader final : public cResource {
public:
    explicit cShader(const String& name); // 00557c30
    ~cShader() override; // 00557400: frees bytecode; does not delete device shader
    std::uint32_t resourceType() const override { return 3; } // 00557480
    void preResetGraphics() override; // 00557490
    void restoreGraphics() override; // 00557860
    DWORD shader() const noexcept { return shader_; }
private:
    struct Bytecode { unsigned char* data; unsigned long size; };
    DWORD shader_=0; // +24
    Bytecode* bytecode_=nullptr; // +28
};
#pragma pack(pop)
static_assert(sizeof(cShader)==44,"Original shader resource storage");
}
