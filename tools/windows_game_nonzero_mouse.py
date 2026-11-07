"""Physical motion that cannot cancel as opposite raw deltas in one UI frame."""
import ctypes as C
import time
from ctypes import wintypes as W
from windows_game_gui import GameGui,INPUT,MOUSE,u


class NonzeroMouseGui(GameGui):
    def move(self,hwnd,x,y):
        self.focus();target=W.POINT(x,y)
        assert u.ClientToScreen(hwnd,C.byref(target))
        left,top=u.GetSystemMetrics(76),u.GetSystemMetrics(77)
        width,height=u.GetSystemMetrics(78),u.GetSystemMetrics(79)
        assert width>0 and height>0
        event=INPUT();event.type=0
        event.mouse=MOUSE(int((target.x-left+.5)*65536/width),int((target.y-top+.5)*65536/height),0,0xc001,0,0)
        assert u.SendInput(1,C.byref(event),C.sizeof(event))==1
        # Cursor warp alone may not produce a raw DirectInput axis event.
        # Keep two positive real deltas, restoring the OS cursor after each.
        # A slow frame therefore cannot combine +1/-1 into a zero axis.
        for _ in range(2):
            self.focus();event=INPUT();event.type=0;event.mouse=MOUSE(1,0,0,0x2001,0,0)
            assert u.SendInput(1,C.byref(event),C.sizeof(event))==1
            assert u.SetCursorPos(target.x,target.y);time.sleep(.12)
        self.focus();actual=W.POINT();assert u.GetCursorPos(C.byref(actual))
        assert (actual.x,actual.y)==(target.x,target.y),(
            'Injected pointer missed owned window target',(actual.x,actual.y),(target.x,target.y))
