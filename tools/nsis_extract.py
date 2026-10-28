#!/usr/bin/env python3
"""Decode the supplied NSIS 2.28 solid-LZMA uninstaller without executing it."""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path
from nsis_solid import load_archive

ROOT = Path(__file__).resolve().parents[1]
OPCODES = ("INVALID_OPCODE RET NOP ABORT QUIT CALL UPDATETEXT SLEEP BRINGTOFRONT CHDETAILSVIEW "
    "SETFILEATTRIBUTES CREATEDIR IFFILEEXISTS SETFLAG IFFLAG GETFLAG RENAME GETFULLPATHNAME "
    "SEARCHPATH GETTEMPFILENAME EXTRACTFILE DELETEFILE MESSAGEBOX RMDIR STRLEN ASSIGNVAR STRCMP "
    "READENVSTR INTCMP INTOP INTFMT PUSHPOP FINDWINDOW SENDMESSAGE ISWINDOW GETDLGITEM SETCTLCOLORS "
    "SETBRANDINGIMAGE CREATEFONT SHOWWINDOW SHELLEXEC EXECUTE GETFILETIME GETDLLVERSION REGISTERDLL "
    "CREATESHORTCUT COPYFILES REBOOT WRITEINI READINISTR DELREG WRITEREG READREGSTR REGENUM FCLOSE "
    "FOPEN FPUTS FGETS FSEEK FINDCLOSE FINDNEXT FINDFIRST WRITEUNINSTALLER SECTIONSET INSTTYPESET "
    "GETLABELADDR GETFUNCTIONADDR LOCKWINDOW").split()
VARIABLES = {**{i: f"${i}" for i in range(10)}, **{i + 10: f"$R{i}" for i in range(10)},
             20: "$CMDLINE", 21: "$INSTDIR", 22: "$OUTDIR", 23: "$EXEDIR", 24: "$LANGUAGE",
             25: "$TEMP", 26: "$PLUGINSDIR", 27: "$EXEPATH", 28: "$EXEFILE", 29: "$HWNDPARENT"}
SHELL = {(2, 23): "$SMPROGRAMS", (16, 25): "$DESKTOP", (36, 36): "$WINDIR"}


