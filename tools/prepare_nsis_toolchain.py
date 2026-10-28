#!/usr/bin/env python3
"""Prepare generated headers and a source-only NSIS2.28 toolchain distribution."""
import argparse,hashlib,json,os,shutil,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def write(path,data):
 path.parent.mkdir(parents=True,exist_ok=True)
 if not path.exists() or path.read_bytes()!=data:path.write_bytes(data)
def configure(generated):
 config=json.loads((ROOT/'analysis/nsis-build-config.json').read_text(encoding='utf-8'));headers=[];defines=[]
 for row in config['defines']:
  name,value=row['name'],row['value'];headers.append('#define '+name+(' '+value if value is not None else ''))
  if value is None:defines.append('definedlist.add("'+name+'");')
  else:defines.append('definedlist.add("'+name+'",'+(value if value.startswith('"') else '"'+value+'"')+');')
 include=generated/'include';write(include/'sconf.h',('\n'.join(headers)+'\n').encode());write(include/'defines.h',('\n'.join(defines)+'\n').encode());write(include/'version.h',b'#define NSIS_VERSION "v2.28"\n');write(include/'afxres.h',b'#include <windows.h>\n#ifndef IDC_STATIC\n#define IDC_STATIC -1\n#endif\n')

def runtime_platform(data,config):
 # Source-built executable stub only. Preserve every byte except the original
 # platform compatibility fields; modern LINK defaults alter dialog frames.
 result=bytearray(data);pe=struct.unpack_from('<I',result,0x3c)[0];optional=pe+24
 if result[:2]!=b'MZ' or result[pe:pe+4]!=b'PE\0\0' or struct.unpack_from('<H',result,optional)[0]!=0x10b:raise ValueError('Expected own source PE32 runtime stub')
 if optional+72>len(result):raise ValueError('Truncated own source runtime optional header')
 for offset,name in [(40,'os_version'),(48,'subsystem_version')]:struct.pack_into('<HH',result,optional+offset,*config[name])
 struct.pack_into('<H',result,optional+70,config['dll_characteristics'])
 changed=[i for i,(left,right) in enumerate(zip(data,result)) if left!=right]
 allowed=set(range(optional+40,optional+44))|set(range(optional+48,optional+52))|set(range(optional+70,optional+72))
 if not set(changed)<=allowed:raise ValueError('Unexpected code/resource change in source platform preparation')
 return bytes(result),dict(input_sha256=hashlib.sha256(data).hexdigest(),staged_sha256=hashlib.sha256(result).hexdigest(),changed_byte_offsets=changed,only_platform_header_fields_changed=True,target=config)
def stage(a):
 dist=a.generated/'distribution';app=a.generated/'application';config=json.loads((ROOT/'analysis/nsis-build-config.json').read_text(encoding='utf-8'));low,high=config['asset_filetime'];timestamp=((high<<32)|low)/10000000-11644473600
 for directory in ['Include','Contrib/Modern UI','Contrib/Language files','Contrib/Graphics']:
  shutil.copytree(a.source/directory,dist/directory,dirs_exist_ok=True)
 for path in dist.rglob('*'):
  if path.is_file():os.utime(path,(timestamp,timestamp))
 files=[(a.compiler,'makensis.exe'),(a.lzma,'Stubs/lzma_solid'),(a.zlib,'Stubs/zlib'),(a.langdll,'Plugins/LangDLL.dll'),(a.modern,'Contrib/UIs/modern.exe'),(a.modern_header,'Contrib/UIs/modern_headerbmp.exe'),(a.source/'Source/exehead/uninst.ico','Stubs/uninst'),(a.source/'COPYING','COPYING')]
 platform_reports={}
 for source,destination in files:
  data=source.read_bytes()
  if destination in ['Stubs/lzma_solid','Stubs/zlib']:data,platform_reports[destination]=runtime_platform(data,config['runtime_pe_platform'])
  target=dist/destination;write(target,data);os.utime(target,(timestamp,timestamp))
 write(a.generated/'runtime-platform.json',(json.dumps(platform_reports,indent=2)+'\n').encode('utf-8'))
 for name in ['reconstructed_uninstall.nsi','original_removal.nsh','special_offer_page.ini']:
  text=(ROOT/'src/uninstall'/name).read_text(encoding='utf-8');data=text.replace('\r\n','\n').replace('\n','\r\n').encode('cp1250');target=app/name;write(target,data);os.utime(target,(timestamp,timestamp))
 write(app/'uninstall-artwork.ico',(ROOT/'src/uninstall/uninstall.ico').read_bytes());os.utime(app/'uninstall-artwork.ico',(timestamp,timestamp))
 print('Staged NSIS2.28 source-built compiler/runtime/UI/plugin and CP1250 application sources; no code executed')
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--generated',type=Path,required=True);p.add_argument('--source',type=Path);p.add_argument('--stage',action='store_true')
 for name in ['compiler','lzma','zlib','langdll','modern','modern-header']:p.add_argument('--'+name,type=Path)
 a=p.parse_args();configure(a.generated)
 if a.stage:
  if any(getattr(a,name) is None for name in ['source','compiler','lzma','zlib','langdll','modern','modern_header']):p.error('Staging requires source and all six source-built artifacts')
  stage(a)
if __name__=='__main__':main()
