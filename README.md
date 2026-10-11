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

Explicitní `recovered_game_audio` nyní propojuje SoundManager/cSound/cChannel a zvukové služby MainMenu/Options s vlastní zdrojovou Squall.dll. `game-audio-reference-test` ověřuje skutečný playback cursor, pauzu a 378 stavů v obou x87 přesnostech, také ze samostatného sestavení. Jde o propojený diagnostický scénář; celý herní EXE a audible/full-game porovnání ještě čekají. Postup a rozsah v [SOUND_MANAGER.md](docs/SOUND_MANAGER.md).

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

Obnovený ProgressBar ze skutečného `UI/Load.xml` je porovnaný s originálem: 200 000 transformací, 7 368 stavů a 96 Direct3D8 snímků s 46 080 000 shodnými RGB pixely. Včetně víčka, opakovaného XML a uvolnění referencí; rozsah a další práce jsou v [docs/PROGRESS_BAR.md](docs/PROGRESS_BAR.md). Přednačítání a vytvoření skutečného herního světa pokračují.

```powershell
python -m unittest discover -s tests -v
```

Porovnávání hry bude pokrývat grafiku a české texty, vstup, zvuk, tutoriál, pěstování, výrobu, trh, zakázky, odměny a obousměrnou kompatibilitu uložených her. Podrobnosti jsou v [docs/VALIDATION.md](docs/VALIDATION.md). Neověřené části nesmějí být označeny za shodné s originálem.

Nápověda ocenění `TooltipFrame` / cAwardTip je obnovená včetně čtyř podkladů a vlastněného textu: 7 320 stavů, 720 XML reloadů, 177 116 písmen a 96 snímků / 46 080 000 shodných RGB pixelů. [docs/AWARD_TOOLTIP.md](docs/AWARD_TOOLTIP.md) uvádí původní adresy a hranice. Navazující efekt a controller jsou nyní porovnané níže.

Animovaný `AwardEffect` obnovuje původní phase switch a všech 21 obrazových definic: 10 878 stavů, 10 080 kroků a 168 snímků / 80 640 000 shodných RGB pixelů. [docs/AWARD_EFFECT.md](docs/AWARD_EFFECT.md) popisuje přesnou aritmetiku, zvukové testovací rozhraní; propojená tlačítka/controller jsou popsané v AWARDS.md.

Awards a všech 21 AwardButton nyní propojují obnovované nápovědy, efekty, input a skutečné profilové booleans: 16 632 stavů, 574 992 písmen, 126 potvrzení podmínek ocenění a 156 GPU snímků / 74 880 000 shodných RGB pixelů. Podrobnosti, původní adresy, shodné retained reference a servisní hranice jsou v [docs/AWARDS.md](docs/AWARDS.md). Gameplay award manager/AwardPanel a MainMenu navigation pokračují; hlavní herní EXE ještě není sestavený.

AwardPanel nyní používá skutečné vlastněné texty a OK input, borrowed AwardEffect a Awards factory/confirmation/deletion: 882 stavů, 42 input callbacků a 168 GPU snímků / 80 640 000 shodných RGB pixelů. Původní adresy, Text0/Text zvláštnost XML a sound hranice jsou v [docs/AWARD_PANEL.md](docs/AWARD_PANEL.md). Gameplay condition detection a skutečný herní svět pokračují.

ResourceManager nyní zapisuje resource log podle původní metody 0053fdf0: 62 shodných izolovaných souborů, 8 952 položek a 501 782 UTF-16 code units v PC53/PC24. Snapshot před invalidací výstupního názvu, všechny typy a raw formát jsou popsány v [docs/RESOURCE_LOG.md](docs/RESOURCE_LOG.md). Start a skutečný herní svět pokračují.

MainMenu controller nyní propojuje skutečné Options/Credits/Awards/SelectProfile/ExitDialog a Loading ze Start inputu: 26 click akcí, 234 GPU snímků / 112 320 000 shodných RGB pixelů, 36 celých resource dávek a skutečné WM_QUIT callbacky. Obnovené jsou singleton/FixedBack lifecycle, Escape a profilové návraty; rozsah v [docs/MAIN_MENU.md](docs/MAIN_MENU.md). Konstruktor cGame a plná hra pokračují; diagnostic completion pouze počítá callbacky.

GameConfiguration: skutečné game.xml, 19 produktů/8 semen/15 × 4 cen/5 WString a UpgradeSound. Původní XML instrukce v PC53/PC24: 144 stavů, 96 reloadů, 6 624 appearance stavů a 8 640 cenových hodnot. Detaily v `docs/GAME_CONFIGURATION.md` (ze STATUS: `GAME_CONFIGURATION.md`) a `analysis/game-configuration-validation.json`. Úplný cGame, gameplay/tutoriál/svět a hlavní EXE ještě nejsou obnovené; pokračuje jejich konstrukce.

GameplayEventDefinitions obnovuje všech 383 událostí ze skutečného Gameplay.xml a všech 10 typů. PC53/PC24: 4 772 celých událostí, 1 588 kontraktů, 1 876 popularity záznamů; GameplayTime prošel 417 286 přímými porovnáními původních x87 instrukcí. Adresy, servisní hranice a neověřené oblasti jsou v [docs/GAMEPLAY_EVENTS.md](docs/GAMEPLAY_EVENTS.md). Runtime vykonávání, randomizace a skutečný svět pokračují; celý projekt není dokončen.

Gameplay nyní obnovuje konstruktor, singleton, scheduling, x87 randomizaci a save koeficienty. PC53/PC24: 70 368 stavů, 10 104 kroků, 2 690 706 event stavů a 600 000 RNG porovnání. Povinné světové rozhraní nemá výchozí implementaci; actual dispatch a celý cGame pokračují. Rozsah a hranice jsou v [docs/GAMEPLAY_RUNTIME.md](docs/GAMEPLAY_RUNTIME.md).

Upgrade obnovuje společnou logiku původního abstraktního vylepšení: signed dostupnost/ceny, horní clamp, kumulativní XML reload a sdílený upgrade zvuk. PC53/PC24: 131 296 stavů, 65 536 setterů a 245 760 cenových hodnot. Odvozené herní objekty a skutečný nákup pokračují; rozsah v [docs/UPGRADE.md](docs/UPGRADE.md).

Obě specializace rozšířených tlačítek propojují původní nápovědy farmy, budov a menu s vlastněnými texty, obrázky, vstupem a animacemi. PC53/PC24: 155 448 stavů, 154 656 kroků, 288 GPU snímků / 138 240 000 shodných RGB pixelů. Vyžadují napojení na tutoriál; cGame a controllery světa pokračují. Rozsah v [docs/EXTENDED_BUTTONS.md](docs/EXTENDED_BUTTONS.md).
