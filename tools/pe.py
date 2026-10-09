"""Read PE32/PE32+ metadata without loading or executing the input file."""
from __future__ import annotations

import struct
from pathlib import Path


class PEError(ValueError):
    pass


class PE:
    def __init__(self, path: Path):
        self.data = path.read_bytes()
        if self.data[:2] != b"MZ":
            raise PEError("Missing DOS signature")
        pe = self.u32(0x3C)
        if self.data[pe:pe + 4] != b"PE\0\0":
            raise PEError("Missing PE signature")
        self.machine, section_count = self.unpack("<HH", pe + 4)
        self.timestamp = self.u32(pe + 8)
        optional_size, self.characteristics = self.unpack("<HH", pe + 20)
        optional = pe + 24
        magic = self.u16(optional)
        if magic not in (0x10B, 0x20B):
            raise PEError("Unsupported optional header")
        self.bits = 32 if magic == 0x10B else 64
        self.entry_rva = self.u32(optional + 16)
        self.image_base = self.unpack("<I" if self.bits == 32 else "<Q",
                                      optional + (28 if self.bits == 32 else 24))[0]
        self.header_size = self.u32(optional + 60)
        self.linker_version = list(self.unpack("<BB", optional + 2))
        count_offset = 92 if self.bits == 32 else 108
        directory_offset = 96 if self.bits == 32 else 112
        count = min(self.u32(optional + count_offset), 16)
        if directory_offset + count * 8 > optional_size:
            raise PEError("Truncated optional header directories")
        self.directories = [self.unpack("<II", optional + directory_offset + i * 8)
                            for i in range(count)]
        self.sections = []
        for i in range(section_count):
            off = optional + optional_size + i * 40
            self.check(off, 40)
            name = self.data[off:off + 8].split(b"\0", 1)[0].decode("ascii", "replace")
            virtual_size, rva, raw_size, raw_offset = self.unpack("<IIII", off + 8)
            self.sections.append(dict(name=name, rva=rva, virtual_size=virtual_size,
                                      raw_size=raw_size, raw_offset=raw_offset))

    def check(self, off, size):
        if off < 0 or size < 0 or off + size > len(self.data):
            raise PEError(f"Read outside file: offset={off}, size={size}")

    def unpack(self, fmt, off):
        self.check(off, struct.calcsize(fmt))
        return struct.unpack_from(fmt, self.data, off)

    def u16(self, off):
        return self.unpack("<H", off)[0]

    def u32(self, off):
        return self.unpack("<I", off)[0]

    def offset(self, rva, size=1):
        if rva < self.header_size:
            self.check(rva, size)
            return rva
        for s in self.sections:
            delta = rva - s["rva"]
            if 0 <= delta < max(s["virtual_size"], s["raw_size"]):
                if delta + size > s["raw_size"]:
                    raise PEError(f"RVA {rva:#x} has no file-backed bytes")
                off = s["raw_offset"] + delta
                self.check(off, size)
                return off
        raise PEError(f"Unmapped RVA {rva:#x}")

    def cstring(self, off, limit=8192):
        self.check(off, 1)
        end = self.data.find(b"\0", off, min(len(self.data), off + limit))
        if end < 0:
            raise PEError("Unterminated string")
        return self.data[off:end].decode("ascii", "replace")

    def directory(self, index):
        return self.directories[index] if index < len(self.directories) else (0, 0)

    def imports(self):
        rva, size = self.directory(1)
        if not rva:
            return []
        result = []
        for i in range(max(1, size // 20)):
            fields = self.unpack("<IIIII", self.offset(rva + i * 20, 20))
            if not any(fields):
                return result
            lookup, _, _, name_rva, iat = fields
            symbols = []
            stride = self.bits // 8
            ordinal_bit = 1 << (self.bits - 1)
            for j in range(65536):
                value = self.unpack("<I" if stride == 4 else "<Q",
                                    self.offset((lookup or iat) + j * stride, stride))[0]
                if value == 0:
                    break
                symbol = {"iat_rva": iat + j * stride}
                if value & ordinal_bit:
                    symbol["ordinal"] = value & 0xFFFF
                else:
                    off = self.offset(value, 3)
                    symbol.update(hint=self.u16(off), name=self.cstring(off + 2))
                symbols.append(symbol)
            else:
                raise PEError("Import table has no terminator")
            result.append(dict(library=self.cstring(self.offset(name_rva)), symbols=symbols))
        raise PEError("Import descriptors have no terminator")

    def exports(self):
        rva, size = self.directory(0)
        if not rva:
            return []
        fields = self.unpack("<IIHHIIIIIII", self.offset(rva, 40))
        base, count, named_count, funcs, names, ordinals = fields[5:]
        if max(count, named_count) > 65536:
            raise PEError("Unreasonable export count")
        by_index = {}
        for i in range(named_count):
            name_rva = self.u32(self.offset(names + i * 4, 4))
            index = self.u16(self.offset(ordinals + i * 2, 2))
            if index >= count:
                raise PEError("Export name ordinal outside table")
            by_index.setdefault(index, []).append(self.cstring(self.offset(name_rva)))
        result = []
        for i in range(count):
            address = self.u32(self.offset(funcs + i * 4, 4))
            if not address:
                continue
            item = dict(ordinal=base + i, names=by_index.get(i, []), rva=address,
                        address=f"{self.image_base + address:08x}")
            if rva <= address < rva + size:
                item["forwarder"] = self.cstring(self.offset(address))
            result.append(item)
        return result

    def debug_records(self):
        rva, size = self.directory(6)
        records = []
        if not rva:
            return records
        for i in range(size // 28):
            fields = self.unpack("<IIHHIIII", self.offset(rva + i * 28, 28))
            kind, length, _, off = fields[4:]
            self.check(off, length)
            if kind == 2 and length >= 24 and self.data[off:off + 4] == b"RSDS":
                records.append(dict(type="RSDS", guid_bytes=self.data[off + 4:off + 20].hex(),
                                    age=self.u32(off + 20), pdb=self.cstring(off + 24)))
        return records

    def metadata(self):
        return dict(bits=self.bits, machine=f"{self.machine:04x}", timestamp=self.timestamp,
                    image_base=f"{self.image_base:08x}",
                    entry_address=f"{self.image_base + self.entry_rva:08x}",
                    linker_version=self.linker_version, clr=bool(self.directory(14)[0]),
                    sections=self.sections, imports=self.imports(), exports=self.exports(),
                    debug=self.debug_records())
