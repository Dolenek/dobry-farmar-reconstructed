#!/usr/bin/env python3
"""Prepare generated headers and a source-only NSIS2.28 toolchain distribution."""
import argparse,json,os,shutil
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
def stage(a):
 dist=a.generated/'distribution';app=a.generated/'application';config=json.loads((ROOT/'analysis/nsis-build-config.json').read_text(encoding='utf-8'));low,high=config['asset_filetime'];timestamp=((high<<32)|low)/10000000-11644473600
 for directory in ['Include','Contrib/Modern UI','Contrib/Language files','Contrib/Graphics']:
  shutil.copytree(a.source/directory,dist/directory,dirs_exist_ok=True)
 for path in dist.rglob('*'):
  if path.is_file():os.utime(path,(timestamp,timestamp))
 files=[(a.compiler,'makensis.exe'),(a.lzma,'Stubs/lzma_solid'),(a.zlib,'Stubs/zlib'),(a.langdll,'Plugins/LangDLL.dll'),(a.modern,'Contrib/UIs/modern.exe'),(a.modern_header,'Contrib/UIs/modern_headerbmp.exe'),(a.source/'Source/exehead/uninst.ico','Stubs/uninst'),(a.source/'COPYING','COPYING')]
 for source,destination in files:
  target=dist/destination;write(target,source.read_bytes());os.utime(target,(timestamp,timestamp))
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
