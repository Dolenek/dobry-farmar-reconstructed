"""Retain raw frames and verify reference-observed equal-depth rendering variants."""
from PIL import Image, ImageChops
from record_live_production_verification import ROOT, digest, read, checked


def rgb(path):
    image = Image.open(path).convert('RGB')
    assert image.size == (800,600)
    return image


def compare(left, right):
    a,b = left.tobytes(),right.tobytes()
    return dict(compared_pixels=480000,
        different_pixels=sum(a[n:n+3] != b[n:n+3] for n in range(0,len(a),3)),
        difference_bounds=ImageChops.difference(left,right).getbbox(),exact=a == b)


def audit_original_variants(initial_work):
    path = ROOT/'.local/high-price-pixels-validation.json'
    report = read(path)
    work = checked(report)
    assert report['status'] == 'explored_actual_price80_sale_natural_award15_and_price99_quiescence_only'
    assert all(s['status'] == 'completed' and s['exit_code'] == 0 for s in report['sessions'])
    native_paths = [initial_work/'0-award-0.png',work/'0-award-0.png']
    source_paths = [initial_work/'1-award-0.png',work/'1-award-0.png']
    native = [rgb(p) for p in native_paths]
    native_difference = compare(*native)
    assert native_difference['different_pixels'] == 1
    assert native_difference['difference_bounds'] == (206,83,207,84)
    matches = []
    for source in source_paths:
        actual = rgb(source)
        matching = [i for i,image in enumerate(native) if image.tobytes() == actual.tobytes()]
        assert len(matching) == 1
        reference = native_paths[matching[0]]
        matches.append(dict(source_frame=source.relative_to(ROOT).as_posix(),source_sha256=digest(source),
            original_frame=reference.relative_to(ROOT).as_posix(),original_sha256=digest(reference),
            entire_identical_rgb_pixels=480000,normalizations=[]))
    orders = []
    scenes = []
    for cycle,session in enumerate(report['sessions']):
        assert len(session['actual_award_contexts']) == 1
        context = session['actual_award_contexts'][0]
        assert context['high_sold'] and context['money'] == 3260 and context['market_milk'] == 9 and context['price'] == 80
        samples = session['actual_pixel_diagnostic_samples']
        assert len(samples) == 5
        expected = rgb(work/f'{cycle}-award-0.png').tobytes()
        for sample in samples:
            assert rgb(work/sample['screenshot']).tobytes() == expected
        scene = samples[0]['scene']
        tied = [r for r in scene if r['deep'] == 3090]
        assert len(tied) == 11 and [r['address'] for r in tied] == sorted(r['address'] for r in tied)
        order = [r['texture'] for r in tied]
        orders.append(dict(kind=session['kind'],equal_depth=3090,actual_draw_order=order,
                           actual_control_addresses=[r['address'] for r in tied],pointer_order_is_ascending=True))
        scenes.append({(r['texture'],tuple(r['geometry']),r['deep']):r for r in scene})
    assert set(scenes[0]) == set(scenes[1]) and len(scenes[0]) == 14
    checked_words = 0
    for key,left in scenes[0].items():
        right = scenes[1][key]
        assert left['actual_alpha'] == right['actual_alpha']
        for field in ['sprite','second_sprite']:
            # Word13 is the actual process-owned shader pointer. Every other
            # word, including null shader-constant pointer18, must match.
            for word,value in enumerate(left[field]):
                if word == 13:
                    continue
                assert value == right[field][word],(key,field,word,value,right[field][word])
                checked_words += 1
    a,b = [o['actual_draw_order'] for o in orders]
    assert (a.index('awards/animation_01.jng') < a.index('awards/animation_02.jng')) != (b.index('awards/animation_01.jng') < b.index('awards/animation_02.jng'))
    return dict(status='source_whole_frames_match_both_actual_original_rendering_variants',
        diagnostic_report=path.relative_to(ROOT).as_posix(),diagnostic_report_sha256=digest(path),
        original_to_original_raw_difference=native_difference,
        observed_original_pixel_values=[list(i.getpixel((206,83))) for i in native],
        whole_frame_original_source_matches=matches,diagnostic_equal_depth_orders=orders,
        identical_nonshader_pointer_sprite_words=checked_words,
        interpretation='Actual original and source registries sort equal depths by real control address;opposite order of overlapping icons01/02 accounts for the observed reference-compatible one-channel-one-unit pixel variants',
        initial_pair_draw_order_captured=False,initial_pair_heap_order_attribution_is_inference=True,
        same_heap_addresses_or_equal_draw_order_claimed=False,images_modified=False,normalizations=[])
