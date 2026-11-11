"""Actual repeated capacity/mixed-crop actions; no profile edits or game writes."""
import time
from live_farm_fields import read_live_farm_fields
from record_live_production_verification import ROOT


def fields(t,layout):
    return read_live_farm_fields(t.observer,ROOT,layout,t.observer.visible_scene(t.hwnd))


def queue_fields(t,targets):
    assert t.paused
    t.pause(True)
    initial=t.state();baseline=sum((r['texture'] or '').lower()=='person/mark.jng'
        for r in t.observer.visible_scene(t.hwnd))
    observations=[]
    for number,field in enumerate(targets,baseline+1):
        for attempt in range(3):
            t.click(*field['position'])
            count=sum((r['texture'] or '').lower()=='person/mark.jng'
                for r in t.observer.visible_scene(t.hwnd))
            if count==number:break
            assert count==number-1,('Unexpected actual queued markers',count,number)
        assert count==number,('Actual field click failed to enqueue',field['id'])
        at=t.state();assert at['game_time']==initial['game_time'] and at['carried']==initial['carried']
        observations.append(dict(field=field['id'],actual_markers=count,game_time=at['game_time']))
    t.session.setdefault('queued_field_batches',[]).append(observations)


def take_spade_to_resolve(t,index,await_condition):
    t.click(80,420)
    return await_condition(t,f'warning-{index}-resolved-by-spade',lambda:
        t.observer.step()==-1 and t.observer.property('booleans',f'tip{index}') and
        t.observer.property('uints','PCarry')==2,expected_active=(index,))


def full_warehouse(t,await_condition,observe_warning,layout):
    initial=t.state();t.session['initial_state']=initial
    capacity=int(t.probe.resource('Farm/ActionAreas/Warehouse.xml').find('Upgrades').find(
        'Up'+str(initial['warehouse_level'])).get('ProductionNum'))
    assert initial['held']==6 and sum(initial['carried'])==0 and sum(initial['stock'])==capacity
    assert not initial['processed'][41] and initial['counters']['capacity_previous']==-1
    assert initial['counters']['capacity_counter']==100
    t.pause(True)
    target=next(r for r in fields(t,layout) if r['active'] and r['status']==1 and r['culture']==0)
    queue_fields(t,[target]);t.pause(False)
    harvested=await_condition(t,'one-grass-into-mower',lambda:t.observer.property('uints','PCC0')==1)
    assert harvested['collected']==initial['collected']+1 and harvested['stock']==initial['stock']
    # Warehouse uses deep processing: a paused press becomes one pending
    # enabledStep action. Repeating paused presses would collapse that input.
    t.pause(True);t.click(120,100);t.pause(False)
    deadline=time.monotonic()+90
    while t.counters()['capacity_previous']==-1:
        t.probe.check_process();assert time.monotonic()<deadline,'First actual Warehouse arrival missing'
        time.sleep(.01)
    t.pause(True);first=t.state()
    assert first['tutorial']==-1 and first['counters']['capacity_counter']==100
    assert first['counters']['elapsed']-first['counters']['capacity_previous']<3
    t.click(120,100)
    still=t.state();assert still['game_time']==first['game_time']
    assert still['counters']==first['counters']
    t.session['first_capacity_arrival_and_paused_retry']=dict(first=first,after_click=still)
    t.pause(False)
    warning=observe_warning(t,'full_warehouse',41)['state']
    # C10 is updated by the actual second arrival. UI settling/capture may
    # continue to advance manager elapsed after the condition already fired.
    delta=warning['counters']['capacity_previous']-first['counters']['capacity_previous']
    assert 0<=delta<3 and warning['counters']['capacity_counter']==0
    assert warning['stock']==initial['stock'] and warning['carried']==[1]+[0]*7
    final=take_spade_to_resolve(t,41,await_condition)
    assert final['stock']==initial['stock'] and final['market']==initial['market']
    assert final['carried']==[1]+[0]*7 and final['collected']==initial['collected']+1
    t.session['final_state']=final;t.capture('full-warehouse-resolved')
    t.session['capacity_warning_evidence']=dict(original_capacity=capacity,
        attempted_store_count=2,second_attempt_game_seconds_since_first=delta,
        actual_original_C14_reset_to_zero=True,actual_held_type_change_resolved_predicate=True,
        all19_stock_and_market_words_unchanged=True,carried_grass_retained=1)


