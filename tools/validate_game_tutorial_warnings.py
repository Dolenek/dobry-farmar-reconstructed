#!/usr/bin/env python3
"""Exercise actual complete-game tutorial warnings from genuine unchanged checkpoints."""
import argparse
import copy
import datetime
import json
import math
import shutil
import sys
import time
from pathlib import Path
from PIL import Image,ImageChops
from record_live_production_verification import ROOT,digest,read,write
from tutorial_warning_session import WarningSuite
from live_farm_fields import read_live_farm_fields


parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--cases',nargs='+',choices=['full_carry','poor_contract','full_warehouse','mixed_products'],default=['full_carry','poor_contract'])
parser.add_argument('--backends',nargs='+',choices=['native','source'],default=['native','source'])
parser.add_argument('--completed-prefix-report',type=Path,help='Reuse only completed native sessions from an immutable earlier raw report')
parser.add_argument('--output',type=Path,default=ROOT/'.local/tutorial-warning-runtime-validation.json')
args=parser.parse_args();assert len(args.cases)==len(set(args.cases)) and len(args.backends)==len(set(args.backends))
stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
suite=WarningSuite(ROOT/'.local/tutorial-warning-runtime'/stamp)
shutil.copyfile(__file__,suite.work/'driver.py')
suite.report['source_files']['tools/validate_game_tutorial_warnings.py']=digest(__file__)
suite.report['source_file_snapshots']['tools/validate_game_tutorial_warnings.py']=(suite.work/'driver.py').relative_to(ROOT).as_posix()
for name in ['tools/live_farm_fields.py','tools/tutorial_warning_layout.cpp','tools/tutorial_warning_layout/CMakeLists.txt','tools/tutorial_warning_scenarios.py']:
    suite.report['source_files'][name]=digest(ROOT/name)
    snapshot=suite.work/(name.replace('/','_'))
    shutil.copyfile(ROOT/name,snapshot)
    suite.report['source_file_snapshots'][name]=snapshot.relative_to(ROOT).as_posix()
layout_report=ROOT/'analysis/no-dry-award-state-validation.json';field_layout=read(layout_report)['field_layout']['metadata']
suite.report['field_layout_report']=dict(path=layout_report.relative_to(ROOT).as_posix(),sha256=digest(layout_report))
suite.report['requested_cases']=args.cases;suite.report['fixtures']={}
templates={}


def fixture(case):
    if case=='full_carry':
        path=ROOT/'analysis/contract-streak-forty-checkpoints.json';report=read(path)
        row=next(s for s in report['profile_sets'] if s['kind']=='native')
        directory=ROOT/row['profile_directory']
        assert digest(ROOT/row['source_report'])==row['source_report_sha256']
        assert {p.name:digest(p) for p in directory.iterdir() if p.is_file()}==row['files']
    else:
        names={'poor_contract':'contract-runtime-validation.json','full_warehouse':'full-warehouse-state-validation.json',
            'mixed_products':'cucumber-sowing-state-validation.json'}
        path=ROOT/'analysis'/names[case];report=read(path)
        index=next(n for n,s in enumerate(report['sessions']) if s['kind']=='native')
        assert report['sessions'][index]['exit_code']==0
        directory=ROOT/report['work_directory']/f'{index}-persisted/Profiles'
    suite.report['fixtures'][case]=dict(report=path.relative_to(ROOT).as_posix(),sha256=digest(path),
        profile_directory=directory.relative_to(ROOT).as_posix(),files={p.name:digest(p) for p in directory.iterdir() if p.is_file()})
    return directory


def service_incidental(t):
    step=t.observer.step()
    if step==-1:return False
    style='Common' if step==43 else 'Arrow'
    assert step in [11,12,19,24,25,26,27,28,29,43,46,47,49],('Unexpected actual tutorial',step,t.state())
    controls=t.observer.snapshot(style);point=controls['ok']['geometry'][:2]
    t.capture(f'incidental-{step}');t.session.setdefault('incidental_tutorials',[]).append(dict(step=step,style=style))
    t.click(*point)
    deadline=time.monotonic()+30
    while t.observer.step()==step:
        t.probe.check_process();assert time.monotonic()<deadline,('Incidental tutorial did not close',step);time.sleep(.05)
    return True


def await_condition(t,label,predicate,timeout=90,expected_active=()):
    deadline=time.monotonic()+timeout
    while not predicate():
        t.probe.focus();t.probe.check_process()
        if t.observer.step()!=-1 and t.observer.step() not in expected_active:service_incidental(t)
        assert time.monotonic()<deadline,(label,t.state())
        time.sleep(.05)
    return t.state()


def glyph_bounds(signature):
    bounds=[max(0,math.floor(min(r[0][axis]-abs(r[0][axis+2])/2 for r in signature))-2) for axis in range(2)]
    bounds += [min(limit,math.ceil(max(r[0][axis]+abs(r[0][axis+2])/2 for r in signature))+2) for axis,limit in [(0,800),(1,600)]]
    return bounds


