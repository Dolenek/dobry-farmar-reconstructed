"""Shared Windows desktop probe for isolated game processes; physical input requires owned foreground."""
import ctypes as C,time,xml.etree.ElementTree as ET,json,os
from ctypes import wintypes as W
from pathlib import Path
from desktop_runtime_hold import require_desktop_runtime_available
u = C.WinDLL('user32', use_last_error=True)
k = C.WinDLL('kernel32', use_last_error=True)
g = C.WinDLL('gdi32', use_last_error=True)
k.SetErrorMode(3)
CALLBACK = C.WINFUNCTYPE(W.BOOL, W.HWND, W.LPARAM)
u.GetWindowThreadProcessId.argtypes = [W.HWND, C.POINTER(W.DWORD)]
u.GetClassNameW.argtypes = [W.HWND, W.LPWSTR, C.c_int]
u.GetWindowTextW.argtypes = [W.HWND, W.LPWSTR, C.c_int]
u.PostMessageW.argtypes = [W.HWND, W.UINT, W.WPARAM, W.LPARAM]
u.SendMessageW.argtypes = [W.HWND, W.UINT, W.WPARAM, W.LPARAM]
u.SendMessageW.restype = W.LPARAM
u.SendMessageTimeoutW.argtypes=[W.HWND,W.UINT,W.WPARAM,W.LPARAM,W.UINT,W.UINT,C.POINTER(C.c_size_t)]
u.SendMessageTimeoutW.restype=W.LPARAM
u.SetForegroundWindow.argtypes = [W.HWND]
u.GetClientRect.argtypes = [W.HWND, C.POINTER(W.RECT)]
u.ClientToScreen.argtypes = [W.HWND, C.POINTER(W.POINT)]
u.GetDC.argtypes = [W.HWND]
u.GetDC.restype = W.HDC
u.ReleaseDC.argtypes = [W.HWND, W.HDC]
g.CreateCompatibleDC.argtypes = [W.HDC]
g.CreateCompatibleDC.restype = W.HDC
g.CreateCompatibleBitmap.argtypes = [W.HDC, C.c_int, C.c_int]
g.CreateCompatibleBitmap.restype = W.HBITMAP
g.SelectObject.argtypes = [W.HDC, W.HGDIOBJ]
g.SelectObject.restype = W.HGDIOBJ
g.BitBlt.argtypes = [W.HDC, C.c_int, C.c_int, C.c_int, C.c_int, W.HDC, C.c_int, C.c_int, W.DWORD]
g.DeleteObject.argtypes = [W.HGDIOBJ]
g.DeleteDC.argtypes = [W.HDC]
g.GetDIBits.argtypes = [W.HDC, W.HBITMAP, W.UINT, W.UINT, C.c_void_p, C.c_void_p, W.UINT]

class KEY(C.Structure):
    _fields_ = [('vk', W.WORD), ('scan', W.WORD), ('flags', W.DWORD), ('time', W.DWORD), ('extra', C.c_size_t)]

class MOUSE(C.Structure):
    _fields_ = [('dx', W.LONG), ('dy', W.LONG), ('mouseData', W.DWORD), ('flags', W.DWORD), ('time', W.DWORD), ('extra', C.c_size_t)]

class UNION(C.Union):
    _fields_ = [('keyboard', KEY), ('mouse', MOUSE)]

class INPUT(C.Structure):
    _anonymous_ = ['data']
    _fields_ = [('type', W.DWORD), ('data', UNION)]
u.SendInput.argtypes = [W.UINT, C.POINTER(INPUT), C.c_int]
u.GetForegroundWindow.restype = W.HWND
u.SetWindowPos.argtypes = [W.HWND, W.HWND, C.c_int, C.c_int, C.c_int, C.c_int, W.UINT]

