#!/usr/bin/env python3
"""Compare native cProfile header loading/lookup with explicit XML/CRT boundaries."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
from oracle_precision import run_precision_cases
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/profile-headers-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/profile-headers-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    oracle=args.oracle.resolve(strict=True);contexts=[]
    work=ROOT/'.local/profile-header-validation-work';work.mkdir(parents=True,exist_ok=True)
    for precision,output in run_precision_cases(oracle,[original,work],timeout=90):
        context=json.loads(output)
        assert context['constructors']==100 and context['header_loads']==8000 and context['lookups']==80000
        assert context['name_lists']==400 and context['current_ids']==8000
        assert context['xml_fixtures']==640
        context['precision']=precision;contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in contexts[0] if key!='precision'}
    report.update(status='passed',precision_contexts=contexts,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=expected,oracle_sha256=hashlib.sha256(oracle.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['004ecb20','004ea000','004e93d0','004dfd90','004e8900','004e4110','004e75b0'],
        coverage=['Empty-selected-name constructor defaults, repeated header merge and first-insert duplicate IDs',
            'Header CurrentUser updates stored name at +8 while selected name at +24 is unchanged; absent numeric attributes retain values',
            'Unsigned UTF-16 name ordering, empty/missing/non-Profile tags, long/non-ASCII names and entity decoding',
            'getNames appends without clearing, current-ID lookup and complete tested tree allocation cleanup',
            '640 cached UTF-16 XML fixtures per context; filesystem writes restricted to the mounted .local diagnostic directory'],
        test_adapters=['Native XML virtual calls wrap recovered XmlDocument; original parser and old CRT scalar extraction are unverified',
            'Native String/WString compare/assignment and allocation use recovered ABI/diagnostic CRT',
            'Native getProfileID lookup executes; decimal-width-eight CRT formatter is replaced by recovered formatter',
            'Constructor executes original load only with empty selected name, where its documented early-return branch does no file work',
            'Cleanup runs original name-tree erase and diagnostic WString/head cleanup; full profile destructor is not executed'],
        pending=['Whole cProfile typed property stores, create/delete/switch, save/load and destructor','Native old CRT ID formatting and XML numeric conversion',
            'Nonempty cProfile constructor, profile initialization callback, singleton exception and full ABI','Profiles menu and main-game/save compatibility'])
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Profile headers: {report['header_loads']} native loads, {report['lookups']} lookups, {report['constructors']} constructors")
if __name__=='__main__':main()
