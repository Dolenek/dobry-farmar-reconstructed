#!/usr/bin/env python3
"""Read reference PE pointer tables without modifying binaries or Ghidra."""
import argparse
import hashlib
import json
from read_function import ROOT,NAMES
from pe import PE
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('address',type=lambda value:int(value,16))
    parser.add_argument('count',type=int)
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář'/NAMES['game']
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    pe=PE(original)
    rows={int(row['address'],16):row for row in map(json.loads,(ROOT/'analysis/raw/game/functions.jsonl').read_text().splitlines())}
    for slot in range(args.count):
        offset=pe.offset(args.address-pe.image_base+slot*4)
        address=int.from_bytes(pe.data[offset:offset+4],'little')
        row=rows.get(address)
        print(f'{slot*4:03x}: {address:08x} '+(row['name'] if row else '(not indexed)'))
if __name__=='__main__': main()
