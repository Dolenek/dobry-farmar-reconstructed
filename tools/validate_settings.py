#!/usr/bin/env python3
"""Validate recovered settings through archives/XML and isolated UTF-16 writes."""
import argparse
import datetime
import hashlib
import json
import subprocess
import tempfile
import xml.etree.ElementTree as ET
import zipfile
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--test',type=Path,default=ROOT/'build/windows/Release/settings-test.exe')
    parser.add_argument('--loader',type=Path,default=ROOT/'build/windows/Release/resource-inspect.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/settings-validation.json')
    args=parser.parse_args()
    test=args.test.resolve(strict=True)
    loader=args.loader.resolve(strict=True)
    scratch=ROOT/'.local/settings-validation-work'
    scratch.mkdir(parents=True,exist_ok=True)
    report=dict(recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                test_sha256=hashlib.sha256(test.read_bytes()).hexdigest(),
                loader_sha256=hashlib.sha256(loader.read_bytes()).hexdigest(),
                original_settings_runtime_compared=False,
                pending=['Original settings runtime oracle','CRT numeric malformed/overflow inputs',
                         'Non-ASCII narrow conversions','Other property types and engine ABI'])
    with tempfile.TemporaryDirectory(dir=scratch,prefix='run-') as temporary:
        work=Path(temporary)
        data=work/'data'; data.mkdir()
        common=work/'common'; common.mkdir()
        xml='<main><game window_title="Dobrý farmář"/><graphics width="800" height="600" ShowSysCursor="0"/></main>'
        with zipfile.ZipFile(data/'Data.pack','w',compression=zipfile.ZIP_DEFLATED) as archive:
            archive.writestr('sys/settings.xml',xml.encode('utf-8'))
            archive.writestr('extra.xml',b'<main><graphics width="1024"/><duplicate value="first"/><duplicate value="last"/></main>')
        result=subprocess.run([str(test),str(data),str(common)],capture_output=True,text=True,timeout=60,check=True)
        report['native_checks']=int(result.stdout.strip())
        saved=(common/'sys/settings.xml').read_bytes()
        expected='<?xml version="1.0" encoding="UTF-16"?><main><duplicate value="last" /><game ascii="abc" window_title="Dobrý farmář" /><graphics ShowSysCursor="1" Width="" height="600" width="1280" /></main>'
        assert saved==b'\xff\xfe'+expected.encode('utf-16le'), 'serialized byte/ordering mismatch'
        report['exact_utf16_fixture_bytes']=len(saved)
        tree=ET.fromstring(saved)
        expected_properties={child.tag:child.attrib for child in tree}
        output=work/'settings.json'
        subprocess.run([str(loader),'settings',str(data),'',str(output),str(common)],check=True,timeout=60)
        assert json.loads(output.read_bytes())==expected_properties
        report['saved_properties_compared']=sum(map(len,expected_properties.values()))
        # Real data is read-only. Its loader output must match independent XML parsing.
        original=ROOT.parent/'farmář/Dobrý farmář'
        inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
        all_entries={}
        for name in ('Data.pack','cz.pack'):
            packed=original/'Data'/name
            record=next(row for row in inventory['files'] if row['path']==f'Data/{name}')
            assert hashlib.sha256(packed.read_bytes()).hexdigest()==record['sha256']
            with zipfile.ZipFile(packed) as archive:
                for entry in archive.infolist():
                    if entry.filename.lower()=='sys/settings.xml': all_entries[name]=archive.read(entry)
        reference=ET.fromstring(all_entries[sorted(all_entries)[-1]])
        properties={}
        for child in sorted(reference,key=lambda node:node.tag): properties.setdefault(child.tag,{}).update(child.attrib)
        subprocess.run([str(loader),'settings',str(original/'Data'),'',str(output)],check=True,timeout=60)
        assert json.loads(output.read_bytes())==properties
        report['original_data_properties_compared']=sum(map(len,properties.values()))
    report['status']='passed'
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(f"Settings: {report['native_checks']} native checks, exact UTF-16 save, {report['original_data_properties_compared']} original properties")

if __name__=='__main__': main()
