#!/usr/bin/env python3
"""Run original cProfile lifecycle against isolated file/XML/CRT adapters."""
import argparse,datetime,hashlib,json
from pathlib import Path
from oracle_precision import run_precision_cases
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/profile-lifecycle-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/profile-lifecycle-validation.json')
    args=parser.parse_args();original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    oracle=args.oracle.resolve(strict=True);work=ROOT/'.local/profile-lifecycle-validation-work';work.mkdir(parents=True,exist_ok=True);contexts=[]
    for precision,output in run_precision_cases(oracle,[original,work],timeout=90):
        context=json.loads(output);assert context['constructors']==80 and context['destructors']==80
        assert context['game_default_entries']==790 and context['game_initializations']==110
        assert context['remove_calls']==84 and context['removed_files']==179
        context['precision']=precision;contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in contexts[0] if key not in ('precision','game_default_entries')}
    report['game_default_entries']=790
    report.update(status='passed',precision_contexts=contexts,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=expected,
        oracle_sha256=hashlib.sha256(oracle.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['004ecb20','004ec310','004ea000','004ea5d0','004e3a70','004e6cf0','004ead60','00411390','0040ab50','004e8d20','004e4520','004e14e0','004e15e0'],
        coverage=['Nonempty and empty selected-name constructors, actual files and exact UTF-16/BOM header/property writes in isolated mounts',
            'Missing/existing profiles, caller-selected name takes precedence over header CurrentUser, missing/empty stored name, repeated create and ID consumption',
            'Repeated full load, XML ownership, profile initialization callback count/order, all typed maps and header state',
            'Startup header existence and stored CurrentUser lookup before profile construction, including missing files and empty names',
            'Destructor clears NowPlaying, saves headers/properties, clears all maps/names and releases singleton/heap',
            'Remove missing/selected/other/last profiles, ascending/descending sequences; selects first remaining name and suppresses destructor writes',
            'Actual prefix-related property/save/longer-ID files removed and invalidated in order; packs and directories retained',
            'Actual cGame::onInitProfile instructions initialize 790 entries: 291 Float, 62 Int, 306 UInt and 131 Bool; original 21 award-name static getters execute'],
        test_adapters=['Native XML/file virtual calls use recovered XmlDocument/FileIndex; original parser/file manager and engine error ABI not covered',
            'Original stream construction/extraction/destruction, formatting and scalar parsing use modern diagnostic CRT',
            'Directory Settings lookup and ID formatter use recovered routines; their old CRT behavior remains pending',
            'Indexed property-key formatting and tutorial tip-key formatting use recovered decimal adapters; original old CRT formatting remains unverified',
            'Native file enumeration uses recovered FileIndex and diagnostic original-layout String list; native erase and deletion instructions execute',
            'Only mounted .local diagnostic directories receive writes; source implementation never calls original code'],
        pending=['SelectProfile UI and switch/delete navigation actions, higher game saveLevelState/loadLevelState','Original XML/old CRT conversion, full ABI/exception paths',
            'Native game launch and bidirectional whole-game save compatibility'])
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Profile lifecycle: {report['constructors']} native constructors/destructors, {report['utf16_writes']} UTF-16 file writes")
if __name__=='__main__':main()
