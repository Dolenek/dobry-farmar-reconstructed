"""Optimistic, bounded scene-tree read; never suspends or modifies the target."""
import struct
import time


class SceneSnapshotChanged(RuntimeError):
    pass


def read_scene_tree(read,head,nil_offset,value_offset,decode_control,deadline):
    """Reject a changing topology instead of stopping its rendering thread.

    Validation detects observed changes, not every possible concurrent ABA
    mutation. The result is a best-effort observation, never an atomic frame.
    """
    size=max(12,nil_offset+1,value_offset+4);seen={};controls=[]
    def bounded(address):
        if time.monotonic()>deadline:raise SceneSnapshotChanged('Scene read exceeded its time bound')
        if address<65536:raise SceneSnapshotChanged('Scene link is not a readable object address')
        try:return read(address,f'<{size}s')[0]
        except OSError as error:raise SceneSnapshotChanged('Scene node disappeared during observation') from error
    initial_head=bounded(head);root=struct.unpack_from('<I',initial_head,4)[0]
    stack=[(root,False)];scheduled=set()
    while stack:
        address,visit=stack.pop()
        if visit:
            raw=seen[address];control=struct.unpack_from('<I',raw,value_offset)[0]
            try:controls.append(decode_control(control))
            except OSError as error:raise SceneSnapshotChanged('Scene control disappeared during observation') from error
            continue
        raw=bounded(address);nil=raw[nil_offset]
        if nil==1:continue
        if nil!=0 or address in scheduled or len(scheduled)>=10000:
            raise SceneSnapshotChanged('Scene topology changed or exceeded its bounded size')
        scheduled.add(address);seen[address]=raw
        left,_,right=struct.unpack_from('<III',raw)
        stack.extend([(right,False),(address,True),(left,False)])
    if bounded(head)!=initial_head:raise SceneSnapshotChanged('Scene sentinel changed during observation')
    for address,raw in seen.items():
        if bounded(address)!=raw:raise SceneSnapshotChanged('Scene links changed during observation')
    return controls
