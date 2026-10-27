// Diagnostic ONLY: run the untouched original executable and isolate its
// SHGetFolderPathA COMMON_APPDATA call at the OS boundary. No game code replaced.
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <cstdint>
static_assert(sizeof(void*)==4,"Build this diagnostic as Win32");
void need(bool ok,const char* message) {if(!ok)throw std::runtime_error(std::string(message)+" Win32="+std::to_string(GetLastError()));}
void read(HANDLE process,uintptr_t address,void* out,size_t bytes) {SIZE_T done=0;need(ReadProcessMemory(process,(void*)address,out,bytes,&done)&&done==bytes,"ReadProcessMemory");}
void patch(HANDLE process,uintptr_t address,const void* bytes,size_t count) {
 DWORD old=0;need(VirtualProtectEx(process,(void*)address,count,PAGE_EXECUTE_READWRITE,&old),"VirtualProtectEx");SIZE_T done=0;
 need(WriteProcessMemory(process,(void*)address,bytes,count,&done)&&done==count,"WriteProcessMemory");DWORD ignored;need(VirtualProtectEx(process,(void*)address,count,old,&ignored),"Restore protection");need(FlushInstructionCache(process,(void*)address,count),"FlushInstructionCache");
}
struct Image {
 std::vector<unsigned char> data;const IMAGE_NT_HEADERS32* nt=nullptr;const IMAGE_SECTION_HEADER* sections=nullptr;
 explicit Image(const std::filesystem::path& path) {std::ifstream f(path,std::ios::binary);need(bool(f),"Read original PE");data={std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};need(data.size()>sizeof(IMAGE_DOS_HEADER),"PE size");auto* dos=(const IMAGE_DOS_HEADER*)data.data();need(dos->e_magic==IMAGE_DOS_SIGNATURE&&dos->e_lfanew>0&&size_t(dos->e_lfanew)+sizeof(IMAGE_NT_HEADERS32)<data.size(),"DOS header");nt=(const IMAGE_NT_HEADERS32*)(data.data()+dos->e_lfanew);need(nt->Signature==IMAGE_NT_SIGNATURE&&nt->FileHeader.Machine==IMAGE_FILE_MACHINE_I386,"Win32 PE");sections=IMAGE_FIRST_SECTION(nt);}
 const void* at(DWORD rva,size_t size=1) const {
  if(rva<nt->OptionalHeader.SizeOfHeaders&&size_t(rva)+size<=data.size())return data.data()+rva;
  for(unsigned n=0;n<nt->FileHeader.NumberOfSections;++n) {auto& s=sections[n];if(rva>=s.VirtualAddress&&uint64_t(rva-s.VirtualAddress)+size<=s.SizeOfRawData&&uint64_t(s.PointerToRawData)+(rva-s.VirtualAddress)+size<=data.size())return data.data()+s.PointerToRawData+(rva-s.VirtualAddress);}
  throw std::runtime_error("Unmapped file RVA");
 }
 DWORD folderIAT() const {
  auto directory=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
  for(unsigned n=0;n<directory.Size/sizeof(IMAGE_IMPORT_DESCRIPTOR);++n) {
   const auto& d=*(const IMAGE_IMPORT_DESCRIPTOR*)at(directory.VirtualAddress+n*sizeof(IMAGE_IMPORT_DESCRIPTOR),sizeof(IMAGE_IMPORT_DESCRIPTOR));if(!d.Name)break;
   for(unsigned i=0;;++i) {const auto& thunk=*(const DWORD*)at((d.OriginalFirstThunk?d.OriginalFirstThunk:d.FirstThunk)+i*4,4);if(!thunk)break;if(thunk&IMAGE_ORDINAL_FLAG32)continue;auto* name=(const char*)at(thunk+2);if(!std::strcmp(name,"SHGetFolderPathA"))return d.FirstThunk+i*4;}
  }
  throw std::runtime_error("No SHGetFolderPathA import");
 }
};
void emit32(std::vector<unsigned char>& code,DWORD value){for(unsigned i=0;i<4;++i)code.push_back((unsigned char)(value>>(i*8)));}
BOOL CALLBACK closeWindow(HWND w,LPARAM pid){DWORD owner=0;GetWindowThreadProcessId(w,&owner);if(owner==DWORD(pid))PostMessageW(w,WM_CLOSE,0,0);return TRUE;}
int wmain(int argc,wchar_t** argv) {
 PROCESS_INFORMATION process{};bool debug=false;HANDLE eventThread=nullptr;
 try {
  need(argc==4,"Expected original-exe isolated-working-dir isolated-common-root");const auto exe=std::filesystem::absolute(argv[1]);const auto cwd=std::filesystem::absolute(argv[2]);const auto common=std::filesystem::absolute(argv[3]);
  need(exe.filename()==L"Dobrý farmář.exe","Expected original executable basename");Image image(exe);const auto iatRva=image.folderIAT();
  std::filesystem::create_directories(cwd);std::filesystem::create_directories(common);BOOL fallback=FALSE;
  int length=WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,common.c_str(),-1,nullptr,0,nullptr,&fallback);need(length>0&&length<MAX_PATH&&!fallback,"Shared path ANSI conversion");std::string folder(length,'\0');WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,common.c_str(),-1,folder.data(),length,nullptr,&fallback);need(!fallback,"Shared path ANSI loss");
  std::wstring identity=L"Melesta_"+exe.filename().wstring();CharLowerBuffW(identity.data(),DWORD(identity.size()));need(!FindWindowW(identity.c_str(),nullptr),"Original game already has a window; refusing to disturb it");
  std::wstring command=L"\""+exe.wstring()+L"\"";STARTUPINFOW startup{sizeof(startup)};
  need(CreateProcessW(exe.c_str(),command.data(),nullptr,nullptr,FALSE,DEBUG_ONLY_THIS_PROCESS,nullptr,cwd.c_str(),&startup,&process),"Create original reference process");debug=true;DebugSetProcessKillOnExit(TRUE);
  uintptr_t base=0,entry=0;unsigned char originalByte=0;bool installed=false,ready=false;
  while(!ready) {
   DEBUG_EVENT e{};need(WaitForDebugEvent(&e,30000),"WaitForDebugEvent");DWORD action=DBG_CONTINUE;
   if(e.dwDebugEventCode==CREATE_PROCESS_DEBUG_EVENT) {
    base=(uintptr_t)e.u.CreateProcessInfo.lpBaseOfImage;entry=base+image.nt->OptionalHeader.AddressOfEntryPoint;
    if(e.u.CreateProcessInfo.hFile)CloseHandle(e.u.CreateProcessInfo.hFile);
    read(process.hProcess,entry,&originalByte,1);const unsigned char trap=0xcc;patch(process.hProcess,entry,&trap,1);installed=true;
   } else if(e.dwDebugEventCode==LOAD_DLL_DEBUG_EVENT) {if(e.u.LoadDll.hFile)CloseHandle(e.u.LoadDll.hFile);}
   else if(e.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT) throw std::runtime_error("Original exited before entry-boundary setup");
   else if(e.dwDebugEventCode==EXCEPTION_DEBUG_EVENT) {
    const auto& x=e.u.Exception.ExceptionRecord;
    if(x.ExceptionCode==EXCEPTION_BREAKPOINT && (uintptr_t)x.ExceptionAddress==entry && installed) {
     patch(process.hProcess,entry,&originalByte,1);eventThread=OpenThread(THREAD_GET_CONTEXT|THREAD_SET_CONTEXT,FALSE,e.dwThreadId);need(eventThread!=nullptr,"Open entry thread");CONTEXT context{};context.ContextFlags=CONTEXT_CONTROL;need(GetThreadContext(eventThread,&context),"Get entry context");need(context.Eip==entry+1,"Entry breakpoint EIP");context.Eip=DWORD(entry);need(SetThreadContext(eventThread,&context),"Restore entry EIP");CloseHandle(eventThread);eventThread=nullptr;
     DWORD original=0;read(process.hProcess,base+iatRva,&original,4);need(original!=0&&original!=image.nt->OptionalHeader.ImageBase+iatRva,"Resolved OS folder import");
     auto* remote=VirtualAllocEx(process.hProcess,nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);need(remote!=nullptr,"Allocate OS boundary thunk");
     std::vector<unsigned char> code={0x81,0x7c,0x24,0x08,0x23,0x80,0x00,0x00,0x75,0x00,0x57,0x56,0x8b,0x7c,0x24,0x1c,0xbe};
     emit32(code,DWORD(uintptr_t(remote)+512));code.push_back(0xb9);emit32(code,DWORD(folder.size()));
     code.insert(code.end(),{0xf3,0xa4,0x5e,0x5f,0x33,0xc0,0xc2,0x14,0x00});code[9]=static_cast<unsigned char>(code.size()-10);code.push_back(0xb8);emit32(code,original);code.insert(code.end(),{0xff,0xe0});
     SIZE_T written=0;need(WriteProcessMemory(process.hProcess,(char*)remote+512,folder.data(),folder.size(),&written)&&written==folder.size(),"Write isolated root");patch(process.hProcess,(uintptr_t)remote,code.data(),code.size());DWORD protection=0;need(VirtualProtectEx(process.hProcess,remote,4096,PAGE_EXECUTE_READ,&protection),"Make OS thunk read-only executable");
     DWORD target=DWORD(uintptr_t(remote));patch(process.hProcess,base+iatRva,&target,4);ready=true;
    }else if(x.ExceptionCode!=EXCEPTION_BREAKPOINT)action=DBG_EXCEPTION_NOT_HANDLED;
   }
   need(ContinueDebugEvent(e.dwProcessId,e.dwThreadId,action),"ContinueDebugEvent");
  }
  need(DebugActiveProcessStop(process.dwProcessId),"Detach original diagnostic");debug=false;
  std::cout<<"{\"event\":\"ready\",\"pid\":"<<process.dwProcessId<<",\"image_base\":"<<base<<",\"iat_rva\":"<<iatRva<<",\"original_entry_point_executed\":true,\"only_os_common_folder_import_redirected\":true}"<<std::endl;
  DWORD wait=WaitForSingleObject(process.hProcess,180000);if(wait==WAIT_TIMEOUT){EnumWindows(closeWindow,process.dwProcessId);wait=WaitForSingleObject(process.hProcess,15000);}
  if(wait!=WAIT_OBJECT_0){TerminateProcess(process.hProcess,124);throw std::runtime_error("Owned diagnostic process timed out");}
  DWORD result=0;need(GetExitCodeProcess(process.hProcess,&result),"Read exit code");CloseHandle(process.hThread);CloseHandle(process.hProcess);process={};std::cout<<"{\"event\":\"exit\",\"exit_code\":"<<result<<"}"<<std::endl;return result==0?0:1;
 }catch(const std::exception& error){std::cerr<<error.what()<<'\n';if(eventThread)CloseHandle(eventThread);if(process.hProcess){TerminateProcess(process.hProcess,125);if(debug)DebugActiveProcessStop(process.dwProcessId);WaitForSingleObject(process.hProcess,5000);CloseHandle(process.hThread);CloseHandle(process.hProcess);}return 1;}
}
