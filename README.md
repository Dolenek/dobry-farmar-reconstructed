# Dobrý farmář — rekonstrukce

[Zkratky cLogic a grafický reset](docs/LOGIC_SHORTCUTS.md) jsou nově zapojené do hlavního EXE:3661 nativně porovnaných stavů a skutečné přepnutí okno/fullscreen. Dva resetové snímky mají2553600 přesně shodných RGB pixelů; původní texture-suppression zvláštnost i DPI hranice jsou evidované. Úplný runtime a ostatní podmínky projektu pokračují.

[Porovnání celých procesů a saves](docs/SAVE_INTEROP.md) nyní ověřuje úvodní profily v obou směrech a připravený bohatší stav se sáčkem, prodejem, zprávami a383 koeficienty. Původní EXE běží pouze jako reference s izolovanou systémovou datovou složkou; produkce používá vlastní zdroje. Rozsah a neověřené stavy jsou výslovně evidované.

Hlavní [zdrojové herní EXE](docs/APP_RUNTIME.md) se nyní sestaví a spustí s původními daty: nový profil → menu → skutečná farma, normální ukončení, opětovné načtení save, pauza, nápověda i deník. Obě načtené herní DLL pocházejí z našeho sestavení. [Startup](docs/STARTUP.md) navíc přímo souhlasí s originálem ve čtyřech cyklech / 786 snímcích / 377 280 000 RGB pixelech (PC53/PC24). Úplný nativní runtime, obousměrné saves, všechny systémy, úplná Squall a izolovaný odinstalátor stále pokračují.

[Původní JPEG loga](docs/JPEG.md) mají obnovený reader a MMX barevnou aritmetiku: všechna3 loga souhlasí v každém dekódovaném bajtu i celé GPU textuře, PC53/PC24; vyčerpávající porovnání16777216 barevných trojic také prošlo. Celé spuštění a hlavní herní EXE dál pokračují.

[První profil NewUser](docs/NEW_USER.md) je obnovený a porovnaný s přechodem do skutečného menu: PC53/PC24, čtyři životní cykly, 88 stavů a 56 snímků / 26 880 000 shodných RGB pixelů. Zdrojové profily se skutečně ukládají; nativní Profile služba má výslovný testovací adaptér. Celé spuštění a herní EXE pokračují.

[Skutečný cGame a binární save/load](docs/GAME_CONTROLLER.md) už propojuje všechny hlavní obnovené objekty. Zdrojové porovnání save/restore/save a skutečné mouse/tutoriálové cykly ověřuje nový integrační běh v PC53/PC24; nativní úplný svět, obousměrné saves mezi programy a úplné porovnání hlavního interaktivního EXE dál čekají.

[Řídicí objekt herních ocenění](docs/GAME_AWARD_MANAGER.md) propojuje automatické vyhodnocení se skutečným efektem, obrazovkou, panelem a návratem. PC53/PC24: 18 podmínek, 36 celých cyklů a 2012 snímků / 965 760 000 shodných RGB pixelů. Tři podmínky nad úplným světem a celý cGame/EXE zůstávají neověřené; source cleanup Help/Journal při chybě konstrukce prošel osmi kontrolami.

[Skutečný Journal](docs/JOURNAL.md) už otevírá a zavírá skutečný HUD, rozvrhuje zprávy a označuje je za přečtené. PC53/PC24: 430 stavů / 102 operací a 362 celých snímků / 173 760 000 shodných RGB pixelů, včetně obou směrů, indexových hranic a XML reloadu. Obnovené jsou i potřebné textové copy/measure metody; celý cGame/EXE dále pokračuje.

[Skutečná nápověda](docs/HELP_VIEW.md) je propojená s pauzovacím menu: otevření, stránky, návrat i XML reload probíhají skutečnou myší. PC53/PC24: 30 stavů / 24 operací a 376 celých snímků / 180 480 000 shodných RGB pixelů včetně ostatních pauzovacích obrazovek. Pokračují Journal, award manager a celý cGame/EXE.

