"""CPU-only observer safety checks: no Windows APIs, game, GPU, or input."""
import ast
import json
import struct
import sys
import tempfile
import time
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from read_only_scene_tree import read_scene_tree,SceneSnapshotChanged
from desktop_runtime_hold import require_desktop_runtime_available


class SceneTests(unittest.TestCase):
    def tree(self,native=True):
        nil,value=(17,12) if native else (13,16)
        size=max(nil+1,value+4);head,root,left,right=0x10000,0x20000,0x21000,0x22000
        data={}
        def node(address,l,p,r,v,is_nil=0):
            raw=bytearray(size);struct.pack_into('<III',raw,0,l,p,r)
            struct.pack_into('<I',raw,value,v);raw[nil]=is_nil;data[address]=bytes(raw)
        node(head,left,root,right,0,1);node(root,left,head,right,2)
        node(left,head,root,head,1);node(right,head,root,head,3)
        def read(address,fmt):
            if address not in data:raise OSError('Concurrent removal')
            return list(struct.unpack(fmt,data[address]))
        return data,read,head,nil,value,root

    def test_both_original_tree_layouts_preserve_order_without_os_calls(self):
        for native in [True,False]:
            data,read,head,nil,value,root=self.tree(native)
            self.assertEqual(read_scene_tree(read,head,nil,value,lambda v:v,time.monotonic()+1),[1,2,3])

    def test_concurrent_rotation_is_rejected(self):
        data,read,head,nil,value,root=self.tree()
        def decode(v):
            if v==2:
                changed=bytearray(data[root]);struct.pack_into('<I',changed,0,head);data[root]=bytes(changed)
            return v
        with self.assertRaisesRegex(SceneSnapshotChanged,'links changed'):
            read_scene_tree(read,head,nil,value,decode,time.monotonic()+1)

    def test_root_replacement_is_rejected(self):
        data,read,head,nil,value,root=self.tree()
        def decode(v):
            changed=bytearray(data[head]);struct.pack_into('<I',changed,4,head);data[head]=bytes(changed)
            return v
        with self.assertRaisesRegex(SceneSnapshotChanged,'sentinel changed'):
            read_scene_tree(read,head,nil,value,decode,time.monotonic()+1)

    def test_disappearing_nodes_and_controls_fail_closed(self):
        for remove_control in [False,True]:
            data,read,head,nil,value,root=self.tree()
            if not remove_control:del data[0x21000]
            def decode(v):
                if remove_control:raise OSError('Control released')
                return v
            with self.assertRaises(SceneSnapshotChanged):
                read_scene_tree(read,head,nil,value,decode,time.monotonic()+1)

    def test_cycles_are_bounded(self):
        data,read,head,nil,value,root=self.tree()
        raw=bytearray(data[root]);struct.pack_into('<I',raw,0,root);data[root]=bytes(raw)
        with self.assertRaises(SceneSnapshotChanged):
            read_scene_tree(read,head,nil,value,lambda v:v,time.monotonic()+1)

    def test_timeout_occurs_without_any_memory_read(self):
        def forbidden(*args):raise AssertionError('Read after deadline')
        with self.assertRaisesRegex(SceneSnapshotChanged,'time bound'):
            read_scene_tree(forbidden,0x10000,17,12,lambda v:v,time.monotonic()-1)

    def test_observer_contains_no_thread_suspension_call(self):
        tree=ast.parse((ROOT/'tools/windows_game_memory.py').read_text(encoding='utf-8'))
        forbidden={'SuspendThread','ResumeThread','Wow64SuspendThread','OpenThread'}
        calls={n.func.attr for n in ast.walk(tree) if isinstance(n,ast.Call) and isinstance(n.func,ast.Attribute)}
        self.assertFalse(forbidden & calls)

    def test_probe_never_joins_another_UI_input_queue(self):
        tree=ast.parse((ROOT/'tools/windows_game_gui.py').read_text(encoding='utf-8'))
        calls={n.func.attr for n in ast.walk(tree) if isinstance(n,ast.Call) and isinstance(n.func,ast.Attribute)}
        self.assertNotIn('AttachThreadInput',calls)

    def test_active_and_malformed_incident_hold_block_desktop_runtime(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'.local').mkdir();path=root/'.local/desktop-runtime-hold.json'
            for body in [json.dumps({'active':True}),'not valid JSON']:
                path.write_text(body,encoding='utf-8')
                with self.assertRaises(RuntimeError):require_desktop_runtime_available(root)
            path.write_text(json.dumps({'active':False}),encoding='utf-8')
            require_desktop_runtime_available(root)

    @unittest.skipUnless(sys.platform=='win32' and (ROOT/'.local/desktop-runtime-hold.json').exists(),
        'Actual host incident guard is Windows/local only')
    def test_actual_game_probe_constructor_refuses_before_runtime(self):
        from windows_game_gui import GameGui
        with self.assertRaisesRegex(RuntimeError,'Interactive host game tests are held'):
            GameGui(ROOT/'.local',ROOT/'.local')


if __name__=='__main__':unittest.main()
