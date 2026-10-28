// Owned HWND automation on the explicitly supplied noninteractive desktop only.
#define WIN32_LEAN_AND_MEAN
#include "ui_driver.h"
#include <fstream>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <thread>
#include <exception>
#include <tlhelp32.h>
namespace fs=std::filesystem;
namespace {
void need(bool ok,const char* text) {if(!ok)throw std::runtime_error(std::string(text)+" Win32="+std::to_string(GetLastError()));}
std::wstring text(HWND window) {
    DWORD_PTR length=0;need(SendMessageTimeoutW(window,WM_GETTEXTLENGTH,0,0,SMTO_ABORTIFHUNG,1000,&length)!=0,"Owned UI text length");
    need(length<8192,"Owned UI text bound");std::wstring result(size_t(length)+1,0);DWORD_PTR copied=0;
    need(SendMessageTimeoutW(window,WM_GETTEXT,result.size(),LPARAM(result.data()),SMTO_ABORTIFHUNG,1000,&copied)!=0,"Owned UI text");result.resize(copied);return result;
}
std::string quote(const std::wstring& value) {
    std::string out="\"";const char digits[]="0123456789abcdef";for(auto c:value)if(c=='"'||c=='\\'){out+='\\';out+=char(c);}else if(c<32||c>=127){out+="\\u";for(int s=12;s>=0;s-=4)out+=digits[(c>>s)&15];}else out+=char(c);return out+'"';
}
struct Windows {DWORD pid;std::vector<HWND> found;};
BOOL CALLBACK owned(HWND window,LPARAM argument) {auto& state=*reinterpret_cast<Windows*>(argument);DWORD pid=0;GetWindowThreadProcessId(window,&pid);if(pid==state.pid)state.found.push_back(window);return TRUE;}
BOOL CALLBACK collect(HWND window,LPARAM argument) {reinterpret_cast<std::vector<HWND>*>(argument)->push_back(window);return TRUE;}
bool hasCombo(HWND window) {std::vector<HWND> children;EnumChildWindows(window,collect,LPARAM(&children));for(auto child:children){wchar_t cls[128]{};GetClassNameW(child,cls,128);if(!_wcsicmp(cls,L"ComboBox"))return true;}return false;}
void snapshot(HWND window,const fs::path& root,const std::string& phase) {
    RECT frame{};need(GetWindowRect(window,&frame)!=FALSE,"Owned dialog rectangle");std::vector<HWND> children{window};EnumChildWindows(window,collect,LPARAM(&children));std::ofstream report(root/("ui-"+phase+".json"));
    report<<"{\"phase\":\""<<phase<<"\",\"controls\":[";bool first=true;
    for(auto child:children){wchar_t cls[128]{};GetClassNameW(child,cls,128);RECT rect{};GetWindowRect(child,&rect);if(!first)report<<',';first=false;
        report<<"{\"id\":"<<GetDlgCtrlID(child)<<",\"class\":"<<quote(cls)<<",\"text\":"<<quote(text(child))<<",\"visible\":"<<(IsWindowVisible(child)?"true":"false")<<",\"enabled\":"<<(IsWindowEnabled(child)?"true":"false")<<",\"style\":"<<DWORD(GetWindowLongW(child,GWL_STYLE))<<",\"rectangle\":["<<rect.left-frame.left<<','<<rect.top-frame.top<<','<<rect.right-frame.left<<','<<rect.bottom-frame.top<<"]}";
    }report<<"]}\n";
    RECT client{};need(GetClientRect(window,&client)!=FALSE&&client.right>0&&client.bottom>0,"Owned dialog client size");
    BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=client.right;info.bmiHeader.biHeight=-client.bottom;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    void* pixels=nullptr;HDC dc=CreateCompatibleDC(nullptr);need(dc!=nullptr,"Owned capture memory DC");HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);need(bitmap!=nullptr,"Owned capture bitmap");auto old=SelectObject(dc,bitmap);const DWORD size=DWORD(client.right*client.bottom*4);std::memset(pixels,0x73,size);
    const BOOL captured=PrintWindow(window,dc,PW_CLIENTONLY);DWORD_PTR printed=0;
    // The noninteractive station's desktop parent is invisible. Explicit print
    // flags omit PRF_CHECKVISIBLE while retaining the actual dialog rendering.
    const auto direct=SendMessageTimeoutW(window,WM_PRINT,WPARAM(dc),PRF_CLIENT|PRF_CHILDREN|PRF_ERASEBKGND,SMTO_ABORTIFHUNG|SMTO_BLOCK,2000,&printed);GdiFlush();
    BITMAPFILEHEADER header{};header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(BITMAPINFOHEADER);header.bfSize=header.bfOffBits+size;
    std::ofstream image(root/("ui-"+phase+".bmp"),std::ios::binary);image.write(reinterpret_cast<char*>(&header),sizeof(header));image.write(reinterpret_cast<char*>(&info.bmiHeader),sizeof(BITMAPINFOHEADER));image.write(static_cast<char*>(pixels),size);SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);need((captured!=FALSE||direct!=0)&&bool(image),"Owned dialog PrintWindow");
}
void click(HWND dialog,int id,DWORD expectedPid) {
    HWND button=GetDlgItem(dialog,id);DWORD pid=0;GetWindowThreadProcessId(button,&pid);need(button&&pid==expectedPid&&IsWindowEnabled(button),"Owned enabled dialog button");
    // Dispatch the button's normal WM_COMMAND to its own dialog, never input to
    // another desktop or a global HWND_BROADCAST.
    need(PostMessageW(dialog,WM_COMMAND,MAKEWPARAM(id,BN_CLICKED),LPARAM(button))!=FALSE,"Owned dialog button command");
}
}
void drive(const PROCESS_INFORMATION& process,HDESK desktop,const fs::path& root,const std::wstring& action) {
    need(SetThreadDesktop(desktop)!=FALSE,"Attach own UI observer thread to private desktop");
    need(action==L"complete"||action==L"cancel"||action==L"language-cancel","Known owned UI action");
    bool language=false,confirm=false,completed=false;std::wstring confirmButton;const auto deadline=GetTickCount64()+35000;ULONGLONG observed=0;std::ofstream observations(root/L"ui-observations.jsonl");
    while(GetTickCount64()<deadline) {
        if(WaitForSingleObject(process.hProcess,0)==WAIT_OBJECT_0)break;
        Windows windows{process.dwProcessId,{}};SetLastError(ERROR_SUCCESS);const auto enumerated=EnumDesktopWindows(desktop,owned,LPARAM(&windows));need(enumerated!=FALSE||GetLastError()==ERROR_SUCCESS,"Enumerate only private desktop");
        if(GetTickCount64()-observed>1000) {
            observed=GetTickCount64();observations<<"{\"windows\":[";bool first=true;for(auto window:windows.found){wchar_t cls[128]{};GetClassNameW(window,cls,128);if(!first)observations<<',';first=false;observations<<"{\"hwnd\":"<<reinterpret_cast<uintptr_t>(window)<<",\"class\":"<<quote(cls)<<",\"visible\":"<<(IsWindowVisible(window)?"true":"false")<<"}";}observations<<"],\"threads\":[";first=true;
            HANDLE list=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);THREADENTRY32 entry{sizeof(entry)};for(BOOL more=Thread32First(list,&entry);more;more=Thread32Next(list,&entry))if(entry.th32OwnerProcessID==process.dwProcessId){GUITHREADINFO gui{sizeof(gui)};const auto guiOk=GetGUIThreadInfo(entry.th32ThreadID,&gui);wchar_t name[256]{};DWORD length=0;auto threadDesktop=GetThreadDesktop(entry.th32ThreadID);if(threadDesktop)GetUserObjectInformationW(threadDesktop,UOI_NAME,name,sizeof(name),&length);if(!first)observations<<',';first=false;observations<<"{\"id\":"<<entry.th32ThreadID<<",\"gui\":"<<(guiOk?"true":"false")<<",\"active\":"<<reinterpret_cast<uintptr_t>(gui.hwndActive)<<",\"desktop\":"<<quote(name)<<"}";}CloseHandle(list);observations<<"]}\n";observations.flush();
        }
        for(auto window:windows.found) {
            wchar_t cls[128]{};GetClassNameW(window,cls,128);if(std::wstring(cls)!=L"#32770"||!(GetWindowLongW(window,GWL_STYLE)&WS_VISIBLE))continue;
            HWND ok=GetDlgItem(window,IDOK);if(!ok||!IsWindowEnabled(ok))continue;
            if(hasCombo(window)) {
                if(!language){Sleep(150);snapshot(window,root,"language");language=true;click(window,action==L"language-cancel"?IDCANCEL:IDOK,process.dwProcessId);}continue;
            }
            if(!confirm&&fs::exists(root/L"payload")) {Sleep(150);snapshot(window,root,"confirm");confirmButton=text(ok);confirm=true;click(window,action==L"cancel"?IDCANCEL:IDOK,process.dwProcessId);}
            else if(confirm&&action==L"complete"&&!completed&&!fs::exists(root/L"payload")&&text(ok)!=confirmButton) {Sleep(150);snapshot(window,root,"completed");completed=true;click(window,IDOK,process.dwProcessId);}
        }
        Sleep(25);
    }
    need(WaitForSingleObject(process.hProcess,0)==WAIT_OBJECT_0,"Owned uninstall UI timeout");
    need(action==L"language-cancel"?language:confirm,"Expected owned UI stage was not reached");if(action==L"complete")need(completed,"Owned completed page was not reached");
    std::ofstream(root/L"ui-driver-result.json")<<"{\"language_seen\":"<<(language?"true":"false")<<",\"confirm_seen\":"<<(confirm?"true":"false")<<",\"completed_seen\":"<<(completed?"true":"false")<<",\"only_private_owned_hwnds\":true}\n";
}
void driveUninstallUi(const PROCESS_INFORMATION& process,HDESK desktop,const fs::path& root,const std::wstring& action) {
    std::exception_ptr error;std::thread observer([&]{try{drive(process,desktop,root,action);}catch(...){error=std::current_exception();}});observer.join();if(error)std::rethrow_exception(error);
}