[Skutečný Farm controller](docs/FARM_SCENE.md) propojuje šedesát polí, dopisy, rozšíření a profilovou obnovu zavlažovačů. PC53/PC24: 188 stavů, 132 CPU operací a 64 celých snímků / 30 720 000 shodných RGB pixelů, včetně skutečného přijetí zakázky a odstranění dopisu. Celý cGame, pracovní vstup nad touto farmou a herní EXE pokračují; přesné hranice jsou v dokumentaci.


[ContractLetterSlot](docs/CONTRACT_LETTER.md) obnovuje příchozí dopisy Farm: přílet, odpočet, blikání, profilovou obnovu a skutečný tutoriál17 → mouse → přečtení → ContractProvide. PC53/PC24:3404 stavů,548 přiřazení,2660 CPU operací a160 celých snímků /76 800 000 shodných RGB pixelů. Úplný Farm/cGame a herní EXE pokračují.


[Skutečné seed/hose cykly](docs/PERSON_FLOWS.md): dalších426 stavů /144 výslovných CPU operací a272 celých snímků /130 560 000 shodných RGB pixelů v PC53/PC24. Vstup sám spustí sebrání sáčku, setí, odložení při převzetí rýče i zalévání hadicí včetně živých částic. Opravený je cíl hadice podle instrukcí a vlastní původní prázdné mouse callbacky postavy. Celý Farm/cGame/EXE a ostatní podmínky cíle pokračují.

Pracovní rekonstrukce původní 32bitové hry pro Windows do upravitelného a sestavitelného projektu. Cílem je stejné chování, vzhled, pravidla a kompatibilní uložené hry. Shoda strojového kódu není požadována: různé kompilátory mohou stejné chování přeložit odlišně.

**Projekt má sestavitelné hlavní zdrojové EXE; úplná rekonstrukce a všechna požadovaná porovnání ještě nejsou dokončené. Export Ghidry zůstává analytickým pseudokódem.** Průběžný stav a ověření jsou v [docs/STATUS.md](docs/STATUS.md).

[Person controller](docs/PERSON_CONTROLLER.md) nově propojuje celou postavu a její ActionProcessor se skutečným polem: kliknutí vytvoří úkol, postava dojede, vykoná kopání a sama přejde do čekání. PC53/PC24:462 stavů /284 výslovných CPU operací,128 GPU snímků /61 440 000 shodných RGB pixelů. Sedm pracovních větví je porovnaných se skutečným polem, nástroji a skladem. Seed/hose nově ověřuje navazující porovnání; zbývající arrival/tutorial větve mají výslovně neověřený stav; celý Farm/EXE a ostatní podmínky cíle pokračují.

[ElasticAnimation](docs/ELASTIC_ANIMATION.md) nyní propojuje skutečné upgrady všech tří nástrojových budov s pohybem, zvukovým požadavkem a finálním vzhledem budovy. PC53/PC24:1 524 stavů /1 354 akcí,168 GPU snímků /80 640 000 přesně shodných RGB pixelů. Skutečné kliknutí myší na upgrade sekačky spustí celý porovnaný let. Úplná postava a její pracovní akce, Farm/EXE a ostatní podmínky celého cíle nadále zbývají.

Obnovená [výrobní scéna Manufacture](docs/MANUFACTURE.md) již propojuje skutečné budovy, nákupní panel, HUD, výběr, umístění, přesun a demolici. Přímé porovnání v obou přesnostech: 3 210 stavů a 170 GPU snímků / 81 600 000 shodných pixelů; celý Win32 projekt prošel 67 CTest kontrolami. Navazující [FieldObject](docs/FIELD_OBJECT.md) a [ActionArea](docs/ACTION_AREAS.md) obnovují základ akčních oblastí farmy včetně výškového offsetu, klikacích cílů, parkovacích bodů a původní přesnosti. ActionArea: dalších 17 842 stavů a 84 GPU snímků / 40 320 000 shodných pixelů; skutečné Farm/Person služby zbývají.

