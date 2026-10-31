"""Audit actual contract controls separately from their translucent world backdrop."""
import collections
import io
import struct
import zipfile
from PIL import Image, ImageChops


def modal_sprites(scene):
    return collections.Counter(
        (tuple(g['geometry']), g['deep'], g['texture'], tuple(g['sprite']), g['flags'])
        for g in scene if 2029 <= g['deep'] < 2100 and g['texture'])


def panel_alpha(root):
    with zipfile.ZipFile(root.parent / 'farmář/Dobrý farmář/Data/Data.pack') as archive:
        name = next(n for n in archive.namelist() if n.lower() == 'ui/gameui/contract.jng')
        data = archive.read(name)
    assert data[:8] == b'\x8bJNG\r\n\x1a\n'
    offset = 8
    alpha_parts = []
    while offset < len(data):
        length = struct.unpack_from('>I', data, offset)[0]
        kind = data[offset + 4:offset + 8]
        payload = data[offset + 8:offset + 8 + length]
        assert len(payload) == length
        if kind == b'JHDR':
            assert struct.unpack_from('>II', payload) == (580, 460)
            assert list(payload[8:]) == [14, 8, 8, 8, 8, 8, 0, 0]
        if kind == b'JDAA':
            alpha_parts.append(payload)
        offset += length + 12
    assert offset == len(data) and alpha_parts
    return Image.open(io.BytesIO(b''.join(alpha_parts))).convert('L')


def audit_offer(root, work, label, scenes):
    left, right = [modal_sprites(scene) for scene in scenes]
    assert left == right, (label, list((left - right).elements())[:2], list((right - left).elements())[:2])
    assert sum(left.values()) == 118
    assert sum(n for row, n in left.items() if row[2] == 'ui/menu_font.dds') == 111
    images = [Image.open(work / f'{i}-{label}.png').convert('RGB') for i in [0, 1]]
    note_rect = (140, 100, 660, 170)
    assert images[0].crop(note_rect).tobytes() == images[1].crop(note_rect).tobytes(), label
    rect = (210, 170, 590, 405)
    # The actual panel occupies x110..690, y65..525 without scaling.
    alpha = panel_alpha(root).crop((rect[0] - 110, rect[1] - 65, rect[2] - 110, rect[3] - 65))
    assert alpha.getextrema() == (254, 254)
    delta = ImageChops.difference(*(im.crop(rect) for im in images))
    maximum = max(hi for lo, hi in delta.getextrema())
    assert maximum <= 1, (label, maximum, delta.getbbox())
    return dict(label=label, equal_actual_textured_modal_sprites=118, equal_actual_glyph_sprites=111,
                exact_note_rgb_pixels=520 * 70, note_rect=list(note_rect), backdrop_rect=list(rect),
                backdrop_pixels=380 * 235, backdrop_different_pixels=sum(any(pixel) for pixel in delta.getdata()),
                backdrop_max_channel_difference=maximum, original_panel_alpha=254,
                backdrop_difference_bound=1, background_animation_phases_synchronized=False,
                compared_pixels_modified=False)
