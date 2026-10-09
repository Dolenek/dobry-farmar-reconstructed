#include "Shader.h"
#include "FileIndex.h"
#include <memory>
namespace farmar::recovered {
namespace { ShaderEnvironment environment; }
ShaderEnvironment& shaderEnvironment() noexcept { return environment; }
void createSpriteVertexShader(IDirect3DDevice8* device,const DWORD* function,DWORD& handle) {
    const DWORD declaration[]={0x20000000U,0x40020000U,0x40040005U,0x40010007U,0xffffffffU};
    device->CreateVertexShader(declaration,function,&handle,0); // original ignores HRESULT/size
}
cShader::cShader(const String& name) : cResource(name) {
    auto file=environment.files->open(name);
    const auto size=file->size();
    auto bytes=std::make_unique<unsigned char[]>(size);
    file->read(bytes.get(),size); file.reset();
    auto storage=std::make_unique<Bytecode>();
    storage->size=size; storage->data=bytes.release(); bytecode_=storage.release();
    restoreGraphics();
}
cShader::~cShader() {
    if(bytecode_) { delete[] bytecode_->data; delete bytecode_; }
}
void cShader::preResetGraphics() { environment.remove(shader_); shader_=0; }
void cShader::restoreGraphics() {
    environment.create(reinterpret_cast<const DWORD*>(bytecode_->data),bytecode_->size,shader_);
}
}
