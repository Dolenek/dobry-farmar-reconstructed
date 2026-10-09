#!/usr/bin/env python3
"""Compare cView hierarchy, callbacks, transitions and deferred registries."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
from oracle_precision import run_precision_cases
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/view-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/view-validation.json')
    parser.add_argument('--frame',action='store_true',help='Validate original main-frame UI traversal and explicit service boundaries')
    args=parser.parse_args()
    if args.frame and args.output==ROOT/'.local/view-validation.json':args.output=ROOT/'.local/ui-frame-validation.json'
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    executable=args.oracle.resolve(strict=True)
    contexts=[]
    for precision,output in run_precision_cases(executable,[original],False):
        context=json.loads(output)
        assert context['operations']==16000 and context['state_comparisons']==192012 and context['owned_views_deleted']==12 and context['precision_comparisons']==10000
        if args.frame:assert context['frame_comparisons']==512
        context['precision']=precision;contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in contexts[0] if key!='precision'}
    report['precision_contexts']=contexts
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=expected,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['004d55b0','004d58a0','004d2e50','004d37d0','004d3720','004d3bb0','004d3fb0',
            '004d4590','004d4980','004d4d80','004d59c0','004d2ec0','004d2f50','004d2fc0',
            '00506f90','00506fe0','00507030','00507080','005069d0','00506b90','00506d90'],
        test_adapters=['Allocation/free use diagnostic CRT','Timer +8 returns controlled elapsed value',
            'Virtual callbacks record order then call original base handlers; child deleting destructor records then calls original base destructor'],
        pending=['Modern container/object layout is not the original 44-byte ABI','loadXML typed extraction comparison',
            'Completed transparent discard diagnostic String and queued deletion comparison',
            'Copy constructor 004d56b0','Whole frame traversal, input, control rendering and menu integration',
            'Nonfinite transparency/elapsed behavior'])
    if args.frame:
        report['original_functions']+=['00531b20','004fb180']
        report['coverage']=['Absolute then normal UI traversal with normal pending-removal tests in both passes',
            'Callbacks mutate normal/absolute registration while traversing; additions/removals flush between passes',
            'Transition-set size 00660528 suppresses BOTH step passes while transitions exist; finished transitions are removed after traversal',
            '1024 original main frames across both x87 contexts, comparing callback order and all 12 view states/registries']
        report['test_adapters']+=['Main-frame cursor update, timer advance, input processing, and graphics begin/finish use empty diagnostic boundary callbacks',
            'Sound, mesh, and drawable-control registries are empty in frame fixtures; no complete rendering or physical input comparison',
            'Completed discard mode is converted to hide in frame fixtures; native discard-map traversal is empty']
        report['pending']+=['Main-frame sound/mesh/graphics/input integration and native nonempty queued deletion']
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"UI views: {report['state_comparisons']} states, {report['callback_events']} callback events, {report['registry_comparisons']} registry snapshots")
if __name__=='__main__': main()
