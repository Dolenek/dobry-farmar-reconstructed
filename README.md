# Dobrý farmář — rekonstrukce

Pracovní rekonstrukce původní 32bitové hry pro Windows do upravitelného a sestavitelného projektu. Cílem je stejné chování, vzhled, pravidla a kompatibilní uložené hry. Shoda strojového kódu není požadována: různé kompilátory mohou stejné chování přeložit odlišně.

**Projekt zatím neobsahuje kompletní rekonstruovanou hru ani sestavené herní EXE. Export Ghidry je analytický pseudokód, nikoliv hotový zdrojový projekt.** Průběžný stav a ověření jsou v [docs/STATUS.md](docs/STATUS.md).

## Původní programy a knihovny

| Soubor | Doložená úloha | Stav |
| --- | --- | --- |
| `Dobrý farmář.exe` | Hlavní hra; nativní C++ program, Direct3D 8, DirectInput 8, přibalené zvukové a obrazové knihovny. | Obnoveny načítací/resource cesta, Direct3D8 sprity, bitmapové fonty, lokalizace a hierarchie UI, cControl/cPicture a vlastněné TextLine/Letter s animovanou geometrií. Dále jsou obnovené Button/AnimationButton a vlastněné textové varianty, klikací JNG masky a DirectInput binding/polling cesta. Herní EXE a kompletní engine/systémy ještě nejsou obnovené. |
| `Uninstall.exe` | NSIS 2.28 odinstalátor. Maže instalaci, zástupce a vybrané klíče registru. | Dekódováno 137 instrukcí, obnoveno mazací jádro do NSIS skriptu; celé EXE se ještě nesestavuje. |
| `Squall.dll` | Zvuková knihovna; importuje DirectSound, WinMM a exportuje 97 pojmenovaných funkcí `SQUALL_*`. | Zdrojový kandidát má všech 97 exportních názvů/ordinálů. Native parametry, forward/inverse volume, celé PCM 45 zvuků přes skutečně načtenou DLL a 564 480 bajtů fyzických DirectSound bufferů jsou porovnané v obou přesnostech, také z čistého sestavení. Úplné API, playback a celý herní audio lifecycle čekají. Postup a hranice: [SQUALL.md](docs/SQUALL.md). |
| `JNGLoad.dll` | Dekódování obrazových dat pomocí LIBMNG; exportuje `readMNG` a `freeData`. | Sestavena MSVC Win32 DLL; všechny pixely 416 JNG a PE exportní kontrakt shodné s originálem. Další chybové/animované vstupy zatím neověřené. |

V dodaném adresáři byly nalezeny právě dvě EXE. `Registrator.ini` odkazuje na `D:\setup.exe`, který v podkladech chybí. Z tohoto odkazu nelze obnovit instalátor ani mu přisuzovat další chování.

Původní EXE obsahuje odkazy na soubory jako `Game/Farm/Farm.cpp`, `Game/Manufacture/Manufacture.cpp`, `Game/Tutorial/TutorialManager.cpp` a `graphics/Graphics.cpp`. Odkaz na `Farmer.pdb` je pouze historická cesta; samotné ladicí symboly nemáme.

## Uspořádání

- `src/`: ručně rekonstruovaný a sestavitelný kód. Přidávají se pouze skutečně obnovené části.
- `tools/`: inventarizace vstupů, Ghidra export a příprava lokálních dat.
- `analysis/`: kontrolní součty a ověřená metadata. `analysis/raw/` obsahuje lokální exporty pseudokódu a není v Gitu.
- `docs/`: stav, zjištění, reprodukce analýzy a scénáře ověření.
- `tests/`: ověřování nástrojů a následně rekonstruovaných částí.
- `work/`, `.local/`, `build/`, `dist/`: ignorované pracovní soubory, kopie databáze Ghidry, data a výsledky sestavení.

Původní adresář hry a původní Ghidra projekt leží mimo tento repozitář. Nástroje je používají pouze jako vstupy. Analýza zapisuje do vlastní kopie projektu.

Repozitář je samostatná složka `dobry-farmar-reconstructed` vedle `farmář` a `farmář - Ghidra`. Zdrojový projekt patří sem; Ghidra databáze slouží k analýze a není vhodným kořenem budoucího GitHub repozitáře.

## Inventarizace

