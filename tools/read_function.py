#!/usr/bin/env python3
"""Read selected local Ghidra pseudocode and original PE instructions without mutation."""
import argparse
import hashlib
import json
import re
import shutil
import subprocess
import tempfile
from pathlib import Path
from pe import PE

ROOT = Path(__file__).resolve().parents[1]
NAMES = dict(game='Dobrý farmář.exe', jngload='JNGLoad.dll', squall='Squall.dll', uninstall='Uninstall.exe')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('addresses', nargs='+', type=lambda v: int(v, 16))
    parser.add_argument('--binary', choices=NAMES, default='game')
    parser.add_argument('--asm', action='store_true')
    parser.add_argument('--bytes', type=int, default=256, help='Instruction range when no function is indexed')
    args = parser.parse_args()
    inventory = json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    original = ROOT.parent/'farmář'/'Dobrý farmář'/NAMES[args.binary]
    expected = next(r['sha256'] for r in inventory['files'] if r['path']==NAMES[args.binary])
    if hashlib.sha256(original.read_bytes()).hexdigest() != expected:
        parser.error('Reference binary hash differs; addresses must be re-established')
    raw = ROOT/'analysis/raw'/args.binary
    source = (raw/'pseudocode.c').read_text(encoding='utf-8')
    functions = {int(r['address'],16):r for r in map(json.loads,(raw/'functions.jsonl').read_text().splitlines())}
    scratch = ROOT / '.local' / 'function-inspection'
    scratch.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='farmar-disassembly-', dir=scratch) as temporary:
        target = Path(temporary)/'reference.exe'
        if args.asm: shutil.copyfile(original,target)
        for address in args.addresses:
            match = re.search(r'/\* ADDRESS '+f'{address:08x}'+r'.*?(?=/\* ADDRESS |\Z)',source,re.S)
            print(match.group(0) if match else f'{address:08x}: no Ghidra function; inspect instructions')
            if args.asm:
                row = functions.get(address)
                end = int(row['body_max'],16)+1 if row else address+args.bytes
                result = subprocess.run(['objdump','-d','-Mintel',f'--start-address={address}',
                                         f'--stop-address={end}',target.name],cwd=temporary,
                                        check=True,capture_output=True,text=True)
                print(result.stdout)


if __name__ == '__main__': main()
