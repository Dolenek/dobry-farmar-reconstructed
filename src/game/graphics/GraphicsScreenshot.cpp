#include "GraphicsScreenshot.h"
#include <ctime>
#include <string>
namespace farmar::recovered {
void GraphicsScreenshot::release() {if(backBuffer_) backBuffer_->Release();backBuffer_=nullptr;}
void GraphicsScreenshot::prepare() {
    if(!backBuffer_) device_->GetBackBuffer(0,D3DBACKBUFFER_TYPE_MONO,&backBuffer_);
    char date[16]{},time[16]{};_strdate_s(date,sizeof date);_strtime_s(time,sizeof time);
    std::string name=std::string(date)+time+".dds";
    for(auto& character:name) if(character=='/' || character==':') character='-';
    // This is the actual end of00512c40. Do not invent an output operation.
}
}
