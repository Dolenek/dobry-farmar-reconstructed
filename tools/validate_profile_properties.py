#!/usr/bin/env python3
"""Compare all thirteen original profile property trees with recovered source."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path
from oracle_precision import run_precision_cases
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/profile-properties-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/profile-properties-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    oracle=args.oracle.resolve(strict=True);contexts=[]
    for precision,output in run_precision_cases(oracle,[original],timeout=90):
        context=json.loads(output)
        assert context['scenarios']==2400 and context['property_accesses']==31200 and context['typed_dispatches']==33600
        assert context['stores']==13 and context['clears']==142
        context['precision']=precision;contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in contexts[0] if key not in ('precision','stores')}
    report.update(stores=13,status='passed',precision_contexts=contexts,recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        reference_sha256=expected,oracle_sha256=hashlib.sha256(oracle.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['004ebd10','004e9d80','004106a0','00410770','004105e0','004e98c0','004e9990','004e9a70','004e9b40','004e9c00','004e9cc0','004eac20','004eb1d0'],
        coverage=['Thirteen distinct process-global typed maps; default insertion, repeated lookup and overwrite',
            'Original unsigned-byte key ordering, long/empty/embedded-NUL/high-byte keys; independent same-name keys across every type',
            'Original type dispatcher and all thirteen setters; valid scalar extremes, color/vector/string values and unknown case-sensitive tags',
            'Original clear routine erases every tree, resets sentinels/counts and permits further insertion; diagnostic allocation cleanup'],
        test_adapters=['Recovered String/WString assignment/compare and diagnostic allocator',
            'Modern numeric wide streams and ACP conversion replace original old CRT parsers/conversion; conversion equivalence is not proved',
            'Empty sentinel nodes initialized from instruction-confirmed layout, including four-byte pair padding for Bool and UInt16',
            'Mapped original image is copied to a distinct diagnostic address and explicit operands relocated; original entry point never runs'],
        pending=['Native old CRT conversion, invalid/empty numeric setter values and formatting',
            'Property XML persistence, profile lifecycle and menu integration','Whole-game save compatibility'])
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Profile properties: {report['property_accesses']} native accesses, {report['typed_dispatches']} dispatches, {report['map_comparisons']} map comparisons")
if __name__=='__main__':main()
