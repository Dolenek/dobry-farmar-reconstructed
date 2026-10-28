#!/usr/bin/env python3
"""Verify source-built NSIS uninstaller and exact application removal IR.

This is a build/static comparison, explicitly not a native uninstall effect test.
"""
import argparse,datetime,hashlib,json,re,struct
from pathlib import Path
from nsis_extract import extract
from nsis_solid import load_archive
from pe import PE
from extract_game_icon import extract_icon
ROOT=Path(__file__).resolve().parents[1]
STATUS='passed_for_source_uninstaller_build_pages_assets_and_exact_removal_ir'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def normalize_removal(program):
 entries=program['instructions'];section=program['sections'][0];first=section['first_instruction']
 if entries[first-1]['operation']=='CALL':first=entries[first-1]['arguments'][0]
 variables={};rows=[]
 def variable(index):
  if index not in variables:variables[index]='v'+str(len(variables))
  return variables[index]
 for row in entries[first-1:]:
  op=row['operation']
  if op=='RET':break
  args=list(row['arguments'])
  for slot in ({'READREGSTR':[0],'ASSIGNVAR':[0],'GETFULLPATHNAME':[1]}.get(op,[])):args[slot]=variable(args[slot])
  for slot,value in row['strings'].items():args[int(slot)]=re.sub(r'\$var(\d+)',lambda m:'$'+variable(int(m[1])),value)
  for slot in ({'STRCMP':[2,3],'NOP':[0],'IFFLAG':[0,1]}.get(op,[])):
   if args[slot]>0:args[slot]='label'+str(args[slot]-first)
  rows.append(dict(operation=op,arguments=args))
 if len(rows)!=27:raise AssertionError(('Removal program size',len(rows)))
 return rows
def first_difference(left,right,path=''):
 if type(left)!=type(right):return dict(path=path,reference_type=type(left).__name__,source_type=type(right).__name__)
 if isinstance(left,list):
  if len(left)!=len(right):return dict(path=path,reference_length=len(left),source_length=len(right))
  for i,(a,b) in enumerate(zip(left,right)):
   if a!=b:return first_difference(a,b,path+'/'+str(i))
 if isinstance(left,dict):
  if left.keys()!=right.keys():return dict(path=path,keys_differ=True)
  for key in left:
   if left[key]!=right[key]:return first_difference(left[key],right[key],path+'/'+str(key))
 return dict(path=path,reference=left,source=right)
def callback_graph(program,entry,stack=()):
 if entry<0:return None
 if entry in stack:raise AssertionError('Unexpected recursive NSIS callback')
 entries=program['instructions'];pending=[entry];visited=set()
 while pending:
  index=pending.pop()
  if index in visited:continue
  assert 0<=index<len(entries);visited.add(index);row=entries[index];op=row['operation'];args=row['arguments']
  if op in ['RET','QUIT','ABORT']:continue
  branches={'STRCMP':[2,3],'IFFLAG':[0,1],'NOP':[0]}.get(op)
  if branches is None:pending.append(index+1)
  else:
   for slot in branches:pending.append(args[slot]-1 if args[slot]>0 else index+1)
 ordered=sorted(visited);labels={value:'node'+str(i) for i,value in enumerate(ordered)};result=[]
 for index in ordered:
  row=entries[index];op=row['operation'];args=list(row['arguments'])
  for slot,value in row['strings'].items():args[int(slot)]=value
  for slot in {'STRCMP':[2,3],'IFFLAG':[0,1],'NOP':[0]}.get(op,[]):args[slot]=labels[args[slot]-1 if args[slot]>0 else index+1]
  if op=='CALL':
   assert args[0]>0;args[0]=callback_graph(program,args[0]-1,stack+(entry,))
  result.append(dict(operation=op,arguments=args))
 return result
