#!/usr/bin/env python3
"""Fetch and verify exact upstream NSIS2.28 sources; no prebuilt program runtime."""
import argparse,hashlib,json,urllib.request,zipfile
from pathlib import Path,PurePosixPath
ROOT=Path(__file__).resolve().parents[1]
def digest(data):return hashlib.sha256(data).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--verify-only',action='store_true');p.add_argument('--source-dir',type=Path,default=ROOT/'.local/nsis-2.28-source');p.add_argument('--archive',type=Path,default=ROOT/'.local/nsis-2.28-source.zip');a=p.parse_args()
 lock=json.loads((ROOT/'analysis/nsis-source.lock.json').read_text(encoding='utf-8'))
 if not a.verify_only:
  if not a.archive.exists():
   request=urllib.request.Request(lock['archive_url'],headers={'User-Agent':'Farmar-source-reconstruction'})
   with urllib.request.urlopen(request,timeout=40) as response:data=response.read()
   if digest(data)!=lock['archive_sha256']:raise ValueError('NSIS archive hash differs')
   a.archive.parent.mkdir(parents=True,exist_ok=True);a.archive.write_bytes(data)
  if digest(a.archive.read_bytes())!=lock['archive_sha256']:raise ValueError('NSIS archive hash differs')
  with zipfile.ZipFile(a.archive) as archive:
   for row in lock['files']:
    path=PurePosixPath(row['path'])
    if path.is_absolute() or '..' in path.parts:raise ValueError('Unsafe locked source path')
    target=a.source_dir.joinpath(*path.parts)
    if target.exists():continue
    data=archive.read(lock['archive_root']+'/'+row['path'])
    if digest(data)!=row['sha256']:raise ValueError('NSIS member hash differs:'+row['path'])
    target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
 for row in lock['files']:
  if digest((a.source_dir/row['path']).read_bytes())!=row['sha256']:raise ValueError('NSIS source hash differs:'+row['path'])
 print(f'Verified NSIS2.28 commit{lock["commit"]}: {len(lock["files"])} source/license/assets files')
if __name__=='__main__':main()
