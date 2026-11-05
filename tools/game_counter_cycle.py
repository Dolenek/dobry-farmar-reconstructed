"""Actual UI-only grass/milk supply cycle;all observation uses read-only helpers."""
import ctypes as C,time,math
from windows_game_gui import INPUT,KEY,MOUSE,u

def run_cycle(*,hwnd,probe,observer,resource,button,click,focus,check_process,wait_for,service_award,service_tip,session,args):
    val=observer.property
    def drain_modals():
        handled=False
        while service_award() or service_tip():handled=True;focus();check_process()
        return handled
    def select_scene(nav_x):
        # A newly earned award can intercept a navigation click while the
        # background factory completes its next unit. Returning restores the
        # previous scene,so repeat the requested ordinary navigation afterward.
        for _ in range(5):
            drain_modals();click(hwnd,nav_x,550);time.sleep(.65)
            if not drain_modals():return
        raise RuntimeError(('Scene navigation repeatedly intercepted by actual modals',nav_x))
    def stocks():return [val('uints',f'WGoodCount{n}') for n in range(19)]
    initial=dict(collected=val('uints','awQCollect'),processed=val('uints','awQProcess'),grass=val('uints','WGoodCount0'),
        carried=val('uints','PCC0'),milk=val('uints','WGoodCount10'),market_milk=val('uints','MGoodCount10'),money=val('ints','Money'))
    session['counter_cycle_initial']=initial;session['paid_cost']=0;session['ui_cycles']=[]
    assert val('uints','PCarry')==6 and all(val('uints',f'PCC{n}')==0 for n in range(1,8))
    assert all(val('uints',f'MGoodCount{n}')==0 for n in range(19) if n!=10)
    assert val('uints','MGoodPrice10')==2
    starting_loaded=val('uints','ProdCount0')
    def ledger(label):
        # Two identical monotone-counter/inventory readings avoid treating a
        # production tick between RPM calls as an atomic state observation.
        def read():return dict(collected=val('uints','awQCollect'),processed=val('uints','awQProcess'),grass=val('uints','WGoodCount0'),
            carried=val('uints','PCC0'),milk=val('uints','WGoodCount10'),market_milk=val('uints','MGoodCount10'),loaded=val('uints','ProdCount0'),money=val('ints','Money'))
        end=time.monotonic()+5;attempts=0
        while True:
            a=read();b=read();attempts+=1
            if a==b:break
            check_process();assert time.monotonic()<end,('No stable counter snapshot',a,b);time.sleep(.01)
        harvested=b['collected']-initial['collected'];produced=b['processed']-initial['processed']
        assert harvested>=0 and produced>=0 and b['carried']<=16
        assert initial['grass']+initial['carried']+starting_loaded+harvested==b['grass']+b['carried']+b['loaded']+produced,('Grass conservation',label,initial,b)
        sold=initial['milk']+initial['market_milk']+produced-b['milk']-b['market_milk'];assert sold>=0
        assert b['money']==initial['money']-session['paid_cost']+2*sold,('Milk income or additional world money event',label,initial,b,sold)
        b.update(label=label,harvested=harvested,produced=produced,milk_sold=sold,game_time=val('floats','GameTime'),run_time=val('floats','RunTime'),stable_read_attempts=attempts)
        session.setdefault('conservation_steps',[]).append(b);return b
    market=resource('UI/gameUI/Market.xml');milk_place=list(market.find('ProductPlaces'))[10]
    mx,my=map(float,milk_place.get('pos').split());dx,dy=map(float,market.find('ProductLine').find('IncAllQtyButton').get('position').split())
    def sell_milk(label):
        select_scene(300);before=ledger(label+'-before-milk-transfer')
        if before['milk']:
            delivered_before=before['produced']-before['milk']
            target=delivered_before+before['milk'];deadline=time.monotonic()+15
            while True:
                drain_modals();click(hwnd,mx+dx,my+dy);time.sleep(.12);after=ledger(label+'-milk-transferred')
                if after['produced']-after['milk']>=target:break
                assert time.monotonic()<deadline,('Actual milk all-quantity button did not transfer',before,after)
        return ledger(label+'-after-market')
    sell_milk('initial')
    select_scene(500)
    costs=[int(x.text) for x in list(resource('Game.xml').find('Costs'))[2]]
    if val('ints','Upgrade0')==-1:
        assert all(val('ints',f'Upgrade{n}')==-1 for n in range(20))
        click(hwnd,*button('ui/gameui/manufacture.xml','BuildButton'));time.sleep(.5)
        place=list(resource('UI/gameUI/BuyBuilding.xml').find('ItemPlace'))[2];x,y=map(float,place.get('place').split())
        click(hwnd,x,y);time.sleep(.2);click(hwnd,400,250)
        wait_for('paid-cowshed-created',lambda:val('ints','Upgrade0')==0 and val('uints','ProdType0')==2)
        session['paid_cost']+=costs[0];ledger('paid-cowshed-created')
    assert val('uints','ProdType0')==2
    # The whole-game reference fixtures already prove the paid level callbacks;
    # these are the same actual buttons,with no cheat or profile writes.
    for level in range(val('ints','Upgrade0')+1,4):
        drain_modals();click(hwnd,400,250);time.sleep(.15);click(hwnd,*button('ui/gameui/manufacture.xml','UpgradeButton'))
        wait_for(f'paid-cowshed-level-{level}',lambda level=level:val('ints','Upgrade0')==level)
        session['paid_cost']+=costs[level];ledger(f'paid-cowshed-level-{level}')
    assert val('ints','Upgrade0')==3
    cow=resource('Manufacture/CowShed.xml');capacity=int(list(cow.find('Upgrades'))[3].get('ProductionNum'));assert capacity==12
    height=float(cow.get('size').split()[1]);sx,sy=map(float,cow.find('Loaded').get('position').split())
    slider=(400-100+sx,250+height/2+sy)
    def mouse(flags):
        focus();event=INPUT();event.type=0;event.mouse=MOUSE(0,0,0,flags,0,0);assert u.SendInput(1,C.byref(event),C.sizeof(event))==1
    def feed(label):
        select_scene(500);click(hwnd,400,250);time.sleep(.12)
        before=ledger(label+'-before-feed')
        if before['grass'] and before['loaded']<capacity:
            probe.move(hwnd,round(slider[0]),round(slider[1]));mouse(2);time.sleep(.08);probe.move(hwnd,round(slider[0]+50),round(slider[1]));time.sleep(.12);mouse(4);time.sleep(.08)
            after=ledger(label+'-after-slider-feed');assert after['grass']<before['grass'],('Actual bulk load slider did not take grass',before,after)
        return ledger(label+'-after-manufacture')
    def key(code,down):
        focus();event=INPUT();event.type=1;event.keyboard=KEY(0,code&0x7f,8|(1 if code&0x80 else 0)|(0 if down else 2),0,0)
        assert u.SendInput(1,C.byref(event),C.sizeof(event))==1;time.sleep(.08)
    key(0x38,True);key(0xd2,True);key(0xd2,False);key(0x38,False)
    session['actual_speed_input']='Alt+Insert original15x shortcut'
    positions=[tuple(map(float,row.get('Center').split())) for row in resource('UI/gameUI/farm.xml').find('FieldArray')]
    sprinklers=[(val('floats',f'SPXPos{n}'),val('floats',f'SPYPos{n}')) for n in range(3)]
    radius=float(resource('Farm/ActionAreas/Sprinkle.xml').get('Radius'))
    covered=[n for n,(x,y) in enumerate(positions) if n not in [18,27,42,50] and any(math.hypot(x-sx,y-sy)<radius for sx,sy in sprinklers)]
    assert len(covered)>=16;session['covered_harvest_pool']=covered
    deadline=time.monotonic()+1700
    for cycle in range(250):
        check_process();assert time.monotonic()<deadline,'Counter cycle exceeded its bounded1700-second observation'
        drain_modals();current=ledger(f'cycle-{cycle}-start')
        if current['collected']>=args.harvest_target and current['processed']>=args.production_target:break
        sell_milk(f'cycle-{cycle}');feed(f'cycle-{cycle}')
        select_scene(400)
        before=ledger(f'cycle-{cycle}-before-store')
        if before['carried']:
            click(hwnd,120,100)
            wait_for(f'cycle-{cycle}-stored',lambda:val('uints','PCC0')<before['carried'] or sum(stocks())>=100)
            ledger(f'cycle-{cycle}-after-store')
        current=ledger(f'cycle-{cycle}-before-harvest')
        need_more=current['collected']<args.harvest_target or current['grass']+current['loaded']<max(0,args.production_target-current['processed'])
        amount=min(12,16-current['carried']) if need_more else 0
        chosen=[]
        for _ in range(amount):
            drain_modals()
            ready=[n for n in covered if val('uints',f'FFC{n}')==0 and val('uints',f'FFS{n}')==1 and val('floats',f'FFW{n}')>15]
            if not ready:break
            field=ready[0];count=val('uints','awQCollect');click(hwnd,*positions[field])
            wait_for(f'cycle-{cycle}-harvest-field-{field}',lambda:val('uints','awQCollect')==count+1)
            chosen.append(field);ledger(f'cycle-{cycle}-harvest-field-{field}')
        current=ledger(f'cycle-{cycle}-finish');session['ui_cycles'].append(dict(cycle=cycle,harvested_fields=chosen,collected=current['collected'],processed=current['processed']))
    else:raise RuntimeError('Counter cycle exhausted250 actual UI cycles')
    drain_modals();final=ledger('final')
    assert final['collected']>=args.harvest_target and final['processed']>=args.production_target
    for index,threshold,key in [(1,100,'Award01Produce100'),(2,1000,'Award02Grow1000'),(3,1000,'Award03Produce1000')]:
        value=final['collected'] if index==2 else final['processed']
        if value>=threshold:
            wait_for(f'natural-counter-award-{index}-owned',lambda key=key,index=index:val('booleans',key) and val('booleans',f'awardShow{index}'))
    session['counter_cycle_final']=ledger('final-awards-confirmed')
