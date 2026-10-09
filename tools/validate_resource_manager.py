#!/usr/bin/env python3
"""Exercise resource caching and settings through isolated archive/common writes."""
import argparse
import datetime
import hashlib
import json
import subprocess
import tempfile
import zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--test',type=Path,default=ROOT/'build/windows/Release/resource-manager-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/resource-manager-validation.json')
    args=parser.parse_args()
    executable=args.test.resolve(strict=True)
    scratch=ROOT/'.local/resource-manager-validation-work'; scratch.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(dir=scratch) as directory:
        work=Path(directory); data=work/'data'; common=work/'common'; data.mkdir(); common.mkdir()
        with zipfile.ZipFile(data/'Data.pack','w',compression=zipfile.ZIP_DEFLATED) as archive:
            archive.writestr('sys/settings.xml',b'<main><graphics width="800"/></main>')
        result=subprocess.run([str(executable),str(data),str(common)],capture_output=True,text=True,timeout=60,check=True)
        assert (common/'sys/settings.xml').read_bytes()==b'<main><graphics width="1024"/></main>'
    report=dict(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                native_checks=int(result.stdout),executable_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),
                original_manager_runtime_compared=False,
                pending=['Original manager runtime oracle','40-byte manager and 96-byte cXML ABI',
                         'Shader/font/mesh/animation/sound factories','Resource logging','Sound deletion with real sound resources'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Resource manager: {report['native_checks']} archive/cache/settings/write checks")
if __name__=='__main__': main()
