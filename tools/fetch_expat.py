#!/usr/bin/env python3
"""Fetch official pinned Expat sources, preserving paths and verifying Git blobs."""
import argparse
import concurrent.futures
import hashlib
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--verify-only', action='store_true')
    parser.add_argument('--source-dir', type=Path, default=ROOT/'.local/expat-source')
    args = parser.parse_args()
    lock = json.loads((ROOT/'analysis/expat-source.lock.json').read_text())

    def fetch(item):
        relative = Path(item['path'])
        if relative.is_absolute() or '..' in relative.parts or relative.parts[0] != 'expat':
            raise ValueError('Invalid locked source path')
        target = args.source_dir/relative
        target.parent.mkdir(parents=True, exist_ok=True)
        temporary = target
        if not target.exists():
            if args.verify_only:
                raise ValueError(f'Missing pinned Expat file: {relative}')
            temporary = target.with_name(target.name+'.download')
            url = f"https://raw.githubusercontent.com/libexpat/libexpat/{lock['commit']}/{item['path']}"
            subprocess.run(['curl.exe' if __import__('os').name == 'nt' else 'curl', '-fLsS',
                            '--connect-timeout', '10', '--max-time', '60', '--retry', '2',
                            url, '-o', str(temporary)], check=True)
        data = temporary.read_bytes()
        actual = hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()
        if actual != item['git_blob_sha1']:
            raise ValueError(f'Expat source hash mismatch: {relative}')
        if temporary != target:
            temporary.replace(target)

    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
        list(pool.map(fetch, lock['files']))
    print(f"Verified Expat candidate 1.95.8: {len(lock['files'])} files at {lock['commit']}")


if __name__ == '__main__':
    main()
