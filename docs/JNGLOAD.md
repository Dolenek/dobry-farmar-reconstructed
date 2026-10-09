# Rekonstrukce aplikační části JNGLoad

Vstupní hash a exportní ABI jsou v `analysis/inventory.json`. První export Ghidry nalezl 781 funkcí: 776 dekompilací, čtyři přesměrování a jednu chybu. Velká část DLL je staticky přilinkovaná LIBMNG, JPEG, zlib a kompilátorový runtime.

| Původní adresa | Obnovená funkce / význam |
| --- | --- |
| `10001010` | Callback čtení z paměti; kopíruje požadovaný počet bajtů a posouvá offset. |
| `10001060` | Ukazatel na řádek výstupní bitmapy. |
| `10001080` | Callbacky timer/refresh vracející true. |
| `10001090` | Přesměrování na Windows GetTickCount. |
| `100010a0` | Alokace nulované paměti pro dekodér. |
| `100010b0` | Uvolnění paměti pro dekodér. |
| `100010c0` | Veřejná funkce readMNG. |
| `100012c0` | Veřejná funkce freeData; ponechává hodnotu ukazatele. |

Callbacky na některých těchto adresách nejsou v prvním automatickém exportu rozpoznány jako samostatné funkce; jejich těla byla zkontrolována i nezávislým disassemblerem. Opakované nastavení getcanvasline, původní chybové texty, globální nereentrantní stav a ignorované návratové hodnoty display/cleanup jsou zachované.

LIBMNG používá canvas BGRA8, číselná hodnota `0x1001`. API bylo ověřeno proti [zdrojovému hlavičkovému souboru LIBMNG](https://github.com/LuaDist/libmng/blob/a629b36ff49fbf35626c040410b6d43aa32b1b0a/libmng.h). Samotná verze LIBMNG přilinkovaná v původní DLL dosud není určena.

## Co je a není hotové

`src/jngload` obsahuje rekonstruovaný aplikační wrapper a používá zdrojově sestavenou LIBMNG. Windows Win32 DLL byla sestavena a porovnána s originálem: všechny pixely 416 herních JNG souhlasí, stejně jako exportní názvy, ordinály a architektura. Vnitřní původní verze LIBMNG, ostatní vstupy a exception ABI zatím nejsou plně ověřené. Linuxové sestavení zůstává diagnostikou.

Používáme zdrojově sestavenou LIBMNG 1.0.10, připnutou na commit v `analysis/libmng-source.lock.json`. Závislost se stáhne do ignorovaného `.local/libmng-source`, včetně vlastní licence. Při stažení i při konfiguraci CMake se ověřuje všech 45 souborů proti Git blob hashům.

Nutné nastavení je `MNG_SUPPORT_FULL`, `MNG_NO_OPEN_CLOSE_STREAM`, `MNG_INTERNAL_MEMMNGMT`. Původní wrapper nemá callbacky otevírání/zavírání streamu; běžná binární LIBMNG 2.0.3 s jiným nastavením kvůli tomu při zkoušce neprošla `mng_read`.

Řetězce původní DLL identifikují JPEG `6b  27-Mar-1998` a deflate/inflate `1.2.3`. Tyto knihovny se sestavují z historických [zdrojů IJG](https://www.ijg.org/files/jpegsrc.v6b.tar.gz) a [zdrojů zlib](https://zlib.net/fossils/zlib-1.2.3.tar.gz), připnutých pomocí SHA-256 archivů i jednotlivých zdrojů v `analysis/codecs.lock.json`. Výchozí sestavení nepotřebuje systémové zlib/libjpeg. Původní verze LIBMNG a přesná konfigurace závislostí zatím nejsou potvrzené.

## Sestavení

Python 3.10+, curl, CMake 3.20+ a kompilátor C/C++17:

```powershell
python tools/fetch_libmng.py
python tools/fetch_codecs.py
```

Windows: MSVC s C++ workloadem a Windows SDK, cílová architektura Win32. Všechny tři obrazové závislosti se sestaví automaticky ze stažených zdrojů:

```powershell
cmake -S . -B build/windows -G "Visual Studio 17 2022" -A Win32
cmake --build build/windows --config Release
ctest --test-dir build/windows -C Release --output-on-failure
```

Výsledky v `build/windows/Release` zahrnují `JNGLoad.dll`, `jng-decode.exe` a `jng-reference-decode.exe`. Původní zdobení symbolů a ordinály definuje `src/jngload/JNGLoad.def`. Sestavení bylo ověřené nativně na Windows s MSVC 19.43.34810 a SDK 10.0.26100.0. Wrapper deklaruje pouze potřebné Kernel32 API GetTickCount; tím se vyhne kolizi typedef INT32 mezi JPEG 6b a moderními Windows hlavičkami, bez změn připnutých závislostí.

Linux s gcc/g++ nebo jiným kompilátorem C/C++17:

```bash
cmake -S . -B build/linux -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux -j 4
ctest --test-dir build/linux --output-on-failure
python3 tools/validate_jng.py --decoder build/linux/jng-decode
```

Pro Linux je připraven i `python3 tools/build_jng_diagnostic.py`, který sestavuje všechny tři závislosti a diagnostiku přímo přes gcc/g++ bez CMake. Výstup je `build/jng-decode`. Ověřované CMake sestavení s původními kodeky je lokálně v `build/original-codecs`. Dřívější pokus se systémovými zlib/libjpeg je jen pracovní diagnostika v `build/cmake-diagnostic`.

## Dosavadní výsledky

- Dekódováno všech 416 JNG z `Data.pack` a `cz.pack`; rozměry každého souboru souhlasí s nezávisle přečteným JHDR a výstup má správný počet bajtů.
- Syntetické PNG ověřují známé červené, zelené a modré pixely v BGRA a rozměry 1×1, 3×2 a 7×5.
- Nativní porovnání s původní DLL: 416/416 obrázků má stejné rozměry a všechny BGRA bajty, bez tolerance. Dokládá shodu barev, JPEG výstupu i alfa kanálu pro tyto konkrétní herní obrázky.
- `analysis/jngload-windows-abi.json` dokládá PE32/i386 a přesně dva původní exporty v původním pořadí ordinálů. Přeposílané exporty nejsou použité.
- Neověřené zůstávají obecné MNG/animace, chybové vstupy, opakovaná volání v jednom procesu a kompatibilita C++ výjimek s původním runtime. Tyto výsledky samy neznamenají dokončenou rekonstrukci celé DLL pro všechny vstupy.

## Porovnání s původní DLL na Windows

Po sestavení spustit z kořene repozitáře:

```powershell
python tools/validate_jng.py --decoder build/windows/Release/jng-decode.exe --reference-decoder build/windows/Release/jng-reference-decode.exe --reference-dll "../farmář/Dobrý farmář/JNGLoad.dll" --output .local/jng-validation-windows.json
python tools/validate_exports.py --binary build/windows/Release/JNGLoad.dll --original-name JNGLoad.dll --output analysis/jngload-windows-abi.json
```

Referenční nástroj dynamicky načte původní DLL a její dva exporty. Nástroj kontroluje hash původní DLL, porovná rozměry i každý bajt BGRA všech obrázků a zaznamená výsledek do `.local/jng-validation-windows.json`. Každý dekodér běží v samostatném procesu; původní DLL není součástí obnoveného sestavení. Jakákoli chyba nebo odchylka skončí nenulovým návratovým kódem. Přenositelný souhrn posledního ověření je v `analysis/jngload-windows-validation.json`.
