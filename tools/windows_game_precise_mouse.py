"""Physical input with fewer Win32 round trips; game outcomes remain independently checked."""
import ctypes as C
import time
from ctypes import wintypes as W
from windows_game_gui import GameGui,INPUT,MOUSE,u


class PreciseGameGui(GameGui):
    def move(self,hwnd,x,y):
        self.focus();point=W.POINT(x,y);assert u.ClientToScreen(hwnd,C.byref(point))
        left,top=u.GetSystemMetrics(76),u.GetSystemMetrics(77)
        width,height=u.GetSystemMetrics(78),u.GetSystemMetrics(79)
        assert width>0 and height>0
        item=INPUT();item.type=0
        item.mouse=MOUSE(int((point.x-left+.5)*65536/width),int((point.y-top+.5)*65536/height),0,0xc001,0,0)
        assert u.SendInput(1,C.byref(item),C.sizeof(item))==1
        # Keep the same .24s movement interval across DirectInput/UI polls,
        # using two opposite real relative events instead of six. No game
        # message, memory, callback or cursor state is injected into the app.
        for delta in [1,-1]:
            self.focus();item=INPUT();item.type=0;item.mouse=MOUSE(delta,0,0,0x2001,0,0)
            assert u.SendInput(1,C.byref(item),C.sizeof(item))==1;time.sleep(.12)
        assert u.SetCursorPos(point.x,point.y);time.sleep(.05);self.focus()
        actual=W.POINT();assert u.GetCursorPos(C.byref(actual))
        assert (actual.x,actual.y)==(point.x,point.y),('Injected pointer missed owned window target',(actual.x,actual.y),(point.x,point.y))
