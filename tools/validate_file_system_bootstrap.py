#!/usr/bin/env python3
"""Check reconstructed pre-Settings common-directory mounting using isolated source fixtures."""
import argparse,datetime,hashlib,json,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--test',type=Path,default=ROOT/'build/windows/Release/file-system-bootstrap-test.exe');p.add_argument('--output',type=Path,default=ROOT/'.local/file-system-bootstrap-validation.json');a=p.parse_args();exe=a.test.resolve(strict=True)
    result=subprocess.run([str(exe),str(ROOT/'.local/bootstrap-fixtures')],capture_output=True,text=True,timeout=30,cwd=ROOT)
    if result.returncode:raise RuntimeError(result.stderr)
    state=json.loads(result.stdout);assert state==dict(bootstrap_cases=6,mounted_overlays=3,rejected_cases=3,native_constructor_executed=False),state
    paths=['CMakeLists.txt','tools/validate_file_system_bootstrap.py','tools/file_system_bootstrap_test.cpp','src/game/engine/GameFileSystem.h','src/game/engine/GameFileSystem.cpp']
    report=dict(status='passed_for_source_bootstrap_names_overlay_order_and_folder_api_boundary',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),result=state,test_path=exe.relative_to(ROOT).as_posix(),test_sha256=digest(exe),source_files={n:digest(ROOT/n) for n in paths},original_function='0054d720',native_constructor_executed=False,coverage=['Configured app_directory and window_title fallback; empty names and missing game rejected','Exact SHGetFolderPathA(null,0x8023,null,0,out) request; failed folder API rejected','Mount precedes Settings and common settings overlay is read; explicit isolated path bypasses bootstrap settings'],pending=['Full native FileManager constructor lifecycle and default real ProgramData branch; tests deliberately isolate writes'])
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');print('Source filesystem bootstrap:6 fixtures /3 overlays /3 rejected inputs; native constructor not executed')
if __name__=='__main__':main()
