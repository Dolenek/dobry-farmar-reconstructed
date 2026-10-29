#!/usr/bin/env python3
"""Audit repeated real transitions; record drift without declaring timed runs identical."""
import argparse,hashlib,json
from pathlib import Path
from game_save_state import load_save
ROOT=Path(__file__).resolve().parents[1]

def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,default=ROOT/'.local/seed-transition-validation.json')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/seed-transition-state-validation.json')
    args=parser.parse_args();report=json.loads(args.runtime_report.read_text(encoding='utf-8'))
    assert report['status']=='explored_repeated_native_source_seed_transition_coordinates_only'
    assert [s['kind'] for s in report['sessions']]==['native']*3+['source']*2
    for name,value in report['source_files'].items():assert digest(ROOT/name)==value,name
    work=ROOT/report['work_directory']
    for name,value in report['screenshots'].items():assert digest(work/name)==value,name
    rows=[]
    for i,session in enumerate(report['sessions']):
        def package(key):
            found=[c for c in session[key] if c['texture'] and 'seed_tomat' in c['texture'].lower()]
            assert len(found)==1;return found[0]['geometry']
        before,after=package('before_scene'),package('after_scene')
        state=load_save(work/f'{i}-persisted/Profiles/00000001.save');seed=state['seeds'][0]
        assert state['bytes']==2523 and len(state['seeds'])==1
        assert before[0]==122 and before[1]==after[1]==seed['y']==280
        assert seed['x']==after[0]+1 and seed['kind']==1 and seed['count']==3 and seed['status']==seed['unpaid']==0
        rows.append(dict(index=i,kind=session['kind'],before_world_x=before[0],after_world_x=after[0],save_x=seed['x'],difference=after[0]-before[0]))
    assert len({row['save_x'] for row in rows[:3]})==3,'Original repeated-run variation not reproduced'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    for row in inventory['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    report.update(status='passed_for_repeated_native_transition_coordinate_drift_observation_only',
        runtime_report_sha256=digest(args.runtime_report),coordinate_observations=rows,
        audit_files={name:digest(ROOT/name) for name in ['tools/validate_seed_transition_state.py','tools/game_save_state.py']},
        pending=['Exact whole-world comparison with replayed frame intervals and synchronized animated backgrounds','All other full-goal scopes'])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(report['status'])

if __name__=='__main__':main()
