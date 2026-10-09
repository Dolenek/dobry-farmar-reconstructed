#!/usr/bin/env python3
"""Inspect original menu XML/font references without changing archives."""
from pathlib import Path
import zipfile
import sys
sys.stdout.reconfigure(encoding='utf-8')
ROOT=Path(__file__).resolve().parents[1]
for pack in ('Data.pack','cz.pack'):
    with zipfile.ZipFile(ROOT.parent/'farmář/Dobrý farmář/Data'/pack) as archive:
        for name in archive.namelist():
            if (name.lower().endswith('.xml') and ('menu' in name.lower() or name.lower()=='sys/strings.xml')) or ('font' in name.lower() and not name.endswith('/')):
                print(pack,name)
                if name.lower().endswith('.xml'):
                    data=archive.read(name)
                    text=data.decode('utf-16') if data[:2] in (b'\xff\xfe',b'\xfe\xff') else data.decode('utf-8')
                    print(text)
