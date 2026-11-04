#!/usr/bin/env python3
"""Retain native instructions for every dispatched tracker loader and old field uses."""
import datetime
import json
from pathlib import Path
import re
import subprocess

from validate_squall_tracker_stages import ROOT,digest

LOADERS=[('XM','load_xm.cpp','100258a0'),('IT','load_it.cpp','1001d240'),
         ('S3M','load_s3m.cpp','10024450'),('STM','load_stm.cpp','10024c50'),
         ('Med','load_med.cpp','1001fa10'),('MTM','load_mtm.cpp','10022920'),
         ('MDL','load_mdl.cpp','1001ee40'),('DBM','load_dbm.cpp','1001aa80'),
         ('669','load_669.cpp','100187b0'),('FAR','load_far.cpp','1001c670'),
         ('AMS','load_ams.cpp','100197f0'),('OKT','load_okt.cpp','10022e30'),
         ('PTM','load_ptm.cpp','10023d10'),('Ult','load_ult.cpp','10025160'),
         ('DMF','load_dmf.cpp','1001b4f0'),('DSM','load_dsm.cpp','1001c110'),
         ('UMX','load_umx.cpp','100257b0'),('AMF','load_amf.cpp','10018f80'),
         ('PSM','load_psm.cpp','10023520'),('MT2','load_mt2.cpp','10021b40'),
         ('Mod','load_mod.cpp','10021250')]


def main():
    work=ROOT/'.local/squall-tracker-loader-offsets'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work.mkdir(parents=True)
    raw=ROOT/'analysis/raw/squall/pseudocode.c';text=raw.read_text(encoding='utf-8')
    functions=sorted(int(json.loads(line)['address'],16) for line in (ROOT/'analysis/raw/squall/functions.jsonl').read_text(encoding='utf-8').splitlines())
    dump=Path('C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.43.34808/bin/Hostx64/x86/dumpbin.exe')
    binary=ROOT.parent/'farmář/Dobrý farmář/Squall.dll'
    report=dict(status='native_tracker_loader_offset_instructions_not_complete_body_equivalence',
                work_directory=work.relative_to(ROOT).as_posix(),reference_sha256=digest(binary),
                raw_pseudocode_sha256=digest(raw),dumpbin_sha256=digest(dump),loaders=[])
    for name,path,address in LOADERS:
        value=int(address,16);end=min(a for a in functions if a>value)
        output=work/f'{name}-{address}.txt'
        result=subprocess.run([str(dump),'/DISASM:BYTES',f'/RANGE:0x{value:x},0x{end:x}',str(binary)],capture_output=True,check=True)
        output.write_bytes(result.stdout)
        a=text.index('/* ADDRESS '+address+' ');b=text.index('/* ADDRESS ',a+15);body=text[a:b]
        fields={}
        for offset,meaning in [(0xb7a8,'old Order / new SamplesInFile'),(0xd048,'old default speed'),
                               (0xd04c,'old default tempo / new default speed'),(0xd054,'old song flags / new default global volume'),
                               (0xd058,'old channels / new song flags'),(0xd068,'old type / new buffer count'),
                               (0xd06c,'old sample count / new type'),(0xd0e4,'old title / new pattern-name pointer')]:
            spelling=f'{offset:X}h'
            lines=[line.strip() for line in result.stdout.decode('utf-8','replace').splitlines() if re.search(r'\+0*'+spelling+r'\]',line)]
            if lines:fields[hex(offset)]=dict(meaning=meaning,instruction_count=len(lines),instructions=lines)
        source=ROOT/'src/squall/runtime/SoundFile/track'/path
        report['loaders'].append(dict(name=name,path=path,address=address,instruction_range_end=hex(end),
                                      instruction_sha256=digest(output),field_instructions=fields,
                                      source_sha256=digest(source),legacy_view_already_present='legacy_m_nChannels' in source.read_text(encoding='utf-8')))
    encoded=(json.dumps(report,indent=2)+'\n').encode('utf-8')
    (work/'report.json').write_bytes(encoded);(ROOT/'.local/squall-tracker-loader-offsets-validation.json').write_bytes(encoded)
    print('Retained21 exact-entry instruction ranges and old field accesses;20 loaders still require reconstruction after MOD.')


if __name__=='__main__':main()
