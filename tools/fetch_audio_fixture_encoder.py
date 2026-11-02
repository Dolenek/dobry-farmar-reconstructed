#!/usr/bin/env python3
"""Fetch a pinned test-only FFmpeg executable; never a production dependency."""
import argparse,hashlib,json,urllib.request,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
URL='https://files.pythonhosted.org/packages/2c/c6/fa760e12a2483469e2bf5058c5faff664acf66cadb4df2ad6205b016a73d/imageio_ffmpeg-0.6.0-py3-none-win_amd64.whl'
SHA256='02fa47c83703c37df6bfe4896aab339013f62bf02c5ebf2dce6da56af04ffc0a'


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--verify-only',action='store_true');args=p.parse_args()
    folder=ROOT/'.local/audio-fixture-encoder';wheel=folder/'imageio_ffmpeg-0.6.0-py3-none-win_amd64.whl';exe=folder/'ffmpeg.exe'
    if not wheel.exists():
        assert not args.verify_only,'Test encoder is missing; run without --verify-only'
        folder.mkdir(parents=True,exist_ok=True)
        with urllib.request.urlopen(URL,timeout=60) as response:payload=response.read()
        assert hashlib.sha256(payload).hexdigest()==SHA256,'Encoder wheel SHA256 differs'
        wheel.write_bytes(payload)
    assert hashlib.sha256(wheel.read_bytes()).hexdigest()==SHA256,'Encoder wheel SHA256 differs'
    with zipfile.ZipFile(wheel) as archive:
        names=[name for name in archive.namelist() if name.startswith('imageio_ffmpeg/binaries/ffmpeg-win') and name.endswith('.exe')]
        assert len(names)==1,names
        expected=archive.read(names[0]);expected_sha=hashlib.sha256(expected).hexdigest()
        if not exe.exists():
            assert not args.verify_only,'Extracted test encoder is missing'
            exe.write_bytes(expected)
        assert hashlib.sha256(exe.read_bytes()).hexdigest()==expected_sha,'Extracted encoder SHA256 differs'
        value=dict(status='pinned_test_fixture_encoder_only',metadata_source='https://pypi.org/pypi/imageio-ffmpeg/0.6.0/json',
            package_url=URL,package_sha256=SHA256,package_bytes=wheel.stat().st_size,archive_member=names[0],executable_sha256=expected_sha,
            executable_path=exe.relative_to(ROOT).as_posix(),production_dependency=False,downloaded_program_code_used_in_game=False)
        text=json.dumps(value,indent=2)+'\n'
        if not args.verify_only:(folder/'provenance.json').write_text(text,encoding='utf-8')
    print(str(exe));print(expected_sha)


if __name__=='__main__':main()