def observe_warning(t,case,index):
    deadline=time.monotonic()+90
    while t.observer.step()!=index:
        t.probe.focus();t.probe.check_process()
        if t.observer.step()!=-1:service_incidental(t)
        assert time.monotonic()<deadline,('Actual warning did not open',index,t.state())
        time.sleep(.05)
    while True:
        controls=t.observer.snapshot('Action')
        # MultilineText loads this depth into its TextLine prototype. Its own
        # Control depth is not the rendered letters' depth (004f5e70/004dd350).
        definition=t.probe.resource('Tutorial/ActionTip.xml').find('Text')
        depth=float(definition.get('deep'));font=definition.get('Texture').lower()
        scene=t.observer.visible_scene(t.hwnd)
        glyphs=[row for row in scene if (row['texture'] or '').lower()==font and 0<=row['deep']-depth<1]
        if controls['image']['alpha']>=.999 and controls['image']['state'][2]==0 and glyphs:
            signature=sorted([[r['geometry'],r['sprite']] for r in glyphs]);time.sleep(.25)
            late=t.observer.visible_scene(t.hwnd)
            again=[r for r in late if (r['texture'] or '').lower()==font and 0<=r['deep']-depth<1]
            if signature==sorted([[r['geometry'],r['sprite']] for r in again]):break
        assert time.monotonic()<deadline,('Actual warning text did not finish',index)
        time.sleep(.05)
    bounds=glyph_bounds(signature)
    image=t.capture(f'{case}-warning-{index}')
    stage=dict(step=index,case=case,controls=controls,glyph_geometry_and_sprites=signature,
        text_bounds=bounds,screenshot=f'{t.index}-{case}-warning-{index}.png',state=t.state())
    key=(case,index)
    if t.observer.kind=='native':templates[key]=stage
    elif key in templates:
        native=templates[key];assert stage['glyph_geometry_and_sprites']==native['glyph_geometry_and_sprites']
        left=Image.open(suite.work/native['screenshot']).convert('RGB').crop(tuple(native['text_bounds']))
        right=image.convert('RGB').crop(tuple(native['text_bounds']))
        assert left.width>0 and left.height>0
        delta=ImageChops.difference(left,right)
        stage['text_composite_pixel_comparison']=dict(rect=native['text_bounds'],pixels=left.width*left.height,
            identical=left.tobytes()==right.tobytes(),difference_bounds=delta.getbbox(),
            glyph_geometry_and_sprites_exact=True,whole_timed_world_frame_equality_claimed=False)
    t.session.setdefault('stages',[]).append(stage)
    print(json.dumps(dict(kind=t.observer.kind,case=case,event='actual_warning_opened',step=index)),flush=True)
    return stage


def full_carry(t):
    initial=t.state();assert initial['held']==6 and initial['mower_level']==3
    assert initial['carried']==[12]+[0]*7 and not initial['processed'][40]
    definition=t.probe.resource('Farm/ActionAreas/MowBuilding.xml').find('Upgrades').find('Up'+str(initial['mower_level']))
    capacity=int(definition.get('ProductionNum'));needed=capacity-sum(initial['carried'])
    assert 0<needed<10
    t.session['original_mower_capacity']=dict(level=initial['mower_level'],capacity=capacity,
        source='Farm/ActionAreas/MowBuilding.xml Upgrades/UpN ProductionNum')
    fields=read_live_farm_fields(t.observer,ROOT,field_layout,t.observer.visible_scene(t.hwnd))
    targets=[row for row in fields if row['active'] and row['status']==1 and row['culture']==0][:needed+1]
    assert len(targets)==needed+1
    t.session['actual_target_fields']=targets;t.session['initial_state']=initial
    t.pause(True);paused=t.state();t.session['enqueued_field_observations']=[]
    for number,field in enumerate(targets,1):
        for attempt in range(3):
            t.click(*field['position']);scene=t.observer.visible_scene(t.hwnd)
            count=sum((r['texture'] or '').lower()=='person/mark.jng' for r in scene)
            if count==number:break
            assert count==number-1,('Unexpected actual task marker count',count,number)
        assert count==number,('Physical field input did not queue its actual marker',number)
        state=t.state();assert state['game_time']==paused['game_time'] and state['carried']==initial['carried']
        t.session['enqueued_field_observations'].append(dict(field=field['id'],actual_markers=count,game_time=state['game_time']))
    t.capture('full-carry-tasks-paused');t.pause(False)
    warning=observe_warning(t,'full_carry',40)['state']
    assert warning['carried']==[capacity]+[0]*7 and warning['collected']==initial['collected']+needed
    t.click(120,100)
    final=await_condition(t,'full-carry-unloaded',lambda:t.observer.step()==-1 and
        t.observer.property('booleans','tip40') and sum(t.observer.property('uints',f'PCC{n}') for n in range(8))==0,expected_active=(40,))
    expected=initial['stock'].copy();expected[0]+=capacity
    assert final['stock']==expected and final['market']==initial['market']
    assert final['collected']==initial['collected']+needed
    t.session['final_state']=final;t.capture('full-carry-resolved')
    t.session['material_balance']=dict(carried_grass_removed=capacity,warehouse_grass_added=capacity,harvests_added=needed,
        all19_warehouse_and_market_words_checked=True)


