#!/usr/bin/env python3
"""Assemble the public Windows ZIP or a separate complete archive for local use."""
import argparse
import hashlib
import json
from pathlib import Path
from tempfile import TemporaryDirectory
from zipfile import ZIP_DEFLATED, ZipFile, ZipInfo

from pe import PE


ROOT = Path(__file__).resolve().parents[1]
ARCHIVE_NAME = 'Dobry-farmar-Windows.zip'
LOCAL_ARCHIVE_NAME = 'Dobry-farmar-Windows-komplet.zip'
GAME_FILES = {
    'Dobrý farmář.exe': 'DobryFarmar.exe',
    'JNGLoad.dll': 'JNGLoad.dll',
    'Squall.dll': 'Squall.dll',
}
DATA_FILES = ('Data/Data.pack', 'Data/cz.pack')
LICENSE_FILES = {
    'Expat.txt': '.local/expat-source/expat/COPYING',
    'IJG-JPEG.txt': '.local/jpeg-6b/README',
    'libmng.txt': '.local/libmng-source/LICENSE',
    'libogg.txt': '.local/libogg-1.1/COPYING',
    'libvorbis.txt': '.local/libvorbis-1.1.0/COPYING',
    'Squall.txt': 'src/squall/runtime/LICENSE',
    'zlib.txt': '.local/zlib-1.2.3/README',
}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8'))


def checked_bytes(path, expected):
    data = path.read_bytes()
    if digest(data) != expected:
        raise ValueError(f'SHA-256 mismatch: {path}')
    return data


def validate_binaries(paths):
    programs = {name.lower(): PE(path) for name, path in paths.items()}
    for name, program in programs.items():
        if program.machine != 0x14c or program.bits != 32:
            raise ValueError(f'{name} must be a Win32 binary')
        if bool(program.characteristics & 0x2000) != name.endswith('.dll'):
            raise ValueError(f'Unexpected PE file type: {name}')

    exports = {}
    for name, program in programs.items():
        if name.endswith('.dll'):
            rows = program.exports()
            exports[name] = (
                {symbol for row in rows for symbol in row['names']},
                {row['ordinal'] for row in rows},
            )
    for name, program in programs.items():
        for entry in program.imports():
            library = entry['library'].lower()
            if library not in exports:
                if library.startswith(('msvcp', 'vcruntime', 'concrt', 'vcomp')):
                    raise ValueError(f'{name} needs missing runtime DLL {library}')
                continue
            names, ordinals = exports[library]
            for symbol in entry['symbols']:
                if symbol.get('name') not in names and symbol.get('ordinal') not in ordinals:
                    raise ValueError(f'{library} does not satisfy import {symbol} of {name}')


def release_files(crt, report, local_original):
    inventory = {row['path']: row['sha256']
                 for row in read_json(ROOT / 'analysis/inventory.json')['files']}
    staged = report['staged_source_manifest']
    if staged['original_game_binaries_present'] or staged['original_reference_source_tree_exposed']:
        raise ValueError('The production build must come from an independent source tree')
    for name, expected in staged['copied_source_files'].items():
        if name.startswith(('src/', 'tools/cmake/')) or name == 'CMakeLists.txt':
            checked_bytes(ROOT / name, expected)

    files = {}
    binaries = {}
    for name, destination in GAME_FILES.items():
        artifact = report['artifacts'][name]
        path = ROOT / artifact['path']
        data = checked_bytes(path, artifact['sha256'])
        if artifact['sha256'] == inventory[name]:
            raise ValueError(f'Build contains an unchanged original binary: {name}')
        files[destination] = data
        binaries[destination] = path
    for name, expected in read_json(ROOT / 'analysis/windows-release-crt.lock.json')['files'].items():
        path = crt / name
        files[name] = checked_bytes(path, expected)
        binaries[name] = path
    validate_binaries(binaries)

    if local_original is not None:
        for name in DATA_FILES:
            files[name] = checked_bytes(local_original / name, inventory[name])
    for name in ('Hrat.cmd', 'README.txt'):
        text = (ROOT / 'tools/release' / name).read_text(encoding='ascii')
        files[name] = text.replace('\n', '\r\n').encode('ascii')
    for name, source in LICENSE_FILES.items():
        files[f'Licenses/{name}'] = (ROOT / source).read_bytes()
    minizip = (ROOT / '.local/minizip-1.01/unzip.h').read_bytes().split(b'*/', 1)[0]
    files['Licenses/minizip.txt'] = minizip + b'*/\n'
    return files


def write_release(output, filename, files):
    output.mkdir(parents=True, exist_ok=True)
    archive = output / filename
    checksum = output / f'{filename}.sha256'
    if archive.exists() or checksum.exists():
        raise ValueError(f'Choose an output directory without existing release assets: {output}')
    with TemporaryDirectory(dir=output) as temp:
        staged = Path(temp) / filename
        with ZipFile(staged, 'w') as zip_file:
            for name, data in sorted(files.items()):
                zip_file.writestr(ZipInfo(name), data, compress_type=ZIP_DEFLATED, compresslevel=9)
        with ZipFile(staged) as zip_file:
            if zip_file.namelist() != sorted(files) or any(zip_file.read(name) != data for name, data in files.items()):
                raise ValueError('The completed ZIP does not match the selected files')
        archive_hash = digest(staged.read_bytes())
        staged_checksum = staged.with_suffix('.zip.sha256')
        staged_checksum.write_bytes(f'{archive_hash}  {archive.name}\n'.encode('ascii'))
        staged.rename(archive)
        staged_checksum.rename(checksum)
    print(f'{archive_hash}  {archive}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--local-original-dir', type=Path,
                        help='Include verified original data in a separately named local archive')
    parser.add_argument('--crt-dir', required=True, type=Path)
    parser.add_argument('--build-report', type=Path, default=ROOT / 'analysis/production-source-only-build-v4.json')
    parser.add_argument('--output-dir', type=Path)
    args = parser.parse_args()
    try:
        local = args.local_original_dir is not None
        default_output = ROOT / 'dist' / ('local-complete' if local else 'v1.0.0')
        filename = LOCAL_ARCHIVE_NAME if local else ARCHIVE_NAME
        files = release_files(args.crt_dir, read_json(args.build_report), args.local_original_dir)
        write_release(args.output_dir or default_output, filename, files)
    except (KeyError, OSError, ValueError) as error:
        parser.error(str(error))


if __name__ == '__main__':
    main()
