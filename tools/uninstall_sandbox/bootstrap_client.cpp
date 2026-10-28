// Own safety probe only, not an uninstall implementation or substitute.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <stdexcept>
void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){
    try {
        char module[MAX_PATH]{};need(GetModuleFileNameA(nullptr,module,MAX_PATH)>0,"Owned bootstrap probe image");std::string image(module),root;const auto temporary=image.find("\\temp\\~nsu.tmp\\");
        const bool child=temporary!=std::string::npos;
        if(child)root=image.substr(0,temporary);else {const auto payload=image.find("\\payload\\");need(payload!=std::string::npos,"Owned bootstrap parent location");root=image.substr(0,payload);}
        if(!child){
            const auto directory=root+"\\temp\\~nsu.tmp",destination=directory+"\\Au_.exe";need(CreateDirectoryA(directory.c_str(),nullptr)||GetLastError()==ERROR_ALREADY_EXISTS,"Owned bootstrap directory");
            need(CopyFileA(module,destination.c_str(),TRUE)!=FALSE,"Owned bootstrap copy");need(MoveFileExA(destination.c_str(),nullptr,MOVEFILE_DELAY_UNTIL_REBOOT|MOVEFILE_REPLACE_EXISTING)!=FALSE,"Owned copy delayed intent");
            std::string command="\""+destination+"\" /S _?="+root+"\\payload\\";STARTUPINFOA startup{sizeof(startup)};PROCESS_INFORMATION process{};
            need(CreateProcessA(nullptr,command.data(),nullptr,nullptr,FALSE,0,nullptr,nullptr,&startup,&process)!=FALSE,"Owned bootstrap genuine child creation");CloseHandle(process.hThread);CloseHandle(process.hProcess);
            need(MoveFileExA(directory.c_str(),nullptr,MOVEFILE_DELAY_UNTIL_REBOOT|MOVEFILE_REPLACE_EXISTING)!=FALSE,"Owned directory delayed intent");return 0;
        }
        HANDLE token=nullptr;need(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token)&&IsTokenRestricted(token),"Owned descendant restricted token");BYTE data[256]{};DWORD length=sizeof(data);need(GetTokenInformation(token,TokenIntegrityLevel,data,sizeof(data),&length)!=FALSE,"Owned descendant integrity query");CloseHandle(token);auto* label=reinterpret_cast<TOKEN_MANDATORY_LABEL*>(data);need(*GetSidSubAuthority(label->Label.Sid,*GetSidSubAuthorityCount(label->Label.Sid)-1)==0,"Owned descendant Untrusted integrity");
        for(auto hive:{HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE}){HKEY key=nullptr;need(RegCreateKeyExA(hive,"Software\\FarmarBootstrapClient",0,nullptr,0,KEY_ALL_ACCESS,nullptr,&key,nullptr)==ERROR_SUCCESS,"Owned descendant private registry mapping");DWORD proof=85;need(RegSetValueExA(key,"proof",0,REG_DWORD,reinterpret_cast<BYTE*>(&proof),4)==ERROR_SUCCESS,"Owned descendant private registry write");RegCloseKey(key);}
        HANDLE file=CreateFileA((root+"\\bootstrap-client-proof.txt").c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,0,nullptr);need(file!=INVALID_HANDLE_VALUE,"Owned descendant proof");constexpr char proof[]="restricted-descendant-before-entry";DWORD written=0;need(WriteFile(file,proof,sizeof(proof)-1,&written,nullptr)&&written==sizeof(proof)-1,"Owned descendant actual inside write");CloseHandle(file);return 0;
    }catch(const std::exception& error){OutputDebugStringA(error.what());return 11;}
}
