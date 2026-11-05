"""Bounded read-only gameplay coefficients and contract dialog visibility."""


class LiveGameplayState:
    def __init__(self, observer, root, layout):
        self.observer = observer
        self.layout = layout
        if observer.kind == 'native':
            self.gameplay_global = 0x65f75c
            # Original VC6 vector object+28 has allocator word then begin/end.
            self.coefficient_begin = 0x2c
            self.dialog_globals = dict(provide=0x65f760,confirm=0x65f76c)
            self.view_flags = 0x24
        else:
            rows = (root/'build/windows/Release/DobryFarmar.map').read_text(encoding='latin1').splitlines()

            def symbol(part):
                found = [row.split() for row in rows if part in row]
                assert len(found) == 1,(part,len(found))
                return int(found[0][2],16)

            tutorial = symbol('3PAVTutorialManager@')
            self.gameplay_global = observer.singleton+symbol('?currentGameplay@')-tutorial
            self.coefficient_begin = layout['coefficient_vector']
            assert layout['vector_bytes'] == 12 and layout['gameplay_size'] == 64
            self.dialog_globals = {name:observer.singleton+symbol(part)-tutorial
                for name,part in [('provide','?currentProvide@'),('confirm','?confirmInstance@')]}
            self.view_flags = observer.offsets['control_state']+12

    def coefficients(self):
        read = self.observer.read
        gameplay = read(self.gameplay_global,'<I')[0]
        assert gameplay
        begin,end = read(gameplay+self.coefficient_begin,'<II')
        assert begin and end-begin == 383*4,(hex(gameplay),hex(begin),hex(end))
        return read(begin,'<383I')

    def dialog(self, name):
        read = self.observer.read
        address = read(self.dialog_globals[name],'<I')[0]
        assert address
        flags = read(address+self.view_flags,'<6B')
        return dict(address=address,shown=bool(flags[0] and flags[1]),flags=flags,
                    transition=read(address+self.observer.offsets['control_state'],'<ffI'))
