"""Read-only observation of owned Win32 game processes; never alters game state."""
import ctypes as C
import json
import struct
import subprocess
import time
from pathlib import Path
from ctypes import wintypes as W
from windows_game_gui import k,u
from read_only_scene_tree import read_scene_tree,SceneSnapshotChanged

class TutorialMemory:
    def __init__(self, pid, kind, root):
        self.handle = k.OpenProcess(0x1010, False, pid)  # QUERY_LIMITED + VM_READ only
        assert self.handle
        self.kind = kind
        self.pid = pid
        self.layout = None
        k.ReadProcessMemory.argtypes = [W.HANDLE, C.c_void_p, C.c_void_p, C.c_size_t, C.POINTER(C.c_size_t)]
        if kind == 'native':
            self.singleton = 0x65f73c
            self.step_offset = 0xbfc
            self.offsets = dict(image=0x40,text=0x44,ok=0x48,geometry=0x5c,sprite=0x70,alpha=0x58,control_state=8,flags=0x6c,texture=0x30)
        else:
            self.layout = json.loads(subprocess.check_output([str(root/'build/windows/Release/game-runtime-layout.exe')]))
            self.offsets = self.layout
            self.step_offset = self.layout['current_step']
            ps = C.WinDLL('psapi', use_last_error=True)
            modules = (W.HMODULE * 2048)(); size = W.DWORD()
            ps.EnumProcessModulesEx.argtypes = [W.HANDLE,C.c_void_p,W.DWORD,C.POINTER(W.DWORD),W.DWORD]
            assert ps.EnumProcessModulesEx(self.handle,modules,C.sizeof(modules),C.byref(size),3)
            base = modules[0]
            rows = (root/'build/windows/Release/DobryFarmar.map').read_text().splitlines()
            def symbol(part):
                found = [line.split() for line in rows if part in line]
                assert len(found) == 1, (part,found)
                return base + int(found[0][2],16) - 0x400000
            self.singleton = symbol('3PAVTutorialManager@')
            self.property_values = symbol('       ?values@?1??profileProperties@')
            self.vtables = {name:symbol('??_7Tutorial'+name+'Tip@') for name in ['Common','Arrow','DisabledAnchor','Action','RepeatedAction']}

    def close(self):
        if self.handle:
            k.CloseHandle(self.handle); self.handle = None

    def read(self, address, fmt):
        size = struct.calcsize(fmt)
        assert 0 < size <= 65536
        buffer = C.create_string_buffer(size); count = C.c_size_t()
        if not k.ReadProcessMemory(self.handle,address,buffer,size,C.byref(count)) or count.value != size:
            raise OSError(C.get_last_error(), 'ReadProcessMemory')
        return list(struct.unpack(fmt,buffer.raw))

    def manager(self):
        return self.read(self.singleton,'<I')[0]

    def step(self):
        manager = self.manager()
        return self.read(manager+self.step_offset,'<i')[0] if manager else None

    def tip(self, style):
        manager = self.manager()
        if self.kind == 'native':
            offset = dict(Common=0xbe8,Arrow=0xbec,DisabledAnchor=0xbf0,Action=0xbf4,RepeatedAction=0xbf8)[style]
            return self.read(manager+offset,'<I')[0]
        candidates = []
        for address in self.read(manager,'<'+str(self.layout['manager_size']//4)+'I'):
            if address < 65536:
                continue
            try:
                if self.read(address,'<I')[0] == self.vtables[style]:
                    candidates.append(address)
            except OSError:
                pass
        assert len(candidates) == 1, candidates
        return candidates[0]

    def snapshot(self, style):
        tip = self.tip(style); controls = {}
        for name in ['image','text','ok']:
            control = self.read(tip+self.offsets[name],'<I')[0]
            alpha = self.read(control+self.offsets['alpha'],'<I')[0]
            texture = self.read(control+self.offsets['texture'],'<I')[0]
            controls[name] = dict(
                geometry=self.read(control+self.offsets['geometry'],'<ffff'),
                sprite=self.read(control+self.offsets['sprite'],'<19I'),
                alpha=self.read(alpha,'<f')[0],
                state=self.read(control+self.offsets['control_state'],'<ffI'),
                flags=self.read(control+self.offsets['flags'],'<I')[0],
                texture=self.read(texture+0x28,'<8I') if texture else None)
        return controls

    def anchor_info(self, x, y):
        if self.kind != 'native':
            return {}
        tip = self.tip('Action'); anchor = self.read(tip+0x54,'<I')[0]
        geometry = self.read(anchor+0x5c,'<ffff')
        logic = self.read(0x65ff60,'<I')[0]
        result = dict(anchor=anchor,geometry=geometry,button_state=self.read(anchor+0x238,'<I')[0],
                      selected=self.read(0x6604dc,'<I')[0],pending=self.read(0x6604e0,'<I')[0],
                      cursor=self.read(logic+16,'<ff'),transitions=self.read(0x660528,'<I')[0])
        if self.read(anchor+0x268,'<B')[0]:
            begin,end = self.read(anchor+0x25c,'<II'); columns = (end-begin)//20
            gx,gy,width,height = geometry
            gy -= self.read(anchor+0x27c,'<f')[0]
            col = int((x-(gx-width/2))*(columns-1)/width)
            count,_,words,_,_ = self.read(begin+col*20,'<5I')
            bit = int((y-(gy-height/2))*(count-1)/height)
            result['mask_hit'] = bool(self.read(words+(bit//32)*4,'<I')[0] & (1<<(bit%32)))
        return result

    def visible_scene(self, hwnd):
        # Never suspend a rendering/UI thread: it may own graphics/USER locks.
        # Concurrent topology changes cause a bounded retry or an explicit
        # incomplete observation, rather than synchronization of the target.
        owner=W.DWORD();thread_id=u.GetWindowThreadProcessId(hwnd,C.byref(owner))
        assert thread_id and owner.value==self.pid,'Snapshot window does not belong to observed game process'
        last=None
        for _ in range(3):
            try:return self._visible_scene(time.monotonic()+.5)
            except SceneSnapshotChanged as error:last=error;time.sleep(.02)
        raise SceneSnapshotChanged('Could not observe a stable scene without stopping its thread: '+str(last)) from last

    def _visible_scene(self,deadline):
        if self.kind == 'native':
            head = self.read(0x660500,'<I')[0]; nil_offset=17; value_offset=12
            geometry_offset=0x5c; deep_offset=0x228; texture_offset=0x30; sprite_offset=0x70; flags_offset=0x6c
        else:
            image=self.read(self.tip('Action')+self.offsets['image'],'<I')[0]
            registry=self.read(image+self.layout['registry'],'<I')[0]
            head=self.read(registry,'<I')[0];nil_offset=13;value_offset=16
            geometry_offset=self.layout['geometry'];deep_offset=self.layout['deep'];texture_offset=self.layout['texture'];sprite_offset=self.layout['sprite'];flags_offset=self.layout['flags']
        def decode(control):
            texture=self.read(control+texture_offset,'<I')[0]
            name=None
            if texture:
                size,capacity=self.read(texture+8+20,'<II')
                assert size<1024
                address=self.read(texture+12,'<I')[0] if capacity>=16 else texture+12
                name=bytes(self.read(address,'<'+str(size)+'B')).decode('latin1') if size else ''
            return dict(address=control,geometry=self.read(control+geometry_offset,'<ffff'),deep=self.read(control+deep_offset,'<f')[0],texture=name,sprite=self.read(control+sprite_offset,'<19I'),flags=self.read(control+flags_offset,'<I')[0])
        return read_scene_tree(self.read,head,nil_offset,value_offset,decode,deadline)

    def property(self, store, name):
        # Bounded get-only lookup of the four scalar profile maps. No original
        # accessor is called (it would insert missing properties).
        kinds=dict(floats=('<f',0x65ff38),ints=('<i',0x65ff20),uints=('<I',0x65ff44),booleans=('<B',0x65ff2c))
        fmt,native_base=kinds[store]
        if self.kind=='native':
            head=self.read(native_base+4,'<I')[0];nil_offset=45;key_offset=12;value_offset=40;string_prefix=4
        else:
            head=self.read(self.property_values+self.layout['profile_'+store],'<I')[0]
            nil_offset=self.layout['profile_node_nil'];key_offset=self.layout['profile_node_key'];value_offset=self.layout['profile_node_value'];string_prefix=0
        node=self.read(head+4,'<I')[0];wanted=name.encode('ascii');visited=set()
        while not self.read(node+nil_offset,'<B')[0]:
            assert node not in visited and len(visited)<64;visited.add(node)
            key=node+key_offset+string_prefix
            size,capacity=self.read(key+16,'<II');assert size<=1024 and size<=capacity
            text=self.read(key,'<I')[0] if capacity>=16 else key
            value=bytes(self.read(text,'<'+str(size)+'B')) if size else b''
            if value==wanted:
                result=self.read(node+value_offset,fmt)[0]
                return bool(result) if store=='booleans' else result
            node=self.read(node+(0 if wanted<value else 8),'<I')[0]
        raise KeyError((store,name))
