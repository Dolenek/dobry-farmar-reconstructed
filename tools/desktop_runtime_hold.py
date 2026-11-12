"""Local incident hold for interactive host tests; no environment-variable bypass."""
import json
import os
from pathlib import Path


def verified_watchdog_context(root,hold):
    """An environment path alone grants nothing: require the live owning Job."""
    import ctypes as C
    from ctypes import wintypes as W
    if not hold.get('monitored_host_tests_authorized') or os.name!='nt':return None
    value=os.environ.get('FARMAR_HOST_WATCHDOG_LEASE')
    if not value:return None
    path=Path(value).resolve(strict=True);base=(root/'.local/monitored-host-runtime').resolve()
    if base not in path.parents:raise RuntimeError('Watchdog lease is outside the owned monitored-run directory')
    lease=json.loads(path.read_text(encoding='utf-8'))
    if lease.get('schema')!=1 or lease.get('driver_pid')!=os.getpid() or not lease.get('job_kill_on_close'):
        raise RuntimeError('Watchdog lease does not identify this supervised test process')
    k=C.WinDLL('kernel32',use_last_error=True)
    k.GetTickCount64.restype=C.c_ulonglong
    if not k.GetTickCount64()<int(lease['deadline_tick'])<=k.GetTickCount64()+900000:
        raise RuntimeError('Watchdog lease is expired or exceeds its allowed duration')
    k.OpenProcess.argtypes=[W.DWORD,W.BOOL,W.DWORD];k.OpenProcess.restype=W.HANDLE
    k.OpenJobObjectW.argtypes=[W.DWORD,W.BOOL,W.LPCWSTR];k.OpenJobObjectW.restype=W.HANDLE
    k.GetCurrentProcess.restype=W.HANDLE
    k.IsProcessInJob.argtypes=[W.HANDLE,W.HANDLE,C.POINTER(W.BOOL)]
    k.GetExitCodeProcess.argtypes=[W.HANDLE,C.POINTER(W.DWORD)]
    k.QueryFullProcessImageNameW.argtypes=[W.HANDLE,W.DWORD,W.LPWSTR,C.POINTER(W.DWORD)]
    k.CloseHandle.argtypes=[W.HANDLE]
    guardian=k.OpenProcess(0x1000,False,int(lease['guardian_pid']));job=None
    try:
        if not guardian:raise RuntimeError('Independent watchdog is not accessible')
        code=W.DWORD();name=C.create_unicode_buffer(32768);length=W.DWORD(len(name))
        if not k.GetExitCodeProcess(guardian,C.byref(code)) or code.value!=259:
            raise RuntimeError('Independent watchdog is no longer running')
        if not k.QueryFullProcessImageNameW(guardian,0,name,C.byref(length)):
            raise RuntimeError('Cannot verify independent watchdog executable')
        expected=root/'build/host-test-watchdog/Release/host-test-watchdog.exe'
        if Path(name.value).resolve()!=expected.resolve():raise RuntimeError('Unexpected watchdog executable')
        if not lease['job_name'].startswith('Local\\FarmarHostTest-'):
            raise RuntimeError('Unexpected monitored Job name')
        job=k.OpenJobObjectW(4,False,lease['job_name']);member=W.BOOL()
        if not job or not k.IsProcessInJob(k.GetCurrentProcess(),job,C.byref(member)) or not member.value:
            raise RuntimeError('This process is not in the live watchdog-owned Job')
    finally:
        if job:k.CloseHandle(job)
        if guardian:k.CloseHandle(guardian)
    return dict(directory=path.parent,lease=lease)


def require_desktop_runtime_available(root=None):
    root=Path(root) if root is not None else Path(__file__).resolve().parents[1]
    path=root/'.local/desktop-runtime-hold.json'
    if not path.exists():return
    try:hold=json.loads(path.read_text(encoding='utf-8'))
    except (OSError,ValueError) as error:
        raise RuntimeError(f'Cannot verify desktop runtime incident hold: {path}') from error
    if hold.get('active',True):
        try:context=verified_watchdog_context(root,hold)
        except (OSError,ValueError,KeyError) as error:raise RuntimeError('Cannot verify monitored test lease') from error
        if context:return context
        raise RuntimeError('Interactive host game tests are held by the local runtime gate. '
            'Do not launch a game, capture the desktop, or inject input on this host. '
            f'See {path} and docs/RUNTIME_TESTING.md; an independently watched Job or an isolated guest is required.')