Skutečný [Warehouse](docs/WAREHOUSE.md) nyní propojuje zásoby profilu, výrobní budovu, popisky Manufacture a HUD; výrobní callbacky odebírají/vracejí vstup a absoluteStep uloží vyrobený výstup do stejného skladu. PC53/PC24: 1 608 stavů, 1 024 transferů a 48 snímků / 23 040 000 shodných pixelů. Navazující [WareDetail tooltip](docs/WAREHOUSE_TIP.md) už přijímá skutečné notifikace Warehouse a ovládání HUD: dalších 2 100 stavů a 40 snímků / 19 200 000 shodných pixelů. Celá regrese prošla 68 CTest kontrolami. Native BGRA masky jsou porovnané přes původní D3DX loader. Obchod, Farm/Person, další systémy, celý cGame a herní EXE pokračují.

[Pole a jejich akce](docs/FARM_FIELDS.md) nyní propojují růst osmi kultur, typed profil, skutečné Farm prostorové dotazy a zavlažování přes Sprinkle/WaterSource s vodními efekty. PC53/PC24: 4 460 stavů a 128 GPU snímků / 61 440 000 shodných pixelů, včetně myši a původních tutorial21/22/23 větví. Úplný Farm controller, FieldTip/SeedPackage, Person, herní EXE a obousměrné saves ještě zbývají.

[SeedPackage a ProgressDiscrete](docs/SEED_PACKAGE.md) nyní propojují skutečný BuySeeds callback, čtyřpoužitelný sáček, platbu při umístění, myš, Farm masky a odstranění odmítnutého nákupu. Skutečné pole spouští tutorial22 s existujícím sáčkem. PC53/PC24:550 SeedPackage stavů /504 akcí a1038 ProgressDiscrete stavů,64 GPU snímků /30 720 000 shodných RGB pixelů. Úplné sestavení a81/81 CTest prošlo za338,02 s, obě game-audio sestavení a striktní source/binary hashe také; všech17 originálů zůstalo beze změny. V tomto SeedPackage režimu je Person queue pozorovaná hranice; navazující PersonTasks ji už vykonává skutečně; tutorial20 a neurčený původní počáteční status na recyklované haldě mají výslovně evidované omezení. Celý Farm/Person, herní EXE, obousměrné saves, úplná Squall a izolovaný uninstaller nadále zbývají.

[PersonTasks](docs/PERSON_TASKS.md) nyní obnovuje skutečnou frontu postavy a značky napojené na myší vstup Field/SeedPackage:102 stavů /84 akcí a16 dalších GPU snímků /7 680 000 shodných RGB pixelů v PC53/PC24. Dokončení přesouvá značky, zachovává uložené cíle a používá původní deferred discard; FIFO, vyřazení i unsigned hraniční indexy jsou přímo porovnané. Úplné Win32 sestavení a82/82 CTest prošlo za341,53 s, obě game-audio sestavení a striktní hashe také; všech17 originálů je beze změny. Nativní Person je v tomto porovnání explicitní adaptér příslušných dat. Celý jeho constructor, pohyb, carrying/work, animace/zvuk a úplná Farm/EXE dále zbývají.

[PersonSounds](docs/PERSON_SOUNDS.md) obnovuje výběr a vlastnictví zvukových kanálů postavy přes skutečný SoundManager/cChannel a fyzické DLL. PC53/PC24 na vlastní i původní knihovně:332 stavů /328 akcí /616 API událostí,40 kontrol postupu smyček a8 kontrol paused pozice; samostatné sestavení prošlo stejným scénářem. Původní sound metody běží jen jako reference na explicitních datech; actual replay/stop pořadí, resource reference, cached hose resume i null sloty jsou porovnané. Úplné sestavení a86/86 CTest prošlo za386.41 s, oba game-audio audity a striktní source/binary hashe také; všech17 originálů je beze změny. Plný actor/action/Farm/EXE, celé saves, zbývající Squall a izolovaný uninstaller pokračují; audible/refill kvalita tím není vykázaná jako hotová.