def mixed_products(t,await_condition,observe_warning,layout):
    initial=t.state();t.session['initial_state']=initial
    assert initial['mower_level']==0 and initial['held']==1 and sum(initial['carried'])==0
    assert not initial['processed'][40] and not initial['processed'][44]
    assert initial['counters']['capacity_counter']==100
    # First settle warning40 through a genuine full tomato load and Store.
    t.pause(True);t.click(736,407);t.pause(False)
    await_condition(t,'actual-mower-picked-up',lambda:t.observer.property('uints','PCarry')==6)
    t.pause(True)
    tomatoes=[r for r in fields(t,layout) if r['active'] and r['status']==1 and r['culture']==1]
    assert len(tomatoes)>=4
    targets=tomatoes[:4];t.session['original_tomato_targets']=targets
    queue_fields(t,targets);t.pause(False)
    warning40=observe_warning(t,'mixed_precondition',40)['state']
    assert warning40['carried']==[0,3]+[0]*6 and warning40['collected']==initial['collected']+3
    t.click(120,100)
    stored=await_condition(t,'three-tomatoes-unloaded',lambda:t.observer.step()==-1 and
        t.observer.property('booleans','tip40') and t.observer.property('uints','PCC1')==0,expected_active=(40,))
    expected=initial['stock'].copy();expected[1]+=3
    assert stored['stock']==expected and stored['counters']['capacity_counter']==100
    t.pause(True);remaining=next(r for r in fields(t,layout) if r['id']==targets[-1]['id'])
    assert remaining['status']==1 and remaining['culture']==1
    queue_fields(t,[remaining]);t.pause(False)
    carried=await_condition(t,'fourth-tomato-harvested',lambda:t.observer.property('uints','PCC1')==1)
    assert carried['collected']==initial['collected']+4
    t.pause(True)
    planted=[r for r in fields(t,layout) if r['active'] and r['culture']==3]
    assert planted,'Genuine cucumber checkpoint has no actual planted cucumber'
    t.session['cucumber_before_natural_readiness_wait']=planted
    if not any(r['status']==1 for r in planted):
        t.pause(False)
        await_condition(t,'natural-cucumber-readiness',lambda:any(
            t.observer.property('uints',f"FFS{r['id']}")==1 for r in planted),timeout=180)
        t.pause(True)
    cucumbers=[r for r in fields(t,layout) if r['active'] and r['status']==1 and r['culture']==3]
    assert cucumbers,'Actual readiness did not remain present while paused'
    target=cucumbers[0];t.session['actual_mixed_cucumber_target']=target
    queue_fields(t,[target,target]);t.pause(False)
    warning=observe_warning(t,'mixed_products',44)['state']
    assert warning['carried']==[0,1]+[0]*6 and warning['collected']==initial['collected']+4
    assert warning['counters']['capacity_counter']==100 and warning['counters']['mixed_counter']==0
    assert warning['counters']['mixed_previous']>=0
    final=take_spade_to_resolve(t,44,await_condition)
    assert final['stock']==expected and final['market']==initial['market']
    assert final['carried']==[0,1]+[0]*6 and final['collected']==initial['collected']+4
    t.session['final_state']=final;t.capture('mixed-products-resolved')
    t.session['mixed_warning_evidence']=dict(actual_blocked_cucumber_attempts=2,
        tomato_harvests=4,stored_tomatoes=3,carried_tomatoes_retained=1,cucumbers_harvested=0,
        original_wrong_C14_comparison_preserved=True,initial_undefined_C1c_not_compared=True,
        actual_C1c_reset_zero=True,all19_stock_and_market_words_checked=True)
