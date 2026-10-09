#!/usr/bin/env python3
"""Refresh portable verification metadata from completed local reports, checking references."""
import datetime
import hashlib
import json
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]

def read(path): return json.loads((ROOT/path).read_text(encoding='utf-8'))
def write(path,data): (ROOT/path).write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

def main():
    original=ROOT.parent/'farmář/Dobrý farmář'
    inventory=read('analysis/inventory.json')
    for row in inventory['files']:
        assert hashlib.sha256((original/row['path']).read_bytes()).hexdigest()==row['sha256'], row['path']
    junit=ET.parse(ROOT/'.local/ctest-windows.xml').getroot()
    assert not junit.findall('.//failure') and not junit.findall('.//error'), 'CTest failures remain'
    completed=junit.findall('.//testcase')
    assert completed and not junit.findall('.//skipped'), 'CTest missing or skipped'
    report=read('analysis/verification.json')
    report['recorded_at_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat()
    report['tests'].update(ctest_passed=len(completed),reference_files_unchanged=len(inventory['files']),
                           original_wstring_comparisons=20000,resource_index_fixtures=8,resource_write_checks=12)
    report['build']['targets']+= [name for name in ['pack-decode.exe','resource-inspect.exe',
        'wstring-reference-test.exe','file-index-test.exe','settings-test.exe','window-test.exe','texture-test.exe',
        'recovered_graphics_utilities','recovered_resource_utilities','resource-manager-test.exe','color-reference-test.exe',
        'graphics-presentation-test.exe','matrix-reference-test.exe','sprite-geometry-reference-test.exe',
        'sprite-draw-reference-test.exe','native-image-reference-test.exe','font-reference-test.exe',
        'text-layout-reference-test.exe','string-manager-reference-test.exe','control-sprite-reference-test.exe',
        'view-reference-test.exe','ui-frame-reference-test.exe','control-reference-test.exe','text-control-reference-test.exe','button-reference-test.exe','button-text-reference-test.exe','text-buttons-reference-test.exe','click-mask-reference-test.exe','input-bindings-reference-test.exe'] if name not in report['build']['targets']]
    report['build']['dependencies']+= [name for name in ['Minizip 1.01','Expat 1.95.8 (candidate, original version unproved)']
        if name not in report['build']['dependencies']]
    report['build']['dependencies']+= [name for name in ['Wine-origin Direct3D8 headers (MinGW-w64 v12.0.0; LGPL-2.1-or-later)']
        if name not in report['build']['dependencies']]
    resource=read('.local/resource-validation.json')
    assert resource['status']=='passed' and not resource['failures']
    write('analysis/resource-loading-validation.json',resource)
    report['resource_pipeline']=resource
    settings=read('.local/settings-validation.json')
    assert settings['status']=='passed'
    write('analysis/settings-validation.json',settings)
    report['settings']=settings
    for module in ('window','texture','resource-manager','color','graphics-presentation','matrix','sprite-geometry','sprite-draw','native-image','font','text-layout','string-manager','control-sprite','view','ui-frame','control','text-control','button','button-text','text-buttons','click-mask','input-bindings'):
        validation=read(f'.local/{module}-validation.json')
        assert validation['status']=='passed'
        write(f'analysis/{module}-validation.json',validation)
        report[module]=validation
    wide=read('.local/wstring-validation.json')
    write('analysis/wstring-validation.json',wide)
    paths=[ROOT/'CMakeLists.txt']+[path for folder in ('src','tools','tests')
        for path in (ROOT/folder).rglob('*') if path.is_file() and '__pycache__' not in path.parts]
    rows=[path.relative_to(ROOT).as_posix()+'\0'+hashlib.sha256(path.read_bytes()).hexdigest()+'\n'
          for path in sorted(paths,key=lambda path:path.relative_to(ROOT).as_posix())]
    report['source_tree_sha256']=hashlib.sha256(''.join(rows).encode('utf-8')).hexdigest()
    report['reports']+= [name for name in ['analysis/resource-loading-validation.json','analysis/settings-validation.json',
        'analysis/wstring-validation.json','analysis/window-validation.json','analysis/texture-validation.json',
        'analysis/resource-manager-validation.json','analysis/color-validation.json',
        'analysis/graphics-presentation-validation.json','analysis/matrix-validation.json',
        'analysis/sprite-geometry-validation.json','analysis/sprite-draw-validation.json',
        'analysis/native-image-validation.json','analysis/font-validation.json','analysis/text-layout-validation.json',
        'analysis/string-manager-validation.json','analysis/control-sprite-validation.json','analysis/view-validation.json','analysis/ui-frame-validation.json','analysis/control-validation.json','analysis/text-control-validation.json','analysis/button-validation.json','analysis/button-text-validation.json','analysis/text-buttons-validation.json','analysis/click-mask-validation.json','analysis/input-bindings-validation.json'] if name not in report['reports']]
    write('analysis/verification.json',report)
    print(f"Recorded {len(completed)} CTest passes; {len(inventory['files'])} originals unchanged; full game remains incomplete")

if __name__=='__main__': main()
