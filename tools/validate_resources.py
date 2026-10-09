#!/usr/bin/env python3
"""Validate integrated original indexing -> iFile -> XML/JNG resource loading."""
import argparse
import datetime
import hashlib
import json
import subprocess
import struct
import tempfile
import xml.parsers.expat
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def xml_tree(data):
    """Independent event-to-tree model of the recovered original cXML callbacks."""
    parser = xml.parsers.expat.ParserCreate()
    root = dict(attributes={}, text='', children=[])
    stack = []
    def start(name, attributes):
        node = root if not stack else dict(attributes={}, text='', children=[])
        if stack:
            stack[-1]['children'].append(dict(name=name, node=node))
        node['attributes'].update(attributes)
        stack.append(node)
    def end(name):
        node = stack.pop()
        node['children'].sort(key=lambda child: child['name'])
    def characters(text):
        (stack[-1] if stack else root)['text'] += text
    parser.StartElementHandler = start
    parser.EndElementHandler = end
    parser.CharacterDataHandler = characters
    parser.Parse(data, True)
    return root


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--loader', type=Path, default=ROOT/'build/windows/Release/resource-inspect.exe')
    parser.add_argument('--original-dir', type=Path, default=ROOT.parent/'farmář/Dobrý farmář')
    parser.add_argument('--reference-decoder', type=Path)
    parser.add_argument('--file-index-test', type=Path)
    parser.add_argument('--fixtures-only', action='store_true')
    parser.add_argument('--output', type=Path, default=ROOT/'.local/resource-validation.json')
    args = parser.parse_args()
    loader = args.loader.resolve(strict=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    report = dict(recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  loader_sha256=hashlib.sha256(loader.read_bytes()).hexdigest(),
                  indexing_fixtures=0, write_checks=0, raw_resources_compared=0, xml_trees_compared=0,
                  xml_oracle='Python Expat event tree; original cXML runtime still pending',
                  original_jng_pixels_compared=0, failures=[])
    scratch = ROOT/'.local/resource-validation-work'
    scratch.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='run-', dir=scratch) as temporary:
        work = Path(temporary)
        output = work/'result.bin'
        def invoke(mode, root, resource, common=None):
            command = [str(loader), mode, str(root.resolve()), resource, str(output)]
            if common is not None:
                command.append(str(common.resolve()))
            result = subprocess.run(command, capture_output=True, text=True, timeout=60)
            if result.returncode:
                raise RuntimeError(f'{mode} {resource}: exit {result.returncode}, {result.stderr.strip()}')
            return output.read_bytes()
        # Independently built collision / overlay fixtures, never the real profiles.
        data_root = work/'data'
        data_root.mkdir()
        common = work/'common'
        (common/'sys').mkdir(parents=True)
        for pack, entries in [('A.pack', {'only-a.bin':b'A', 'collision.bin':b'A',
                                       'loose.bin':b'pack A', 'sys/settings.xml':b'<root v="original"><game/></root>'}),
                              ('B.pack', {'collision.bin':b'B', 'loose.bin':b'pack B'})]:
            with zipfile.ZipFile(data_root/pack, 'w', compression=zipfile.ZIP_DEFLATED) as archive:
                for name, value in entries.items(): archive.writestr(name, value)
                archive.writestr('directory/', b'')
        (data_root/'loose.bin').write_bytes(b'loose')
        (data_root/'.hidden').write_bytes(b'ignored')
        (data_root/'nested').mkdir()
        (data_root/'nested/entry.bin').write_bytes(b'nested')
        assert invoke('file', data_root, 'ONLY-A.BIN') == b'A'
        assert invoke('file', data_root, 'Collision.BIN') == b'B'
        assert invoke('file', data_root, 'LOOSE.BIN') == b'loose'
        assert invoke('file', data_root, 'NESTED/ENTRY.BIN') == b'nested'
        index = json.loads(invoke('index', data_root, ''))
        assert '.hidden' not in index and 'directory/' not in index
        assert index['collision.bin']=='B.pack' and index['loose.bin']==''
        (common/'collision.bin').write_bytes(b'common')
        assert invoke('file', data_root, 'collision.bin', common) == b'common'
        xml = '<root a="1"><z id="1"/><a>přesný\ntext</a><z id="2"/></root>'.encode('utf-8')
        (common/'sys/settings.xml').write_bytes(xml)
        assert json.loads(invoke('xml', data_root, 'sys/settings.xml', common)) == xml_tree(xml)
        # Invalid before any element starts: the original retry does not need to
        # clear a partial tree/deque here. Partial-element recovery is not asserted.
        (common/'sys/settings.xml').write_bytes(b'\x00')
        assert json.loads(invoke('xml', data_root, 'sys/settings.xml', common)) == xml_tree(b'<root v="original"><game/></root>')
        assert not (common/'sys/settings.xml').exists()
        report['indexing_fixtures'] = 8
        if args.file_index_test:
            # The oracle's baseline is B.pack, so remove only this fixture override.
            (common/'collision.bin').unlink()
            result=subprocess.run([str(args.file_index_test.resolve(strict=True)), str(data_root), str(common)],
                                  capture_output=True, text=True, timeout=60)
            if result.returncode: raise RuntimeError(result.stderr.strip())
            report['write_checks']=int(result.stdout.strip())
        if not args.fixtures_only:
            original = args.original_dir.resolve(strict=True)
            inventory = json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
            expected = {}
            tags = {}
            # WindowsPath's ordering folds case, unlike the original String set.
            for pack in sorted((original/'Data').glob('*.pack'), key=lambda p:p.name.encode('ascii')):
                row = next(row for row in inventory['files'] if row['path']==f'Data/{pack.name}')
                if hashlib.sha256(pack.read_bytes()).hexdigest()!=row['sha256']:
                    raise ValueError('Original pack differs from inventory')
                with zipfile.ZipFile(pack) as archive:
                    for name in archive.namelist():
                        if not name.endswith('/'):
                            key=name.lower()
                            expected[key]=archive.read(name)
                            tags[key]=pack.name
            observed = json.loads(invoke('index', original/'Data', ''))
            if observed != tags: raise ValueError('Original asset index does not match ZIP inventory')
            reference = args.reference_decoder.resolve(strict=True) if args.reference_decoder else None
            dll = original/'JNGLoad.dll'
            if reference:
                row = next(row for row in inventory['files'] if row['path']=='JNGLoad.dll')
                if hashlib.sha256(dll.read_bytes()).hexdigest()!=row['sha256']:
                    raise ValueError('Original decoder differs from inventory')
            image_input, image_output = work/'image.jng', work/'reference.bin'
            for index, (name, data) in enumerate(sorted(expected.items())):
                try:
                    if invoke('file', original/'Data', name)!=data:
                        raise ValueError('Decompressed bytes differ')
                    report['raw_resources_compared'] += 1
                    if name.endswith('.xml'):
                        if json.loads(invoke('xml', original/'Data', name))!=xml_tree(data):
                            raise ValueError('XML tree differs')
                        report['xml_trees_compared'] += 1
                    elif name.endswith('.jng') and reference:
                        pixels=invoke('image', original/'Data', name)
                        image_input.write_bytes(data)
                        decoded = subprocess.run([str(reference), str(dll), str(image_input), str(image_output)],
                                                 check=True, capture_output=True, timeout=60)
                        dimensions = tuple(map(int, decoded.stdout.split()))
                        original_pixels = struct.pack('<II', *dimensions)+image_output.read_bytes()
                        if pixels!=original_pixels: raise ValueError('Original JNG pixels differ')
                        report['original_jng_pixels_compared'] += 1
                except Exception as error:
                    report['failures'].append(dict(resource=name, error=str(error)))
                if (index+1)%100==0: print(f'Compared {index+1}/{len(expected)} resources', flush=True)
    report['status'] = 'passed' if not report['failures'] else 'failed'
    args.output.write_text(json.dumps(report, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
    print(f"Resources: {report['status']}; {report['raw_resources_compared']} byte-exact files, "
          f"{report['xml_trees_compared']} XML trees, {report['original_jng_pixels_compared']} original image comparisons")
    if report['failures']: raise SystemExit(1)


if __name__=='__main__': main()
