"""Read actual field identity/activation from owned game objects; no game writes."""


def read_live_farm_fields(observer, root, metadata, scene):
    if observer.kind == 'native':
        vtable=0x5f1870
        offsets=dict(id=0x508,upgrade_index=0x50c,active=0x510)
    else:
        lines=(root/'build/windows/Release/DobryFarmar.map').read_text(encoding='latin-1').splitlines()
        def preferred(part):
            found=[line.split() for line in lines if part in line]
            assert len(found)==1,(part,len(found))
            return int(found[0][2],16)
        base=observer.singleton-preferred('3PAVTutorialManager@')+0x400000
        vtable=base+preferred('??_7FarmField@')-0x400000
        offsets=metadata
    rows=[]
    for control in scene:
        address=control['address']
        if observer.read(address,'<I')[0]!=vtable:continue
        index=observer.read(address+offsets['id'],'<I')[0]
        assert 0<=index<60
        rows.append(dict(id=index,upgrade=observer.read(address+offsets['upgrade_index'],'<i')[0],
            active=bool(observer.read(address+offsets['active'],'<B')[0]),position=control['geometry'][:2],
            water=observer.property('floats',f'FFW{index}'),progress=observer.property('floats',f'FFP{index}'),
            culture=observer.property('uints',f'FFC{index}'),status=observer.property('uints',f'FFS{index}')))
    rows.sort(key=lambda row:row['id'])
    assert [row['id'] for row in rows]==list(range(60))
    return rows