def poor_contract(t):
    initial=t.state();assert initial['money']==2 and not initial['processed'][42]
    tasks=[t.observer.property('ints',f'task{n}') for n in range(3)]
    assert sum(n!=-1 for n in tasks)<3
    t.session['initial_state']=initial;t.session['initial_active_tasks']=tasks
    await_condition(t,'next-real-letter',lambda:any(t.observer.property('ints',f'letter{n}')>=0 for n in range(3)),timeout=90)
    slot=next(n for n in range(3) if t.observer.property('ints',f'letter{n}')>=0)
    offer=t.observer.property('ints',f'letter{slot}');letters=list(t.probe.resource('UI/gameUI/farm.xml').find('LetterPlace'))
    position=[float(v) for v in letters[slot].get('Pos').split()]
    t.session['actual_letter']=dict(slot=slot,offer=offer,position=position)
    t.click(*position);observe_warning(t,'poor_contract',42)
    assert t.observer.property('ints','Money')==2
    assert [t.observer.property('ints',f'task{n}') for n in range(3)]==tasks
    t.click(*t.probe.button('Gameplay/ContractProvide.xml','WaitButton'))
    final=await_condition(t,'poor-contract-refused',lambda:t.observer.step()==-1 and t.observer.property('booleans','tip42'),expected_active=(42,))
    assert final['money']==initial['money'] and final['stock']==initial['stock'] and final['market']==initial['market']
    assert [t.observer.property('ints',f'task{n}') for n in range(3)]==tasks
    t.session['final_state']=final;t.capture('poor-contract-resolved')
    t.session['no_disabled_acceptance_or_deposit_occurred']=True


def full_warehouse(t):
    from tutorial_warning_scenarios import full_warehouse as scenario
    scenario(t,await_condition,observe_warning,field_layout)


def mixed_products(t):
    from tutorial_warning_scenarios import mixed_products as scenario
    scenario(t,await_condition,observe_warning,field_layout)


try:
    fixtures={case:fixture(case) for case in args.cases}
    completed=set()
    if args.completed_prefix_report:
        path=args.completed_prefix_report.resolve(strict=True);prior=read(path)
        assert prior['reference_files_unchanged']==17
        assert prior['implementation_files']==suite.report['implementation_files']
        imported=[]
        for session in prior['sessions']:
            if session['kind']!='native' or session.get('status')!='completed':continue
            case=session['case'];assert case in args.cases and session['exit_code']==0
            assert session['input_profile_files']==suite.report['fixtures'][case]['files']
            assert json.loads(session['launcher_completion'])==dict(event='exit',exit_code=0,watchdog_triggered=False)
            for stage in session['stages']:
                stage=copy.deepcopy(stage);stage['glyph_geometry_and_sprites'].sort()
                stage['text_bounds']=glyph_bounds(stage['glyph_geometry_and_sprites'])
                templates[(stage['case'],stage['step'])]=stage
                source=ROOT/prior['work_directory']/stage['screenshot']
                assert digest(source)==prior['screenshots'][source.name]
                shutil.copy2(source,suite.work/source.name)
            row=copy.deepcopy(session);row['reused_completed_evidence_report']=path.relative_to(ROOT).as_posix()
            suite.report['sessions'].append(row);completed.add(('native',case));imported.append(case)
        assert imported
        suite.report['completed_native_prefix']=dict(path=path.relative_to(ROOT).as_posix(),sha256=digest(path),
            imported_cases=imported,original_driver_status=prior['status'],failed_sessions_reused=False)
    for kind in args.backends:
        for case in args.cases:
            if (kind,case) in completed:continue
            with suite.open(kind,case,fixtures[case]) as active:
                globals()[case](active)
    suite.report['status']='passed_actual_'+'_and_'.join(args.cases)+'_warning_subset_only'
except Exception as error:
    suite.report['status']='failed';suite.report['error']=repr(error)
    import traceback
    suite.report['traceback']=traceback.format_exc()
finally:
    suite.unchanged();suite.report['reference_files_unchanged']=17
    suite.report['screenshots']={p.name:digest(p) for p in suite.work.glob('*.png')}
    write(suite.work/'report.json',suite.report);write(args.output,suite.report)
    print(json.dumps(dict(status=suite.report['status'],work_directory=suite.report['work_directory'],error=suite.report.get('error'))),flush=True)
if suite.report['status']=='failed':sys.exit(1)
