#!/usr/bin/env python3
"""Fetch source declarations for system Direct3D8; no runtime DLL is downloaded."""
import argparse
import hashlib
import json
import subprocess
import os
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--verify-only',action='store_true')
    parser.add_argument('--source-dir',type=Path,default=ROOT/'.local/direct3d8-headers')
    args=parser.parse_args()
    lock=json.loads((ROOT/'analysis/direct3d8-headers.lock.json').read_text())
    for row in lock['files']:
        name=Path(row['path'])
        if name.name!=row['path']: raise ValueError('Unexpected source path')
        target=args.source_dir/name
        if not target.exists():
            if args.verify_only: raise ValueError(f'Missing Direct3D8 declaration: {name}')
            target.parent.mkdir(parents=True,exist_ok=True)
            temporary=target.with_suffix(target.suffix+'.download')
            subprocess.run(['curl.exe' if os.name=='nt' else 'curl','-fLsS','--connect-timeout','10',
                '--max-time','60','--retry','2',row['url'],'-o',str(temporary)],check=True)
            if hashlib.sha256(temporary.read_bytes()).hexdigest()!=row['sha256']: raise ValueError('Source hash mismatch')
            temporary.replace(target)
        if hashlib.sha256(target.read_bytes()).hexdigest()!=row['sha256']: raise ValueError(f'Source changed: {name}')
    print(f"Verified {len(lock['files'])} Direct3D8 header/license files at {lock['commit']}")

if __name__=='__main__': main()
