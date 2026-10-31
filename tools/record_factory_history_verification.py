#!/usr/bin/env python3
"""Record actual cumulative manufacture history, natural award8 and complete saves."""
import datetime,json
from PIL import Image,ImageChops
from record_live_production_verification import ROOT,digest,read,write,checked


def pixels(left,right,rect=None):
    images=[Image.open(p).convert('RGB') for p in [left,right]]
    if rect:images=[im.crop(rect) for im in images]
    assert images[0].size==images[1].size
    a,b=[im.tobytes() for im in images]
    return dict(compared_pixels=len(a)//3,different_pixels=sum(a[n:n+3]!=b[n:n+3] for n in range(0,len(a),3)),difference_bounds=ImageChops.difference(*images).getbbox(),exact=a==b)


def main():
    state=read(ROOT/'.local/factory-history-state-validation.json');checked(state)
    assert state['status']=='passed_for_all_factory_kinds_one_profile_and_natural_award8_only'
    assert state['auditor_sha256']==digest(ROOT/'tools/validate_factory_history_state.py')
    c=state['comparison'];assert c['business_snapshots_per_executable']==16 and c['actual_alias_word_snapshots']==98
    assert c['complete_identical_save_bytes']==4252 and not c['normalizations']
    assert c['actual_award_panel_pixels']['exact'] and c['actual_award_panel_pixels']['compared_pixels']==480000
    interop=read(ROOT/'.local/factory-history-interop-validation.json');work=checked(interop)
    assert interop['status']=='passed_for_cumulative_eleven_factory_kind_history_bidirectional_saves_only'
    assert interop['fixture_report_sha256']==digest(ROOT/'.local/factory-history-state-validation.json')
    assert len(interop['sessions'])==len(interop['comparison']['transfer_comparisons'])==4
    for row in interop['sessions']:assert row['exit_code']==0 and row['status']=='completed'
    for row in interop['comparison']['transfer_comparisons']:assert row['identical_complete_save_bytes']==4252 and not row['normalizations']
    traces={}
    for path in work.glob('*-crash-trace.jsonl'):
        rows=[json.loads(line) for line in path.read_text(encoding='utf-8').splitlines()]
        assert rows[-1]==dict(event='exit',exit_code=0) and not any(row['event']=='access_violation' for row in rows)
        traces[path.name]=dict(sha256=digest(path),events=rows)
    assert len(traces)==2
    initial=ROOT/'.local/factory-history/20261031T111826990921Z'
    early=pixels(initial/'0-award-0.png',initial/'1-award-0.png')
    assert early['different_pixels']==9561
    bad_depth=read(ROOT/'.local/factory-history/20261031T112724410306Z/report.json')
    assert bad_depth['status']=='failed' and 'did not create visible text lines' in bad_depth['error']
    diagnostic=read(ROOT/'.local/award-history-visual-validation.json');late_work=checked(diagnostic)
    assert diagnostic['status']=='observed_actual_pending_award8_visuals_only'
    late=[]
    for index in range(4):
        frame=pixels(late_work/f'0-award-observation-{index}.png',late_work/f'1-award-observation-{index}.png')
        text=pixels(late_work/f'0-award-observation-{index}.png',late_work/f'1-award-observation-{index}.png',(260,340,550,470))
        assert text['exact']
        late.append(dict(index=index,whole_frame=frame,congratulation_rectangle=[260,340,550,470],congratulation_pixels=text))
    observations=dict(status='early_capture_and_incorrect_diagnostic_depth_replaced_by_observed_glyph_readiness',
        early_runtime_report_sha256=digest(initial/'report.json'),early_award_pixels=early,
        incorrect_depth_probe=dict(work_directory=bad_depth['work_directory'],report_sha256=digest(ROOT/bad_depth['work_directory']/'report.json'),error=bad_depth['error'],recorded_as_game_failure=False),
        late_diagnostic=dict(work_directory=diagnostic['work_directory'],report_sha256=digest(ROOT/'.local/award-history-visual-validation.json'),frames=late),
        actual_congratulation_glyphs=79,actual_congratulation_rows=4,actual_depth_range=[6400,6401],
        explanation='Root XML adds3100 to child3300. Wait for all79 glyphs on four rows and a subsequent rendered frame. Four late pending-award observations match the text; the complete new eleven-kind workflow matches all480000 RGB pixels.',
        production_implementation_changed=False,whole_game_complete=False)
    reports={}
    for name,value in [('factory-history-state-validation.json',state),('factory-history-interop-validation.json',interop),('factory-history-initial-observations.json',observations)]:
        write(ROOT/'analysis'/name,value);reports[name]=digest(ROOT/'analysis'/name)
    full=read(ROOT/'analysis/squall-channel-vectors-regression.json')
    for name,value in full['production_source_files'].items():assert digest(ROOT/name)==value,name
    for row in read(ROOT/'analysis/inventory.json')['files']:assert digest(ROOT.parent/'farmář/Dobrý farmář'/row['path'])==row['sha256']
    result=dict(status='passed_actual_cumulative_eleven_factory_kinds_natural_award8_and_restores',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),recorder_sha256=digest(__file__),reports=reports,
        natural_predicate='004c8650: all eleven awManDepotBuild0..10 flags',paid_kinds=list(range(4,11)),paid_cost=1850,business_snapshots_per_executable=16,actual_alias_word_snapshots=98,
        alias_padding_claimed_equal=False,all_current_profile_discretes_equal=c['all_profile_discretes_equal'],actual_award_panel_exact_rgb_pixels=480000,
        natural_new_journal_note=c['natural_new_journal_note'],complete_identical_save_bytes=4252,full_process_bidirectional_transfers=4,source_restore_debugger_traces=traces,
        production_implementation_changed=False,current_full_ctest_source_hashes_match=True,current_full_ctest_report='analysis/squall-channel-vectors-regression.json',original_files_unchanged=17,whole_game_complete=False,
        pending=['Five remaining crops and natural simultaneous-eight-culture award11','Other natural award predicates and modal overlap','Remaining Squall97, JNG and uninstaller scopes'])
    write(ROOT/'analysis/factory-history-regression.json',result)
    verification=read(ROOT/'analysis/verification.json')
    for name in list(reports)+['factory-history-regression.json']:
        if 'analysis/'+name not in verification['reports']:verification['reports'].append('analysis/'+name)
    for name in ['factory-history-state-validation.json','factory-history-interop-validation.json']:
        if 'analysis/'+name not in verification['original_game_runtime']['scope_reports']:verification['original_game_runtime']['scope_reports'].append('analysis/'+name)
    verification['latest_scoped_regression']=dict(report='analysis/factory-history-regression.json',recorded_at_utc=result['recorded_at_utc'],whole_game_complete=False)
    write(ROOT/'analysis/verification.json',verification)
    print('Recorded cumulative11 kinds, natural award8,98 alias words,480000 exact RGB pixels and four4252-byte transfers')


if __name__=='__main__':main()
