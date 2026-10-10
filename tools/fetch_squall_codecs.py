#!/usr/bin/env python3
"""Fetch hash-pinned historical Ogg/Vorbis candidates, without claiming original versions."""
import argparse
import hashlib
import json
import subprocess
import tarfile
from pathlib import Path, PurePosixPath
ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--verify-only',action='store_true')
    args=parser.parse_args()
    lock=json.loads((ROOT/'analysis/squall-codecs.lock.json').read_text(encoding='utf-8'))
    cache=ROOT/'.local/squall-codec-archives';cache.mkdir(parents=True,exist_ok=True)
    for dependency in lock['dependencies']:
        source=ROOT/'.local'/dependency['name']
        if not args.verify_only:
            path=cache/dependency['archive']
            if not path.exists():
                subprocess.run(['curl.exe','-fLsS','--retry','2','--connect-timeout','10','--max-time','60',dependency['url'],'-o',str(path)],check=True)
            assert hashlib.sha256(path.read_bytes()).hexdigest()==dependency['sha256'],dependency['archive']
            with tarfile.open(path,'r:gz') as archive:
                for item in dependency['files']:
                    relative=PurePosixPath(item['path'])
                    if relative.is_absolute() or '..' in relative.parts: raise ValueError('Unsafe source path')
                    target=source.joinpath(*relative.parts)
                    if target.exists(): continue
                    member=archive.getmember(dependency['name']+'/'+item['path'])
                    if not member.isfile(): raise ValueError('Expected a regular source file')
                    data=archive.extractfile(member).read()
                    assert hashlib.sha256(data).hexdigest()==item['sha256'],item['path']
                    target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
        for item in dependency['files']:
            assert hashlib.sha256((source/item['path']).read_bytes()).hexdigest()==item['sha256'],item['path']
        print(f"Verified {dependency['name']} candidate: {len(dependency['files'])} source/license files")

if __name__=='__main__':main()
