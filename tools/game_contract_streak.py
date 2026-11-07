"""Operate real offers, deposits, capacity-limited supplies and contract receipts."""
import ctypes as C
import json
import math
import struct
import time
from windows_game_gui import u, INPUT, KEY, MOUSE
from live_gameplay_state import LiveGameplayState
from gameplay_coefficient_audit import f32
from game_save_state import load_save
from contract_supply_plan import another_product_command_can_help


def run_streak(env):
    probe,observer = env['probe'],env['observer']
    root,session,hwnd = env['r'],env['session'],env['hwnd']
    resource,button,click = env['resource'],env['button'],env['click']
    focus,check = env['focus'],env['check_process']
    capture = env['capture']
    val = observer.property
    target = env['args'].target
    baseline = env['seed_profiles']
    live = LiveGameplayState(observer,root,env['report']['gameplay_layout']['value'])
    initial_coefficients = live.coefficients()
    assert initial_coefficients == load_save(baseline/'00000001.save')['coefficient_bits']
    session['coefficient_epochs'] = [dict(game_time=val('floats','GameTime'),words=initial_coefficients,reason='actual loaded unmodified save')]
    events = list(resource('Gameplay/gameplay.xml').find('Events'))
    event_seconds = lambda e:sum(int(v)*m for v,m in zip(e.get('Time').split(':'),[3600,60,1]))
    events.sort(key=event_seconds)
    models = {}
    for index,event in enumerate(events):
        if event.get('Type') != '5':
            continue
        data = event.find('Data')
        reqs = [dict(product=int(row.get('ProductType')),initial_count=int(row.get('Count')),count=int(row.get('Count')))
                for row in list(data.find('Requirements'))[:2] if row.get('ReqType','1') == '0']
        models[index] = dict(index=index,initial_bonus=int(data.get('Bonus')),initial_penalty=int(data.get('Penalty')),
            bonus=int(data.get('Bonus')),penalty=int(data.get('Penalty')),requirements=reqs)

    def apply(words):
        for index,model in models.items():
            coefficient = struct.unpack('<f',struct.pack('<I',words[index]))[0]
            for field in ['bonus','penalty']:
                result = model[field]+math.trunc(f32(model['initial_'+field]*coefficient))
                if result > 100:
                    result = math.trunc(f32(math.trunc(f32(result*f32(.1)))*10))
                model[field] = result
            for req in model['requirements']:
                req['count'] = (req['count']+math.trunc(f32(req['initial_count']*coefficient))) & 0xffffffff

    apply(initial_coefficients)
    last_time = val('floats','GameTime')
    deadline = time.monotonic()+env['args'].phase_seconds
    fast = False
    paused = False
    paused_state = None
    initial_streak = val('uints','awQTask')
    assert initial_streak < target <= 40
    active_task = val('ints','task0')
    assert all(val('ints',f'task{n}') == -1 for n in [1,2])
    assert active_task == -1 or session.get('normally_closed_failure_checkpoint')
    assert all(val('ints',f'Upgrade{n}') == -1 for n in range(20))
    session['initial_streak'] = initial_streak
    session['supply_fixture_policy'] = 'Only actual original WM_CHAR cheatproduct;no award command or direct counter/profile/memory write'
    session['contract_receipts'] = []
    session['supply_commands'] = []
    session['market_transfers'] = []
    session['live_progress'] = []

    def clock():
        nonlocal last_time
        check()
        assert time.monotonic() < deadline,('Bounded contract phase exceeded',env['args'].phase_seconds)
        now = val('floats','GameTime')
        if paused:
            assert now == paused_state['game_time'] and val('floats','taskTime0') == paused_state['remaining'],('Original pause did not preserve actual contract clock',paused_state,now,val('floats','taskTime0'))
        if last_time-now > 1000:
            words = live.coefficients()
            apply(words)
            session['coefficient_epochs'].append(dict(before=last_time,game_time=now,words=words,reason='actual14400-to11190 loop'))
        last_time = now
        return now

    def snapshot():
        return dict(game_time=clock(),money=val('ints','Money'),streak=val('uints','awQTask'),
            stock=[val('uints',f'WGoodCount{n}') for n in range(19)],
            market=[val('uints',f'MGoodCount{n}') for n in range(19)],
            tasks=[val('ints',f'task{n}') for n in range(3)],
            remaining=[val('floats',f'taskTime{n}') for n in range(3)])

    def key(code,down):
        focus();item=INPUT();item.type=1
        item.keyboard=KEY(0,code&0x7f,8|(1 if code&0x80 else 0)|(0 if down else 2),0,0)
        assert u.SendInput(1,C.byref(item),C.sizeof(item)) == 1
        time.sleep(.1)

    def speed(value):
        nonlocal fast
        if fast == value:
            return
        key(0x38,True);key(0xd2,True);key(0xd2,False);key(0x38,False)
        fast = value
        clock()

    def pause(value):
        nonlocal paused,paused_state
        if paused == value:
            return
        key(0x38,True);key(0xc5,True);key(0xc5,False);key(0x38,False)
        if value:
            # Input can set pause during an already dispatched game step.
            # Observe its final commit before starting the strict interval;
            # never permit clock drift during any stock/input operations.
            limit=time.monotonic()+5;stable_since=time.monotonic();observations=[];previous=None
            while True:
                check();now=time.monotonic()
                current=dict(game_time=val('floats','GameTime'),remaining=val('floats','taskTime0'))
                observations.append(current.copy())
                if current!=previous:previous=current;stable_since=now
                if now-stable_since>=.35:break
                assert now<limit,('Original pause did not settle',observations)
                time.sleep(.05)
            session.setdefault('actual_pause_settle_observations',[]).append(observations)
            paused_state=current;paused=True
            session.setdefault('actual_supply_pause_intervals',[]).append(dict(start=paused_state.copy()))
        else:
            paused=False
            session['actual_supply_pause_intervals'][-1]['end'] = dict(game_time=val('floats','GameTime'),remaining=val('floats','taskTime0'))
        clock()

    def drain():
        if paused:
            return False
        handled = False
        while env['service_award']() or env['service_tip']():
            handled = True;clock()
        return handled

    backgrounds = {300:resource('UI/gameUI/Market.xml').find('Background').find('Picture').get('Picture').lower(),
                   400:resource('UI/gameUI/farm.xml').find('Background').find('Picture').get('Picture').lower()}

    def select_scene(x):
        if paused:
            assert x == 300
            scene=observer.visible_scene(hwnd)
            assert any((r['texture'] or '').lower()==backgrounds[x] and abs(r['geometry'][0]-400)<.01 for r in scene)
            return
        limit = time.monotonic()+15
        while True:
            drain();click(hwnd,x,550);time.sleep(.7)
            if drain():
                continue
            scene = observer.visible_scene(hwnd)
            if any((r['texture'] or '').lower() == backgrounds[x] and abs(r['geometry'][0]-400)<.01 for r in scene):
                return
            assert time.monotonic() < limit,('Actual navigation did not reach its requested scene',x)

    market = resource('UI/gameUI/Market.xml')
    places = [tuple(map(float,row.get('pos').split())) for row in market.find('ProductPlaces')]
    inc = tuple(map(float,market.find('ProductLine').find('IncPriceButton').get('position').split()))
    transfer = tuple(map(float,market.find('ProductLine').find('IncAllQtyButton').get('position').split()))
    letters = [tuple(map(float,row.get('Pos').split())) for row in resource('UI/gameUI/farm.xml').find('LetterPlace')]

    select_scene(300)
    session['actual_price_clicks'] = []
    for product,(x,y) in enumerate(places):
        probe.move(hwnd,round(x+inc[0]),round(y+inc[1]))
        limit = time.monotonic()+15
        while val('uints',f'MGoodPrice{product}') < 99:
            before = val('uints',f'MGoodPrice{product}')
            for flags in [2,4]:
                item=INPUT();item.type=0;item.mouse=MOUSE(0,0,0,flags,0,0)
                assert u.SendInput(1,C.byref(item),C.sizeof(item)) == 1
                time.sleep(.06)
            after = val('uints',f'MGoodPrice{product}')
            session['actual_price_clicks'].append(dict(product=product,before=before,after=after,game_time=clock()))
            assert before <= after <= 99 and time.monotonic() < limit
    assert all(val('uints',f'MGoodPrice{n}') == 99 for n in range(19))
    session['all_prices99'] = snapshot()
    last_progress = 0

    def complete(accepted):
        model = accepted['model']
        if not live.dialog('confirm')['shown']:
            ready=all(val('uints',f"WGoodCount{req['product']}")>=req['count'] for req in model['requirements'])
            if not ready:
                return False
            pause(False)
            limit=time.monotonic()+8
            while not live.dialog('confirm')['shown']:
                focus();clock();assert time.monotonic()<limit,('Actual stocked contract did not open its ready dialog',accepted,snapshot());time.sleep(.05)
        pause(False)
        speed(False)
        limit = time.monotonic()+8
        while live.dialog('confirm')['transition'][2] != 0:
            focus();check();assert time.monotonic() < limit;time.sleep(.05)
        before = snapshot()
        capture(f"{env['cycle']}-contract-{len(session['contract_receipts'])}-ready.png")
        click(hwnd,*button('Gameplay/ContractPassConfirm.xml','OkButton'))
        limit = time.monotonic()+8;retry_at=time.monotonic()+1
        while val('uints','awQTask') != accepted['before']['streak']+1:
            if time.monotonic()>=retry_at:
                current=snapshot()
                assert live.dialog('confirm')['shown'] and current['tasks'][0]==accepted['offer']
                assert current['money']==before['money'] and current['stock']==before['stock']
                session.setdefault('confirmation_input_retries',[]).append(dict(offer=accepted['offer'],state=current,dialog=live.dialog('confirm')))
                click(hwnd,*button('Gameplay/ContractPassConfirm.xml','OkButton'));retry_at=time.monotonic()+1
            focus();clock();assert time.monotonic() < limit,('Contract receipt counter did not increment',accepted,snapshot());time.sleep(.05)
        after = snapshot()
        expected = before['stock'].copy()
        for req in model['requirements']:
            assert expected[req['product']] >= req['count']
            expected[req['product']] -= req['count']
        assert after['stock'] == expected and after['market'] == before['market']
        assert after['money'] == before['money']+model['bonus']
        assert before['money'] == accepted['before']['money']-(0 if accepted.get('restored_active') else model['penalty'])
        assert all(t == -1 for t in after['tasks'])
        receipt = dict(offer=accepted['offer'],model=model,epoch=accepted['epoch'],
            before_acceptance=None if accepted.get('restored_active') else accepted['before'],before_completion=before,after_completion=after)
        if accepted.get('restored_active'):
            receipt['restored_active_state']=accepted['before']
            receipt['acceptance_and_deposit_occurred_before_this_process']=True
        session['contract_receipts'].append(receipt)
        print(json.dumps(dict(kind=env['kind'],event='actual_contract_completed',offer=accepted['offer'],streak=after['streak'],money=after['money']),ensure_ascii=True),flush=True)
        drain()
        return True

    restored_pending = None
    if active_task != -1:
        restored_pending = dict(offer=active_task,model=json.loads(json.dumps(models[active_task])),
            before=snapshot(),epoch=0,restored_active=True)
        session['restored_active_contract']=json.loads(json.dumps(restored_pending))
    while val('uints','awQTask') < target:
        if restored_pending is not None:
            accepted=restored_pending;restored_pending=None
            offer,model=accepted['offer'],accepted['model']
        else:
            drain();select_scene(400);speed(True)
            while True:
                focus();now=clock();drain()
                options=[]
                for slot in range(3):
                    offer = val('ints',f'letter{slot}')
                    if offer < 0:
                        continue
                    model = models[offer]
                    if sum(req['count'] for req in model['requirements']) <= 100 and all(req['count']<=env['args'].max_product_requirement for req in model['requirements']) and model['penalty'] <= val('ints','Money'):
                        options.append((slot,offer))
                if options:
                    break
                if time.monotonic()-last_progress > 5:
                    progress=dict(kind=env['kind'],event='waiting_actual_offer',game_time=now,streak=val('uints','awQTask'),epochs=len(session['coefficient_epochs']))
                    session['live_progress'].append(progress);print(json.dumps(progress),flush=True);last_progress=time.monotonic()
                time.sleep(.1)
            speed(False)
            slot,offer=options[0]
            if val('ints',f'letter{slot}') != offer:
                continue
            click(hwnd,*letters[slot]);limit=time.monotonic()+8
            while not live.dialog('provide')['shown'] or live.dialog('provide')['transition'][2] != 0:
                focus();clock();assert time.monotonic()<limit,('Actual offer dialog did not open',offer);time.sleep(.05)
            # Copy the actual coefficient-derived model, not a mutable future epoch.
            model=json.loads(json.dumps(models[offer]));before=snapshot()
            accepted=dict(offer=offer,model=model,before=before,epoch=len(session['coefficient_epochs'])-1)
            time.sleep(.5)
            click(hwnd,*button('Gameplay/ContractProvide.xml','YesButton'));limit=time.monotonic()+8;retry_at=time.monotonic()+1
            while val('ints','task0') != offer:
                if time.monotonic()>=retry_at:
                    current=snapshot()
                    assert live.dialog('provide')['shown'] and current['tasks']==[-1]*3 and current['money']==before['money']
                    session.setdefault('acceptance_input_retries',[]).append(dict(offer=offer,state=current,dialog=live.dialog('provide')))
                    capture(f"{env['cycle']}-offer-{offer}-acceptance-retry-{len(session['acceptance_input_retries'])}.png")
                    click(hwnd,*button('Gameplay/ContractProvide.xml','YesButton'));retry_at=time.monotonic()+1
                focus();clock();assert time.monotonic()<limit,('Actual offer was not accepted',offer,snapshot());time.sleep(.05)
            assert val('ints','Money') == before['money']-model['penalty']
        keep={r['product'] for r in model['requirements']}
        done=False
        for supply_pass in range(60):
            drain()
            if complete(accepted):
                done=True;break
            select_scene(300)
            if complete(accepted):
                done=True;break
            pause(True)
            stock_for_plan=snapshot()['stock']
            can_batch=another_product_command_can_help(stock_for_plan,model['requirements'])
            if can_batch:session.setdefault('supply_clear_batches_avoided',[]).append(dict(pass_number=supply_pass,stock=stock_for_plan))
            if not can_batch:
                for product,(x,y) in enumerate(places):
                    if product in keep or not val('uints',f'WGoodCount{product}'):
                        continue
                    if complete(accepted):
                        done=True;break
                    timing_start=time.perf_counter()
                    before_transfer=snapshot();quantity=before_transfer['stock'][product]
                    timing_snapshot=time.perf_counter()
                    # GameGui.click already performs the closed-loop pointer move.
                    probe.click(hwnd,round(x+transfer[0]),round(y+transfer[1]))
                    timing_click=time.perf_counter()
                    limit=time.monotonic()+5
                    while val('uints',f'WGoodCount{product}'):
                        focus();clock();assert time.monotonic()<limit,('Actual market transfer did not clear product',product);time.sleep(.05)
                    after_transfer=snapshot()
                    timing_after=time.perf_counter()
                    expected=before_transfer['stock'].copy();expected[product]=0
                    expected_market=before_transfer['market'].copy();expected_market[product]+=quantity
                    assert after_transfer['stock']==expected and after_transfer['market']==expected_market and after_transfer['money']==before_transfer['money']
                    session['market_transfers'].append(dict(product=product,quantity=quantity,before=before_transfer,after=after_transfer))
                    timing=dict(product=product,before_snapshot_seconds=timing_snapshot-timing_start,
                        physical_click_seconds=timing_click-timing_snapshot,
                        receipt_and_snapshot_seconds=timing_after-timing_click)
                    session.setdefault('market_transfer_durations',[]).append(timing)
                    if timing_after-timing_start>5:
                        print(json.dumps(dict(kind=env['kind'],event='slow_actual_market_transfer',**timing)),flush=True)
                    drain()
            if done:
                break
            before_supply=snapshot()
            assert before_supply['remaining'][0]>0 and sum(before_supply['stock'])<=100
            for ch in 'cheatproduct':
                focus();assert u.PostMessageW(hwnd,0x102,ord(ch),0);time.sleep(.02)
            time.sleep(.1)
            after_supply=snapshot();expected=before_supply['stock'].copy();free=100-sum(expected)
            for product in range(19):
                added=min(2,free);expected[product]+=added;free-=added
            assert after_supply['stock']==expected and after_supply['market']==before_supply['market'] and after_supply['money']==before_supply['money']
            session['supply_commands'].append(dict(command='cheatproduct',pass_number=supply_pass,before=before_supply,after=after_supply))
        assert done,('Contract supplies exhausted60 bounded passes',offer,snapshot())
        assert val('uints','awQTask') == initial_streak+len(session['contract_receipts'])
    speed(False);drain()
    thresholds = [(4,'Award04Make20Task')] if target >= 20 else []
    if target >= 40:
        thresholds.append((5,'Award05Make40Task'))
    for ordinal,owned_key in thresholds:
        shown_key=f'awardShow{ordinal}'
        observed=dict(ordinal=ordinal,streak=val('uints','awQTask'),
            before_owned=val('booleans',owned_key),before_shown=val('booleans',shown_key))
        session.setdefault('threshold_award_waits',[]).append(observed)
        limit=time.monotonic()+60
        while not (val('booleans',owned_key) and val('booleans',shown_key)):
            drain();focus();clock()
            assert time.monotonic()<limit,('Actual contract threshold award did not finish',ordinal,snapshot())
            time.sleep(.1)
        drain()
        observed.update(after_owned=val('booleans',owned_key),after_shown=val('booleans',shown_key),game_time=clock())
    session['final_streak_state']=snapshot()
    assert session['final_streak_state']['streak']==target
    session['final_coefficient_words']=live.coefficients()
