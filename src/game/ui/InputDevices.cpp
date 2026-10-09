#include "InputDevices.h"
#include "GameWindow.h"
#include <stdexcept>
namespace farmar::recovered {
namespace {
InputDevices* windowInput=nullptr;
void initialize(IDirectInput8A* input,const GUID& guid,const DIDATAFORMAT& format,HWND window,IDirectInputDevice8A*& device) {
    input->CreateDevice(guid,&device,nullptr);
    if(!device) throw std::runtime_error("DirectInput CreateDevice did not return the original required device");
    // Original initKeyboard/initMouse ignore these HRESULTs and return zero.
    device->SetDataFormat(&format);device->SetCooperativeLevel(window,6);
    DIPROPDWORD buffer{};buffer.diph.dwSize=20;buffer.diph.dwHeaderSize=16;buffer.dwData=1024;
    device->SetProperty(DIPROP_BUFFERSIZE,&buffer.diph);
}
void checkRead(HRESULT result,const char* message) { if(result==DIERR_INPUTLOST) throw std::runtime_error(message); }
}
InputDevices::InputDevices(HINSTANCE instance,HWND window):window_(window) {
    IDirectInput8A* input=nullptr;
    DirectInput8Create(instance,0x800,IID_IDirectInput8A,reinterpret_cast<void**>(&input),nullptr);
    if(!input) throw std::runtime_error("DirectInput8Create did not return the original required interface");
    try { initialize(input,GUID_SysKeyboard,c_dfDIKeyboard,window,keyboard_);initialize(input,GUID_SysMouse,c_dfDIMouse2,window,mouse_);input->Release(); }
    catch(...) { if(keyboard_)keyboard_->Release();if(mouse_)mouse_->Release();input->Release();throw; }
}
InputDevices::InputDevices(HWND window,IDirectInputDevice8A* keyboard,IDirectInputDevice8A* mouse) noexcept :window_(window),keyboard_(keyboard),mouse_(mouse) {}
InputDevices::~InputDevices() { if(windowInput==this)windowInput=nullptr;if(keyboard_)keyboard_->Release();if(mouse_)mouse_->Release(); }
bool InputDevices::acquire() { const auto keyboard=keyboard_->Acquire(),mouse=mouse_->Acquire();return SUCCEEDED(keyboard)&&SUCCEEDED(mouse); }
bool InputDevices::unacquire() { const auto keyboard=keyboard_->Unacquire(),mouse=mouse_->Unacquire();return SUCCEEDED(keyboard)&&SUCCEEDED(mouse); }
void InputDevices::exclusiveMouse(bool value) {
    if(exclusive_==value)return;exclusive_=value;mouse_->Unacquire();mouse_->SetCooperativeLevel(window_,value ? 5:6);mouse_->Acquire();
}
void InputDevices::character(unsigned value) noexcept { if(characterCount_<characters_.size())characters_[characterCount_++]=static_cast<wchar_t>(value); }
bool InputDevices::keyPressed(unsigned control) {
    if(!keyboardStateValid_) { keyboardState_.fill(0);keyboard_->GetDeviceState(256,keyboardState_.data());keyboardStateValid_=true; }
    return keyboardState_.at(control)>>7;
}
void InputDevices::process() {
    // Unlike control/mouse dispatch, character handlers are a live set; a
    // handler may append characters to the fixed buffer during this traversal.
    if(!bindings_.keyboard().empty()) for(unsigned index=0;index<characterCount_;++index) bindings_.dispatchCharacter(characters_[index]);
    characterCount_=0;
    std::array<DIDEVICEOBJECTDATA,1024> data{};DWORD count=1024;std::vector<unsigned> events;
    auto result=keyboard_->GetDeviceData(20,data.data(),&count,0);
    checkRead(result,"m_keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_keyboard_buffer, &event_count, 0)");
    if(result==DIERR_NOTACQUIRED)keyboard_->Acquire();
    else if(result>=0 && result<2) for(unsigned index=0;index<count;++index)events.push_back(data[index].dwOfs+((data[index].dwData&0x80)*2));
    count=1024;result=mouse_->GetDeviceData(20,data.data(),&count,0);
    checkRead(result,"m_mouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_mouse_buffer, &event_count, 0)");
    if(result==DIERR_NOTACQUIRED)mouse_->Acquire();
    else if(result>=0 && result<2) for(unsigned index=0;index<count;++index) {
        const auto& event=data[index];if(event.dwOfs>11 && event.dwOfs<20) events.push_back(500+event.dwOfs+((event.dwData>>4)&8));
    }
    bindings_.dispatchControls(events);
    DIMOUSESTATE2 mouseState{};result=mouse_->GetDeviceState(20,&mouseState);
    checkRead(result,"m_mouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_mouse_buffer, &event_count, 0)"); // original reused message
    if(result==DIERR_NOTACQUIRED)mouse_->Acquire();
    else if(result>=0 && result<2)bindings_.dispatchMouse({mouseState.lX,mouseState.lY,mouseState.lZ});
    keyboardStateValid_=false;
}
void InputDevices::bindWindowTargets() {
    windowInput=this;auto& targets=windowTargets();
    targets.acquireInput=[] { if(windowInput)windowInput->acquire(); };targets.releaseInput=[] { if(windowInput)windowInput->unacquire(); };
    targets.character=[](unsigned value) { if(windowInput)windowInput->character(value); };
}
}
