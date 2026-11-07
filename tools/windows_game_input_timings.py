"""Measure the driver's Windows input API calls without changing their arguments."""
import json
import time
import ctypes as C
from windows_game_gui import u, INPUT


def record_input_api_timings(rows):
    """Wrap this Python driver's API objects only; no target process is patched."""
    for name in ['SendInput', 'SetCursorPos', 'GetCursorPos', 'ClientToScreen',
                 'GetWindowDpiAwarenessContext', 'SetThreadDpiAwarenessContext',
                 'GetForegroundWindow', 'SetForegroundWindow', 'AttachThreadInput']:
        original = getattr(u, name)
        def measured(*args, _name=name, _original=original):
            start = time.perf_counter()
            try:
                return _original(*args)
            finally:
                seconds = time.perf_counter() - start
                row = dict(api=_name, seconds=seconds)
                if _name == 'SendInput':
                    count = int(args[0])
                    assert 0 <= count <= 64
                    events = C.cast(args[1], C.POINTER(INPUT))
                    row['input_types'] = [events[n].type for n in range(count)]
                    row['input_flags'] = [events[n].mouse.flags if events[n].type == 0 else events[n].keyboard.flags for n in range(count)]
                rows.append(row)
                if seconds >= 1:
                    print(json.dumps(dict(event='slow_driver_input_api', **row)), flush=True)
        setattr(u, name, measured)
