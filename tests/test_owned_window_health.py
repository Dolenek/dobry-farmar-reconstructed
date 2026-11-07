"""Mock-only responsiveness policy tests; no real window or message is used."""
import ctypes as C
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))


@unittest.skipUnless(sys.platform=='win32','Windows ctypes declarations only')
class OwnedWindowHealthTests(unittest.TestCase):
    def probe(self,path):
        import windows_game_gui as gui
        probe=gui.GameGui.__new__(gui.GameGui)
        probe._runtime_watchdog=dict(directory=path)
        probe._last_progress=None;probe._last_progress_time=0
        probe.hwnd=gui.W.HWND(0x1234)
        probe.process=mock.Mock(pid=2718)
        return gui,probe

    def test_responsive_window_is_rate_limited_and_logged(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory);gui,probe=self.probe(path);clock=[100.0]
            def owner(window,output):C.cast(output,C.POINTER(gui.W.DWORD))[0]=2718;return 17
            def acknowledge(window,message,wparam,lparam,flags,timeout,result):
                self.assertEqual((window.value,message,wparam,lparam,flags,timeout),(0x1234,0,0,0,34,250))
                clock[0]+=.01;return 1
            with mock.patch.object(gui.u,'GetWindowThreadProcessId',owner), \
                 mock.patch.object(gui.u,'SendMessageTimeoutW',side_effect=acknowledge) as send, \
                 mock.patch.object(gui.time,'monotonic',lambda:clock[0]):
                probe._check_owned_window_health();probe._check_owned_window_health()
                self.assertEqual(send.call_count,1)
                clock[0]+=.5;probe._check_owned_window_health();self.assertEqual(send.call_count,2)
            rows=[json.loads(s) for s in (path/'owned-window-health.jsonl').read_text().splitlines()]
            self.assertEqual([r['consecutive_misses'] for r in rows],[0,0])
            self.assertTrue(all(r['responsive'] for r in rows))

    def test_one_startup_miss_is_not_failure_but_three_consecutive_misses_stop(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory);gui,probe=self.probe(path);clock=[100.0]
            def owner(window,output):C.cast(output,C.POINTER(gui.W.DWORD))[0]=2718;return 17
            with mock.patch.object(gui.u,'GetWindowThreadProcessId',owner), \
                 mock.patch.object(gui.u,'SendMessageTimeoutW',side_effect=[0,1,0,0,0]), \
                 mock.patch.object(gui.time,'monotonic',lambda:clock[0]):
                for _ in range(4):probe._check_owned_window_health();clock[0]+=.6
                with self.assertRaisesRegex(RuntimeError,'three consecutive'):
                    probe._check_owned_window_health()
            rows=[json.loads(s) for s in (path/'owned-window-health.jsonl').read_text().splitlines()]
            self.assertEqual([r['consecutive_misses'] for r in rows],[1,0,1,2,3])

    def test_reused_window_handle_never_receives_a_message(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory);gui,probe=self.probe(path)
            def wrong_owner(window,output):C.cast(output,C.POINTER(gui.W.DWORD))[0]=99;return 17
            with mock.patch.object(gui.u,'GetWindowThreadProcessId',wrong_owner), \
                 mock.patch.object(gui.u,'SendMessageTimeoutW',side_effect=AssertionError('Unowned message')) as send:
                with self.assertRaisesRegex(RuntimeError,'changed process'):
                    probe._check_owned_window_health()
                send.assert_not_called()
            self.assertFalse((path/'owned-window-health.jsonl').exists())


if __name__=='__main__':unittest.main()
