#!/usr/bin/env python3
"""Extract and verify pinned Minizip 1.01 from the existing zlib source archive."""
import argparse
import hashlib
import json
import subprocess
import sys
import tarfile
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    lock=json.loads((ROOT/'analysis/minizip-source.lock.json').read_text(encoding='utf-8'))
    source=ROOT/'.local/minizip-1.01'
    if not args.verify_only:
        archive_path=ROOT/'.local/codec-archives'/lock['archive']
        if not archive_path.is_file():
            subprocess.run([sys.executable,str(ROOT/'tools/fetch_codecs.py')],check=True)
        if hashlib.sha256(archive_path.read_bytes()).hexdigest()!=lock['archive_sha256']:
            raise ValueError('Minizip archive hash mismatch')
        source.mkdir(parents=True,exist_ok=True)
        with tarfile.open(archive_path) as archive:
            for row in lock['files']:
                relative=PurePosixPath(row['path'])
                if relative.is_absolute() or len(relative.parts)!=1 or '..' in relative.parts:
                    raise ValueError('Expected flat Minizip paths')
                data=archive.extractfile(lock['archive_root']+'/'+row['path']).read()
                if hashlib.sha256(data).hexdigest()!=row['sha256']:
                    raise ValueError('Archived Minizip source hash mismatch')
                target=source/row['path']
                if not target.exists(): target.write_bytes(data)
    for row in lock['files']:
        if hashlib.sha256((source/row['path']).read_bytes()).hexdigest()!=row['sha256']:
            raise ValueError('Modified Minizip source: '+row['path'])
    print(f"Verified Minizip 1.01: {len(lock['files'])} sources")


if __name__=='__main__': main()
