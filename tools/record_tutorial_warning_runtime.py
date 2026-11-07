#!/usr/bin/env python3
"""Independently retain completed native/source actual tutorial warning evidence."""
import argparse
import copy
import datetime
import json
import math
from pathlib import Path
import xml.etree.ElementTree as ET
from PIL import Image,ImageChops
from current_production_regression import current_regression
from record_live_production_verification import ROOT,digest,read,write

CASES={'full_carry':40,'full_warehouse':41,'poor_contract':42,'mixed_products':44}


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime-report',type=Path,action='append',required=True)
    parser.add_argument('--output',type=Path,default=ROOT/'analysis/tutorial-warning-runtime-regression.json')
    args=parser.parse_args();pairs={};pair_work={};reports={};implementation=None
    for path in args.runtime_report:
        path=path.resolve(strict=True);raw=read(path);work=ROOT/raw['work_directory']
        assert raw['status'].startswith('passed_actual_') and raw['reference_files_unchanged']==17
        reports[path.relative_to(ROOT).as_posix()]=digest(path)
        assert not raw['original_profile_fields_or_memory_written'] and not raw['original_program_code_used_in_implementation']
        if implementation is None:implementation=raw['implementation_files']
        assert implementation==raw['implementation_files']
        for name,sha in raw['source_files'].items():
            source=ROOT/raw.get('source_file_snapshots',{}).get(name,name)
            assert digest(source)==sha,name
        for name,sha in raw['screenshots'].items():assert digest(work/name)==sha,name
        for name,sha in implementation.items():
            assert digest(work/'runtime'/name)==sha,name
            current='Dobrý farmář.exe' if name=='FarmarTutorialWarnings.exe' else name
            assert digest(ROOT/'build/windows/Release'/current)==sha,name
        prefix=raw.get('completed_native_prefix')
        if prefix:
            assert digest(ROOT/prefix['path'])==prefix['sha256']
            prior=read(ROOT/prefix['path']);reports[prefix['path']]=prefix['sha256']
            assert prior['implementation_files']==implementation
        for session in raw['sessions']:
            assert session['status']=='completed' and session['exit_code']==0
            if session['kind']=='native':
                assert json.loads(session['launcher_completion'])==dict(event='exit',exit_code=0,watchdog_triggered=False)
            origin=session.get('reused_completed_evidence_report')
            if origin:
                assert prefix and origin==prefix['path']
                original=next(s for s in prior['sessions'] if s['kind']=='native' and s['case']==session['case'])
                retained=copy.deepcopy(session);del retained['reused_completed_evidence_report']
                assert retained==original and original['status']=='completed'
            case=session['case'];key=(case,session['kind']);assert key not in pairs,key
            assert session['input_profile_files']==raw['fixtures'][case]['files']
            for name,sha in session['input_profile_files'].items():
                assert digest(ROOT/session['seed_profile_directory']/name)==sha
            initial,final=session['initial_state'],session['final_state']
            step=CASES[case];assert not initial['processed'][str(step)] and final['processed'][str(step)]
            assert final['tutorial']==-1 and final['market']==initial['market']
            properties={n.get('Name'):(n.get('Type'),n.get('Val')) for n in ET.parse(
                ROOT/session['persisted_profile_directory']/'00000001.xml').getroot()}
            assert properties[f'tip{step}']==('Bool','1')
            for index in range(19):
                assert properties[f'WGoodCount{index}']==('UInt',str(final['stock'][index]))
                assert properties[f'MGoodCount{index}']==('UInt',str(final['market'][index]))
            for index in range(8):assert properties[f'PCC{index}']==('UInt',str(final['carried'][index]))
            assert properties['awQCollect']==('UInt',str(final['collected']))
            assert properties['PCarry']==('UInt',str(final['held']))
            if case=='full_carry':
                capacity=session['original_mower_capacity']['capacity'];needed=capacity-sum(initial['carried'])
                expected=initial['stock'].copy();expected[0]+=capacity
                assert final['stock']==expected and final['carried']==[0]*8
                assert final['collected']==initial['collected']+needed
            elif case=='poor_contract':
                assert initial['money']==final['money']==2 and initial['stock']==final['stock']
                assert initial['carried']==final['carried'] and session['no_disabled_acceptance_or_deposit_occurred']
            elif case=='full_warehouse':
                assert final['stock']==initial['stock'] and final['carried']==[1]+[0]*7
                assert final['collected']==initial['collected']+1 and final['held']==2
                assert final['counters']['capacity_counter']==0
                assert 0<=session['capacity_warning_evidence']['second_attempt_game_seconds_since_first']<3
            else:
                expected=initial['stock'].copy();expected[1]+=3
                assert final['stock']==expected and final['carried']==[0,1]+[0]*6
                assert final['collected']==initial['collected']+4 and final['held']==2
                assert final['counters']['mixed_counter']==0 and final['counters']['capacity_counter']==100
            pairs[key]=session;pair_work[key]=work
    cases=sorted({key[0] for key in pairs});comparisons=[]
    for case in cases:
        native,source=[pairs[(case,kind)] for kind in ['native','source']]
        assert native['input_profile_files']==source['input_profile_files']
        for key in ['stock','market','carried','collected','held']:
            assert native['initial_state'][key]==source['initial_state'][key],(case,'initial',key)
            assert native['final_state'][key]==source['final_state'][key],(case,'final',key)
        nstage=next(s for s in native['stages'] if s['step']==CASES[case])
        sstage=next(s for s in source['stages'] if s['step']==CASES[case])
        assert sorted(nstage['glyph_geometry_and_sprites'])==sorted(sstage['glyph_geometry_and_sprites'])
        for key in ['geometry','sprite','alpha','flags']:
            assert nstage['controls']['image'][key]==sstage['controls']['image'][key],(case,key)
        glyphs=nstage['glyph_geometry_and_sprites']
        # Native letters have negative height. Bounds use absolute extents;
        # retain rather than endorse earlier driver's empty-area comparison.
        rect=[max(0,math.floor(min(g[0][axis]-abs(g[0][axis+2])/2 for g in glyphs))-2) for axis in range(2)]
        rect += [min(limit,math.ceil(max(g[0][axis]+abs(g[0][axis+2])/2 for g in glyphs))+2) for axis,limit in [(0,800),(1,600)]]
        left=Image.open(pair_work[(case,'native')]/nstage['screenshot']).convert('RGB').crop(tuple(rect))
        right=Image.open(pair_work[(case,'source')]/sstage['screenshot']).convert('RGB').crop(tuple(rect))
        assert left.width>0 and left.height>0
        delta=ImageChops.difference(left,right)
        pixels=dict(rect=rect,pixels=left.width*left.height,identical=left.tobytes()==right.tobytes(),
            difference_bounds=delta.getbbox(),raw_driver_empty_area_comparison_not_used=
                sstage.get('text_composite_pixel_comparison',{}).get('pixels')==0,
            whole_timed_world_frame_equality_claimed=False)
        comparisons.append(dict(case=case,step=CASES[case],normal_process_exits=[0,0],
            text_glyphs_exact=len(nstage['glyph_geometry_and_sprites']),bubble_geometry_sprite_alpha_flags_exact=True,
            text_composite_pixel_comparison=pixels,
            all19_stock_and_market_words_checked=True,input_profile_files=native['input_profile_files']))
    name,_=current_regression()
    for row in read(ROOT/'analysis/inventory.json')['files']:
        assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    complete=set(cases)==set(CASES)
    if complete:
        path=ROOT/'analysis/tutorial-native-callers-audit.json';audit=read(path)
        assert audit['status']=='passed_native_compiled_tutorial_callers_have_no_index0_or10'
        assert audit['compiled_caller_indices']==[n for n in range(50) if n not in [0,10]]
        assert audit['independent_text_section_relative_call_scan_exact']
        reports[path.relative_to(ROOT).as_posix()]=digest(path)
    report=dict(status='passed_actual_full_carry_storage_capacity_mixed_products_and_contract_warning_paths_only' if complete else
        'passed_actual_paired_tutorial_warning_subset_only',
        recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),
        reports=reports,implementation_files=implementation,verified_steps=sorted(CASES[c] for c in cases),
        comparisons=comparisons,sessions=[dict(kind=s['kind'],case=s['case'],exit_code=0,
            stages=[dict(step=stage['step'],screenshot=stage['screenshot']) for stage in s['stages']]) for s in pairs.values()],
        original_files_unchanged=17,production_changed=False,current_full_ctest_source_hashes_match=True,
        current_full_ctest_report=name,new_full_ctest_run=False,
        whole_timed_world_frame_equality_claimed=False,whole_game_complete=False)
    write(args.output,report)
    verification=read(ROOT/'analysis/verification.json');entry=args.output.resolve().relative_to(ROOT).as_posix()
    if entry not in verification['reports']:verification['reports'].append(entry)
    verification['current_tutorial_warning_gate']=dict(report=entry,complete=complete,required_actual_steps=[40,41,42,44])
    write(ROOT/'analysis/verification.json',verification)
    print('Verified actual warning steps:',report['verified_steps'],'complete warning gate:',complete)


if __name__=='__main__':main()