[PersonVehicleWalk](docs/PERSON_VEHICLE.md) obnovuje přesun s nástroji včetně přepnutí os a směru:8660 stavů /8532 kroků,128 GPU snímků /61 440 000 shodných RGB pixelů v PC53/PC24. Kliknutí na skutečné pole vytvoří PersonTasks úkol a vykreslená postava s Dig/Mow nástrojem dojde po původní trase; následuje actual marker cleanup. Porovnané jsou hranice2/10, direct/L trasa, clipped čas, profilová pozice, axis appearance, cancel a skutečné SeedPackage stavy. Úplné sestavení a85/85 CTest prošlo za359.55 s, obě aktuální audio sestavení a striktní hashe také;17 originálů je beze změny. Výslovný drawing/input a world adaptér nadále platí. Skutečný arrival/work dispatcher, plný Person/Farm/EXE, obousměrné saves, zbytek Squall a izolovaný uninstaller zůstávají další prací.

[PersonWalk](docs/PERSON_WALK.md) obnovuje běžný pěší přesun k úkolu:1306 stavů /1210 kroků,96 GPU snímků /46 080 000 přesně shodných RGB pixelů v PC53/PC24. Skutečné kliknutí na Field vytvoří PersonTasks úkol a značku; obnovený pohyb spotřebuje cíl, posune vykreslenou postavu a dojde k poli. Porovnané jsou také profilové XPersPos/YPersPos, posunutý Farm.movingPicture, cancel, unordered souřadnice a skutečný SeedPackage status2/0. Úplné sestavení a84/84 CTest prošlo za350.78 s, oba game-audio buildy a striktní source/reference hashe také; všech17 originálů je beze změny. Explicitní drawing/input a world adaptéry nadále platí. Arrival/work dispatcher, vozidlová větev, úplný Person/Farm/EXE, saves, zbývající Squall a izolovaný odinstalátor pokračují.

[PersonVisuals](docs/PERSON_VISUALS.md) obnovuje skutečná appearance/XML data postavy, výběr idle/work/move/hose animací, hotspoty a UI nesených věcí. PC53/PC24:9014 stavů /8928 akcí,10 XML reloadů a40 GPU snímků /19 200 000 shodných RGB pixelů včetně skutečného pole a značek fronty. Pořadí aktualizací, původní x87 přesnost i retained UI/sound reference jsou přímo porovnané. Úplné sestavení a83/83 CTest prošlo za371,54 s, oba game-audio buildy a striktní hashe také; všech17 originálů je beze změny. Drawing/input base a některé world hodnoty mají výslovné adaptéry. Celý Person constructor, pracovní akce a úplný actor lifecycle dále zbývají; běžný přesun nyní popisuje PersonWalk; stejně jako kompletní Farm/EXE a ostatní podmínky celého cíle.

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

ContractStatus propojuje skutečné vlastněné UI, všech 131 definic zakázek, deadline, požadavky a profilové klíče. PC53/PC24: 31 264 stavů, 5 144 přiřazení a 64 GPU snímků / 30 720 000 shodných RGB pixelů; osm skutečných input release otevře dialog, osm blokuje jiný tutorial step. Skutečné GameMenu, warehouse, dialogy, tutoriál, hlavní EXE a saves pokračují. Rozsah v [docs/CONTRACT_STATUS.md](docs/CONTRACT_STATUS.md).

PriceText a TextMoney jsou obnovené a nativně porovnané v obou x87 přesnostech, včetně vlastněné Coin ikony a všech 14 XML definic. Rozsah: [docs/PRICE_TEXT.md](docs/PRICE_TEXT.md). Celá hra zůstává rozpracovaná.

Dialogy zakázek (požadavky, výsledek a potvrzení) jsou propojené s obnoveným ContractStatus a porovnané s originálem včetně input callbacks a UI přechodů. Rozsah a zbývající světové vazby: [docs/CONTRACT_DIALOGS.md](docs/CONTRACT_DIALOGS.md).

Menu pro pauzu propojuje skutečné Options, Awards, YesNoDialog a Loading/MainMenu. PC53/PC24: 28 navigačních akcí a 264 GPU snímků / 126 720 000 shodných pixelů. Uložení a zrušení celého cGame stále vyžaduje dokončení herního světa; rozsah v [docs/GAME_PAUSE_MENU.md](docs/GAME_PAUSE_MENU.md).

