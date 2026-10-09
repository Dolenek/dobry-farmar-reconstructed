#!/usr/bin/env python3
"""Validate integrated Settings -> window creation/message dispatch on Windows."""
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
    parser.add_argument('--test',type=Path,default=ROOT/'build/windows/Release/window-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/window-validation.json')
    args=parser.parse_args()
    test=args.test.resolve(strict=True)
    scratch=ROOT/'.local/window-validation-work'; scratch.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(dir=scratch,prefix='run-') as temporary:
        with zipfile.ZipFile(Path(temporary)/'Data.pack','w') as archive:
            archive.writestr('sys/settings.xml',('<main><game window_title="Dobrý farmář"/>'
                '<graphics width="800" height="600" ShowSysCursor="0"/></main>').encode('utf-8'))
        result=subprocess.run([str(test),temporary],capture_output=True,text=True,check=True,timeout=60)
    report=dict(recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                test_sha256=hashlib.sha256(test.read_bytes()).hexdigest(),
                native_checks=int(result.stdout.strip()),original_window_runtime_compared=False,
                hidden_window_only=True,status='passed',
                pending=['Original window runtime comparison','Actual input/main/sound bindings',
                         'Graphics initialization and visible game screens','Engine exception ABI'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Window: {report['native_checks']} native creation/dispatch/pump checks")

if __name__=='__main__': main()
