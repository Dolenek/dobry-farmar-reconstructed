"""Bounded read-only retry when suspension catches an in-progress tree rotation."""
import time
from pathlib import Path


def read_stable_scene(observer, hwnd, retry_log):
    for attempt in range(6):
        try:
            return observer.visible_scene(hwnd)
        except AssertionError as error:
            trace=error.__traceback__
            while trace.tb_next is not None:trace=trace.tb_next
            frame=trace.tb_frame
            values=frame.f_locals
            if (Path(frame.f_code.co_filename).name!='windows_game_memory.py'
                    or frame.f_code.co_name!='walk'
                    or 'node' not in values or 'visited' not in values
                    or values['node'] not in values['visited']):
                raise
            retry_log.append(dict(kind=observer.kind,pid=observer.pid,attempt=attempt+1,
                repeated_node=values['node'],visited_nodes=len(values['visited']),
                game_memory_written=False,scope='Rejected inconsistent diagnostic snapshot; resume before retry'))
            if attempt==5:raise
            time.sleep(.02)
