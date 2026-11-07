#!/usr/bin/env python3
"""Stage editable production sources and pinned dependencies without game binaries."""
import argparse
import base64
import datetime
import shutil
import subprocess
from pathlib import Path
from record_live_production_verification import ROOT,digest,read,write


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();out=args.output.resolve()
    assert out.is_relative_to(ROOT/'work') and not out.exists(),out
    out.mkdir(parents=True)
    tracked=subprocess.check_output(['git','ls-files','src','tools','CMakeLists.txt'],cwd=ROOT,text=True,encoding='utf8').splitlines()
    locks=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'analysis').glob('*.lock.json'))]
    names=tracked+locks+['analysis/inventory.json','analysis/nsis-build-config.json']
    copied={}
    for name in names:
        src=ROOT/name
        if not src.is_file():continue
        assert src.suffix.lower() not in ['.exe','.dll','.obj','.lib','.pdb'],name
        dest=out/name;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(src,dest)
        copied[name]=digest(dest)
    # Only source/license dependencies are linked. No original game directory,
    # executable/DLL or reference-only Squall1.48 tree is exposed here.
    dependencies=['libmng-source','expat-source','minizip-1.01','direct3d8-headers','nsis-2.28-source']
    dependencies += [d['name'] for d in read(ROOT/'analysis/codecs.lock.json')['dependencies']]
    dependencies += [d['name'] for d in read(ROOT/'analysis/squall-codecs.lock.json')['dependencies']]
    dependencies=list(dict.fromkeys(dependencies))
    local=out/'.local';local.mkdir()
    for name in dependencies:
        src=(ROOT/'.local'/name).resolve(strict=True);dest=local/name
        assert src.is_dir() and src.is_relative_to(ROOT/'.local') and not dest.exists()
        # Junction creation stays entirely within the explicitly owned workspace.
        quote=lambda value: "'"+str(value).replace("'","''")+"'"
        script=f"$ProgressPreference='SilentlyContinue'; New-Item -ItemType Junction -Path {quote(dest)} -Target {quote(src)} -ErrorAction Stop | Out-Null"
        encoded=base64.b64encode(script.encode('utf-16-le')).decode('ascii')
        result=subprocess.run(['powershell','-NoProfile','-EncodedCommand',encoded],capture_output=True)
        assert result.returncode==0,result.stderr.decode(errors='replace')
    icon=ROOT/'.local/game-artwork/game.ico';dest=local/'game-artwork/game.ico';dest.parent.mkdir()
    shutil.copyfile(icon,dest);copied['.local/game-artwork/game.ico']=digest(dest)
    forbidden=[]
    inventory=read(ROOT/'analysis/inventory.json')['files']
    original_hashes={r['sha256'] for r in inventory if Path(r['path']).suffix.lower() in ['.exe','.dll']}
    for name in copied:
        if copied[name] in original_hashes:forbidden.append(name)
    assert not forbidden
    assert not (out.parent/'farm\u00e1\u0159/Dobr\u00fd farm\u00e1\u0159').exists()
    report=dict(status='staged_source_only_not_yet_built',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        stager_sha256=digest(__file__),directory=out.relative_to(ROOT).as_posix(),copied_source_files=copied,
        pinned_source_dependency_junctions=dependencies,original_game_binaries_present=False,
        original_reference_source_tree_exposed=False,icon_is_artwork_only=True,whole_game_complete=False)
    write(out/'source-stage.json',report)
    print(str(out))


if __name__=='__main__':main()