Vyžaduje Python 3.10 nebo novější; bez dodatečných balíčků:

```powershell
python tools/inventory.py --original-dir "..\farmář\Dobrý farmář"
```

Výstup `analysis/inventory.json` obsahuje SHA-256 souborů, PE importy/exporty, sekce, ladicí odkazy a seznamy položek ZIP archivů. Neobsahuje samotnou grafiku, zvuky ani původní binární soubory. Při použití jiné edice uchovat původní referenční inventář a nový vygenerovat přes `--output` do pracovního adresáře.

`Data.pack` má 617 položek, `cz.pack` 21 položek, včetně adresářů. Obsahují také XML konfigurace rozhraní a herních systémů. Samotná přítomnost těchto konfigurací nenahrazuje rekonstrukci kódu, který je interpretuje.

## Analýza pomocí Ghidry

Vyžaduje Ghidru 12.1.4, JDK 21+ a Python. MCP není potřeba. Postup a příkazy jsou v [docs/ANALYSIS.md](docs/ANALYSIS.md).

Exportuje se pseudokód, seznam funkcí, graf volání, symboly, řetězce s odkazy a typy. Soubor `summary.json` rozlišuje dokončený a přerušený export; neříká, že byla dokončena rekonstrukce hry. Automaticky určené podpisy funkcí nejsou považovány za ověřené ABI.

## Sestavení a spuštění

Cílové prostředí je Windows x86, MSVC 2022 a CMake, se zachováním Direct3D 8. Sestavitelné cíle jsou `JNGLoad`, diagnostika `jng-decode`, `resource-inspect`, `pack-decode`, `recovered_game_utilities` a `recovered_squall_utilities`. Postup, závislosti a Windows porovnání jsou v [docs/JNGLOAD.md](docs/JNGLOAD.md) a [docs/RESOURCE_LOADING.md](docs/RESOURCE_LOADING.md). Před konfigurací herních utilit je nutné také `python tools/fetch_minizip.py` a `python tools/fetch_expat.py`. Cíl hlavní hry ještě není k dispozici. Obnovené herní metody popisuje [docs/GAME.md](docs/GAME.md), zvuk [docs/SQUALL.md](docs/SQUALL.md) a odinstalátor [docs/UNINSTALL.md](docs/UNINSTALL.md).

Příprava **dat bez hry** do samostatné lokální složky:

```powershell
python tools/prepare_runtime.py --original-dir "..\farmář\Dobrý farmář"
```

Po dokončení všech rekonstruovaných binárních souborů lze předat `--build-dir`. Nástroj kontroluje referenční hashe dat, požaduje nový nebo prázdný výstupní adresář a odmítne použít nezměněné původní EXE/DLL jako údajný výsledek rekonstrukce. Původní uložené hry nepřepisuje.

## GitHub a vydání

Repozitář obsahuje obnovované zdroje, dokumentaci a nástroje. Původní EXE, DLL, datové balíky, média, profily, databáze Ghidry a pracovní výstupy se nepublikují. Vydání budou obsahovat námi sestavené EXE a DLL; pro spuštění budou potřebovat původní herní data dodaná lokálně.

Projekt zatím nebyl publikován na GitHub. Automaticky mu nebyla přidělena licence k původnímu kódu ani médiím.

Další aktuálně sestavitelné části: datový správce nastavení, Windows okno/message pump, cResource/cTexture a Direct3D8 JNG textury. Postup a omezení jsou v [docs/SETTINGS.md](docs/SETTINGS.md) a [docs/WINDOW_GRAPHICS.md](docs/WINDOW_GRAPHICS.md). Pro jejich sestavení doplnit `python tools/fetch_direct3d_headers.py`; pro okenní diagnostiku také `python tools/extract_game_icon.py`. Ikona je původní výtvarné médium pouze v ignorovaném lokálním adresáři, ne kopie herního kódu.

## Ověření

```powershell
python -m unittest discover -s tests -v
```

Porovnávání hry bude pokrývat grafiku a české texty, vstup, zvuk, tutoriál, pěstování, výrobu, trh, zakázky, odměny a obousměrnou kompatibilitu uložených her. Podrobnosti jsou v [docs/VALIDATION.md](docs/VALIDATION.md). Neověřené části nesmějí být označeny za shodné s originálem.