u.GetWindowDpiAwarenessContext.argtypes=[W.HWND]
u.GetWindowDpiAwarenessContext.restype=C.c_void_p
u.SetThreadDpiAwarenessContext.argtypes=[C.c_void_p]
u.SetThreadDpiAwarenessContext.restype=C.c_void_p
_unwrapped_input_apis={}
class GameGui:

    def __init__(self, work, data_root):
        self._runtime_watchdog=require_desktop_runtime_available()
        self._last_progress=None;self._last_progress_time=0
        self.work = Path(work)
        self.data_root = Path(data_root)
        self.process = None
        self.hwnd = None
        self._slow_input_call=None
        if self._runtime_watchdog:self._install_input_monitoring()

    def _install_input_monitoring(self):
        for name in ['SendInput','SetCursorPos','SetForegroundWindow']:
            original=_unwrapped_input_apis.setdefault(name,getattr(u,name))
            def measured(*args,_name=name,_original=original):
                values={}
                if _name=='SendInput':
                    count=int(args[0]);assert 0<=count<=64
                    events=C.cast(args[1],C.POINTER(INPUT))
                    values['input_flags']=[events[n].mouse.flags if events[n].type==0 else events[n].keyboard.flags for n in range(count)]
                self.progress('before-'+_name,**values);start=time.monotonic()
                result=_original(*args);seconds=time.monotonic()-start
                event=dict(api=_name,seconds=seconds,returned=int(result),**values)
                with (self._runtime_watchdog['directory']/'input-api-timings.jsonl').open('a',encoding='utf-8') as output:
                    output.write(json.dumps(event)+'\n')
                self.progress('after-'+_name,seconds=seconds)
                if seconds>1:self._slow_input_call=event
                return result
            setattr(u,name,measured)

    def progress(self,label,**values):
        if not self._runtime_watchdog:return
        now=time.monotonic()
        if label==self._last_progress and now-self._last_progress_time<.2:return
        directory=self._runtime_watchdog['directory'];path=directory/'progress.json'
        event=dict(driver_pid=os.getpid(),monotonic=now,label=label,**values)
        temporary=path.with_suffix('.tmp');temporary.write_bytes((json.dumps(event)+'\n').encode('utf-8'));temporary.replace(path)
        if label!=self._last_progress or now-self._last_progress_time>=1:
            with (directory/'driver-events.jsonl').open('a',encoding='utf-8') as output:output.write(json.dumps(event)+'\n')
        self._last_progress=label;self._last_progress_time=now

    def scan(self, code):
        self.focus()
        for flags in [8, 10]:
            item = INPUT()
            item.type = 1
            item.keyboard = KEY(0, code, flags, 0, 0)
            assert u.SendInput(1, C.byref(item), C.sizeof(item)) == 1
            time.sleep(0.08)

    def move(self, hwnd, x, y):
        self.focus()
        pt = W.POINT(x, y)
        assert u.ClientToScreen(hwnd, C.byref(pt))
        # A cursor warp need not produce DirectInput mouse-axis callbacks.
        # Deliver a real absolute move, including virtual-desktop normalization.
        left, top = u.GetSystemMetrics(76), u.GetSystemMetrics(77)
        width, height = u.GetSystemMetrics(78), u.GetSystemMetrics(79)
        assert width > 0 and height > 0
        item = INPUT()
        item.type = 0
        item.mouse = MOUSE(int((pt.x-left+0.5)*65536/width),
                           int((pt.y-top+0.5)*65536/height), 0, 0xc001, 0, 0)
        assert u.SendInput(1, C.byref(item), C.sizeof(item)) == 1
        # A short sequence keeps movement live while the game samples its
        # cursor and DirectInput state in separate calls within a frame.
        for dx in (1, -1, 1, -1, 1, -1):
            self.focus()
            item = INPUT(); item.type = 0
            item.mouse = MOUSE(dx, 0, 0, 0x2001, 0, 0)
            assert u.SendInput(1, C.byref(item), C.sizeof(item)) == 1
            time.sleep(0.04)
        assert u.SetCursorPos(pt.x, pt.y)
        time.sleep(0.05)
        self.focus()
        actual = W.POINT()
        assert u.GetCursorPos(C.byref(actual))
        assert (actual.x, actual.y) == (pt.x, pt.y), ('Injected pointer missed owned window target', (actual.x, actual.y), (pt.x, pt.y))
    def click(self, hwnd, x, y):
        self.move(hwnd, x, y)
        for flags in [2, 4]:
            item = INPUT()
            item.type = 0
            item.mouse = MOUSE(0, 0, 0, flags, 0, 0)
            assert u.SendInput(1, C.byref(item), C.sizeof(item)) == 1
            time.sleep(0.1)

    def windows(self, pid):
        found = []

        @CALLBACK
        def each(hwnd, _):
            owner = W.DWORD()
            u.GetWindowThreadProcessId(hwnd, C.byref(owner))
            if owner.value == pid:
                title = C.create_unicode_buffer(512)
                name = C.create_unicode_buffer(256)
                u.GetWindowTextW(hwnd, title, 512)
                u.GetClassNameW(hwnd, name, 256)
                found.append((int(hwnd), title.value, name.value))
            return True
        u.EnumWindows(each, 0)
        return found

    def errors(self, pid):
        output = []
        for hwnd, title, name in self.windows(pid):
            if name != '#32770':
                continue
            texts = []

            @CALLBACK
            def child(window, _):
                value = C.create_unicode_buffer(2048)
                u.GetWindowTextW(window, value, 2048)
                if value.value:
                    texts.append(value.value)
                return True
            u.EnumChildWindows(W.HWND(hwnd), child, 0)
            output.append(dict(title=title, text=texts))
            u.PostMessageW(W.HWND(hwnd), 16, 0, 0)
        return output

    def screenshot(self, hwnd, name):
        self.progress('before-window-capture',file=name)
        self.adopt_window_dpi(hwnd)
        from PIL import Image
        rect = W.RECT()
        assert u.GetClientRect(hwnd, C.byref(rect))
        width, height = (rect.right, rect.bottom)
        dc = u.GetDC(hwnd)
        copy = g.CreateCompatibleDC(dc)
        bitmap = g.CreateCompatibleBitmap(dc, width, height)
        old = g.SelectObject(copy, bitmap)
        assert g.BitBlt(copy, 0, 0, width, height, dc, 0, 0, 13369376)
        g.SelectObject(copy, old)

        class HEADER(C.Structure):
            _fields_ = [('size', W.DWORD), ('width', W.LONG), ('height', W.LONG), ('planes', W.WORD), ('bits', W.WORD), ('compression', W.DWORD), ('imageSize', W.DWORD), ('x', W.LONG), ('y', W.LONG), ('used', W.DWORD), ('important', W.DWORD)]
        header = HEADER(40, width, -height, 1, 32, 0, width * height * 4, 0, 0, 0, 0)
        pixels = C.create_string_buffer(width * height * 4)
        assert g.GetDIBits(dc, bitmap, 0, height, pixels, C.byref(header), 0) == height
        image = Image.frombytes('RGB', (width, height), pixels.raw, 'raw', 'BGRX')
        image.save(self.work / name)
        g.DeleteObject(bitmap)
        g.DeleteDC(copy)
        u.ReleaseDC(hwnd, dc)
        return image

    def adopt_window_dpi(self, hwnd=None):
        # Match the target's own coordinate system. D3D8 fullscreen compatibility
        # can change its awareness after startup; the Python probe's original
        # unaware coordinates would otherwise shrink800x600 to640x480 at125%.
        window=self.hwnd if hwnd is None else hwnd
        context=u.GetWindowDpiAwarenessContext(window)
        if context:
            assert u.SetThreadDpiAwarenessContext(context), 'Cannot adopt owned window DPI context'

    def focus(self):
        self.progress('before-owned-window-focus')
        if self._slow_input_call:
            raise RuntimeError(('Stopping after slow input API;no further interaction',self._slow_input_call))
        self.adopt_window_dpi()
        if u.GetForegroundWindow() != self.hwnd.value:
            u.SetForegroundWindow(self.hwnd)
            # Foreground activation across input queues is asynchronous. Wait
            # for the owned window to process that notification, with a bound;
            # never attach input queues to force synchronous activation.
            self.progress('before-bounded-foreground-notification')
            result=C.c_size_t()
            assert u.SendMessageTimeoutW(self.hwnd,0,0,0,2|32,1500,C.byref(result)), \
                'Owned window did not process its foreground notification within1500ms'
        # Never join another application's UI input queue to force focus.
        # A denied foreground request stops the test before sending input.
        if u.GetForegroundWindow()!=self.hwnd.value and self._runtime_watchdog:
            request=self._runtime_watchdog['directory']/'manual-focus-request.json'
            if request.exists():
                settings=json.loads(request.read_text(encoding='utf-8'))
                assert settings['driver_pid']==os.getpid() and 0<settings['seconds']<=20
                deadline=time.monotonic()+settings['seconds']
                while u.GetForegroundWindow()!=self.hwnd.value and time.monotonic()<deadline:
                    self.progress('waiting-for-user-owned-window-activation')
                    self.check_process();time.sleep(.05)
        assert u.GetForegroundWindow() == self.hwnd.value, 'Source application lost foreground; no global input was injected'
        if self._slow_input_call:
            raise RuntimeError(('Stopping after slow foreground API;no input was injected',self._slow_input_call))

    def resource(self, name):
        import zipfile
        data = None
        for packed in ['Data.pack', 'cz.pack']:
            with zipfile.ZipFile(self.data_root / packed) as z:
                for item in z.infolist():
                    if item.filename.lower() == name.lower():
                        data = z.read(item)
        assert data is not None, name
        return ET.fromstring(data)

    def button(self, file, name):
        row = self.resource(file).find(name)
        return [int(value) for value in row.get('position').split()]

    def modules(self, pid):
        ps = C.WinDLL('psapi', use_last_error=True)
        k.OpenProcess.argtypes = [W.DWORD, W.BOOL, W.DWORD]
        k.OpenProcess.restype = W.HANDLE
        k.CloseHandle.argtypes = [W.HANDLE]
        ps.EnumProcessModulesEx.argtypes = [W.HANDLE, C.c_void_p, W.DWORD, C.POINTER(W.DWORD), W.DWORD]
        ps.GetModuleFileNameExW.argtypes = [W.HANDLE, W.HMODULE, W.LPWSTR, W.DWORD]
        handle = k.OpenProcess(1040, False, pid)
        assert handle
        try:
            handles = (W.HMODULE * 2048)()
            size = W.DWORD()
            assert ps.EnumProcessModulesEx(handle, handles, C.sizeof(handles), C.byref(size), 3)
            result = {}
            for module in handles[:size.value // C.sizeof(W.HMODULE)]:
                path = C.create_unicode_buffer(32768)
                if ps.GetModuleFileNameExW(handle, module, path, len(path)):
                    result[Path(path.value).name.lower()] = path.value
            return result
        finally:
            k.CloseHandle(handle)

    def check_process(self):
        problem = self.errors(self.process.pid)
        if problem:
            raise RuntimeError(str(problem))
        if self.process.poll() is not None:
            raise RuntimeError('Application exited unexpectedly: ' + str(self.process.poll()))

    def capture(self, name):
        self.check_process()
        return self.screenshot(self.hwnd, name)

    def settle(self, name):
        time.sleep(0.8)
        return self.capture(name)

    def roi(self, image, rect):
        return image.crop(rect).tobytes()

    def wait_match(self, name, rect, expected, timeout=60):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            frame = self.capture(name)
            if self.roi(frame, rect) == expected:
                return frame
            time.sleep(0.2)
        raise RuntimeError('Expected screen region did not return: ' + name)