def extract(path: Path):
    archive=load_archive(path)
    data=archive.pe.data;start=archive.firstheader_offset;flags=archive.flags
    header=archive.header;header_size=len(header);decoded=archive.decoded
    blocks = [struct.unpack_from("<II", header, 4 + i * 8) for i in range(8)]
    names = ["pages", "sections", "entries", "strings", "languages", "colors", "background", "data"]
    for offset, _ in blocks[:6]:
        if offset > len(header):
            raise ValueError("NSIS block outside header")
    string_data = header[blocks[3][0]:blocks[4][0]]
    language = header[blocks[4][0]:blocks[5][0]]
    if len(language) < 10 or (len(language) - 10) % 4:
        raise ValueError("Unexpected NSIS language table")
    language_offsets = struct.unpack_from("<" + "i" * ((len(language) - 10) // 4), language, 10)

    def string(offset, depth=0):
        if depth > 12:
            raise ValueError("Recursive NSIS string")
        if offset < 0:
            slot = -offset - 1
            if slot >= len(language_offsets):
                raise ValueError("Language string index outside table")
            offset = language_offsets[slot]
        if not 0 <= offset < len(string_data):
            raise ValueError("String offset outside table")
        parts, plain = [], bytearray()

        def flush():
            if plain:
                parts.append(plain.decode("cp1250"))
                plain.clear()

        while string_data[offset]:
            code = string_data[offset]
            offset += 1
            if code == 252:
                plain.append(string_data[offset])
                offset += 1
            elif code > 252:
                flush()
                a, b = string_data[offset:offset + 2]
                offset += 2
                value = (a & 127) | ((b & 127) << 7)
                if code == 253:
                    parts.append(VARIABLES.get(value, f"$var{value}"))
                elif code == 254:
                    if a & 128:
                        key = string(a & 63, depth + 1)
                        parts.append("$PROGRAMFILES" if key == "ProgramFilesDir" else f"${{SHELL_REG:{key}}}")
                    else:
                        parts.append(SHELL.get((a, b), f"${{SHELL:{a},{b}}}"))
                else:
                    parts.append(string(-value - 1, depth + 1))
            else:
                plain.append(code)
            if offset >= len(string_data):
                raise ValueError("Unterminated NSIS string")
        flush()
        return "".join(parts)

    strings = []
    offset = 0
    while offset < len(string_data):
        start_offset = offset
        while offset < len(string_data) and string_data[offset]:
            code = string_data[offset]
            offset += 1 + (1 if code == 252 else 2 if code > 252 else 0)
        if offset >= len(string_data):
            raise ValueError("Unterminated string table")
        strings.append(dict(offset=start_offset, text=string(start_offset)))
        offset += 1
    entries = []
    offset, count = blocks[2]
    if offset + count * 28 > len(header):
        raise ValueError("Instruction table outside header")
    # Only these parameter positions are string-table references in this dialect.
    text_slots = {6: [0], 7: [0], 10: [0], 11: [0], 12: [0], 13: [1], 17: [0], 18: [1],
                  19: [1], 20: [1, 5], 21: [0], 22: [1], 23: [0], 24: [1], 25: [1],
                  26: [0, 1], 27: [1], 33: [1, 2, 3, 4], 35: [1, 2], 36: [0], 37: [0], 38: [1, 2, 3], 39: [0, 1],
                  40: [0, 1, 2], 41: [0], 44: [0, 1], 48: [0, 1, 2, 3],
                  49: [1, 2, 3], 50: [2], 52: [2, 3]}
    for i in range(count):
        opcode, *arguments = struct.unpack_from("<7i", header, offset + i * 28)
        if not 0 <= opcode < len(OPCODES):
            raise ValueError(f"Unsupported NSIS opcode {opcode}")
        texts = {str(slot): string(arguments[slot]) for slot in text_slots.get(opcode, [])}
        if opcode == 31 and arguments[1] == 0 and arguments[2] == 0:
            texts["0"] = string(arguments[0])
        entries.append(dict(instruction=i + 1, opcode=opcode, operation=OPCODES[opcode],
                            arguments=arguments, strings=texts))
    sections = []
    offset, count = blocks[1]
    for i in range(count):
        # Standard ANSI NSIS_MAX_STRLEN=1024; the supplied section size is 1048.
        name, install_types, section_flags, code, size, size_kb = struct.unpack_from("<6i", header, offset + i * 1048)
        sections.append(dict(name=string(name), flags=section_flags,
                             first_instruction=code + 1, instruction_count=size, size_kb=size_kb))
    # Exact default NSIS2.28 Win32 header/page layout (Source/exehead/fileform.h).
    pages=[]
    page_offset,page_count=blocks[0]
    if page_offset+page_count*64>len(header):raise ValueError('Page table outside header')
    for i in range(page_count):
        values=struct.unpack_from('<16i',header,page_offset+i*64)
        pages.append(dict(dialog_id=values[0],window_proc=values[1],pre_code=values[2],show_code=values[3],leave_code=values[4],flags=values[5],caption=string(values[6]),back=string(values[7]),next=string(values[8]),click_next=string(values[9]),cancel=string(values[10]),parameters=list(values[11:]),parameter_text={str(slot):string(values[11+slot]) for slot in ({4:[0,1],3:[1,2]}.get(values[1],[]))}))
    callback_names=['onInit','onInstSuccess','onInstFailed','onUserAbort','onGUIInit','onGUIEnd','onMouseOverSection','onVerifyInstDir','onSelChange','onRebootFailed']
    runtime_header=dict(flags=struct.unpack_from('<I',header)[0],callbacks={name:struct.unpack_from('<i',header,108+4*i)[0] for i,name in enumerate(callback_names)})
    return dict(input_sha256=hashlib.sha256(data).hexdigest(), dialect="NSIS 2.28 ANSI, default opcode order",
                firstheader_offset=start, flags=flags, header_size=header_size,
                solid_decoded_size=len(decoded), language_id=struct.unpack_from("<H", language)[0],
                blocks={name: dict(offset=o, count=n) for name, (o, n) in zip(names, blocks)},
                sections=sections, strings=strings, instructions=entries,pages=pages,runtime_header=runtime_header,
                status="decoded_not_runtime_verified")


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--input", type=Path, default=ROOT.parent / "farmář" / "Dobrý farmář" / "Uninstall.exe")
    p.add_argument("--output", type=Path, default=ROOT / "analysis" / "uninstall-program.json")
    args = p.parse_args()
    result = extract(args.input)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"Decoded {len(result['instructions'])} instructions, {len(result['strings'])} strings -> {args.output}")


if __name__ == "__main__":
    main()
