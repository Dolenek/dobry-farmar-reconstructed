#!/usr/bin/env python3
"""Read an original XML resource from shipped packages without mutation."""
import argparse
import sys
import zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('name')
args=parser.parse_args()
sys.stdout.reconfigure(encoding='utf-8')
for pack in ('Data.pack','cz.pack'):
    with zipfile.ZipFile(ROOT.parent/'farmář/Dobrý farmář/Data'/pack) as archive:
        found=next((name for name in archive.namelist() if name.lower()==args.name.lower()),None)
        if found:
            data=archive.read(found)
            print(pack,found)
            print(data.decode('utf-16') if data[:2] in (b'\xff\xfe',b'\xfe\xff') else data.decode('utf-8'))