ContractProvide propojuje nabidku zakazky s realnym ContractStatus. PC53/PC24: 2 738 stavu, 1 308 startu, 96 GPU snimku / 46 080 000 shodnych RGB pixelu a ctyri input release. Diagnostika vykonava puvodni aktivaci do tri skutecnych controlleru; plny zdrojovy GameMenu a Farm letter actor pokracuji. Rozsah v [docs/CONTRACT_PROVIDE.md](docs/CONTRACT_PROVIDE.md).

StateView nyní obnovuje skutečný sdílený pohyb herních obrazovek: 22 242 stavů, 2508 pohybů a 108 GPU snímků / 51 840 000 shodných RGB pixelů v PC53/PC24. Skutečné XML Farm/Manufacture/WareDetail ověřuje společný základ; odvozené herní controllery, GameMenu a celý svět pokračují. Rozsah a původní adresy v [STATE_VIEW.md](docs/STATE_VIEW.md).

GameMenu propojuje HUD, tři ContractStatus, skutečný potvrzovací dialog a pauzu: 4 778 stavů, 1 150 akcí a 242 GPU snímků / 116 160 000 shodných RGB pixelů v PC53/PC24. Zahrnuje všech 131 zakázek, obnovu uložených tasků, skutečné přechody StateView, myš a Escape/Enter. Světové závislosti a zbývající práci eviduje [docs/GAME_MENU.md](docs/GAME_MENU.md); celý projekt zůstává rozpracovaný.

Pět typů oken tutoriálu má ověřené skutečné texty, šipky, rušení akcí a OK input: 3 474 stavů a 64 GPU snímků / 30 720 000 shodných pixelů v PC53/PC24. Správce tutoriálu a skutečný svět pokračují; rozsah v [docs/TUTORIAL_TIPS.md](docs/TUTORIAL_TIPS.md).

TutorialManager propojuje skutečný HUD, okna a profil: 2 500 stavů, 14 848 predicate porovnání a 204 GPU snímků / 97 920 000 shodných pixelů. Původní adresy, zvláštnosti a dosud řízené world hranice jsou v [docs/TUTORIAL_MANAGER.md](docs/TUTORIAL_MANAGER.md). Skutečný herní svět a celý průchod tutoriálem pokračují.

MovableUnion propojuje společný základ nákupních panelů s HUD: 11 604 stavů, 1 036 pohybů a 144 GPU snímků / 69 120 000 shodných pixelů v PC53/PC24. BuySeeds/BuyBuilding controllery a skutečný nákup pokračují; rozsah v [docs/MOVABLE_UNION.md](docs/MOVABLE_UNION.md).

BuySeeds propojuje osivo, HUD a původní návody 14/15: 1 696 stavů, 176 předání Farm hranici a 248 GPU snímků / 119 040 000 shodných pixelů. Skutečný seed mover a celý svět pokračují; rozsah a doložená původní undefined-memory chyba jsou v [docs/BUY_SEEDS.md](docs/BUY_SEEDS.md).

ProductionBuilding nyní obsahuje skutečný controller všech jedenácti výrobních budov: čtyři uložené úrovně a nová stavba, zásoby/slider, upgrade refresh, výroba, pause, drag a build/focus callbacky. PC53/PC24: 16 188 controller stavů, 110 objektů, spolu s ExtendedPictureButton základem 176 D24S8 snímků / 84 480 000 shodných pixelů. Celé Win32 sestavení a 63 CTest kontrol prošly, canonical i samostatné game-audio sestavení po 378 stavech. Manufacture/warehouse jsou v controller oracle explicitní pozorované hranice; skutečný world a hlavní EXE pokračují. Přesné adresy, opravy pointer vazeb, raw padding a neověřené oblasti jsou v [docs/PRODUCTION_HOUSES.md](docs/PRODUCTION_HOUSES.md).

Navazující skutečný [FieldTip](docs/FIELD_TIP.md) je nyní zapojený do pole a přímo porovnaný v PC53/PC24:4 998 tip stavů a společně4 472 field stavů,140 GPU snímků /67 200 000 shodných pixelů, čtyři XML reloady. Úplná regrese prošla80/80 za721,22 s a oba game-audio buildy; současné source/binary hashe i všech17 originálů jsou ověřené. Farm indication a Person zůstávají výslovné hranice.
