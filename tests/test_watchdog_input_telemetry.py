"""CPU-only mock input telemetry; every input/focus API is replaced by a fake."""
import ctypes as C
import json
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))


@unittest.skipUnless(sys.platform=='win32','Windows ctypes declarations only')
class InputTelemetryTests(unittest.TestCase):
    def test_explicit_manual_wait_uses_real_activation_without_injecting_input(self):
        import windows_game_gui as gui
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory);(path/'manual-focus-request.json').write_text(json.dumps(dict(driver_pid=os.getpid(),seconds=1)),encoding='utf-8')
            probe=gui.GameGui.__new__(gui.GameGui);probe._runtime_watchdog=dict(directory=path)
            probe._last_progress=None;probe._last_progress_time=0;probe._slow_input_call=None
            probe.hwnd=gui.W.HWND(0x1234);probe.adopt_window_dpi=lambda:None;probe.check_process=lambda:None
            current=[0x5678];clock=[100.0]
            def user_activates(seconds):clock[0]+=seconds;current[0]=0x1234
            with mock.patch.object(gui.u,'GetForegroundWindow',lambda:current[0]), \
                 mock.patch.object(gui.u,'SetForegroundWindow',lambda *args:0), \
                 mock.patch.object(gui.u,'SendMessageTimeoutW',lambda *args:1), \
                 mock.patch.object(gui.u,'SendInput',side_effect=AssertionError('Unexpected injected input')), \
                 mock.patch.object(gui.time,'monotonic',lambda:clock[0]), \
                 mock.patch.object(gui.time,'sleep',user_activates):
                probe.focus()

    def test_async_foreground_notification_is_bounded_without_queue_attachment(self):
        import windows_game_gui as gui
        probe=gui.GameGui.__new__(gui.GameGui);probe._runtime_watchdog=None;probe._slow_input_call=None
        probe.hwnd=gui.W.HWND(0x1234);probe.adopt_window_dpi=lambda:None;current=[0x5678]
        def acknowledge(hwnd,message,wparam,lparam,flags,timeout,result):
            self.assertEqual((hwnd.value,message,timeout),(0x1234,0,1500));current[0]=0x1234;return 1
        with mock.patch.object(gui.u,'GetForegroundWindow',lambda:current[0]), \
             mock.patch.object(gui.u,'SetForegroundWindow',lambda *args:1), \
             mock.patch.object(gui.u,'SendMessageTimeoutW',acknowledge):
            probe.focus()

    def test_denied_foreground_is_not_bypassed(self):
        import windows_game_gui as gui
        probe=gui.GameGui.__new__(gui.GameGui);probe._runtime_watchdog=None;probe._slow_input_call=None
        probe.hwnd=gui.W.HWND(0x1234);probe.adopt_window_dpi=lambda:None
        with mock.patch.object(gui.u,'GetForegroundWindow',lambda:0x5678), \
             mock.patch.object(gui.u,'SetForegroundWindow',lambda *args:0), \
             mock.patch.object(gui.u,'SendMessageTimeoutW',lambda *args:1):
            with self.assertRaisesRegex(AssertionError,'lost foreground'):probe.focus()

    def test_slow_down_is_logged_and_up_can_finish_before_next_action_is_refused(self):
        import windows_game_gui as gui
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory);clock=[100.0];flags=[]
            probe=gui.GameGui.__new__(gui.GameGui);probe._runtime_watchdog=dict(directory=path)
            probe._last_progress=None;probe._last_progress_time=0;probe._slow_input_call=None
            def send(count,events,size):
                before=json.loads((path/'progress.json').read_text(encoding='utf-8'))
                self.assertEqual(before['label'],'before-SendInput')
                value=C.cast(events,C.POINTER(gui.INPUT))[0].mouse.flags;flags.append(value)
                self.assertEqual(before['input_flags'],[value]);clock[0]+=2 if value==2 else .01
                return count
            with mock.patch.dict(gui._unwrapped_input_apis,{},clear=True), \
                 mock.patch.object(gui.u,'SendInput',send), \
                 mock.patch.object(gui.u,'SetCursorPos',lambda *args:1), \
                 mock.patch.object(gui.u,'SetForegroundWindow',lambda *args:1), \
                 mock.patch.object(gui.time,'monotonic',lambda:clock[0]):
                probe._install_input_monitoring()
                for value in [2,4]:
                    event=gui.INPUT();event.type=0;event.mouse=gui.MOUSE(0,0,0,value,0,0)
                    self.assertEqual(gui.u.SendInput(1,C.byref(event),C.sizeof(event)),1)
                self.assertEqual(flags,[2,4])
                self.assertEqual(probe._slow_input_call['input_flags'],[2])
                with self.assertRaisesRegex(RuntimeError,'Stopping after slow input API'):probe.focus()
            rows=[json.loads(line) for line in (path/'input-api-timings.jsonl').read_text(encoding='utf-8').splitlines()]
            self.assertEqual([r['input_flags'] for r in rows],[[2],[4]])
            self.assertEqual(rows[0]['seconds'],2)


if __name__=='__main__':unittest.main()
