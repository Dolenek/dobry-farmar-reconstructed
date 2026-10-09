#pragma once
// Test-only forwarding COM fixture; records native API calls without changing
// the real device or its vtable. Only methods used by sprite draw are exposed.
#include "Direct3D8.h"
#include <array>
#include <vector>
struct SpriteDeviceTrace {
    void** vtable;
    IDirect3DDevice8* real;
    using Call=std::array<DWORD,6>;
    std::vector<Call> calls;
    std::array<void*,97> table{};
    static HRESULT WINAPI getRender(SpriteDeviceTrace* self,D3DRENDERSTATETYPE type,DWORD* value) {
        self->calls.push_back({51,static_cast<DWORD>(type)}); return self->real->GetRenderState(type,value);
    }
    static HRESULT WINAPI setRender(SpriteDeviceTrace* self,D3DRENDERSTATETYPE type,DWORD value) {
        self->calls.push_back({50,static_cast<DWORD>(type),value}); return self->real->SetRenderState(type,value);
    }
    static HRESULT WINAPI getStage(SpriteDeviceTrace* self,DWORD stage,D3DTEXTURESTAGESTATETYPE type,DWORD* value) {
        self->calls.push_back({62,stage,static_cast<DWORD>(type)}); return self->real->GetTextureStageState(stage,type,value);
    }
    static HRESULT WINAPI setStage(SpriteDeviceTrace* self,DWORD stage,D3DTEXTURESTAGESTATETYPE type,DWORD value) {
        self->calls.push_back({63,stage,static_cast<DWORD>(type),value}); return self->real->SetTextureStageState(stage,type,value);
    }
    static HRESULT WINAPI texture(SpriteDeviceTrace* self,DWORD stage,IDirect3DBaseTexture8* value) {
        self->calls.push_back({61,stage,reinterpret_cast<DWORD>(value)}); return self->real->SetTexture(stage,value);
    }
    static HRESULT WINAPI shader(SpriteDeviceTrace* self,DWORD value) {
        self->calls.push_back({76,value}); return self->real->SetVertexShader(value);
    }
    static HRESULT WINAPI constants(SpriteDeviceTrace* self,DWORD index,const void* data,DWORD count) {
        self->calls.push_back({79,index,count}); return self->real->SetVertexShaderConstant(index,data,count);
    }
    static HRESULT WINAPI draw(SpriteDeviceTrace* self,D3DPRIMITIVETYPE type,UINT start,UINT vertices,UINT index,UINT count) {
        self->calls.push_back({71,static_cast<DWORD>(type),start,vertices,index,count}); return self->real->DrawIndexedPrimitive(type,start,vertices,index,count);
    }
    static HRESULT WINAPI indices(SpriteDeviceTrace* self,IDirect3DIndexBuffer8* value,UINT base) {
        self->calls.push_back({85,reinterpret_cast<DWORD>(value),base}); return self->real->SetIndices(value,base);
    }
    static HRESULT WINAPI stream(SpriteDeviceTrace* self,UINT index,IDirect3DVertexBuffer8* value,UINT stride) {
        self->calls.push_back({83,index,reinterpret_cast<DWORD>(value),stride}); return self->real->SetStreamSource(index,value,stride);
    }
    explicit SpriteDeviceTrace(IDirect3DDevice8* device) : real(device) {
        table[51]=reinterpret_cast<void*>(&getRender); table[50]=reinterpret_cast<void*>(&setRender);
        table[62]=reinterpret_cast<void*>(&getStage); table[63]=reinterpret_cast<void*>(&setStage);
        table[61]=reinterpret_cast<void*>(&texture); table[76]=reinterpret_cast<void*>(&shader);
        table[79]=reinterpret_cast<void*>(&constants); table[71]=reinterpret_cast<void*>(&draw);
        table[85]=reinterpret_cast<void*>(&indices); table[83]=reinterpret_cast<void*>(&stream); vtable=table.data();
    }
    IDirect3DDevice8* device() { return reinterpret_cast<IDirect3DDevice8*>(this); }
};
