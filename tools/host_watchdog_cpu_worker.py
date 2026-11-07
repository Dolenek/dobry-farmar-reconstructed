"""CPU-only watchdog fixture; intentionally hangs only its own disposable Job."""
import json
import os
from pathlib import Path
import subprocess
import sys
import time
from desktop_runtime_hold import require_desktop_runtime_available

context=require_desktop_runtime_available();assert context
directory=context['directory'];mode=sys.argv[1]
def progress():
    path=directory/'progress.json';temporary=path.with_suffix('.tmp')
    temporary.write_text(json.dumps(dict(pid=os.getpid(),mode=mode,time=time.monotonic())),encoding='utf-8');temporary.replace(path)
progress()
if mode in ['stall','guardian_exit']:
    child=subprocess.Popen([sys.executable,'-c','import time; time.sleep(60)'])
    (directory/'fixture-child.json').write_text(json.dumps(dict(pid=child.pid)),encoding='utf-8')
    time.sleep(60)
elif mode=='heartbeat':
    while True:progress();time.sleep(.1)
elif mode=='healthy':
    for _ in range(5):progress();time.sleep(.1)
else:raise ValueError(mode)