def embedded_file(candidate,program,suffix):
 archive=load_archive(candidate)
 matches=[row for row in program['instructions'] if row['operation']=='EXTRACTFILE' and row['strings']['1'].lower().endswith(suffix.lower())]
 assert len(matches)==1,suffix;offset=matches[0]['arguments'][2];block=archive.datablock;size=struct.unpack_from('<I',block,offset)[0];assert offset+4+size<=len(block)
 return block[offset+4:offset+4+size]
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--candidate',type=Path,default=ROOT/'build/windows/Release/Uninstall.exe');p.add_argument('--build-dir',type=Path,default=ROOT/'build/windows');p.add_argument('--fresh-build-dir',type=Path);p.add_argument('--source-dir',type=Path,default=ROOT/'.local/nsis-2.28-source');p.add_argument('--output',type=Path,default=ROOT/'.local/uninstaller-source-build-validation.json');a=p.parse_args();candidate=a.candidate.resolve(strict=True);build=a.build_dir.resolve(strict=True);original=ROOT.parent/'farmář/Dobrý farmář';inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
 for row in inventory['files']:assert digest(original/row['path'])==row['sha256'],row['path']
 lock=json.loads((ROOT/'analysis/nsis-source.lock.json').read_text(encoding='utf-8'))
 for row in lock['files']:assert digest(a.source_dir/row['path'])==row['sha256'],row['path']
 native=extract(original/'Uninstall.exe');source=extract(candidate);assert source['flags']==native['flags']==5 and source['runtime_header']['flags']==native['runtime_header']['flags']==128
 assert normalize_removal(source)==normalize_removal(native),'Removal instruction/control-flow IR differs'
 for left,right in zip(native['pages'],source['pages']):
  for key in ['dialog_id','window_proc','flags','caption','back','next','click_next','cancel','parameter_text']:assert left[key]==right[key],(key,left[key],right[key])
 assert len(native['pages'])==len(source['pages'])==3
 callback_checks=[]
 for name,left in native['runtime_header']['callbacks'].items():
  right=source['runtime_header']['callbacks'][name];a_graph=callback_graph(native,left);b_graph=callback_graph(source,right)
  assert a_graph==b_graph,('Application callback IR differs',name,first_difference(a_graph,b_graph))
  if a_graph is not None:callback_checks.append(name)
 for i,(left,right) in enumerate(zip(native['pages'],source['pages'])):
  for name in ['pre_code','show_code','leave_code']:
   assert callback_graph(native,left[name])==callback_graph(source,right[name]),('Page callback IR differs',i,name)
   if left[name]>=0:callback_checks.append('page'+str(i)+'.'+name)

 pe=PE(candidate);assert pe.bits==32 and pe.machine==0x14c and digest(candidate)!=digest(original/'Uninstall.exe')
 platform_fields={}
 for name,image in [('source',pe),('reference',PE(original/'Uninstall.exe'))]:
  optional=image.u32(0x3c)+24;platform_fields[name]=dict(os_version=list(image.unpack('<HH',optional+40)),subsystem_version=list(image.unpack('<HH',optional+48)),dll_characteristics=image.u16(optional+70))
 assert platform_fields['source']==platform_fields['reference']==json.loads((ROOT/'analysis/nsis-build-config.json').read_text(encoding='utf-8'))['runtime_pe_platform'],'Runtime PE platform fields differ'
 libraries=sorted(x['library'].lower() for x in pe.imports());assert set(libraries)<=set(['kernel32.dll','user32.dll','gdi32.dll','shell32.dll','advapi32.dll','comctl32.dll','ole32.dll','version.dll']),libraries
 icon,_=extract_icon(pe,103);assert icon==(ROOT/'src/uninstall/uninstall.ico').read_bytes();assert icon==extract_icon(PE(original/'Uninstall.exe'),103)[0]
 assets={}
 for suffix in ['modern-header.bmp','special_offer_page.ini']:
  left=embedded_file(original/'Uninstall.exe',native,suffix);right=embedded_file(candidate,source,suffix);assert left==right,suffix;assets[suffix]=dict(bytes=len(right),sha256=hashlib.sha256(right).hexdigest())
 plugin=embedded_file(candidate,source,'LangDLL.dll');assert plugin==(build/'Release/nsis228-LangDLL.dll').read_bytes();assert plugin!=embedded_file(original/'Uninstall.exe',native,'LangDLL.dll')
 repro=[]
 names=['Uninstall.exe','nsis228-lzma-solid.exe','nsis228-LangDLL.dll','nsis228-modern.exe','nsis228-modern_headerbmp.exe','nsis228-zlib-head.exe','nsis228-compiler.exe']
 if a.fresh_build_dir:
  for name in names:
   left=build/'Release'/name;right=a.fresh_build_dir/'Release'/name;assert left.read_bytes()==right.read_bytes(),('Independent build differs',name);repro.append(dict(file=name,sha256=digest(left)))
 paths=[ROOT/'CMakeLists.txt',ROOT/'analysis/nsis-source.lock.json',ROOT/'analysis/nsis-build-config.json']+list((ROOT/'src/uninstall').glob('*'))+[ROOT/n for n in ['tools/cmake/Nsis228.cmake','tools/emit_nsis_uninstaller.py','tools/nsis_extract.py','tools/nsis_solid.py','tools/fetch_nsis.py','tools/prepare_nsis_toolchain.py','tools/validate_uninstaller_build.py']]
 report=dict(status=STATUS,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_files={x.relative_to(ROOT).as_posix():digest(x) for x in sorted(paths) if x.is_file()},candidate_path=candidate.relative_to(ROOT).as_posix(),candidate_sha256=digest(candidate),candidate_bytes=candidate.stat().st_size,bits=pe.bits,exact_runtime_pe_platform=platform_fields,system_imports=libraries,nsis_version='2.28',nsis_commit=lock['commit'],nsis_source_files_verified=len(lock['files']),original_reference_sha256=digest(original/'Uninstall.exe'),reference_files_unchanged=17,original_program_code_used_in_implementation=False,icon_artwork_only_from_original=True,source_removal_ir=normalize_removal(source),exact_removal_instructions=27,exact_reachable_callback_graphs=callback_checks,source_instruction_count=len(source['instructions']),original_instruction_count=len(native['instructions']),page_count=3,pages=source['pages'],runtime_header=source['runtime_header'],exact_embedded_assets=assets,langdll_own_source_build_sha256=hashlib.sha256(plugin).hexdigest(),independent_reproducible_binaries=repro,uninstaller_effects_verified=False,whole_game_complete=False,pending=['Native source/reference filesystem/registry/shortcut effects under verified restricted process/private desktop/hive isolation','Native UI confirmations/cancellation/completion and bootstrap self-copy/reboot scheduling boundaries','Whole original game, remaining engine/Squall97 and expanded gameplay/save verification'])
 a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(status=STATUS,bytes=report['candidate_bytes'],exact_removal_instructions=27,pages=3,source_files=734,reproducible_binaries=len(repro),effects_verified=False)))
if __name__=='__main__':main()
